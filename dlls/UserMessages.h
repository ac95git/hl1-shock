/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

inline int gmsgShake = 0;
inline int gmsgFade = 0;
inline int gmsgFlashlight = 0;
inline int gmsgFlashBattery = 0;
inline int gmsgResetHUD = 0;
inline int gmsgInitHUD = 0;
inline int gmsgShowGameTitle = 0;
inline int gmsgCurWeapon = 0;
inline int gmsgHealth = 0;
inline int gmsgDamage = 0;
inline int gmsgBattery = 0;
inline int gmsgTrain = 0;
inline int gmsgLogo = 0;
inline int gmsgWeaponList = 0;
inline int gmsgAmmoX = 0;
inline int gmsgHudText = 0;
inline int gmsgDeathMsg = 0;
inline int gmsgScoreInfo = 0;
inline int gmsgTeamInfo = 0;
inline int gmsgTeamScore = 0;
inline int gmsgGameMode = 0;
inline int gmsgMOTD = 0;
inline int gmsgServerName = 0;
inline int gmsgAmmoPickup = 0;
inline int gmsgWeapPickup = 0;
inline int gmsgItemPickup = 0;
inline int gmsgHideWeapon = 0;
inline int gmsgSetCurWeap = 0;
inline int gmsgSayText = 0;
inline int gmsgTextMsg = 0;
inline int gmsgSetFOV = 0;
inline int gmsgShowMenu = 0;
inline int gmsgGeigerRange = 0;
inline int gmsgTeamNames = 0;

inline int gmsgStatusText = 0;
inline int gmsgStatusValue = 0;

inline int gmsgWeapons = 0;

// Inventory item count sync (max 12-char engine limit): BYTE itemId, BYTE count
inline int gmsgInventoryItem = 0;

// Skill tree sync: fixed-length state only -- unlocked mask, Skill Points,
// Reset Tokens.  Definitions are shared, not sent (see SendSkillTreeToClient).
inline int gmsgSkillTree = 0;

// The Status page's numbers: fixed length, sent with every skill-tree sync
// (see SendSkillStatsToClient).
inline int gmsgSkillStats = 0;

// Inventory sync: variable-length, sent in chunks (see SendInventoryToClient)
inline int gmsgInventory = 0;

// Records found-set: one bit per Record id, fixed at k_RecordMaskBytes.  A
// Record's text never crosses the wire -- the client reads records.txt --
// so this mask is the whole of what the server tells it (SyncPlayerRecords).
inline int gmsgRecords = 0;

// The reader: SHORT, the Record it is showing, or 0 to shut it.  Sent on the
// use press that opens it and on the damage that closes it, never per frame.
inline int gmsgRecordRead = 0;

// The Prompt: 3 bytes (entry kind, id, EPromptClass), then the mapper's
// prompt_title and prompt_action, each empty unless overridden. Sent only
// when what the player is looking at changes; all zero clears it. The client
// resolves everything else from the shared tables.
inline int gmsgPickupPrompt = 0;

// Pulse state: 2 bytes (state, duration in tenths of a second).
// Sent only on a state change -- the client runs the bar off its own clock
// from the duration, so a Recharge costs three messages rather than one a frame.
inline int gmsgPulse = 0;

// Defense Matrix state: 2 bytes (up, duration in tenths of a second).  Sent
// only on a change, like the Pulse's; the armour readout tints while it is up.
inline int gmsgMatrix = 0;

// Concealment state: 1 byte (EConcealState).  Sent only on a threshold
// crossing -- the whole point of quantising a continuous meter to three states
// is that the wire sees three events rather than a value every frame.
// See dlls/perception.cpp and cl_dll/hud_conceal.cpp.
inline int gmsgConceal = 0;

// Status icon: BYTE enable, STRING sprite name, then BYTE r/g/b when enabling.
// CHudStatusIcons shipped with the SDK complete but with no sender anywhere in
// dlls/; the Infusion is the first thing to use it. Variable length because it
// carries a string.
inline int gmsgStatusIcon = 0;

void LinkUserMessages();
