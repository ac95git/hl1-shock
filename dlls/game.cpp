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
#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "client.h"
#include "game.h"
#include "filesystem_utils.h"

void LinkUserMessages();

cvar_t displaysoundlist = {"displaysoundlist", "0"};

// multiplayer server rules
cvar_t fragsleft = {"mp_fragsleft", "0", FCVAR_SERVER | FCVAR_UNLOGGED}; // Don't spam console/log files/users with this changing
cvar_t timeleft = {"mp_timeleft", "0", FCVAR_SERVER | FCVAR_UNLOGGED};	 // "      "

// multiplayer server rules
cvar_t teamplay = {"mp_teamplay", "0", FCVAR_SERVER};
cvar_t fraglimit = {"mp_fraglimit", "0", FCVAR_SERVER};
cvar_t timelimit = {"mp_timelimit", "0", FCVAR_SERVER};
cvar_t friendlyfire = {"mp_friendlyfire", "0", FCVAR_SERVER};
cvar_t falldamage = {"mp_falldamage", "0", FCVAR_SERVER};
cvar_t weaponstay = {"mp_weaponstay", "0", FCVAR_SERVER};
cvar_t forcerespawn = {"mp_forcerespawn", "1", FCVAR_SERVER};
cvar_t flashlight = {"mp_flashlight", "0", FCVAR_SERVER};
cvar_t aimcrosshair = {"mp_autocrosshair", "1", FCVAR_SERVER};
cvar_t decalfrequency = {"decalfrequency", "30", FCVAR_SERVER};
cvar_t teamlist = {"mp_teamlist", "hgrunt;scientist", FCVAR_SERVER};
cvar_t teamoverride = {"mp_teamoverride", "1"};
cvar_t defaultteam = {"mp_defaultteam", "0"};
cvar_t allowmonsters = {"mp_allowmonsters", "0", FCVAR_SERVER};

cvar_t allow_spectators = {"allow_spectators", "0.0", FCVAR_SERVER}; // 0 prevents players from being spectators

cvar_t mp_chattime = {"mp_chattime", "10", FCVAR_SERVER};

cvar_t sv_allowbunnyhopping = {"sv_allowbunnyhopping", "0", FCVAR_SERVER};

//CVARS FOR SKILL LEVEL SETTINGS
// Agrunt
cvar_t sk_agrunt_health1 = {"sk_agrunt_health1", "0"};
cvar_t sk_agrunt_health2 = {"sk_agrunt_health2", "0"};
cvar_t sk_agrunt_health3 = {"sk_agrunt_health3", "0"};

cvar_t sk_agrunt_dmg_punch1 = {"sk_agrunt_dmg_punch1", "0"};
cvar_t sk_agrunt_dmg_punch2 = {"sk_agrunt_dmg_punch2", "0"};
cvar_t sk_agrunt_dmg_punch3 = {"sk_agrunt_dmg_punch3", "0"};

// Apache
cvar_t sk_apache_health1 = {"sk_apache_health1", "0"};
cvar_t sk_apache_health2 = {"sk_apache_health2", "0"};
cvar_t sk_apache_health3 = {"sk_apache_health3", "0"};

// Barney
cvar_t sk_barney_health1 = {"sk_barney_health1", "0"};
cvar_t sk_barney_health2 = {"sk_barney_health2", "0"};
cvar_t sk_barney_health3 = {"sk_barney_health3", "0"};

// Bullsquid
cvar_t sk_bullsquid_health1 = {"sk_bullsquid_health1", "0"};
cvar_t sk_bullsquid_health2 = {"sk_bullsquid_health2", "0"};
cvar_t sk_bullsquid_health3 = {"sk_bullsquid_health3", "0"};

cvar_t sk_bullsquid_dmg_bite1 = {"sk_bullsquid_dmg_bite1", "0"};
cvar_t sk_bullsquid_dmg_bite2 = {"sk_bullsquid_dmg_bite2", "0"};
cvar_t sk_bullsquid_dmg_bite3 = {"sk_bullsquid_dmg_bite3", "0"};

cvar_t sk_bullsquid_dmg_whip1 = {"sk_bullsquid_dmg_whip1", "0"};
cvar_t sk_bullsquid_dmg_whip2 = {"sk_bullsquid_dmg_whip2", "0"};
cvar_t sk_bullsquid_dmg_whip3 = {"sk_bullsquid_dmg_whip3", "0"};

cvar_t sk_bullsquid_dmg_spit1 = {"sk_bullsquid_dmg_spit1", "0"};
cvar_t sk_bullsquid_dmg_spit2 = {"sk_bullsquid_dmg_spit2", "0"};
cvar_t sk_bullsquid_dmg_spit3 = {"sk_bullsquid_dmg_spit3", "0"};


// Big Momma
cvar_t sk_bigmomma_health_factor1 = {"sk_bigmomma_health_factor1", "1.0"};
cvar_t sk_bigmomma_health_factor2 = {"sk_bigmomma_health_factor2", "1.0"};
cvar_t sk_bigmomma_health_factor3 = {"sk_bigmomma_health_factor3", "1.0"};

cvar_t sk_bigmomma_dmg_slash1 = {"sk_bigmomma_dmg_slash1", "50"};
cvar_t sk_bigmomma_dmg_slash2 = {"sk_bigmomma_dmg_slash2", "50"};
cvar_t sk_bigmomma_dmg_slash3 = {"sk_bigmomma_dmg_slash3", "50"};

cvar_t sk_bigmomma_dmg_blast1 = {"sk_bigmomma_dmg_blast1", "100"};
cvar_t sk_bigmomma_dmg_blast2 = {"sk_bigmomma_dmg_blast2", "100"};
cvar_t sk_bigmomma_dmg_blast3 = {"sk_bigmomma_dmg_blast3", "100"};

cvar_t sk_bigmomma_radius_blast1 = {"sk_bigmomma_radius_blast1", "250"};
cvar_t sk_bigmomma_radius_blast2 = {"sk_bigmomma_radius_blast2", "250"};
cvar_t sk_bigmomma_radius_blast3 = {"sk_bigmomma_radius_blast3", "250"};

// Gargantua
cvar_t sk_gargantua_health1 = {"sk_gargantua_health1", "0"};
cvar_t sk_gargantua_health2 = {"sk_gargantua_health2", "0"};
cvar_t sk_gargantua_health3 = {"sk_gargantua_health3", "0"};

cvar_t sk_gargantua_dmg_slash1 = {"sk_gargantua_dmg_slash1", "0"};
cvar_t sk_gargantua_dmg_slash2 = {"sk_gargantua_dmg_slash2", "0"};
cvar_t sk_gargantua_dmg_slash3 = {"sk_gargantua_dmg_slash3", "0"};

cvar_t sk_gargantua_dmg_fire1 = {"sk_gargantua_dmg_fire1", "0"};
cvar_t sk_gargantua_dmg_fire2 = {"sk_gargantua_dmg_fire2", "0"};
cvar_t sk_gargantua_dmg_fire3 = {"sk_gargantua_dmg_fire3", "0"};

cvar_t sk_gargantua_dmg_stomp1 = {"sk_gargantua_dmg_stomp1", "0"};
cvar_t sk_gargantua_dmg_stomp2 = {"sk_gargantua_dmg_stomp2", "0"};
cvar_t sk_gargantua_dmg_stomp3 = {"sk_gargantua_dmg_stomp3", "0"};


// Hassassin
cvar_t sk_hassassin_health1 = {"sk_hassassin_health1", "0"};
cvar_t sk_hassassin_health2 = {"sk_hassassin_health2", "0"};
cvar_t sk_hassassin_health3 = {"sk_hassassin_health3", "0"};


// Headcrab
cvar_t sk_headcrab_health1 = {"sk_headcrab_health1", "0"};
cvar_t sk_headcrab_health2 = {"sk_headcrab_health2", "0"};
cvar_t sk_headcrab_health3 = {"sk_headcrab_health3", "0"};

cvar_t sk_headcrab_dmg_bite1 = {"sk_headcrab_dmg_bite1", "0"};
cvar_t sk_headcrab_dmg_bite2 = {"sk_headcrab_dmg_bite2", "0"};
cvar_t sk_headcrab_dmg_bite3 = {"sk_headcrab_dmg_bite3", "0"};


// Hgrunt
cvar_t sk_hgrunt_health1 = {"sk_hgrunt_health1", "0"};
cvar_t sk_hgrunt_health2 = {"sk_hgrunt_health2", "0"};
cvar_t sk_hgrunt_health3 = {"sk_hgrunt_health3", "0"};

cvar_t sk_hgrunt_kick1 = {"sk_hgrunt_kick1", "0"};
cvar_t sk_hgrunt_kick2 = {"sk_hgrunt_kick2", "0"};
cvar_t sk_hgrunt_kick3 = {"sk_hgrunt_kick3", "0"};

cvar_t sk_hgrunt_pellets1 = {"sk_hgrunt_pellets1", "0"};
cvar_t sk_hgrunt_pellets2 = {"sk_hgrunt_pellets2", "0"};
cvar_t sk_hgrunt_pellets3 = {"sk_hgrunt_pellets3", "0"};

cvar_t sk_hgrunt_gspeed1 = {"sk_hgrunt_gspeed1", "0"};
cvar_t sk_hgrunt_gspeed2 = {"sk_hgrunt_gspeed2", "0"};
cvar_t sk_hgrunt_gspeed3 = {"sk_hgrunt_gspeed3", "0"};

// Houndeye
cvar_t sk_houndeye_health1 = {"sk_houndeye_health1", "0"};
cvar_t sk_houndeye_health2 = {"sk_houndeye_health2", "0"};
cvar_t sk_houndeye_health3 = {"sk_houndeye_health3", "0"};

cvar_t sk_houndeye_dmg_blast1 = {"sk_houndeye_dmg_blast1", "0"};
cvar_t sk_houndeye_dmg_blast2 = {"sk_houndeye_dmg_blast2", "0"};
cvar_t sk_houndeye_dmg_blast3 = {"sk_houndeye_dmg_blast3", "0"};


// ISlave
cvar_t sk_islave_health1 = {"sk_islave_health1", "0"};
cvar_t sk_islave_health2 = {"sk_islave_health2", "0"};
cvar_t sk_islave_health3 = {"sk_islave_health3", "0"};

cvar_t sk_islave_dmg_claw1 = {"sk_islave_dmg_claw1", "0"};
cvar_t sk_islave_dmg_claw2 = {"sk_islave_dmg_claw2", "0"};
cvar_t sk_islave_dmg_claw3 = {"sk_islave_dmg_claw3", "0"};

cvar_t sk_islave_dmg_clawrake1 = {"sk_islave_dmg_clawrake1", "0"};
cvar_t sk_islave_dmg_clawrake2 = {"sk_islave_dmg_clawrake2", "0"};
cvar_t sk_islave_dmg_clawrake3 = {"sk_islave_dmg_clawrake3", "0"};

cvar_t sk_islave_dmg_zap1 = {"sk_islave_dmg_zap1", "0"};
cvar_t sk_islave_dmg_zap2 = {"sk_islave_dmg_zap2", "0"};
cvar_t sk_islave_dmg_zap3 = {"sk_islave_dmg_zap3", "0"};


// Icthyosaur
cvar_t sk_ichthyosaur_health1 = {"sk_ichthyosaur_health1", "0"};
cvar_t sk_ichthyosaur_health2 = {"sk_ichthyosaur_health2", "0"};
cvar_t sk_ichthyosaur_health3 = {"sk_ichthyosaur_health3", "0"};

cvar_t sk_ichthyosaur_shake1 = {"sk_ichthyosaur_shake1", "0"};
cvar_t sk_ichthyosaur_shake2 = {"sk_ichthyosaur_shake2", "0"};
cvar_t sk_ichthyosaur_shake3 = {"sk_ichthyosaur_shake3", "0"};


// Leech
cvar_t sk_leech_health1 = {"sk_leech_health1", "0"};
cvar_t sk_leech_health2 = {"sk_leech_health2", "0"};
cvar_t sk_leech_health3 = {"sk_leech_health3", "0"};

cvar_t sk_leech_dmg_bite1 = {"sk_leech_dmg_bite1", "0"};
cvar_t sk_leech_dmg_bite2 = {"sk_leech_dmg_bite2", "0"};
cvar_t sk_leech_dmg_bite3 = {"sk_leech_dmg_bite3", "0"};

// Controller
cvar_t sk_controller_health1 = {"sk_controller_health1", "0"};
cvar_t sk_controller_health2 = {"sk_controller_health2", "0"};
cvar_t sk_controller_health3 = {"sk_controller_health3", "0"};

cvar_t sk_controller_dmgzap1 = {"sk_controller_dmgzap1", "0"};
cvar_t sk_controller_dmgzap2 = {"sk_controller_dmgzap2", "0"};
cvar_t sk_controller_dmgzap3 = {"sk_controller_dmgzap3", "0"};

cvar_t sk_controller_speedball1 = {"sk_controller_speedball1", "0"};
cvar_t sk_controller_speedball2 = {"sk_controller_speedball2", "0"};
cvar_t sk_controller_speedball3 = {"sk_controller_speedball3", "0"};

cvar_t sk_controller_dmgball1 = {"sk_controller_dmgball1", "0"};
cvar_t sk_controller_dmgball2 = {"sk_controller_dmgball2", "0"};
cvar_t sk_controller_dmgball3 = {"sk_controller_dmgball3", "0"};

// Nihilanth
cvar_t sk_nihilanth_health1 = {"sk_nihilanth_health1", "0"};
cvar_t sk_nihilanth_health2 = {"sk_nihilanth_health2", "0"};
cvar_t sk_nihilanth_health3 = {"sk_nihilanth_health3", "0"};

cvar_t sk_nihilanth_zap1 = {"sk_nihilanth_zap1", "0"};
cvar_t sk_nihilanth_zap2 = {"sk_nihilanth_zap2", "0"};
cvar_t sk_nihilanth_zap3 = {"sk_nihilanth_zap3", "0"};

// Scientist
cvar_t sk_scientist_health1 = {"sk_scientist_health1", "0"};
cvar_t sk_scientist_health2 = {"sk_scientist_health2", "0"};
cvar_t sk_scientist_health3 = {"sk_scientist_health3", "0"};


// Snark
cvar_t sk_snark_health1 = {"sk_snark_health1", "0"};
cvar_t sk_snark_health2 = {"sk_snark_health2", "0"};
cvar_t sk_snark_health3 = {"sk_snark_health3", "0"};

cvar_t sk_snark_dmg_bite1 = {"sk_snark_dmg_bite1", "0"};
cvar_t sk_snark_dmg_bite2 = {"sk_snark_dmg_bite2", "0"};
cvar_t sk_snark_dmg_bite3 = {"sk_snark_dmg_bite3", "0"};

cvar_t sk_snark_dmg_pop1 = {"sk_snark_dmg_pop1", "0"};
cvar_t sk_snark_dmg_pop2 = {"sk_snark_dmg_pop2", "0"};
cvar_t sk_snark_dmg_pop3 = {"sk_snark_dmg_pop3", "0"};



// Zombie
cvar_t sk_zombie_health1 = {"sk_zombie_health1", "0"};
cvar_t sk_zombie_health2 = {"sk_zombie_health2", "0"};
cvar_t sk_zombie_health3 = {"sk_zombie_health3", "0"};

cvar_t sk_zombie_dmg_one_slash1 = {"sk_zombie_dmg_one_slash1", "0"};
cvar_t sk_zombie_dmg_one_slash2 = {"sk_zombie_dmg_one_slash2", "0"};
cvar_t sk_zombie_dmg_one_slash3 = {"sk_zombie_dmg_one_slash3", "0"};

cvar_t sk_zombie_dmg_both_slash1 = {"sk_zombie_dmg_both_slash1", "0"};
cvar_t sk_zombie_dmg_both_slash2 = {"sk_zombie_dmg_both_slash2", "0"};
cvar_t sk_zombie_dmg_both_slash3 = {"sk_zombie_dmg_both_slash3", "0"};


//Turret
cvar_t sk_turret_health1 = {"sk_turret_health1", "0"};
cvar_t sk_turret_health2 = {"sk_turret_health2", "0"};
cvar_t sk_turret_health3 = {"sk_turret_health3", "0"};


// MiniTurret
cvar_t sk_miniturret_health1 = {"sk_miniturret_health1", "0"};
cvar_t sk_miniturret_health2 = {"sk_miniturret_health2", "0"};
cvar_t sk_miniturret_health3 = {"sk_miniturret_health3", "0"};


// Sentry Turret
cvar_t sk_sentry_health1 = {"sk_sentry_health1", "0"};
cvar_t sk_sentry_health2 = {"sk_sentry_health2", "0"};
cvar_t sk_sentry_health3 = {"sk_sentry_health3", "0"};


// PLAYER WEAPONS

// Crowbar whack
cvar_t sk_plr_crowbar1 = {"sk_plr_crowbar1", "0"};
cvar_t sk_plr_crowbar2 = {"sk_plr_crowbar2", "0"};
cvar_t sk_plr_crowbar3 = {"sk_plr_crowbar3", "0"};

// Gauss Katana.  Real defaults rather than "0": the mod ships no skill.cfg of
// its own and falls back to Half-Life's, which has never heard of these, and
// GetSkillCvar treats zero as an error.
cvar_t sk_plr_katana1 = {"sk_plr_katana1", "60"};
cvar_t sk_plr_katana2 = {"sk_plr_katana2", "60"};
cvar_t sk_plr_katana3 = {"sk_plr_katana3", "60"};

// Glock Round
cvar_t sk_plr_9mm_bullet1 = {"sk_plr_9mm_bullet1", "0"};
cvar_t sk_plr_9mm_bullet2 = {"sk_plr_9mm_bullet2", "0"};
cvar_t sk_plr_9mm_bullet3 = {"sk_plr_9mm_bullet3", "0"};

// 357 Round
cvar_t sk_plr_357_bullet1 = {"sk_plr_357_bullet1", "0"};
cvar_t sk_plr_357_bullet2 = {"sk_plr_357_bullet2", "0"};
cvar_t sk_plr_357_bullet3 = {"sk_plr_357_bullet3", "0"};

// MP5 Round
cvar_t sk_plr_9mmAR_bullet1 = {"sk_plr_9mmAR_bullet1", "0"};
cvar_t sk_plr_9mmAR_bullet2 = {"sk_plr_9mmAR_bullet2", "0"};
cvar_t sk_plr_9mmAR_bullet3 = {"sk_plr_9mmAR_bullet3", "0"};


// M203 grenade
cvar_t sk_plr_9mmAR_grenade1 = {"sk_plr_9mmAR_grenade1", "0"};
cvar_t sk_plr_9mmAR_grenade2 = {"sk_plr_9mmAR_grenade2", "0"};
cvar_t sk_plr_9mmAR_grenade3 = {"sk_plr_9mmAR_grenade3", "0"};


// Shotgun buckshot
cvar_t sk_plr_buckshot1 = {"sk_plr_buckshot1", "0"};
cvar_t sk_plr_buckshot2 = {"sk_plr_buckshot2", "0"};
cvar_t sk_plr_buckshot3 = {"sk_plr_buckshot3", "0"};


// Crossbow
cvar_t sk_plr_xbow_bolt_client1 = {"sk_plr_xbow_bolt_client1", "0"};
cvar_t sk_plr_xbow_bolt_client2 = {"sk_plr_xbow_bolt_client2", "0"};
cvar_t sk_plr_xbow_bolt_client3 = {"sk_plr_xbow_bolt_client3", "0"};

cvar_t sk_plr_xbow_bolt_monster1 = {"sk_plr_xbow_bolt_monster1", "0"};
cvar_t sk_plr_xbow_bolt_monster2 = {"sk_plr_xbow_bolt_monster2", "0"};
cvar_t sk_plr_xbow_bolt_monster3 = {"sk_plr_xbow_bolt_monster3", "0"};


// RPG
cvar_t sk_plr_rpg1 = {"sk_plr_rpg1", "0"};
cvar_t sk_plr_rpg2 = {"sk_plr_rpg2", "0"};
cvar_t sk_plr_rpg3 = {"sk_plr_rpg3", "0"};


// Zero Point Generator
cvar_t sk_plr_gauss1 = {"sk_plr_gauss1", "0"};
cvar_t sk_plr_gauss2 = {"sk_plr_gauss2", "0"};
cvar_t sk_plr_gauss3 = {"sk_plr_gauss3", "0"};


// Tau Cannon
cvar_t sk_plr_egon_narrow1 = {"sk_plr_egon_narrow1", "0"};
cvar_t sk_plr_egon_narrow2 = {"sk_plr_egon_narrow2", "0"};
cvar_t sk_plr_egon_narrow3 = {"sk_plr_egon_narrow3", "0"};

cvar_t sk_plr_egon_wide1 = {"sk_plr_egon_wide1", "0"};
cvar_t sk_plr_egon_wide2 = {"sk_plr_egon_wide2", "0"};
cvar_t sk_plr_egon_wide3 = {"sk_plr_egon_wide3", "0"};


// Hand Grendade
cvar_t sk_plr_hand_grenade1 = {"sk_plr_hand_grenade1", "0"};
cvar_t sk_plr_hand_grenade2 = {"sk_plr_hand_grenade2", "0"};
cvar_t sk_plr_hand_grenade3 = {"sk_plr_hand_grenade3", "0"};


// Satchel Charge
cvar_t sk_plr_satchel1 = {"sk_plr_satchel1", "0"};
cvar_t sk_plr_satchel2 = {"sk_plr_satchel2", "0"};
cvar_t sk_plr_satchel3 = {"sk_plr_satchel3", "0"};


// Tripmine
cvar_t sk_plr_tripmine1 = {"sk_plr_tripmine1", "0"};
cvar_t sk_plr_tripmine2 = {"sk_plr_tripmine2", "0"};
cvar_t sk_plr_tripmine3 = {"sk_plr_tripmine3", "0"};

// HORNET
cvar_t sk_plr_hornet_dmg1 = {"sk_plr_hornet_dmg1", "0"};
cvar_t sk_plr_hornet_dmg2 = {"sk_plr_hornet_dmg2", "0"};
cvar_t sk_plr_hornet_dmg3 = {"sk_plr_hornet_dmg3", "0"};


// WORLD WEAPONS
cvar_t sk_12mm_bullet1 = {"sk_12mm_bullet1", "0"};
cvar_t sk_12mm_bullet2 = {"sk_12mm_bullet2", "0"};
cvar_t sk_12mm_bullet3 = {"sk_12mm_bullet3", "0"};

cvar_t sk_9mmAR_bullet1 = {"sk_9mmAR_bullet1", "0"};
cvar_t sk_9mmAR_bullet2 = {"sk_9mmAR_bullet2", "0"};
cvar_t sk_9mmAR_bullet3 = {"sk_9mmAR_bullet3", "0"};

cvar_t sk_9mm_bullet1 = {"sk_9mm_bullet1", "0"};
cvar_t sk_9mm_bullet2 = {"sk_9mm_bullet2", "0"};
cvar_t sk_9mm_bullet3 = {"sk_9mm_bullet3", "0"};


// HORNET
cvar_t sk_hornet_dmg1 = {"sk_hornet_dmg1", "0"};
cvar_t sk_hornet_dmg2 = {"sk_hornet_dmg2", "0"};
cvar_t sk_hornet_dmg3 = {"sk_hornet_dmg3", "0"};

// HEALTH/CHARGE
cvar_t sk_suitcharger1 = {"sk_suitcharger1", "0"};
cvar_t sk_suitcharger2 = {"sk_suitcharger2", "0"};
cvar_t sk_suitcharger3 = {"sk_suitcharger3", "0"};

cvar_t sk_battery1 = {"sk_battery1", "0"};
cvar_t sk_battery2 = {"sk_battery2", "0"};
cvar_t sk_battery3 = {"sk_battery3", "0"};

cvar_t sk_healthcharger1 = {"sk_healthcharger1", "0"};
cvar_t sk_healthcharger2 = {"sk_healthcharger2", "0"};
cvar_t sk_healthcharger3 = {"sk_healthcharger3", "0"};

cvar_t sk_healthkit1 = {"sk_healthkit1", "0"};
cvar_t sk_healthkit2 = {"sk_healthkit2", "0"};
cvar_t sk_healthkit3 = {"sk_healthkit3", "0"};

cvar_t sk_scientist_heal1 = {"sk_scientist_heal1", "0"};
cvar_t sk_scientist_heal2 = {"sk_scientist_heal2", "0"};
cvar_t sk_scientist_heal3 = {"sk_scientist_heal3", "0"};


// monster damage adjusters
cvar_t sk_monster_head1 = {"sk_monster_head1", "2"};
cvar_t sk_monster_head2 = {"sk_monster_head2", "2"};
cvar_t sk_monster_head3 = {"sk_monster_head3", "2"};

cvar_t sk_monster_chest1 = {"sk_monster_chest1", "1"};
cvar_t sk_monster_chest2 = {"sk_monster_chest2", "1"};
cvar_t sk_monster_chest3 = {"sk_monster_chest3", "1"};

cvar_t sk_monster_stomach1 = {"sk_monster_stomach1", "1"};
cvar_t sk_monster_stomach2 = {"sk_monster_stomach2", "1"};
cvar_t sk_monster_stomach3 = {"sk_monster_stomach3", "1"};

cvar_t sk_monster_arm1 = {"sk_monster_arm1", "1"};
cvar_t sk_monster_arm2 = {"sk_monster_arm2", "1"};
cvar_t sk_monster_arm3 = {"sk_monster_arm3", "1"};

cvar_t sk_monster_leg1 = {"sk_monster_leg1", "1"};
cvar_t sk_monster_leg2 = {"sk_monster_leg2", "1"};
cvar_t sk_monster_leg3 = {"sk_monster_leg3", "1"};

// player damage adjusters
cvar_t sk_player_head1 = {"sk_player_head1", "2"};
cvar_t sk_player_head2 = {"sk_player_head2", "2"};
cvar_t sk_player_head3 = {"sk_player_head3", "2"};

cvar_t sk_player_chest1 = {"sk_player_chest1", "1"};
cvar_t sk_player_chest2 = {"sk_player_chest2", "1"};
cvar_t sk_player_chest3 = {"sk_player_chest3", "1"};

cvar_t sk_player_stomach1 = {"sk_player_stomach1", "1"};
cvar_t sk_player_stomach2 = {"sk_player_stomach2", "1"};
cvar_t sk_player_stomach3 = {"sk_player_stomach3", "1"};

cvar_t sk_player_arm1 = {"sk_player_arm1", "1"};
cvar_t sk_player_arm2 = {"sk_player_arm2", "1"};
cvar_t sk_player_arm3 = {"sk_player_arm3", "1"};

cvar_t sk_player_leg1 = {"sk_player_leg1", "1"};
cvar_t sk_player_leg2 = {"sk_player_leg2", "1"};
cvar_t sk_player_leg3 = {"sk_player_leg3", "1"};

// END Cvars for Skill Level settings

cvar_t sv_pushable_fixed_tick_fudge = {"sv_pushable_fixed_tick_fudge", "15"};

cvar_t sv_busters = {"sv_busters", "0", FCVAR_SERVER};

// Inventory Grid sizing.  Tuning knobs only -- the Rows a player actually has
// are saved state, captured at spawn and granted since.  Lowering either of
// these can never shrink a Grid that already has things in it; see
// CPlayerInventory::Rows() and docs/adr/0003-fixed-grid-width-rows-only-growth.md.
cvar_t inv_rows_start = {"inv_rows_start", "5"};
cvar_t inv_rows_max = {"inv_rows_max", "9"};

// item_debug 1 prints what every item touch decided (consumed, taken,
// waiting for a use press, refused) and when a dropped item re-arms.
cvar_t item_debug = {"item_debug", "0"};

// Skill Tree economy.  Both default to zero: every Skill Point and every
// Reset Token is found in the world.  Deliberately uncapped -- the ceiling on
// each is how many pickups a map places, and a cap would let a found pickup
// silently do nothing.  Raise skill_points_start to work on the tree UI
// without hunting for pickups first.  See docs/PILLARS.md pillar 4.
cvar_t skill_points_start = {"skill_points_start", "0"};
cvar_t skill_reset_tokens_start = {"skill_reset_tokens_start", "0"};
// Debugging aid: while set, every Skill in the tree is held, cost and
// prerequisites ignored.  It writes into the same unlocked array a purchase
// does, so it saves, and setting it back to 0 does not take anything away --
// a Reset Token does, or a new game.  See docs/PILLARS.md pillar 4.
cvar_t skill_unlock_all = {"skill_unlock_all", "0"};

// Skill effects.  Every one is a starting guess to be judged in play, and each
// is read server-side where the effect is computed -- never in prediction.
cvar_t skill_health_bonus = {"skill_health_bonus", "25"};
// Multiplies ARMOR_RATIO, the fraction of a blow that gets PAST armor. Lower
// is better armor: 0.9 lets a tenth less through.
cvar_t skill_armor_ratio_scale = {"skill_armor_ratio_scale", "0.9"};
cvar_t skill_fall_damage_scale = {"skill_fall_damage_scale", "0.5"};
cvar_t skill_battery_bonus = {"skill_battery_bonus", "50"};
// The Melee Route.  Reach and Speed are predicted and read from both DLLs
// through skill_tuning.h; Force, the Stat nodes and the Backstab node are
// server-side damage.
cvar_t skill_melee_reach_scale = {"skill_melee_reach_scale", "1.25"};
cvar_t skill_melee_force_scale = {"skill_melee_force_scale", "1.5"};
// Multiplies the swing delay: 0.7 is 30% faster.
cvar_t skill_melee_speed_scale = {"skill_melee_speed_scale", "0.7"};
// Each Melee Damage Stat node adds this to a multiplier on melee damage, so
// five of them at 0.05 are x1.25.  Additive within the stat, multiplied with
// the Skills.
cvar_t skill_stat_melee_damage = {"skill_stat_melee_damage", "0.05"};
cvar_t skill_stat_max_health = {"skill_stat_max_health", "0.05"};   // the hub's Max Health nodes, a fraction each
cvar_t skill_stat_max_armor = {"skill_stat_max_armor", "0.05"};     // the hub's and the Juggernaut's Max Armour nodes
// The Backstab node multiplies the Backstab's own multiplier
// (backstab_damage_scale, 3): 1.5 takes it to 4.5x.
cvar_t skill_backstab_bonus_scale = {"skill_backstab_bonus_scale", "1.5"};
// Cleave, the Melee major.  The first melee hit on a monster after the
// cooldown also strikes every monster in the arc, and every one of those
// hits, the first included, is scaled up.  All first guesses.
// 8 was the first guess and read as too long in play; 4 is the entry point
// that still stops spamming.
cvar_t cleave_cooldown = {"cleave_cooldown", "4"};
// Cosine of the half-angle: 0.77 is 40 degrees either side of the aim.  The
// first guess of 60 degrees was 277 units wide at the far edge and read as
// width where depth was wanted.
cvar_t cleave_arc_dot = {"cleave_arc_dot", "0.77"};
// How far from the player's eyes the arc reaches.  80 read as too short.
cvar_t cleave_radius = {"cleave_radius", "160"};
cvar_t cleave_damage_scale = {"cleave_damage_scale", "1.5"};
// The recovery after a Cleave swing, hit or miss, in seconds, scaled by
// Melee Speed only: the swipe animation is one length on every model that
// carries it, so the weapon's own swing scale does not apply.  Read from
// both DLLs through skill_tuning.h; 0 means the stock delays.  Set to the
// animation's length so it is seen whole: 1.2 s is HL Extended's
// attack_swing_miss3 (36 frames at 30 fps, the swipe itself over in the
// first third), the sequence in the slot since 2026-09-15; the script-made
// swipe before it was 0.65.  A first guess: if the long recovery reads as
// waiting, shorten it and let the next click cut the return, as the stock
// swings' clicks do.
cvar_t cleave_swing_time = {"cleave_swing_time", "1.2"};

// The Gauss Katana.  The slash is burst and the wave is DPS (decided
// 2026-09-14 against the gauss's two clicks, which it replaces): the left
// click hits hard and swings slow, the right click hits light, swings at the
// crowbar's rate, pierces and spends uranium.  The slash's damage goes
// through the sk_plr_katana skill cvars below like every other weapon's; the
// two swing times are read from both DLLs through skill_tuning.h because the
// delay they scale is predicted.  Multiples of the crowbar's own rate: 2.4 is
// 0.6 s after a hit and 1.2 s after a miss.  The slash started at 2.0, went
// to 1.0 when the wave still rode on it and the slow swing read as waiting,
// and is slow again now that the fast click is the other one.
cvar_t katana_swing_time_scale = {"katana_swing_time_scale", "2.4"};
cvar_t katana_wave_swing_time_scale = {"katana_wave_swing_time_scale", "1.0"};
// The wave the right click throws (CKatanaWave, dlls/katana.cpp): an unseen
// projectile whose look is the crescent the client draws (EV_KatanaArc).
// Energy damage to everything on its path, each once: katana_wave_damage
// full out to katana_wave_full_range, then falling off to nothing at
// katana_wave_range.  Speed in units per second.  The client reads range,
// full range and speed by name for the crescent, so the drawn wave and the
// damage are one thing; there are no client copies.  Damage is a plain
// number, not a fraction of the slash's: the two are tuned apart.
cvar_t katana_wave_damage = {"katana_wave_damage", "15"};
cvar_t katana_wave_range = {"katana_wave_range", "1200"};
cvar_t katana_wave_full_range = {"katana_wave_full_range", "800"};
cvar_t katana_wave_speed = {"katana_wave_speed", "1200"};
// Uranium a wave spends, before Energy Efficiency divides it.  Read from both
// DLLs through skill_tuning.h: the check is predicted.  0 is free.
cvar_t katana_wave_cost = {"katana_wave_cost", "5"};
// The blade's damage on the right click, which swings it beside the wave.
// Light, so the left click stays the melee verb; the melee Skills still
// multiply it.
cvar_t katana_wave_blade_damage = {"katana_wave_blade_damage", "10"};
cvar_t skill_weapon_damage_scale = {"skill_weapon_damage_scale", "1.1"};
// The Weapon Specialist Route (docs/SKILL_TREE.md).  Marksman multiplies
// player bullet damage; each Bullet Damage Stat node adds this to one
// multiplier on it, the Melee Damage nodes' shape.  First guesses.
cvar_t skill_marksman_scale = {"skill_marksman_scale", "1.15"};
cvar_t skill_stat_bullet_damage = {"skill_stat_bullet_damage", "0.05"};
// Quick Draw scales the draw delay.  Read from both DLLs through
// skill_tuning.h, because m_flNextAttack is predicted.
cvar_t skill_draw_time_scale = {"skill_draw_time_scale", "0.6"};
// Demolitions multiplies player DMG_BLAST dealt, and DMG_BLAST taken (own
// grenades included).  Headhunter multiplies the head hitgroup multiplier
// for player hits.  First guesses.
cvar_t skill_demolitions_scale = {"skill_demolitions_scale", "1.25"};
cvar_t skill_demolitions_resist_scale = {"skill_demolitions_resist_scale", "0.5"};
cvar_t skill_headhunter_scale = {"skill_headhunter_scale", "1.5"};
// Swap Surge, the Weapon Specialist major: for the window after a weapon
// swap, everything the player deals is scaled; the cooldown counts from the
// swap.  Both count from the swap itself, draw delay included.  First guesses.
cvar_t skill_swap_surge_scale = {"skill_swap_surge_scale", "1.5"};
cvar_t skill_swap_surge_window = {"skill_swap_surge_window", "2"};
cvar_t skill_swap_surge_cooldown = {"skill_swap_surge_cooldown", "6"};
// The Medical Route (docs/SKILL_TREE.md).  Each Healing Stat node adds this
// to one multiplier on an Infusion's rate and a medkit's heal; Leech heals
// this fraction of a melee hit's damage.  First guesses.
cvar_t skill_stat_healing = {"skill_stat_healing", "0.1"};
cvar_t skill_leech_fraction = {"skill_leech_fraction", "0.1"};
// Overheal: how far above the maximum an Infusion may heal, and how fast the
// excess drains, per second, once the Infusion ends.  First guesses.
cvar_t skill_overheal_cap = {"skill_overheal_cap", "50"};
cvar_t skill_overheal_decay = {"skill_overheal_decay", "2"};
// The Energy Route (docs/SKILL_TREE.md).  Energy Damage multiplies player
// DMG_ENERGYBEAM; each Energy Damage Stat node adds to one multiplier on it;
// Energy Efficiency scales the interval between the egon's ammo ticks (above
// 1 is slower) and divides the katana's wave's uranium cost (read from both
// DLLs through skill_tuning.h for that); Insulation scales DMG_ENERGYBEAM and
// DMG_SHOCK taken.  First guesses.
cvar_t skill_energy_damage_scale = {"skill_energy_damage_scale", "1.15"};
cvar_t skill_stat_energy_damage = {"skill_stat_energy_damage", "0.05"};
cvar_t skill_energy_efficiency_scale = {"skill_energy_efficiency_scale", "1.33"};
cvar_t skill_insulation_scale = {"skill_insulation_scale", "0.7"};
// Overdraw (the Energy major): energy attacks drain armour too, at this much
// per uranium spent, after Energy Efficiency has already reduced that cost,
// and never below the floor; while armour sits above the floor, energy
// damage is multiplied.  First guesses.
cvar_t skill_overdraw_armor_per_uranium = {"skill_overdraw_armor_per_uranium", "0.5"};
cvar_t skill_overdraw_floor = {"skill_overdraw_floor", "20"};
cvar_t skill_overdraw_damage_scale = {"skill_overdraw_damage_scale", "1.5"};
// Ricochet (the Juggernaut Route): the chance per bullet hit, while the
// player has armour, that it bounces back at the shooter.  A first guess;
// ranks will raise it when the Route's region is laid out.
cvar_t skill_ricochet_chance = {"skill_ricochet_chance", "0.2"};
// Last Stand (the Medical major) and Glass Cannon (the keystone that arms it
// permanently): the invincibility window, the cooldown before it can fire
// again, the health threshold below which an Infusion heals doubled, and
// that scale.  Glass Cannon's own ceiling is separate.  First guesses.
cvar_t skill_last_stand_invuln = {"skill_last_stand_invuln", "3"};
cvar_t skill_last_stand_cooldown = {"skill_last_stand_cooldown", "60"};
cvar_t skill_last_stand_low_health = {"skill_last_stand_low_health", "50"};
cvar_t skill_last_stand_heal_scale = {"skill_last_stand_heal_scale", "2"};
cvar_t skill_glass_cannon_max_health = {"skill_glass_cannon_max_health", "50"};
// The Dash (docs/PILLARS.md, pillar 3) and the Shinobi Route that grows it.
// The burst's speed in units per second and its length in seconds, the time
// one charge takes to come back; Dash Reach scales the length, Dash Recovery
// and each Dash Recovery Stat node take a fraction off the recharge.  First
// guesses: 800 for 0.15 is about 120 units.  The recharge is long on purpose:
// the base Dash is a rare escape, and the Route is what makes it a habit.
cvar_t dash_speed = {"dash_speed", "800"};
cvar_t dash_time = {"dash_time", "0.15"};
cvar_t dash_recharge = {"dash_recharge", "7"};
cvar_t skill_dash_reach_scale = {"skill_dash_reach_scale", "1.5"};
cvar_t skill_dash_recovery = {"skill_dash_recovery", "0.25"};
cvar_t skill_stat_dash_recovery = {"skill_stat_dash_recovery", "0.05"};
// Scales the reload delay. Read from both DLLs through skill_tuning.h, because
// the delay it sets is m_flNextAttack, which the client predicts.
cvar_t skill_reload_time_scale = {"skill_reload_time_scale", "0.8"};

// The Pulse.  Tuning knobs -- see docs/PILLARS.md pillar 2.  The Recharge is
// deliberately asymmetric: a Shield that negated something recovers faster than
// one that negated nothing, so good reads chain and whiffs strand you.
cvar_t pulse_window = {"pulse_window", "0.25"};
cvar_t pulse_window_bonus = {"pulse_window_bonus", "0.15"};
cvar_t pulse_recharge_hit = {"pulse_recharge_hit", "1.5"};
cvar_t pulse_recharge_miss = {"pulse_recharge_miss", "3.0"};
cvar_t pulse_recharge_scale = {"pulse_recharge_scale", "0.66"};
// The cap is load-bearing, not cosmetic: without it, timing a Pulse against the
// hardest-hitting attacks in the game yields the strongest counter.
cvar_t pulse_discharge_scale = {"pulse_discharge_scale", "0.75"};
cvar_t pulse_discharge_min = {"pulse_discharge_min", "15"};
cvar_t pulse_discharge_max = {"pulse_discharge_max", "60"};
// Shield ring geometry, presentation only.  Style 0 is TE_BEAMCYLINDER (a ring
// expanding along the ground), 1 is TE_BEAMTORUS (screen-aligned, centred on
// the player).  Nothing else in the SDK uses the torus, so the right scale for
// it is not known from any existing call site -- these exist to be dialled in
// by eye rather than by rebuilding.
cvar_t pulse_ring_style = {"pulse_ring_style", "1"};
cvar_t pulse_ring_scale = {"pulse_ring_scale", "320"};
// How much of a melee attacker's view kick survives a deflect. 1 is untouched,
// 0 removes it entirely. Deliberately not 0: a blow that glances off the Shield
// should still register as something happening, just not as something landing.
cvar_t pulse_deflect_punch = {"pulse_deflect_punch", "0.25"};
// Whether deflecting a melee blow fires a Discharge. On by default; this exists
// because that behaviour was never designed -- it falls out of "one Discharge
// per negated hit" -- and is the most likely part of the Pulse to be judged
// wrong under more testing.
cvar_t pulse_discharge_melee = {"pulse_discharge_melee", "1"};
// The Follow-Up. Primed by a deflect, spent on the next crowbar hit -- a whiff
// costs nothing, so the window is what stops it being banked indefinitely.
// Knockback is headcrab-only; see PulseCrowbarFollowUpKnockback for why.
cvar_t pulse_followup_time = {"pulse_followup_time", "2.0"};
cvar_t pulse_followup_damage = {"pulse_followup_damage", "3.0"};
cvar_t pulse_followup_knockback = {"pulse_followup_knockback", "500"};

// The Defense Matrix -- the Juggernaut Route's stance (docs/SKILL_TREE.md).
// Hold the Pulse key for skill_matrix_hold seconds and it comes up; while it
// is up, armour takes a far larger share of every hit and the player is
// slowed.  It drops on release, at skill_matrix_duration, or at zero armour,
// then waits skill_matrix_cooldown.  Every number is a first guess.
cvar_t skill_matrix_hold = {"skill_matrix_hold", "1.0"};
cvar_t skill_matrix_duration = {"skill_matrix_duration", "6"};
cvar_t skill_matrix_cooldown = {"skill_matrix_cooldown", "10"};
// While it is up nothing reaches health and armour pays for the whole hit at
// this much per point of damage (CBasePlayer::TakeDamage).  0.5: a 20 hit
// costs 10 AP and 0 HP, so a point of armour buys two of health.  Reworked
// 2026-09-16 from a larger share of the stock split, which could not be read.
cvar_t skill_matrix_armor_cost_scale = {"skill_matrix_armor_cost_scale", "0.5"};
// The slow: the player's maxspeed as a fraction of sv_maxspeed while up.  The
// Route's whole cost, paid only while the protection is on.
cvar_t skill_matrix_speed_scale = {"skill_matrix_speed_scale", "0.8"};
// Matrix on Kill: armour restored, up to the maximum, per kill while up.
cvar_t skill_matrix_kill_armor = {"skill_matrix_kill_armor", "15"};
// Decaying Armor (the Major): armour granted on activation, above the cap.
// It fades over skill_matrix_duration, so it is gone as the Matrix drops; the
// rate is derived, not a knob.  100 was set "to be toned down".
cvar_t skill_matrix_grant = {"skill_matrix_grant", "100"};

// The Infusion -- see docs/PILLARS.md pillar 3.  40 HP over 10 seconds is more
// than two medkits, and the duration is what pays for it: none of it lands if
// the player does not survive the ten seconds, and it cannot answer burst
// damage the way an instant medkit can.
cvar_t infusion_rate = {"infusion_rate", "4"};
cvar_t infusion_duration = {"infusion_duration", "10"};
// Med Expert. Additive rather than a percentage so it stays legible when
// infusion_duration is tuned -- the same choice pulse_window_bonus makes.
cvar_t infusion_duration_bonus = {"infusion_duration_bonus", "5"};

// The Backstab -- see docs/adr/0010-the-backstab-is-positional.md.
//
// 3x is chosen so a fully invested melee build very nearly one-shots a grunt
// and does not quite: 10 base, x1.5 Crowbar Force, x3 here, x1.1 Weapon Mastery
// is 49.5 against 50 health.  Landing on the wrong side of a round number on
// purpose, because the moment this reliably one-shots a grunt there is no
// reason left to ever fight one head-on.  First guess; judge it in play.
cvar_t backstab_damage_scale = {"backstab_damage_scale", "3"};
// A dot product against the victim's own facing, matching m_flFieldOfView's
// units, so lower is a NARROWER rear arc.  -0.5 is the rear 120 degrees.
cvar_t backstab_arc_dot = {"backstab_arc_dot", "-0.5"};

// Concealment and Suspicion -- see docs/PERCEPTION.md part 2 and
// docs/adr/0009-suspicion-gates-the-relationship-bits.md.  Every one of these
// is a first guess and none has been judged in play.
//
// A master switch first, so the whole gate can be A/B'd against vanilla
// acquisition mid-game without a rebuild.  0 restores the base game exactly.
cvar_t suspicion_enable = {"suspicion_enable", "1"};
// Meter units per second at full exposure, before the Perception Profile's
// scale.  At 2 a Trained monster staring at a standing, lit, point-blank
// player takes about a third of a second to acquire them, and a fully
// invested Stealth player crouched in front of a lit grunt at mid range has
// about three seconds, cover to cover -- and the same monster looking at a
// crouched player in a dark doorway at range takes minutes, because the
// Concealment terms multiply.  1.0 until 2026-09-17; doubled with the words
// "stealth should function behind unaware enemies and in darkness".
cvar_t suspicion_fill = {"suspicion_fill", "2.0"};
// Units per second while the player is not visible.  Roughly three seconds
// from full to forgotten.
cvar_t suspicion_drain = {"suspicion_drain", "0.35"};
// The two thresholds.  Acquisition is the top of the meter, which is where
// everything the base game already does takes over unchanged.  Notice is what
// the player's readout is derived from -- it exists now so the debug view can
// mark it; the HUD element that uses it is the next commit.
cvar_t suspicion_notice = {"suspicion_notice", "0.35"};
cvar_t suspicion_acquire = {"suspicion_acquire", "1.0"};

// The post-aggro step, settled 2026-09-17 (docs/ROADMAP.md).  A monster that
// watches a squadmate die jumps to this -- Noticed, not Spotted: in the open
// the difference from acquisition is a sixth of a second, from cover it is the
// beat the predator loop is made of.
cvar_t suspicion_witness = {"suspicion_witness", "0.75"};
// ...and from there drains as normal, but never again below this for the
// rest of the level.  Just UNDER suspicion_notice on purpose: the readout
// reports Noticed at or above the line, and a floor on the line would leave
// the icon amber forever.
cvar_t suspicion_floor = {"suspicion_floor", "0.3"};
// The Disturbance: a body, as a sound.  Volume is a hearing radius in units
// (a room); duration is modest because the pool is 64 for the whole world and
// the carcass scent already takes 30.
cvar_t disturbance_volume = {"disturbance_volume", "512"};
cvar_t disturbance_duration = {"disturbance_duration", "20"};

// The four Concealment terms.  Each is the exposure fraction at the WORST end
// of that term -- at the rim of the cone, at the limit of sight, crouched, in
// the dark.  None is ever 0: a term that could reach zero would zero the whole
// product and make stealth absolute, which is a bug rather than a build.
cvar_t conceal_angle_edge = {"conceal_angle_edge", "0.25"};
cvar_t conceal_dist_far = {"conceal_dist_far", "0.25"};
cvar_t conceal_stance_duck = {"conceal_stance_duck", "0.4"};
cvar_t conceal_stance_walk = {"conceal_stance_walk", "0.7"};
// Deliberately the mildest of the four.  Vanilla maps are lit for readability
// rather than for hiding, so light cannot be the dominant lever until there
// are maps with dark places in them.
cvar_t conceal_light_dark = {"conceal_light_dark", "0.5"};

// Noise.  UpdatePlayerSound already makes a slow player quiet, but only as a
// side effect of velocity, and that accident was not nearly enough: a crouched
// approach still put a sound inside a grunt's hearing radius at melee range,
// which turned him around and collapsed the Concealment angle term exactly as
// the player arrived to Backstab him.  These make quiet movement a decision.
//
// Sized against the crowbar rather than by feel: a crouched player moves at
// roughly 107 units/sec, monsters hear a sound out to its volume in units, and
// the crowbar reaches about 32.  0.3 puts a crouched approach just inside that.
cvar_t noise_stance_duck = {"noise_stance_duck", "0.3"};
cvar_t noise_stance_walk = {"noise_stance_walk", "0.6"};

// The Stealth region -- docs/SKILL_TREE.md, "Stealth".  Every number a first
// guess, set 2026-09-16 from the design's own words.
//
// Each Concealment Stat node makes every monster's meter fill this much
// slower against this player: ten of them are half speed.
cvar_t skill_stat_concealment = {"skill_stat_concealment", "0.05"};
// Soft Step: the crouched and walking body noise halved again, on top of
// noise_stance_duck / noise_stance_walk.  Running is untouched.
cvar_t skill_soft_step_scale = {"skill_soft_step_scale", "0.5"};
// Nightfall: darkness conceals twice as much -- conceal_light_dark, the
// exposure at the dark end of the light term, scaled by this.
cvar_t skill_nightfall_scale = {"skill_nightfall_scale", "0.5"};
// Slip Away: breaking line of sight while a monster has Noticed but not
// Spotted the player drops its Suspicion by this fraction, once, on the break.
cvar_t skill_slip_away_fraction = {"skill_slip_away_fraction", "0.33"};
// Ambush: player damage to a monster, by how unaware it is at the hit, read
// off the victim's own meter.  Below suspicion_acquire the first; below
// suspicion_notice the second instead, not on top.  1.25 / 1.5 until
// 2026-09-17; raised so a silenced round in the head of a witness at Noticed
// (8 x 3 head x 1.5 Headhunter x 1.5) kills a grunt -- docs/SKILL_TREE.md.
cvar_t skill_ambush_spotted_scale = {"skill_ambush_spotted_scale", "1.5"};
cvar_t skill_ambush_noticed_scale = {"skill_ambush_noticed_scale", "2.0"};
// Shroud: a flat multiplier on the fill for the holder -- the ten Concealment
// roads over again in one node.  Took Cut the Head's cell on 2026-09-17.
cvar_t skill_shroud_scale = {"skill_shroud_scale", "0.8"};
// Phantom: a Backstab KILL on a monster below Noticed buys this many seconds
// of silent movement at this multiple of the run speed.  2 s at x1.2 on a
// Backstab hit was the first shape; Andrei set the kill, 4 s and x1.5 on
// 2026-09-16 after the first play.
cvar_t skill_phantom_duration = {"skill_phantom_duration", "4"};
cvar_t skill_phantom_speed_scale = {"skill_phantom_speed_scale", "1.5"};

// The Alien region's Hive nodes -- docs/SKILL_TREE.md, "Alien".  First guesses.
cvar_t skill_hive_capacity_bonus = {"skill_hive_capacity_bonus", "4"};
cvar_t skill_hive_replenish_scale = {"skill_hive_replenish_scale", "1.5"};
cvar_t skill_hive_attack_speed_scale = {"skill_hive_attack_speed_scale", "0.75"};
cvar_t skill_stat_hornet_replenish = {"skill_stat_hornet_replenish", "0.05"};

// The summon weapon and its ghosts -- docs/ROADMAP.md, "The summon weapon --
// settled".  First guesses, every one; the Route sells more ghosts, longer,
// sooner, so the three Skills below are the three numbers above.  Pack is a
// count rather than a scale because the design's 1 -> 2 -> 3 ranks are one
// node in the tree today.
cvar_t summon_cooldown = {"summon_cooldown", "3"};
cvar_t summon_max_ghosts = {"summon_max_ghosts", "1"};
cvar_t summon_ghost_lifetime = {"summon_ghost_lifetime", "30"};
cvar_t skill_pack_bonus = {"skill_pack_bonus", "1"};
cvar_t skill_tether_scale = {"skill_tether_scale", "1.5"};
cvar_t skill_recall_scale = {"skill_recall_scale", "0.5"};

// Damage debug readout -- see game.h.  Throwaway diagnostic, off by default.
cvar_t debug_damage = {"debug_damage", "0"};
// Monster aim readout -- see game.h.  Also throwaway, and it also shares the
// screen centre, so run only one debug_* readout at a time.
cvar_t debug_monster_aim = {"debug_monster_aim", "0"};
// The monster under the crosshair: its state, squad role, schedule, task,
// meter and Concealment, plus the last kill and the last Search dispatch --
// see perception.h.  Replaced debug_suspicion 2026-09-17.  Same screen
// centre as debug_damage and debug_monster_aim, same one-at-a-time rule.
cvar_t debug_schedule = {"debug_schedule", "0"};
// The player cannot be seen or heard by any monster: FL_NOTARGET (what the
// notarget cheat sets; Look skips the player outright) plus m_fNoPlayerSound
// (the SDK's own silent-movement switch, which nothing else turns on).  Being
// shot still counts.  For standing next to a Search and watching it.
cvar_t debug_invisible = {"debug_invisible", "0"};

static bool SV_InitServer()
{
	if (!FileSystem_LoadFileSystem())
	{
		return false;
	}

	if (UTIL_IsValveGameDirectory())
	{
		g_engfuncs.pfnServerPrint("This mod has detected that it is being run from a Valve game directory which is not supported\n"
			"Run this mod from its intended location\n\nThe game will now shut down\n");
		return false;
	}

	return true;
}

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit()
{
	// Register cvars here:

	g_psv_gravity = CVAR_GET_POINTER("sv_gravity");
	g_psv_aim = CVAR_GET_POINTER("sv_aim");
	g_psv_allow_autoaim = CVAR_GET_POINTER("sv_allow_autoaim");
	g_footsteps = CVAR_GET_POINTER("mp_footsteps");
	g_psv_cheats = CVAR_GET_POINTER("sv_cheats");

	if (!SV_InitServer())
	{
		g_engfuncs.pfnServerPrint("Error initializing server\n");
		//Shut the game down as soon as possible.
		SERVER_COMMAND("quit\n");
		return;
	}

	CVAR_REGISTER(&displaysoundlist);
	CVAR_REGISTER(&allow_spectators);

	CVAR_REGISTER(&teamplay);
	CVAR_REGISTER(&fraglimit);
	CVAR_REGISTER(&timelimit);

	CVAR_REGISTER(&fragsleft);
	CVAR_REGISTER(&timeleft);

	CVAR_REGISTER(&friendlyfire);
	CVAR_REGISTER(&falldamage);
	CVAR_REGISTER(&weaponstay);
	CVAR_REGISTER(&forcerespawn);
	CVAR_REGISTER(&flashlight);
	CVAR_REGISTER(&aimcrosshair);
	CVAR_REGISTER(&decalfrequency);
	CVAR_REGISTER(&teamlist);
	CVAR_REGISTER(&teamoverride);
	CVAR_REGISTER(&defaultteam);
	CVAR_REGISTER(&allowmonsters);

	CVAR_REGISTER(&mp_chattime);

	CVAR_REGISTER(&sv_busters);

	CVAR_REGISTER(&sv_allowbunnyhopping);

	CVAR_REGISTER(&inv_rows_start);
	CVAR_REGISTER(&inv_rows_max);
	CVAR_REGISTER(&item_debug);

	CVAR_REGISTER(&skill_points_start);
	CVAR_REGISTER(&skill_reset_tokens_start);
	CVAR_REGISTER(&skill_unlock_all);

	CVAR_REGISTER(&skill_health_bonus);
	CVAR_REGISTER(&skill_armor_ratio_scale);
	CVAR_REGISTER(&skill_fall_damage_scale);
	CVAR_REGISTER(&skill_battery_bonus);
	CVAR_REGISTER(&skill_melee_reach_scale);
	CVAR_REGISTER(&skill_melee_force_scale);
	CVAR_REGISTER(&skill_melee_speed_scale);
	CVAR_REGISTER(&skill_stat_melee_damage);
	CVAR_REGISTER(&skill_stat_max_health);
	CVAR_REGISTER(&skill_stat_max_armor);
	CVAR_REGISTER(&skill_backstab_bonus_scale);
	CVAR_REGISTER(&cleave_cooldown);
	CVAR_REGISTER(&cleave_arc_dot);
	CVAR_REGISTER(&cleave_radius);
	CVAR_REGISTER(&cleave_damage_scale);
	CVAR_REGISTER(&cleave_swing_time);
	CVAR_REGISTER(&katana_swing_time_scale);
	CVAR_REGISTER(&katana_wave_swing_time_scale);
	CVAR_REGISTER(&katana_wave_damage);
	CVAR_REGISTER(&katana_wave_range);
	CVAR_REGISTER(&katana_wave_full_range);
	CVAR_REGISTER(&katana_wave_speed);
	CVAR_REGISTER(&katana_wave_cost);
	CVAR_REGISTER(&katana_wave_blade_damage);
	CVAR_REGISTER(&skill_reload_time_scale);
	CVAR_REGISTER(&skill_weapon_damage_scale);
	CVAR_REGISTER(&skill_marksman_scale);
	CVAR_REGISTER(&skill_stat_bullet_damage);
	CVAR_REGISTER(&skill_draw_time_scale);
	CVAR_REGISTER(&skill_demolitions_scale);
	CVAR_REGISTER(&skill_demolitions_resist_scale);
	CVAR_REGISTER(&skill_headhunter_scale);
	CVAR_REGISTER(&skill_swap_surge_scale);
	CVAR_REGISTER(&skill_swap_surge_window);
	CVAR_REGISTER(&skill_swap_surge_cooldown);
	CVAR_REGISTER(&skill_stat_healing);
	CVAR_REGISTER(&skill_leech_fraction);
	CVAR_REGISTER(&skill_overheal_cap);
	CVAR_REGISTER(&skill_overheal_decay);
	CVAR_REGISTER(&skill_energy_damage_scale);
	CVAR_REGISTER(&skill_stat_energy_damage);
	CVAR_REGISTER(&skill_energy_efficiency_scale);
	CVAR_REGISTER(&skill_insulation_scale);
	CVAR_REGISTER(&skill_overdraw_armor_per_uranium);
	CVAR_REGISTER(&skill_overdraw_floor);
	CVAR_REGISTER(&skill_overdraw_damage_scale);
	CVAR_REGISTER(&skill_ricochet_chance);
	CVAR_REGISTER(&skill_last_stand_invuln);
	CVAR_REGISTER(&skill_last_stand_cooldown);
	CVAR_REGISTER(&skill_last_stand_low_health);
	CVAR_REGISTER(&skill_last_stand_heal_scale);
	CVAR_REGISTER(&skill_glass_cannon_max_health);
	CVAR_REGISTER(&dash_speed);
	CVAR_REGISTER(&dash_time);
	CVAR_REGISTER(&dash_recharge);
	CVAR_REGISTER(&skill_dash_reach_scale);
	CVAR_REGISTER(&skill_dash_recovery);
	CVAR_REGISTER(&skill_stat_dash_recovery);

	CVAR_REGISTER(&pulse_window);
	CVAR_REGISTER(&pulse_window_bonus);
	CVAR_REGISTER(&pulse_recharge_hit);
	CVAR_REGISTER(&pulse_recharge_miss);
	CVAR_REGISTER(&pulse_recharge_scale);
	CVAR_REGISTER(&pulse_discharge_scale);
	CVAR_REGISTER(&pulse_discharge_min);
	CVAR_REGISTER(&pulse_discharge_max);
	CVAR_REGISTER(&pulse_ring_style);
	CVAR_REGISTER(&pulse_ring_scale);
	CVAR_REGISTER(&pulse_deflect_punch);
	CVAR_REGISTER(&pulse_discharge_melee);
	CVAR_REGISTER(&infusion_rate);
	CVAR_REGISTER(&infusion_duration);
	CVAR_REGISTER(&infusion_duration_bonus);

	CVAR_REGISTER(&pulse_followup_time);
	CVAR_REGISTER(&pulse_followup_damage);
	CVAR_REGISTER(&pulse_followup_knockback);

	CVAR_REGISTER(&skill_matrix_hold);
	CVAR_REGISTER(&skill_matrix_duration);
	CVAR_REGISTER(&skill_matrix_cooldown);
	CVAR_REGISTER(&skill_matrix_armor_cost_scale);
	CVAR_REGISTER(&skill_matrix_speed_scale);
	CVAR_REGISTER(&skill_matrix_kill_armor);
	CVAR_REGISTER(&skill_matrix_grant);

	CVAR_REGISTER(&backstab_damage_scale);
	CVAR_REGISTER(&backstab_arc_dot);

	CVAR_REGISTER(&suspicion_enable);
	CVAR_REGISTER(&suspicion_fill);
	CVAR_REGISTER(&suspicion_drain);
	CVAR_REGISTER(&suspicion_notice);
	CVAR_REGISTER(&suspicion_acquire);
	CVAR_REGISTER(&suspicion_witness);
	CVAR_REGISTER(&suspicion_floor);
	CVAR_REGISTER(&disturbance_volume);
	CVAR_REGISTER(&disturbance_duration);
	CVAR_REGISTER(&skill_shroud_scale);
	CVAR_REGISTER(&noise_stance_duck);
	CVAR_REGISTER(&noise_stance_walk);
	CVAR_REGISTER(&conceal_angle_edge);
	CVAR_REGISTER(&conceal_dist_far);
	CVAR_REGISTER(&conceal_stance_duck);
	CVAR_REGISTER(&conceal_stance_walk);
	CVAR_REGISTER(&conceal_light_dark);

	CVAR_REGISTER(&skill_stat_concealment);
	CVAR_REGISTER(&skill_soft_step_scale);
	CVAR_REGISTER(&skill_nightfall_scale);
	CVAR_REGISTER(&skill_slip_away_fraction);
	CVAR_REGISTER(&skill_ambush_spotted_scale);
	CVAR_REGISTER(&skill_ambush_noticed_scale);
	CVAR_REGISTER(&skill_phantom_duration);
	CVAR_REGISTER(&skill_phantom_speed_scale);

	CVAR_REGISTER(&skill_hive_capacity_bonus);
	CVAR_REGISTER(&skill_hive_replenish_scale);
	CVAR_REGISTER(&skill_hive_attack_speed_scale);
	CVAR_REGISTER(&skill_stat_hornet_replenish);

	CVAR_REGISTER(&summon_cooldown);
	CVAR_REGISTER(&summon_max_ghosts);
	CVAR_REGISTER(&summon_ghost_lifetime);
	CVAR_REGISTER(&skill_pack_bonus);
	CVAR_REGISTER(&skill_tether_scale);
	CVAR_REGISTER(&skill_recall_scale);

	CVAR_REGISTER(&debug_damage);
	CVAR_REGISTER(&debug_schedule);
	CVAR_REGISTER(&debug_invisible);
	CVAR_REGISTER(&debug_monster_aim);

	// REGISTER CVARS FOR SKILL LEVEL STUFF
	// Agrunt
	CVAR_REGISTER(&sk_agrunt_health1); // {"sk_agrunt_health1","0"};
	CVAR_REGISTER(&sk_agrunt_health2); // {"sk_agrunt_health2","0"};
	CVAR_REGISTER(&sk_agrunt_health3); // {"sk_agrunt_health3","0"};

	CVAR_REGISTER(&sk_agrunt_dmg_punch1); // {"sk_agrunt_dmg_punch1","0"};
	CVAR_REGISTER(&sk_agrunt_dmg_punch2); // {"sk_agrunt_dmg_punch2","0"};
	CVAR_REGISTER(&sk_agrunt_dmg_punch3); // {"sk_agrunt_dmg_punch3","0"};

	// Apache
	CVAR_REGISTER(&sk_apache_health1); // {"sk_apache_health1","0"};
	CVAR_REGISTER(&sk_apache_health2); // {"sk_apache_health2","0"};
	CVAR_REGISTER(&sk_apache_health3); // {"sk_apache_health3","0"};

	// Barney
	CVAR_REGISTER(&sk_barney_health1); // {"sk_barney_health1","0"};
	CVAR_REGISTER(&sk_barney_health2); // {"sk_barney_health2","0"};
	CVAR_REGISTER(&sk_barney_health3); // {"sk_barney_health3","0"};

	// Bullsquid
	CVAR_REGISTER(&sk_bullsquid_health1); // {"sk_bullsquid_health1","0"};
	CVAR_REGISTER(&sk_bullsquid_health2); // {"sk_bullsquid_health2","0"};
	CVAR_REGISTER(&sk_bullsquid_health3); // {"sk_bullsquid_health3","0"};

	CVAR_REGISTER(&sk_bullsquid_dmg_bite1); // {"sk_bullsquid_dmg_bite1","0"};
	CVAR_REGISTER(&sk_bullsquid_dmg_bite2); // {"sk_bullsquid_dmg_bite2","0"};
	CVAR_REGISTER(&sk_bullsquid_dmg_bite3); // {"sk_bullsquid_dmg_bite3","0"};

	CVAR_REGISTER(&sk_bullsquid_dmg_whip1); // {"sk_bullsquid_dmg_whip1","0"};
	CVAR_REGISTER(&sk_bullsquid_dmg_whip2); // {"sk_bullsquid_dmg_whip2","0"};
	CVAR_REGISTER(&sk_bullsquid_dmg_whip3); // {"sk_bullsquid_dmg_whip3","0"};

	CVAR_REGISTER(&sk_bullsquid_dmg_spit1); // {"sk_bullsquid_dmg_spit1","0"};
	CVAR_REGISTER(&sk_bullsquid_dmg_spit2); // {"sk_bullsquid_dmg_spit2","0"};
	CVAR_REGISTER(&sk_bullsquid_dmg_spit3); // {"sk_bullsquid_dmg_spit3","0"};


	CVAR_REGISTER(&sk_bigmomma_health_factor1); // {"sk_bigmomma_health_factor1","1.0"};
	CVAR_REGISTER(&sk_bigmomma_health_factor2); // {"sk_bigmomma_health_factor2","1.0"};
	CVAR_REGISTER(&sk_bigmomma_health_factor3); // {"sk_bigmomma_health_factor3","1.0"};

	CVAR_REGISTER(&sk_bigmomma_dmg_slash1); // {"sk_bigmomma_dmg_slash1","50"};
	CVAR_REGISTER(&sk_bigmomma_dmg_slash2); // {"sk_bigmomma_dmg_slash2","50"};
	CVAR_REGISTER(&sk_bigmomma_dmg_slash3); // {"sk_bigmomma_dmg_slash3","50"};

	CVAR_REGISTER(&sk_bigmomma_dmg_blast1); // {"sk_bigmomma_dmg_blast1","100"};
	CVAR_REGISTER(&sk_bigmomma_dmg_blast2); // {"sk_bigmomma_dmg_blast2","100"};
	CVAR_REGISTER(&sk_bigmomma_dmg_blast3); // {"sk_bigmomma_dmg_blast3","100"};

	CVAR_REGISTER(&sk_bigmomma_radius_blast1); // {"sk_bigmomma_radius_blast1","250"};
	CVAR_REGISTER(&sk_bigmomma_radius_blast2); // {"sk_bigmomma_radius_blast2","250"};
	CVAR_REGISTER(&sk_bigmomma_radius_blast3); // {"sk_bigmomma_radius_blast3","250"};

	// Gargantua
	CVAR_REGISTER(&sk_gargantua_health1); // {"sk_gargantua_health1","0"};
	CVAR_REGISTER(&sk_gargantua_health2); // {"sk_gargantua_health2","0"};
	CVAR_REGISTER(&sk_gargantua_health3); // {"sk_gargantua_health3","0"};

	CVAR_REGISTER(&sk_gargantua_dmg_slash1); // {"sk_gargantua_dmg_slash1","0"};
	CVAR_REGISTER(&sk_gargantua_dmg_slash2); // {"sk_gargantua_dmg_slash2","0"};
	CVAR_REGISTER(&sk_gargantua_dmg_slash3); // {"sk_gargantua_dmg_slash3","0"};

	CVAR_REGISTER(&sk_gargantua_dmg_fire1); // {"sk_gargantua_dmg_fire1","0"};
	CVAR_REGISTER(&sk_gargantua_dmg_fire2); // {"sk_gargantua_dmg_fire2","0"};
	CVAR_REGISTER(&sk_gargantua_dmg_fire3); // {"sk_gargantua_dmg_fire3","0"};

	CVAR_REGISTER(&sk_gargantua_dmg_stomp1); // {"sk_gargantua_dmg_stomp1","0"};
	CVAR_REGISTER(&sk_gargantua_dmg_stomp2); // {"sk_gargantua_dmg_stomp2","0"};
	CVAR_REGISTER(&sk_gargantua_dmg_stomp3); // {"sk_gargantua_dmg_stomp3","0"};


	// Hassassin
	CVAR_REGISTER(&sk_hassassin_health1); // {"sk_hassassin_health1","0"};
	CVAR_REGISTER(&sk_hassassin_health2); // {"sk_hassassin_health2","0"};
	CVAR_REGISTER(&sk_hassassin_health3); // {"sk_hassassin_health3","0"};


	// Headcrab
	CVAR_REGISTER(&sk_headcrab_health1); // {"sk_headcrab_health1","0"};
	CVAR_REGISTER(&sk_headcrab_health2); // {"sk_headcrab_health2","0"};
	CVAR_REGISTER(&sk_headcrab_health3); // {"sk_headcrab_health3","0"};

	CVAR_REGISTER(&sk_headcrab_dmg_bite1); // {"sk_headcrab_dmg_bite1","0"};
	CVAR_REGISTER(&sk_headcrab_dmg_bite2); // {"sk_headcrab_dmg_bite2","0"};
	CVAR_REGISTER(&sk_headcrab_dmg_bite3); // {"sk_headcrab_dmg_bite3","0"};


	// Hgrunt
	CVAR_REGISTER(&sk_hgrunt_health1); // {"sk_hgrunt_health1","0"};
	CVAR_REGISTER(&sk_hgrunt_health2); // {"sk_hgrunt_health2","0"};
	CVAR_REGISTER(&sk_hgrunt_health3); // {"sk_hgrunt_health3","0"};

	CVAR_REGISTER(&sk_hgrunt_kick1); // {"sk_hgrunt_kick1","0"};
	CVAR_REGISTER(&sk_hgrunt_kick2); // {"sk_hgrunt_kick2","0"};
	CVAR_REGISTER(&sk_hgrunt_kick3); // {"sk_hgrunt_kick3","0"};

	CVAR_REGISTER(&sk_hgrunt_pellets1);
	CVAR_REGISTER(&sk_hgrunt_pellets2);
	CVAR_REGISTER(&sk_hgrunt_pellets3);

	CVAR_REGISTER(&sk_hgrunt_gspeed1);
	CVAR_REGISTER(&sk_hgrunt_gspeed2);
	CVAR_REGISTER(&sk_hgrunt_gspeed3);

	// Houndeye
	CVAR_REGISTER(&sk_houndeye_health1); // {"sk_houndeye_health1","0"};
	CVAR_REGISTER(&sk_houndeye_health2); // {"sk_houndeye_health2","0"};
	CVAR_REGISTER(&sk_houndeye_health3); // {"sk_houndeye_health3","0"};

	CVAR_REGISTER(&sk_houndeye_dmg_blast1); // {"sk_houndeye_dmg_blast1","0"};
	CVAR_REGISTER(&sk_houndeye_dmg_blast2); // {"sk_houndeye_dmg_blast2","0"};
	CVAR_REGISTER(&sk_houndeye_dmg_blast3); // {"sk_houndeye_dmg_blast3","0"};


	// ISlave
	CVAR_REGISTER(&sk_islave_health1); // {"sk_islave_health1","0"};
	CVAR_REGISTER(&sk_islave_health2); // {"sk_islave_health2","0"};
	CVAR_REGISTER(&sk_islave_health3); // {"sk_islave_health3","0"};

	CVAR_REGISTER(&sk_islave_dmg_claw1); // {"sk_islave_dmg_claw1","0"};
	CVAR_REGISTER(&sk_islave_dmg_claw2); // {"sk_islave_dmg_claw2","0"};
	CVAR_REGISTER(&sk_islave_dmg_claw3); // {"sk_islave_dmg_claw3","0"};

	CVAR_REGISTER(&sk_islave_dmg_clawrake1); // {"sk_islave_dmg_clawrake1","0"};
	CVAR_REGISTER(&sk_islave_dmg_clawrake2); // {"sk_islave_dmg_clawrake2","0"};
	CVAR_REGISTER(&sk_islave_dmg_clawrake3); // {"sk_islave_dmg_clawrake3","0"};

	CVAR_REGISTER(&sk_islave_dmg_zap1); // {"sk_islave_dmg_zap1","0"};
	CVAR_REGISTER(&sk_islave_dmg_zap2); // {"sk_islave_dmg_zap2","0"};
	CVAR_REGISTER(&sk_islave_dmg_zap3); // {"sk_islave_dmg_zap3","0"};


	// Icthyosaur
	CVAR_REGISTER(&sk_ichthyosaur_health1); // {"sk_ichthyosaur_health1","0"};
	CVAR_REGISTER(&sk_ichthyosaur_health2); // {"sk_ichthyosaur_health2","0"};
	CVAR_REGISTER(&sk_ichthyosaur_health3); // {"sk_ichthyosaur_health3","0"};

	CVAR_REGISTER(&sk_ichthyosaur_shake1); // {"sk_ichthyosaur_health3","0"};
	CVAR_REGISTER(&sk_ichthyosaur_shake2); // {"sk_ichthyosaur_health3","0"};
	CVAR_REGISTER(&sk_ichthyosaur_shake3); // {"sk_ichthyosaur_health3","0"};



	// Leech
	CVAR_REGISTER(&sk_leech_health1); // {"sk_leech_health1","0"};
	CVAR_REGISTER(&sk_leech_health2); // {"sk_leech_health2","0"};
	CVAR_REGISTER(&sk_leech_health3); // {"sk_leech_health3","0"};

	CVAR_REGISTER(&sk_leech_dmg_bite1); // {"sk_leech_dmg_bite1","0"};
	CVAR_REGISTER(&sk_leech_dmg_bite2); // {"sk_leech_dmg_bite2","0"};
	CVAR_REGISTER(&sk_leech_dmg_bite3); // {"sk_leech_dmg_bite3","0"};


	// Controller
	CVAR_REGISTER(&sk_controller_health1);
	CVAR_REGISTER(&sk_controller_health2);
	CVAR_REGISTER(&sk_controller_health3);

	CVAR_REGISTER(&sk_controller_dmgzap1);
	CVAR_REGISTER(&sk_controller_dmgzap2);
	CVAR_REGISTER(&sk_controller_dmgzap3);

	CVAR_REGISTER(&sk_controller_speedball1);
	CVAR_REGISTER(&sk_controller_speedball2);
	CVAR_REGISTER(&sk_controller_speedball3);

	CVAR_REGISTER(&sk_controller_dmgball1);
	CVAR_REGISTER(&sk_controller_dmgball2);
	CVAR_REGISTER(&sk_controller_dmgball3);

	// Nihilanth
	CVAR_REGISTER(&sk_nihilanth_health1); // {"sk_nihilanth_health1","0"};
	CVAR_REGISTER(&sk_nihilanth_health2); // {"sk_nihilanth_health2","0"};
	CVAR_REGISTER(&sk_nihilanth_health3); // {"sk_nihilanth_health3","0"};

	CVAR_REGISTER(&sk_nihilanth_zap1);
	CVAR_REGISTER(&sk_nihilanth_zap2);
	CVAR_REGISTER(&sk_nihilanth_zap3);

	// Scientist
	CVAR_REGISTER(&sk_scientist_health1); // {"sk_scientist_health1","0"};
	CVAR_REGISTER(&sk_scientist_health2); // {"sk_scientist_health2","0"};
	CVAR_REGISTER(&sk_scientist_health3); // {"sk_scientist_health3","0"};


	// Snark
	CVAR_REGISTER(&sk_snark_health1); // {"sk_snark_health1","0"};
	CVAR_REGISTER(&sk_snark_health2); // {"sk_snark_health2","0"};
	CVAR_REGISTER(&sk_snark_health3); // {"sk_snark_health3","0"};

	CVAR_REGISTER(&sk_snark_dmg_bite1); // {"sk_snark_dmg_bite1","0"};
	CVAR_REGISTER(&sk_snark_dmg_bite2); // {"sk_snark_dmg_bite2","0"};
	CVAR_REGISTER(&sk_snark_dmg_bite3); // {"sk_snark_dmg_bite3","0"};

	CVAR_REGISTER(&sk_snark_dmg_pop1); // {"sk_snark_dmg_pop1","0"};
	CVAR_REGISTER(&sk_snark_dmg_pop2); // {"sk_snark_dmg_pop2","0"};
	CVAR_REGISTER(&sk_snark_dmg_pop3); // {"sk_snark_dmg_pop3","0"};



	// Zombie
	CVAR_REGISTER(&sk_zombie_health1); // {"sk_zombie_health1","0"};
	CVAR_REGISTER(&sk_zombie_health2); // {"sk_zombie_health3","0"};
	CVAR_REGISTER(&sk_zombie_health3); // {"sk_zombie_health3","0"};

	CVAR_REGISTER(&sk_zombie_dmg_one_slash1); // {"sk_zombie_dmg_one_slash1","0"};
	CVAR_REGISTER(&sk_zombie_dmg_one_slash2); // {"sk_zombie_dmg_one_slash2","0"};
	CVAR_REGISTER(&sk_zombie_dmg_one_slash3); // {"sk_zombie_dmg_one_slash3","0"};

	CVAR_REGISTER(&sk_zombie_dmg_both_slash1); // {"sk_zombie_dmg_both_slash1","0"};
	CVAR_REGISTER(&sk_zombie_dmg_both_slash2); // {"sk_zombie_dmg_both_slash2","0"};
	CVAR_REGISTER(&sk_zombie_dmg_both_slash3); // {"sk_zombie_dmg_both_slash3","0"};


	//Turret
	CVAR_REGISTER(&sk_turret_health1); // {"sk_turret_health1","0"};
	CVAR_REGISTER(&sk_turret_health2); // {"sk_turret_health2","0"};
	CVAR_REGISTER(&sk_turret_health3); // {"sk_turret_health3","0"};


	// MiniTurret
	CVAR_REGISTER(&sk_miniturret_health1); // {"sk_miniturret_health1","0"};
	CVAR_REGISTER(&sk_miniturret_health2); // {"sk_miniturret_health2","0"};
	CVAR_REGISTER(&sk_miniturret_health3); // {"sk_miniturret_health3","0"};


	// Sentry Turret
	CVAR_REGISTER(&sk_sentry_health1); // {"sk_sentry_health1","0"};
	CVAR_REGISTER(&sk_sentry_health2); // {"sk_sentry_health2","0"};
	CVAR_REGISTER(&sk_sentry_health3); // {"sk_sentry_health3","0"};


	// PLAYER WEAPONS

	// Crowbar whack
	CVAR_REGISTER(&sk_plr_katana1);
	CVAR_REGISTER(&sk_plr_katana2);
	CVAR_REGISTER(&sk_plr_katana3);

	CVAR_REGISTER(&sk_plr_crowbar1); // {"sk_plr_crowbar1","0"};
	CVAR_REGISTER(&sk_plr_crowbar2); // {"sk_plr_crowbar2","0"};
	CVAR_REGISTER(&sk_plr_crowbar3); // {"sk_plr_crowbar3","0"};

	// Glock Round
	CVAR_REGISTER(&sk_plr_9mm_bullet1); // {"sk_plr_9mm_bullet1","0"};
	CVAR_REGISTER(&sk_plr_9mm_bullet2); // {"sk_plr_9mm_bullet2","0"};
	CVAR_REGISTER(&sk_plr_9mm_bullet3); // {"sk_plr_9mm_bullet3","0"};

	// 357 Round
	CVAR_REGISTER(&sk_plr_357_bullet1); // {"sk_plr_357_bullet1","0"};
	CVAR_REGISTER(&sk_plr_357_bullet2); // {"sk_plr_357_bullet2","0"};
	CVAR_REGISTER(&sk_plr_357_bullet3); // {"sk_plr_357_bullet3","0"};

	// MP5 Round
	CVAR_REGISTER(&sk_plr_9mmAR_bullet1); // {"sk_plr_9mmAR_bullet1","0"};
	CVAR_REGISTER(&sk_plr_9mmAR_bullet2); // {"sk_plr_9mmAR_bullet2","0"};
	CVAR_REGISTER(&sk_plr_9mmAR_bullet3); // {"sk_plr_9mmAR_bullet3","0"};


	// M203 grenade
	CVAR_REGISTER(&sk_plr_9mmAR_grenade1); // {"sk_plr_9mmAR_grenade1","0"};
	CVAR_REGISTER(&sk_plr_9mmAR_grenade2); // {"sk_plr_9mmAR_grenade2","0"};
	CVAR_REGISTER(&sk_plr_9mmAR_grenade3); // {"sk_plr_9mmAR_grenade3","0"};


	// Shotgun buckshot
	CVAR_REGISTER(&sk_plr_buckshot1); // {"sk_plr_buckshot1","0"};
	CVAR_REGISTER(&sk_plr_buckshot2); // {"sk_plr_buckshot2","0"};
	CVAR_REGISTER(&sk_plr_buckshot3); // {"sk_plr_buckshot3","0"};


	// Crossbow
	CVAR_REGISTER(&sk_plr_xbow_bolt_monster1); // {"sk_plr_xbow_bolt1","0"};
	CVAR_REGISTER(&sk_plr_xbow_bolt_monster2); // {"sk_plr_xbow_bolt2","0"};
	CVAR_REGISTER(&sk_plr_xbow_bolt_monster3); // {"sk_plr_xbow_bolt3","0"};

	CVAR_REGISTER(&sk_plr_xbow_bolt_client1); // {"sk_plr_xbow_bolt1","0"};
	CVAR_REGISTER(&sk_plr_xbow_bolt_client2); // {"sk_plr_xbow_bolt2","0"};
	CVAR_REGISTER(&sk_plr_xbow_bolt_client3); // {"sk_plr_xbow_bolt3","0"};


	// RPG
	CVAR_REGISTER(&sk_plr_rpg1); // {"sk_plr_rpg1","0"};
	CVAR_REGISTER(&sk_plr_rpg2); // {"sk_plr_rpg2","0"};
	CVAR_REGISTER(&sk_plr_rpg3); // {"sk_plr_rpg3","0"};


	// Gauss Gun
	CVAR_REGISTER(&sk_plr_gauss1); // {"sk_plr_gauss1","0"};
	CVAR_REGISTER(&sk_plr_gauss2); // {"sk_plr_gauss2","0"};
	CVAR_REGISTER(&sk_plr_gauss3); // {"sk_plr_gauss3","0"};


	// Egon Gun
	CVAR_REGISTER(&sk_plr_egon_narrow1); // {"sk_plr_egon_narrow1","0"};
	CVAR_REGISTER(&sk_plr_egon_narrow2); // {"sk_plr_egon_narrow2","0"};
	CVAR_REGISTER(&sk_plr_egon_narrow3); // {"sk_plr_egon_narrow3","0"};

	CVAR_REGISTER(&sk_plr_egon_wide1); // {"sk_plr_egon_wide1","0"};
	CVAR_REGISTER(&sk_plr_egon_wide2); // {"sk_plr_egon_wide2","0"};
	CVAR_REGISTER(&sk_plr_egon_wide3); // {"sk_plr_egon_wide3","0"};


	// Hand Grendade
	CVAR_REGISTER(&sk_plr_hand_grenade1); // {"sk_plr_hand_grenade1","0"};
	CVAR_REGISTER(&sk_plr_hand_grenade2); // {"sk_plr_hand_grenade2","0"};
	CVAR_REGISTER(&sk_plr_hand_grenade3); // {"sk_plr_hand_grenade3","0"};


	// Satchel Charge
	CVAR_REGISTER(&sk_plr_satchel1); // {"sk_plr_satchel1","0"};
	CVAR_REGISTER(&sk_plr_satchel2); // {"sk_plr_satchel2","0"};
	CVAR_REGISTER(&sk_plr_satchel3); // {"sk_plr_satchel3","0"};


	// Tripmine
	CVAR_REGISTER(&sk_plr_tripmine1); // {"sk_plr_tripmine1","0"};
	CVAR_REGISTER(&sk_plr_tripmine2); // {"sk_plr_tripmine2","0"};
	CVAR_REGISTER(&sk_plr_tripmine3); // {"sk_plr_tripmine3","0"};

	// HORNET
	CVAR_REGISTER(&sk_plr_hornet_dmg1); // {"sk_plr_hornet_dmg1","0"};
	CVAR_REGISTER(&sk_plr_hornet_dmg2); // {"sk_plr_hornet_dmg2","0"};
	CVAR_REGISTER(&sk_plr_hornet_dmg3); // {"sk_plr_hornet_dmg3","0"};


	// WORLD WEAPONS
	CVAR_REGISTER(&sk_12mm_bullet1); // {"sk_12mm_bullet1","0"};
	CVAR_REGISTER(&sk_12mm_bullet2); // {"sk_12mm_bullet2","0"};
	CVAR_REGISTER(&sk_12mm_bullet3); // {"sk_12mm_bullet3","0"};

	CVAR_REGISTER(&sk_9mmAR_bullet1); // {"sk_9mm_bullet1","0"};
	CVAR_REGISTER(&sk_9mmAR_bullet2); // {"sk_9mm_bullet1","0"};
	CVAR_REGISTER(&sk_9mmAR_bullet3); // {"sk_9mm_bullet1","0"};

	CVAR_REGISTER(&sk_9mm_bullet1); // {"sk_9mm_bullet1","0"};
	CVAR_REGISTER(&sk_9mm_bullet2); // {"sk_9mm_bullet2","0"};
	CVAR_REGISTER(&sk_9mm_bullet3); // {"sk_9mm_bullet3","0"};


	// HORNET
	CVAR_REGISTER(&sk_hornet_dmg1); // {"sk_hornet_dmg1","0"};
	CVAR_REGISTER(&sk_hornet_dmg2); // {"sk_hornet_dmg2","0"};
	CVAR_REGISTER(&sk_hornet_dmg3); // {"sk_hornet_dmg3","0"};

	// HEALTH/SUIT CHARGE DISTRIBUTION
	CVAR_REGISTER(&sk_suitcharger1);
	CVAR_REGISTER(&sk_suitcharger2);
	CVAR_REGISTER(&sk_suitcharger3);

	CVAR_REGISTER(&sk_battery1);
	CVAR_REGISTER(&sk_battery2);
	CVAR_REGISTER(&sk_battery3);

	CVAR_REGISTER(&sk_healthcharger1);
	CVAR_REGISTER(&sk_healthcharger2);
	CVAR_REGISTER(&sk_healthcharger3);

	CVAR_REGISTER(&sk_healthkit1);
	CVAR_REGISTER(&sk_healthkit2);
	CVAR_REGISTER(&sk_healthkit3);

	CVAR_REGISTER(&sk_scientist_heal1);
	CVAR_REGISTER(&sk_scientist_heal2);
	CVAR_REGISTER(&sk_scientist_heal3);

	// monster damage adjusters
	CVAR_REGISTER(&sk_monster_head1);
	CVAR_REGISTER(&sk_monster_head2);
	CVAR_REGISTER(&sk_monster_head3);

	CVAR_REGISTER(&sk_monster_chest1);
	CVAR_REGISTER(&sk_monster_chest2);
	CVAR_REGISTER(&sk_monster_chest3);

	CVAR_REGISTER(&sk_monster_stomach1);
	CVAR_REGISTER(&sk_monster_stomach2);
	CVAR_REGISTER(&sk_monster_stomach3);

	CVAR_REGISTER(&sk_monster_arm1);
	CVAR_REGISTER(&sk_monster_arm2);
	CVAR_REGISTER(&sk_monster_arm3);

	CVAR_REGISTER(&sk_monster_leg1);
	CVAR_REGISTER(&sk_monster_leg2);
	CVAR_REGISTER(&sk_monster_leg3);

	// player damage adjusters
	CVAR_REGISTER(&sk_player_head1);
	CVAR_REGISTER(&sk_player_head2);
	CVAR_REGISTER(&sk_player_head3);

	CVAR_REGISTER(&sk_player_chest1);
	CVAR_REGISTER(&sk_player_chest2);
	CVAR_REGISTER(&sk_player_chest3);

	CVAR_REGISTER(&sk_player_stomach1);
	CVAR_REGISTER(&sk_player_stomach2);
	CVAR_REGISTER(&sk_player_stomach3);

	CVAR_REGISTER(&sk_player_arm1);
	CVAR_REGISTER(&sk_player_arm2);
	CVAR_REGISTER(&sk_player_arm3);

	CVAR_REGISTER(&sk_player_leg1);
	CVAR_REGISTER(&sk_player_leg2);
	CVAR_REGISTER(&sk_player_leg3);
	// END REGISTER CVARS FOR SKILL LEVEL STUFF

	CVAR_REGISTER(&sv_pushable_fixed_tick_fudge);

	InitMapLoadingUtils();

	SERVER_COMMAND("exec skill.cfg\n");

	// Register all user messages here, during DLL init, which is the guaranteed
	// safe window for REG_USER_MSG in GoldSrc. LinkUserMessages guards against
	// double-registration so calling it again from ServerActivate is harmless.
	LinkUserMessages();
}

void GameDLLShutdown()
{
	FileSystem_FreeFileSystem();
}
