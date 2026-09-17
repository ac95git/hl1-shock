#pragma once

// ---------------------------------------------------------
// Records: the documents the player reads in the world and the suit
// keeps in its memory (docs/ROADMAP.md, "Pillar 1: Records").
//
// Nothing about a Record's TEXT lives here.  That is records.txt in the
// mod directory, read by the client alone -- prose gets rewritten
// constantly and must not cost a rebuild of two DLLs, and a 192-byte
// user message could not carry a body anyway.  What the two DLLs have to
// agree on is how many Records there can be and how the found-set is
// packed, so that is all this header carries.
//
// A Record's numeric id IS its bit.  Ids are saved, so they are stable
// once added and never reused -- the Skill rule (skill_defs.h).
//
// Compiled into both DLLs, so the two sides cannot disagree.
// ---------------------------------------------------------

// Sized ONCE, deliberately generously: the Skill ceiling moved twice and
// reset every save each time, which is the mistake this number exists not
// to repeat.  512 bits is 64 bytes -- a third of a user message -- so
// there was no reason to be mean with it.
inline constexpr int k_MaxRecords = 512;

// Bytes needed to carry one bit per id.  Both DLLs derive the sync
// message's length from this, so they cannot disagree about it.
inline constexpr int k_RecordMaskBytes = (k_MaxRecords + 7) / 8;

// Id 0 is "no Record", the way EPromptClass::None is "nothing usable".
inline constexpr int k_RecordIdNone = 0;

inline bool RecordIdValid(int id)
{
	return id > k_RecordIdNone && id < k_MaxRecords;
}

// ---------------------------------------------------------
// The found-set packing.  Defined once here so the server's saved array,
// the sync message and the client's mirror can only ever agree.
// ---------------------------------------------------------
inline bool RecordMaskGet(const unsigned char* mask, int id)
{
	if (!mask || !RecordIdValid(id))
		return false;
	return (mask[id >> 3] & (1 << (id & 7))) != 0;
}

inline void RecordMaskSet(unsigned char* mask, int id, bool value)
{
	if (!mask || !RecordIdValid(id))
		return;
	const unsigned char bit = static_cast<unsigned char>(1 << (id & 7));
	if (value)
		mask[id >> 3] |= bit;
	else
		mask[id >> 3] &= static_cast<unsigned char>(~bit);
}

// ---------------------------------------------------------
// The halo a BRUSH Record wears while unread.
//
// A loose document glows with a kRenderFxGlowShell on the model itself,
// which needs nothing here.  Brushwork never reaches the studio renderer,
// so a fixed source -- a terminal, a notice, a roster -- gets this sprite
// at its centre instead: kRenderGlow, which occludes behind geometry and
// puts no light into the room.  See dlls/record.cpp.
// ---------------------------------------------------------
#define RECORD_GLOW_SPRITE "sprites/glow01.spr"

// ---------------------------------------------------------
// The stand-in model for a loose document, until the real one exists
// (docs/ART_DEBT.md).  A security card is the flattest thing vanilla
// ships; it is wrong and is meant to look wrong.  A mapper overrides it
// with the ordinary "model" keyvalue, and the fixed form -- a terminal, a
// wall notice, a roster -- is brushwork and uses no model at all.
// ---------------------------------------------------------
#define RECORD_PLACEHOLDER_MODEL "models/w_security.mdl"
