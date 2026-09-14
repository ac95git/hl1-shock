// Cleave, the Melee major: the sweep drawn on a Cleave swing.
//
// Event scripts are never parsed by the engine; the file only has to exist so
// PRECACHE_EVENT can name it.  The behaviour is EV_Cleave in
// cl_dll/ev_hldm.cpp, hooked in cl_dll/hl/hl_events.cpp.
//
// The mod ships its own events/ directory for the same reason it ships
// sprites/ and models/: copy it to topmod/events/ after any change.
