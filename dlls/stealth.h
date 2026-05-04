// stealth.h
#ifndef STEALTH_H
#define STEALTH_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"

bool IsBehindEntity(CBaseEntity *pSelf, CBaseEntity *pTarget); // returns true if pTarget is largely behind pSelf
bool IsPlayerStealthed(CBasePlayer *pPlayer);                // simple player stealth check

#endif // STEALTH_H
