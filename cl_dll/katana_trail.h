// The Gauss Katana's swing trail: the ribbon the blade sweeps, drawn in first
// person.  See katana_trail.cpp.
#pragma once

#include "vector.h"

struct cl_entity_s;

// Called by the studio renderer once per frame, from inside the viewmodel's
// own draw, right after the model itself.  That is the only place with this
// frame's attachment positions: the engine draws beams and the transparent-
// triangle hook before the viewmodel with last frame's, which is why the
// beam glow was rejected.  `time` is the renderer's clock; the basis is the
// camera's, in which the samples are kept.
void KatanaTrail_ViewModelDrawn(cl_entity_s* view, const Vector& eye, const Vector& forward, const Vector& right, const Vector& up, float time);
