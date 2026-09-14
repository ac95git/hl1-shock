// The Gauss Katana's swing trail.
//
// The ribbon the blade sweeps during a swing, blade only: a quad strip
// between successive blade lines, each line the segment from where the
// blade leaves the guard (attachment 2) to its point (attachment 1).  Gauss
// orange, additive, textured with the crescent's own beam sprite so the trail
// and the wave it throws read as one substance, fading and tapering from the
// blade back to nothing over katana_trail_life.
//
// Samples are taken only while an attack sequence plays, and kept in VIEW
// space -- forward, right, up from the eye -- so only the swing's own motion
// leaves a trail.  In world space a mouse turn during the swing would smear
// the blade across the screen, which reads as a rendering fault.
//
// Drawn from inside the viewmodel's studio draw (StudioDrawModel), after the
// model.  Two things follow from that.  The leading edge sits on the blade
// this frame, where a beam between the same attachments trailed by one.
// And the triangles are rasterised while the engine's narrowed viewmodel
// depth range is in force -- IF the triangle API leaves it alone, which the
// SDK does not say.  If it does, the trail behaves as the blade does against
// walls; if not, a ribbon a blade's length out is cut by a wall the blade is
// drawn over.  Decided by the wall test in game, not here.
//
// The quads go through OpenGL directly, not the triangle API.  Inside the
// studio pass the API's colour and render-mode calls did not take: the
// ribbon came out white and unblended, through both the float and the byte
// colour calls, with the recipe the particle manager uses successfully from
// the transparent hook.  The state the model renderer leaves is the
// difference, and the API gives no way to set the two states that matter
// -- texture modulation and the blend -- so they are set here by hand, with
// the sprite still bound through the API.  Hardware renderer only; the
// software one has no GL context and draws no trail.
//
// No server part.  The swing is known from the viewmodel's sequence, which
// the engine already has; nothing is sent.

#include "PlatformHeaders.h"
#include <GL/gl.h>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "r_studioint.h"

#include "katana_trail.h"

#include <cstring>

extern engine_studio_api_t IEngineStudio;

namespace
{
constexpr int kMaxSamples = 64;
constexpr int kAttachPoint = 1; // $attachment 1: the blade's point
constexpr int kAttachGuard = 2; // $attachment 2: where the blade leaves the guard
// The crowbar's attack sequences, which the katana keeps in order
// (dlls/weapons.h: CROWBAR_ATTACK1HIT .. CROWBAR_ATTACK3HIT).
constexpr int kFirstAttack = 3;
constexpr int kLastAttack = 8;
const char* const kSprite = "sprites/laserbeam.spr"; // the crescent's (EV_KatanaArcThink)

struct Sample
{
	float time;
	float weight; // how hard the blade was cutting when taken, 0..1
	Vector guard; // view space
	Vector point;
};

Sample g_samples[kMaxSamples];
int g_count = 0; // live samples, oldest first, in g_samples[0..g_count)
float g_lastSampleTime = -1.0f;
// The point last frame, for its speed; kept whether or not a sample was taken.
Vector g_prevPoint;
float g_prevTime = -1.0f;

Vector ToView(const Vector& p, const Vector& eye, const Vector& f, const Vector& r, const Vector& u)
{
	const Vector d = p - eye;
	return Vector(DotProduct(d, f), DotProduct(d, r), DotProduct(d, u));
}

Vector FromView(const Vector& v, const Vector& eye, const Vector& f, const Vector& r, const Vector& u)
{
	return eye + f * v.x + r * v.y + u * v.z;
}

void Forget()
{
	g_count = 0;
	g_lastSampleTime = -1.0f;
	g_prevTime = -1.0f;
}

void Push(const Sample& s)
{
	if (g_count == kMaxSamples)
	{
		std::memmove(&g_samples[0], &g_samples[1], sizeof(Sample) * (kMaxSamples - 1));
		g_count--;
	}
	g_samples[g_count++] = s;
}

// Drop what has faded, from the oldest end.
void Expire(float time, float life)
{
	int dead = 0;
	while (dead < g_count && time - g_samples[dead].time >= life)
		dead++;
	if (dead > 0)
	{
		std::memmove(&g_samples[0], &g_samples[dead], sizeof(Sample) * (g_count - dead));
		g_count -= dead;
	}
}
} // namespace

void KatanaTrail_ViewModelDrawn(cl_entity_s* view, const Vector& eye, const Vector& forward, const Vector& right, const Vector& up, float time)
{
	if (view == nullptr || view->model == nullptr || gEngfuncs.pfnGetCvarFloat("katana_trail") <= 0.0f)
	{
		Forget();
		return;
	}
	if (std::strstr(view->model->name, "v_katana") == nullptr)
	{
		Forget();
		return;
	}
	const float life = gEngfuncs.pfnGetCvarFloat("katana_trail_life");
	if (life <= 0.0f)
	{
		Forget();
		return;
	}
	auto* hdr = static_cast<studiohdr_t*>(IEngineStudio.Mod_Extradata(view->model));
	if (hdr == nullptr || hdr->numattachments <= kAttachGuard)
		return;

	// The blade this frame, in view space.
	const Vector guardNow = ToView(view->attachment[kAttachGuard], eye, forward, right, up);
	const Vector pointNow = ToView(view->attachment[kAttachPoint], eye, forward, right, up);

	// Swinging means the attack animation is still PLAYING, not merely
	// selected: the crowbar leaves its attack sequence on the viewmodel,
	// frozen on the last frame, for ten to fifteen seconds before an idle
	// plays, and sampling through that turned the viewmodel's small lag
	// behind the camera into a smear on every mouse turn.  The engine sets
	// animtime when a sequence starts and holds the last frame after
	// (numframes - 1) / fps seconds, the same estimate the renderer draws by.
	const int seq = view->curstate.sequence;
	bool swinging = seq >= kFirstAttack && seq <= kLastAttack && seq < hdr->numseq;
	if (swinging)
	{
		const auto* seqdesc = reinterpret_cast<const mstudioseqdesc_t*>(reinterpret_cast<const byte*>(hdr) + hdr->seqindex) + seq;
		const float framerate = view->curstate.framerate > 0.0f ? view->curstate.framerate : 1.0f;
		const float elapsed = time - view->curstate.animtime;
		const float frame = elapsed * seqdesc->fps * framerate;
		swinging = elapsed >= 0.0f && frame < (float)(seqdesc->numframes - 1);
	}
	// How hard the blade is cutting: the point's speed through view space,
	// this frame against last.  Every attack is a short burst and a slow
	// recovery (measured on the crowbar's animations: the point does 300 to
	// 700 units a second in the cut and 70 to 220 on the way back, attack2's
	// return excepted), and the ribbon belongs to the burst.  Nothing below
	// half of katana_trail_speed, full at it, a ramp between, so the ribbon
	// thins away as the blade slows rather than stopping at a frame.
	float weight = 0.0f;
	if (g_prevTime >= 0.0f && time > g_prevTime)
	{
		const float full = gEngfuncs.pfnGetCvarFloat("katana_trail_speed");
		const float speed = (pointNow - g_prevPoint).Length() / (time - g_prevTime);
		if (full <= 0.0f)
			weight = 1.0f;
		else
		{
			weight = (speed - 0.5f * full) / (0.5f * full);
			weight = weight < 0.0f ? 0.0f : (weight > 1.0f ? 1.0f : weight);
		}
	}
	if (time != g_prevTime)
	{
		g_prevPoint = pointNow;
		g_prevTime = time;
	}

	if (swinging && weight > 0.0f && time != g_lastSampleTime)
	{
		Push({time, weight, guardNow, pointNow});
		g_lastSampleTime = time;
	}
	Expire(time, life);
	if (g_count == 0)
		return;

	// The ribbon: every live sample, oldest first, then the blade itself as
	// the leading edge, at this frame's weight so the front thins with the
	// blade.  Alpha is weight times age; width falls with age too: the inner
	// edge is pulled toward the point by katana_trail_taper of the blade's
	// length at the tail, so the tail narrows to the point's path.
	int index = 0;
	struct model_s* sprite = gEngfuncs.CL_LoadModel(kSprite, &index);
	if (sprite == nullptr)
		return;
	const float taper = gEngfuncs.pfnGetCvarFloat("katana_trail_taper");
	const int n = g_count + 1;

	auto edge = [&](int i, Vector& guard, Vector& point, float& alpha, float& u) {
		const float age = (i == g_count) ? 0.0f : (time - g_samples[i].time);
		const float k = 1.0f - age / life; // 1 at the blade, 0 at the tail
		const float w = (i == g_count) ? weight : g_samples[i].weight;
		const Vector g = (i == g_count) ? guardNow : g_samples[i].guard;
		const Vector p = (i == g_count) ? pointNow : g_samples[i].point;
		alpha = k * w;
		point = FromView(p, eye, forward, right, up);
		guard = FromView(p + (g - p) * (1.0f - taper * (1.0f - k)), eye, forward, right, up);
		u = (float)i / (float)(n - 1);
	};

	if (IEngineStudio.IsHardware() == 0)
		return;
	if (gEngfuncs.pTriAPI->SpriteTexture(sprite, 0) == 0) // binds the texture
		return;

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive: drawn as light, dark pixels vanish
	glDepthMask(GL_FALSE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);

	glBegin(GL_QUADS);
	for (int i = 0; i + 1 < n; i++)
	{
		Vector g0, p0, g1, p1;
		float a0, a1, u0, u1;
		edge(i, g0, p0, a0, u0);
		edge(i + 1, g1, p1, a1, u1);
		// The beam sprite is soft across its width: run that across the
		// blade (v: guard 0, point 1) so the ribbon's two edges are soft, and
		// its length along the sweep.  Gauss orange, 255/128/0.
		glColor4f(1.0f, 0.5f, 0.0f, a0);
		glTexCoord2f(u0, 0.0f);
		glVertex3fv(g0);
		glTexCoord2f(u0, 1.0f);
		glVertex3fv(p0);
		glColor4f(1.0f, 0.5f, 0.0f, a1);
		glTexCoord2f(u1, 1.0f);
		glVertex3fv(p1);
		glTexCoord2f(u1, 0.0f);
		glVertex3fv(g1);
	}
	glEnd();
	glPopAttrib();
}
