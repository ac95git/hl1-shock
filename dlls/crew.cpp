// The Crew -- the friendly humans of the facility (docs/ROADMAP.md, "The Crew
// -- grilled 2026-09-24").  Four kinds, one classname each, on one talk-
// monster base: monster_miner, monster_security, monster_construction,
// monster_technician.
//
// What they do is small on purpose: stand at a post, look at the player,
// answer a use press with one line.  No following, no fighting, no
// wandering; anything more is the mapper's, through scripted_sequence.
// Harm is the scientist's: they take damage and die, they run from what
// frightens them, and a player who hurts one is someone to run from.  The
// three who gate mines1's sequence carry the vanilla TriggerCondition
// "Death" in the map, which is the whole of the game-over hook.
//
// THE LINES live in crew_lines.txt beside records.txt, one text and an
// optional wav per line, in pools per kind.  A use press prints the text
// (and plays the wav when there is one, which moves the jaw on a model that
// has one -- the engine does that from CHAN_VOICE on its own).  Unprompted
// speech -- hello, idle chatter, "stop staring" -- is audio only and never
// prints: until a pool has recordings in it, that kind is silent unprompted.
//
// Bodies, for now (docs/ART_DEBT.md, "The Crew"): the miner is the maddened's
// model as it stands, in the five clean hair colours; construction wears the
// miner's until its own exists; security stands in as Barney until the
// Barney-rig crew are built; the technician is scientist_suit, the
// scientist's rig in Ivan's suit (2026-09-25).  A mapper can
// set `model` on any of them to try a new body before the default moves.
//
// Server only.

#include <string>
#include <vector>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "talkmonster.h"
#include "schedule.h"
#include "defaultai.h"
#include "animation.h"
#include "soundent.h"
#include "skill.h"

//=========================================================
// The lines file
//=========================================================

// Beside records.txt in the mod directory; copied there by hand like it.
static const char* const k_CrewLinesFile = "crew_lines.txt";

struct CrewLine
{
	std::string id;
	std::string kind; // miner, security, construction, technician
	std::string pool; // use (the default), hello, idle, stare
	std::string text; // lines joined with '\n'; printed on use only
	std::string wav;  // under sound/, empty when there is no recording yet
	float wavSeconds = 0.0f;
};

static std::vector<CrewLine> s_CrewLines;
static char s_szCrewLinesMap[64] = "";
static float s_flCrewLinesLoadedAt = -1.0f;

// How long a wav plays, from its header: the data chunk over the byte rate.
// 0 when the file is not a wav this can read, which only costs the subtitle
// its floor.
static float WavSeconds(const byte* pData, int length)
{
	if (length < 12 || memcmp(pData, "RIFF", 4) != 0 || memcmp(pData + 8, "WAVE", 4) != 0)
		return 0.0f;

	unsigned int byteRate = 0;
	unsigned int dataSize = 0;

	int pos = 12;
	while (pos + 8 <= length)
	{
		const byte* chunk = pData + pos;
		const unsigned int size = chunk[4] | (chunk[5] << 8) | (chunk[6] << 16) | (chunk[7] << 24);

		if (memcmp(chunk, "fmt ", 4) == 0 && pos + 8 + 12 <= length)
			byteRate = chunk[16] | (chunk[17] << 8) | (chunk[18] << 16) | (chunk[19] << 24);
		else if (memcmp(chunk, "data", 4) == 0)
			dataSize = size;

		pos += 8 + size + (size & 1);
	}

	return byteRate > 0 ? static_cast<float>(dataSize) / byteRate : 0.0f;
}

static std::string Trim(const std::string& s)
{
	const size_t first = s.find_first_not_of(" \t\r");
	if (first == std::string::npos)
		return {};
	const size_t last = s.find_last_not_of(" \t\r");
	return s.substr(first, last - first + 1);
}

// A named wav that is not on disk is dropped with one line in the console,
// so a line written before its recording is simply a text line -- which is
// what "optional" means.
static void CheckWav(CrewLine& line)
{
	if (line.wav.empty())
		return;

	const std::string path = "sound/" + line.wav;
	int length = 0;
	byte* pData = LOAD_FILE_FOR_ME(path.c_str(), &length);
	if (!pData)
	{
		ALERT(at_console, "%s: %s names %s, which is not there yet; text only\n",
			k_CrewLinesFile, line.id.c_str(), path.c_str());
		line.wav.clear();
		return;
	}

	line.wavSeconds = WavSeconds(pData, length);
	FREE_FILE(pData);
}

// Keyed blocks, line-based, as records.txt:
//
//   <string id>
//   {
//       kind  <miner | security | construction | technician>
//       pool  <use | hello | idle | stare>     (use when absent)
//       text  <one line of the printed text; repeat for more lines>
//       wav   <path under sound/>              (optional)
//   }
static void LoadCrewLines()
{
	s_CrewLines.clear();

	int length = 0;
	byte* pFile = LOAD_FILE_FOR_ME(k_CrewLinesFile, &length);
	if (!pFile)
	{
		ALERT(at_error, "%s is missing from the mod directory; the Crew have nothing to say\n", k_CrewLinesFile);
		return;
	}

	const std::string file(reinterpret_cast<const char*>(pFile), length);
	FREE_FILE(pFile);

	CrewLine current;
	bool inBlock = false;
	std::string pendingId;

	size_t start = 0;
	while (start <= file.size())
	{
		size_t end = file.find('\n', start);
		if (end == std::string::npos)
			end = file.size();

		std::string ln = Trim(file.substr(start, end - start));
		start = end + 1;

		if (ln.empty() || ln.compare(0, 2, "//") == 0)
			continue;

		if (!inBlock)
		{
			if (ln == "{")
			{
				inBlock = true;
				current = CrewLine();
				current.id = pendingId;
				current.pool = "use";
			}
			else
			{
				pendingId = ln;
			}
			continue;
		}

		if (ln == "}")
		{
			inBlock = false;
			if (current.id.empty() || current.kind.empty())
			{
				ALERT(at_error, "%s: a block without an id or a kind, skipped\n", k_CrewLinesFile);
				continue;
			}
			CheckWav(current);
			s_CrewLines.push_back(current);
			continue;
		}

		const size_t split = ln.find_first_of(" \t");
		const std::string key = ln.substr(0, split);
		const std::string value = split == std::string::npos ? std::string() : Trim(ln.substr(split));

		if (key == "kind")
			current.kind = value;
		else if (key == "pool")
			current.pool = value;
		else if (key == "wav")
			current.wav = value;
		else if (key == "text")
			current.text += (current.text.empty() ? "" : "\n") + value;
		else
			ALERT(at_error, "%s: %s has an unknown key '%s'\n", k_CrewLinesFile, current.id.c_str(), key.c_str());
	}
}

// Read once per map, and again when an earlier save of the same map is
// loaded: a changed file is picked up by any map load.
static void EnsureCrewLines()
{
	const char* map = STRING(gpGlobals->mapname);
	if (FStrEq(map, s_szCrewLinesMap) && gpGlobals->time >= s_flCrewLinesLoadedAt)
		return;

	LoadCrewLines();
	strncpy(s_szCrewLinesMap, map, sizeof(s_szCrewLinesMap) - 1);
	s_szCrewLinesMap[sizeof(s_szCrewLinesMap) - 1] = '\0';
	s_flCrewLinesLoadedAt = gpGlobals->time;
}

static int FindCrewLine(const char* id)
{
	for (size_t i = 0; i < s_CrewLines.size(); ++i)
	{
		if (s_CrewLines[i].id == id)
			return static_cast<int>(i);
	}
	return -1;
}

// A random line from a kind's pool, not the one just said when there is a
// choice.  Unprompted pools want a wav, because unprompted speech never
// prints.  -1 when the pool is empty.
static int PickCrewLine(const char* kind, const char* pool, bool needWav, int iAvoid)
{
	int candidates[64];
	int count = 0;

	for (size_t i = 0; i < s_CrewLines.size() && count < ARRAYSIZE(candidates); ++i)
	{
		const CrewLine& line = s_CrewLines[i];
		if (line.kind != kind || line.pool != pool || (needWav && line.wav.empty()))
			continue;
		if (!needWav && line.text.empty())
			continue;
		candidates[count++] = static_cast<int>(i);
	}

	if (count == 0)
		return -1;

	if (count > 1)
	{
		int pick;
		do
			pick = candidates[RANDOM_LONG(0, count - 1)];
		while (pick == iAvoid);
		return pick;
	}

	return candidates[0];
}

// The HUD's text draws a line as long as it is given, so the wrap is here.
static std::string WrapCrewText(const std::string& text)
{
	constexpr size_t k_Width = 60;

	std::string out;
	size_t lineStart = 0;
	while (lineStart <= text.size())
	{
		size_t lineEnd = text.find('\n', lineStart);
		if (lineEnd == std::string::npos)
			lineEnd = text.size();

		std::string rest = text.substr(lineStart, lineEnd - lineStart);
		while (rest.size() > k_Width)
		{
			size_t cut = rest.rfind(' ', k_Width);
			if (cut == std::string::npos || cut == 0)
				cut = k_Width;
			out += rest.substr(0, cut) + "\n";
			rest = Trim(rest.substr(cut));
		}
		out += rest;

		lineStart = lineEnd + 1;
		if (lineStart <= text.size())
			out += "\n";
	}
	return out;
}

//=========================================================
// The base
//=========================================================

// Its own HUD text channel, so a mapper's game_text on 1 is not wiped by a
// use press.
#define CREW_TEXT_CHANNEL 4

// The tokens m_szGrp holds for the unprompted pools.  PlaySentenceCore
// catches them; a real sentence name never starts with '@'.
static const char* const k_CrewHello = "@hello";
static const char* const k_CrewIdle = "@idle";
static const char* const k_CrewStare = "@stare";

enum
{
	TASK_CREW_ANSWER = LAST_TALKMONSTER_TASK + 1,
};

// Answering a use press: stop, turn to the player, hold the eye contact
// while the line lasts.  The body turns, not only the head, because the
// miner's rig has no head controller.
Task_t tlCrewAnswer[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_SET_ACTIVITY, (float)ACT_IDLE},
		{TASK_FACE_PLAYER, (float)0.5},
		{TASK_TLK_EYECONTACT, (float)0},
};

Schedule_t slCrewAnswer[] =
	{
		{tlCrewAnswer,
			ARRAYSIZE(tlCrewAnswer),
			bits_COND_NEW_ENEMY |
				bits_COND_LIGHT_DAMAGE |
				bits_COND_HEAVY_DAMAGE |
				bits_COND_HEAR_SOUND,
			bits_SOUND_DANGER,
			"CrewAnswer"},
};

class CCrew : public CTalkMonster
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	int Classify() override { return CLASS_PLAYER_ALLY; }
	void SetYawSpeed() override;
	int ObjectCaps() override { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	int ISoundMask() override { return bits_SOUND_WORLD | bits_SOUND_COMBAT | bits_SOUND_DANGER | bits_SOUND_PLAYER; }

	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void SetActivity(Activity newActivity) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	MONSTERSTATE GetIdealState() override;

	// They never fight.  The models' own attack sequences would otherwise
	// hand them the capability in MonsterInit.
	bool CheckMeleeAttack1(float flDot, float flDist) override { return false; }
	bool CheckMeleeAttack2(float flDot, float flDist) override { return false; }
	bool CheckRangeAttack1(float flDot, float flDist) override { return false; }
	bool CheckRangeAttack2(float flDot, float flDist) override { return false; }

	void EXPORT CrewUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	CUSTOM_SCHEDULES;

protected:
	void PlaySentenceCore(const char* pszSentence, float duration, float volume, float attenuation) override;

	// What each kind supplies.
	virtual const char* Kind() const = 0;			// its pools in crew_lines.txt
	virtual const char* DefaultModel() const = 0;	// until the mapper sets `model`
	virtual bool HasHeadController() const = 0;		// whether bone controller 0 turns the head
	virtual void Dress() {}							// body and skin, once the model is set

	bool UsesModel(const char* model) const { return FStrEq(STRING(pev->model), model); }

	// The mapper's `hair` on the kinds that wear the maddened's body: a skin
	// family, or -1 for one at random.  Only read at Spawn, and pev->skin is
	// saved, so nothing here needs to be.
	int m_iHair = -1;

private:
	void TalkInit();
	void IdleChatter();
	float SayLine(int iLine, bool bPrint, CBaseEntity* pListener);
	bool DisregardEnemy(CBaseEntity* pEnemy) const { return !pEnemy->IsAlive() || (gpGlobals->time - m_fearTime) > 15; }

	string_t m_iszLine = iStringNull; // the mapper's `line`: this one says that and nothing else
	float m_fearTime = 0.0f;
	int m_iLastLine = -1; // not saved: at worst a line repeats once after a load
};

TYPEDESCRIPTION CCrew::m_SaveData[] =
	{
		DEFINE_FIELD(CCrew, m_iszLine, FIELD_STRING),
		DEFINE_FIELD(CCrew, m_fearTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CCrew, CTalkMonster);

DEFINE_CUSTOM_SCHEDULES(CCrew){
	slCrewAnswer,
};

IMPLEMENT_CUSTOM_SCHEDULES(CCrew, CTalkMonster);

bool CCrew::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "line"))
	{
		m_iszLine = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	if (FStrEq(pkvd->szKeyName, "hair"))
	{
		m_iHair = atoi(pkvd->szValue);
		return true;
	}

	return CTalkMonster::KeyValue(pkvd);
}

void CCrew::Spawn()
{
	if (FStringNull(pev->model))
		pev->model = MAKE_STRING(DefaultModel());

	Precache();

	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = gSkillData.crewHealth;
	pev->view_ofs = Vector(0, 0, 50); // the eyes, over an origin at the feet: Barney's and the scientist's
	m_flFieldOfView = VIEW_FIELD_WIDE; // the scientist's: wide enough to notice the player and greet him
	m_MonsterState = MONSTERSTATE_NONE;

	m_afCapability = bits_CAP_HEAR | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;
	if (HasHeadController())
		m_afCapability |= bits_CAP_TURN_HEAD;

	pev->body = 0;
	pev->skin = 0;
	Dress();

	if (!FStringNull(m_iszLine) && FindCrewLine(STRING(m_iszLine)) < 0)
	{
		ALERT(at_error, "%s at (%.0f %.0f %.0f): line '%s' is not in %s; using the %s pool\n",
			STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z,
			STRING(m_iszLine), k_CrewLinesFile, Kind());
	}

	MonsterInit();

	// MonsterInit reads attack capabilities off the model's sequences; the
	// miner's model swings a pick and Barney's shoots.
	m_afCapability &= ~(bits_CAP_MELEE_ATTACK1 | bits_CAP_MELEE_ATTACK2 | bits_CAP_RANGE_ATTACK1 | bits_CAP_RANGE_ATTACK2);

	SetUse(&CCrew::CrewUse);
}

void CCrew::Precache()
{
	if (FStringNull(pev->model))
		pev->model = MAKE_STRING(DefaultModel());
	PRECACHE_MODEL(STRING(pev->model));

	EnsureCrewLines();

	// Everything this one could say.  ALLOC_STRING because the engine keeps
	// the precache name's pointer, and the table's strings are rebuilt on a
	// reload.
	for (const CrewLine& line : s_CrewLines)
	{
		if (line.wav.empty())
			continue;
		if (line.kind == Kind() || (!FStringNull(m_iszLine) && line.id == STRING(m_iszLine)))
			PRECACHE_SOUND(STRING(ALLOC_STRING(line.wav.c_str())));
	}

	// Every talk monster must, or nobody talks after a level load.
	TalkInit();

	CTalkMonster::Precache();
}

void CCrew::TalkInit()
{
	CTalkMonster::TalkInit();

	// No sentence groups: the Crew speak only from their lines file.  A
	// scripted_sentence still plays whatever the mapper names.
	for (auto& grp : m_szGrp)
		grp = nullptr;

	// An unprompted pool switches on when it has a recording in it.
	const char* kind = Kind();
	if (PickCrewLine(kind, "hello", true, -1) >= 0)
		m_szGrp[TLK_HELLO] = m_szGrp[TLK_PHELLO] = k_CrewHello;
	if (PickCrewLine(kind, "idle", true, -1) >= 0)
		m_szGrp[TLK_IDLE] = m_szGrp[TLK_PIDLE] = k_CrewIdle;
	if (PickCrewLine(kind, "stare", true, -1) >= 0)
		m_szGrp[TLK_STARE] = k_CrewStare;

	// His recordings are his pitch.
	m_voicePitch = PITCH_NORM;
}

void CCrew::SetYawSpeed()
{
	switch (m_Activity)
	{
	case ACT_WALK:
		pev->yaw_speed = 180;
		break;
	case ACT_RUN:
		pev->yaw_speed = 150;
		break;
	default:
		pev->yaw_speed = 120;
		break;
	}
}

// Three bodies with three different sequence lists; whatever one lacks
// (the miner's rig has no flinch, no signal, no cower) is stood through.
void CCrew::SetActivity(Activity newActivity)
{
	if (LookupActivity(newActivity) == ACTIVITY_NOT_AVAILABLE)
		newActivity = ACT_IDLE;
	CTalkMonster::SetActivity(newActivity);
}

// The scientist's rule: a player who hurts one is remembered, and IRelationship
// turns that into someone to run from.
bool CCrew::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (pevInflictor && (pevInflictor->flags & FL_CLIENT) != 0)
		Remember(bits_MEMORY_PROVOKED);

	return CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// Speaking
//=========================================================

// Says a line: the wav when there is one, the text when bPrint (a use press
// only).  Returns how long it holds the floor.
float CCrew::SayLine(int iLine, bool bPrint, CBaseEntity* pListener)
{
	if (iLine < 0 || iLine >= static_cast<int>(s_CrewLines.size()))
		return 0.0f;

	const CrewLine& line = s_CrewLines[iLine];
	m_iLastLine = iLine;

	float duration = line.wavSeconds;

	if (bPrint && !line.text.empty() && pListener)
	{
		const std::string text = WrapCrewText(line.text);

		// Long enough to read, and never shorter than the recording under it.
		const float hold = V_min(V_max(2.5f + 0.05f * text.size(), line.wavSeconds + 0.5f), 10.0f);

		hudtextparms_t params = {};
		params.x = -1;
		params.y = 0.75f;
		params.r1 = params.g1 = params.b1 = 230;
		params.a1 = 255;
		params.fadeinTime = 0.1f;
		params.fadeoutTime = 0.5f;
		params.holdTime = hold;
		params.channel = CREW_TEXT_CHANNEL;
		UTIL_HudMessage(pListener, params, text.c_str());

		duration = V_max(duration, hold);
	}

	if (!line.wav.empty())
	{
		EMIT_SOUND_DYN(edict(), CHAN_VOICE, line.wav.c_str(), VOL_NORM, ATTN_IDLE, 0, PITCH_NORM);
		CTalkMonster::g_talkWaitTime = gpGlobals->time + line.wavSeconds + 2.0f;
	}

	m_hTalkTarget = pListener;
	Talk(duration);
	SetBits(m_bitsSaid, bit_saidHelloPlayer);
	return duration;
}

// The base's unprompted speech (hello, stare, idle) arrives here as one of
// the '@' tokens TalkInit set, and only when that pool has a recording.
void CCrew::PlaySentenceCore(const char* pszSentence, float duration, float volume, float attenuation)
{
	if (pszSentence[0] != '@')
	{
		CTalkMonster::PlaySentenceCore(pszSentence, duration, volume, attenuation);
		return;
	}

	const int iLine = PickCrewLine(Kind(), pszSentence + 1, true, m_iLastLine);
	SayLine(iLine, false, m_hTalkTarget);
}

// Idle chatter in the Crew's own terms.  The base's FIdleSpeak starts
// conversations with the nearest friend in sentence groups the Crew do not
// have; this only ever speaks to the player, from the idle pool.
void CCrew::IdleChatter()
{
	if (!m_szGrp[TLK_IDLE] || !FOkToSpeak())
		return;

	CBaseEntity* pPlayer = FindNearestFriend(true);
	if (!pPlayer)
		return;

	m_hTalkTarget = pPlayer;
	PlaySentence(m_szGrp[TLK_IDLE], 3.0f, VOL_NORM, ATTN_IDLE);
	m_nSpeak++;
}

void CCrew::CrewUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// A scripted_sentence has the floor.
	if (m_useTime > gpGlobals->time)
		return;

	if (!pCaller || !pCaller->IsPlayer() || !IsAlive())
		return;

	// Someone who has hurt him gets nothing but his back.
	if ((m_afMemory & bits_MEMORY_PROVOKED) != 0)
		return;

	int iLine = -1;
	if (!FStringNull(m_iszLine))
		iLine = FindCrewLine(STRING(m_iszLine));
	if (iLine < 0)
		iLine = PickCrewLine(Kind(), "use", false, m_iLastLine);

	if (iLine < 0)
	{
		ALERT(at_console, "%s: the %s pool in %s has no use lines\n", STRING(pev->classname), Kind(), k_CrewLinesFile);
		return;
	}

	SayLine(iLine, true, pCaller);

	// Turn to the player, unless a script or a fright has him.
	if (m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT)
		ChangeSchedule(slCrewAnswer);
}

//=========================================================
// AI
//=========================================================

void CCrew::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_TLK_SPEAK:
		IdleChatter();
		TaskComplete();
		break;

	default:
		CTalkMonster::StartTask(pTask);
		break;
	}
}

void CCrew::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	// The talk monster greets and chats on the move; the Crew only move when
	// they are running from something.
	case TASK_WAIT_FOR_MOVEMENT:
		CBaseMonster::RunTask(pTask);
		break;

	default:
		CTalkMonster::RunTask(pTask);
		break;
	}
}

Schedule_t* CCrew::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_IDLE_STAND:
	{
		Schedule_t* psched = CTalkMonster::GetScheduleOfType(Type);

		// The base rolls for idle chatter first, and with no idle recording
		// the roll never stops winning (it only backs off after chatter that
		// happened), so he would never get round to watching the player.
		if (!m_szGrp[TLK_IDLE] && psched && FStrEq(psched->pName, "Idle Speak"))
		{
			if (HasConditions(bits_COND_SEE_CLIENT))
				return ScheduleFromName("TlkIdleWatchClient");
			return slIdleStand;
		}
		return psched;
	}
	}

	return CTalkMonster::GetScheduleOfType(Type);
}

// The scientist's, cut to what a man at a post does: run from danger and
// from what he fears, stand while it passes, and forget it after fifteen
// seconds unseen.
Schedule_t* CCrew::GetSchedule()
{
	CBaseEntity* pEnemy = m_hEnemy;

	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound = PBestSound();
		if (pSound && (pSound->m_iType & bits_SOUND_DANGER) != 0)
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
	}

	switch (m_MonsterState)
	{
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		if (pEnemy)
		{
			if (HasConditions(bits_COND_SEE_ENEMY))
				m_fearTime = gpGlobals->time;
			else if (DisregardEnemy(pEnemy))
			{
				m_hEnemy = nullptr;
				m_fearTime = gpGlobals->time;
			}
		}

		// Bumped: he does not step off his post, he looks at who did it.
		if (HasConditions(bits_COND_CLIENT_PUSH))
		{
			ClearConditions(bits_COND_CLIENT_PUSH);
			return slCrewAnswer;
		}
		break;

	case MONSTERSTATE_COMBAT:
		if (HasConditions(bits_COND_SEE_ENEMY))
		{
			m_fearTime = gpGlobals->time;
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
		}

		if (pEnemy && DisregardEnemy(pEnemy))
		{
			m_hEnemy = nullptr;
			m_fearTime = gpGlobals->time;
		}

		// Never the base's combat schedules, which chase.
		return GetScheduleOfType(SCHED_ALERT_STAND);

	default:
		break;
	}

	return CTalkMonster::GetSchedule();
}

MONSTERSTATE CCrew::GetIdealState()
{
	if (m_MonsterState == MONSTERSTATE_COMBAT)
	{
		CBaseEntity* pEnemy = m_hEnemy;
		if (pEnemy && DisregardEnemy(pEnemy))
		{
			m_hEnemy = nullptr;
			m_fearTime = gpGlobals->time;
			m_IdealMonsterState = MONSTERSTATE_ALERT;
			return m_IdealMonsterState;
		}
	}

	return CTalkMonster::GetIdealState();
}

//=========================================================
// The four kinds
//=========================================================

// The miner: the maddened's body as it stands (docs/MADDENED.md), no mouth,
// the five clean hair colours, at random unless `hair` picks one.  Keyvalue
// `pick 1` puts the pick in his hand for a scene at a face.
#define CREW_MINER_MODEL "models/maddened.mdl"
#define CREW_MINER_HAIR_COLOURS 5 // MADDENED_HAIR_COLOURS, dlls/maddened.cpp
#define CREW_MINER_BODY_PICK 1	  // MADDENED_BODY_PICK

// The skin family for a Crew member in the maddened's body: brown, black,
// grey, ginger, blond, in the QC's order.  Out of range means random.
static int CrewMinerSkin(int iHair)
{
	if (iHair >= 0 && iHair < CREW_MINER_HAIR_COLOURS)
		return iHair;
	return RANDOM_LONG(0, CREW_MINER_HAIR_COLOURS - 1);
}

class CCrewMiner : public CCrew
{
public:
	bool KeyValue(KeyValueData* pkvd) override
	{
		if (FStrEq(pkvd->szKeyName, "pick"))
		{
			m_bPick = atoi(pkvd->szValue) != 0;
			return true;
		}
		return CCrew::KeyValue(pkvd);
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

protected:
	const char* Kind() const override { return "miner"; }
	const char* DefaultModel() const override { return CREW_MINER_MODEL; }
	bool HasHeadController() const override { return false; } // his controller 0 is the spine
	void Dress() override
	{
		if (!UsesModel(CREW_MINER_MODEL))
			return;
		pev->skin = CrewMinerSkin(m_iHair);
		pev->body = m_bPick ? CREW_MINER_BODY_PICK : 0;
	}

private:
	bool m_bPick = false;
};

LINK_ENTITY_TO_CLASS(monster_miner, CCrewMiner);

TYPEDESCRIPTION CCrewMiner::m_SaveData[] =
	{
		DEFINE_FIELD(CCrewMiner, m_bPick, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CCrewMiner, CCrew);

// Construction: the miner's body until its own model exists (ROADMAP, the
// cast table), and the same clean colours.
class CCrewConstruction : public CCrew
{
protected:
	const char* Kind() const override { return "construction"; }
	const char* DefaultModel() const override { return CREW_MINER_MODEL; }
	bool HasHeadController() const override { return !UsesModel(CREW_MINER_MODEL); }
	void Dress() override
	{
		if (UsesModel(CREW_MINER_MODEL))
			pev->skin = CrewMinerSkin(m_iHair);
	}
};

LINK_ENTITY_TO_CLASS(monster_construction, CCrewConstruction);

// Security: Barney as he ships until the Barney-rig security model exists.
// Body 0 is his pistol holstered, which is how security stays.
class CCrewSecurity : public CCrew
{
protected:
	const char* Kind() const override { return "security"; }
	const char* DefaultModel() const override { return "models/barney.mdl"; }
	bool HasHeadController() const override { return true; }
};

LINK_ENTITY_TO_CLASS(monster_security, CCrewSecurity);

// The technician: the scientist's rig, heads and sequences in Ivan's suit,
// the protective suit worn to handle the crystals (scientist_suit, 2026-09-25).
// First head.  mines1's lab scene is written against the scientist's
// sequences, which the model keeps.
class CCrewTechnician : public CCrew
{
protected:
	const char* Kind() const override { return "technician"; }
	const char* DefaultModel() const override { return "models/scientist_suit.mdl"; }
	bool HasHeadController() const override { return true; }
};

LINK_ENTITY_TO_CLASS(monster_technician, CCrewTechnician);
