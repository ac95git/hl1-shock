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

// Skill Tree economy.  Both default to zero: every Skill Point and every
// Reset Token is found in the world.  Deliberately uncapped -- the ceiling on
// each is how many pickups a map places, and a cap would let a found pickup
// silently do nothing.  Raise skill_points_start to work on the tree UI
// without hunting for pickups first.  See docs/PILLARS.md pillar 4.
cvar_t skill_points_start = {"skill_points_start", "0"};
cvar_t skill_reset_tokens_start = {"skill_reset_tokens_start", "0"};

// Skill effects.  Every one is a starting guess to be judged in play, and each
// is read server-side where the effect is computed -- never in prediction.
cvar_t skill_health_bonus = {"skill_health_bonus", "25"};
// Multiplies ARMOR_RATIO, the fraction of a blow that gets PAST armor. Lower
// is better armor: 0.9 lets a tenth less through.
cvar_t skill_armor_ratio_scale = {"skill_armor_ratio_scale", "0.9"};
cvar_t skill_fall_damage_scale = {"skill_fall_damage_scale", "0.5"};
// Passive regeneration, in points per second. Deliberately far slower than an
// Infusion (4/s): this one never stops, so it is meant to erode chip damage
// between fights rather than answer a fight.
cvar_t skill_health_regen_rate = {"skill_health_regen_rate", "0.5"};
cvar_t skill_battery_regen_rate = {"skill_battery_regen_rate", "0.5"};
cvar_t skill_battery_bonus = {"skill_battery_bonus", "50"};
cvar_t skill_crowbar_range_scale = {"skill_crowbar_range_scale", "1.25"};
cvar_t skill_crowbar_damage_scale = {"skill_crowbar_damage_scale", "1.5"};
cvar_t skill_weapon_damage_scale = {"skill_weapon_damage_scale", "1.1"};
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

// The Infusion -- see docs/PILLARS.md pillar 3.  40 HP over 10 seconds is more
// than two medkits, and the duration is what pays for it: none of it lands if
// the player does not survive the ten seconds, and it cannot answer burst
// damage the way an instant medkit can.
cvar_t infusion_rate = {"infusion_rate", "4"};
cvar_t infusion_duration = {"infusion_duration", "10"};
// Med Expert. Additive rather than a percentage so it stays legible when
// infusion_duration is tuned -- the same choice pulse_window_bonus makes.
cvar_t infusion_duration_bonus = {"infusion_duration_bonus", "5"};

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

	CVAR_REGISTER(&skill_points_start);
	CVAR_REGISTER(&skill_reset_tokens_start);

	CVAR_REGISTER(&skill_health_bonus);
	CVAR_REGISTER(&skill_armor_ratio_scale);
	CVAR_REGISTER(&skill_fall_damage_scale);
	CVAR_REGISTER(&skill_health_regen_rate);
	CVAR_REGISTER(&skill_battery_regen_rate);
	CVAR_REGISTER(&skill_battery_bonus);
	CVAR_REGISTER(&skill_crowbar_range_scale);
	CVAR_REGISTER(&skill_crowbar_damage_scale);
	CVAR_REGISTER(&skill_reload_time_scale);
	CVAR_REGISTER(&skill_weapon_damage_scale);

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
