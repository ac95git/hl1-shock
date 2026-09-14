// Gauss Katana slash: the blade heats.
//
// Event scripts are never parsed by the engine; the file only has to exist so
// PRECACHE_EVENT can name it.  The behaviour is EV_KatanaSwing in
// cl_dll/ev_hldm.cpp, hooked in cl_dll/hl/hl_events.cpp.  The right click's
// wave is katana_arc.sc, which heats the blade too.
//
// The mod ships its own events/ directory for the same reason it ships
// sprites/ and models/: copy it to topmod/events/ after any change.
