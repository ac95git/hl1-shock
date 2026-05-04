// stealth.cpp
#include "stealth.h"
#include "game.h"

// Angle threshold: dot < -0.7 means target is behind (~135 degrees or more)
#define BEHIND_DOT_THRESHOLD -0.7f
#define STEALTH_MOVE_SPEED_THRESHOLD 80.0f // below this speed considered "silent"

bool IsBehindEntity(CBaseEntity *pSelf, CBaseEntity *pTarget)
{
    if (!pSelf || !pTarget) return false;

    Vector vecForward;
    UTIL_MakeVectorsPrivate(pSelf->pev->angles, vecForward, NULL, NULL); // same util used by engine
    Vector vecToTarget = pTarget->pev->origin - pSelf->pev->origin;
    vecToTarget = vecToTarget.Normalize();

    float dot = DotProduct(vecForward, vecToTarget);
    return (dot < BEHIND_DOT_THRESHOLD);
}

bool IsPlayerStealthed(CBasePlayer *pPlayer)
{
    if (!pPlayer) return false;

    // m_fStealthActive is a boolean we'll add to CBasePlayer
    // also ensure player is moving slowly (walking/crouched)
    if (pPlayer->m_fStealthActive)
    {
        float speed = pPlayer->pev->velocity.Length();
        if (speed <= STEALTH_MOVE_SPEED_THRESHOLD)
            return true;
    }
    return false;
}
