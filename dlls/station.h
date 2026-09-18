#pragma once

// func_station (dlls/station.cpp), as far as the Prompt needs to know it.

#include "prompt_defs.h"

class CBaseEntity;

// The Prompt class a Station shows -- by its type, and whether it has trades
// left.  EPromptClass::None if pEnt is not a Station.
EPromptClass StationPromptClass(CBaseEntity* pEnt);
