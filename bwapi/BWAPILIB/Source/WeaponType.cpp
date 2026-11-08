#include <string>
#include <BWAPI/UnitType.h>
#include <BWAPI/WeaponType.h>
#include <BWAPI/TechType.h>
#include <BWAPI/UpgradeType.h>
#include <BWAPI/DamageType.h>
#include <BWAPI/ExplosionType.h>

#include <Debug.h>
#include "TypeData.h"

namespace BWAPI
{
  // NAMES
#ifdef SCM_SIM
  // scm-sim: BWAPI 4.4's names, by its numbering (TypeData.h); the types the enum has keep them.
  static const std::string typeNames44[TypeData::WEAPON44] =
#else
  template <>
  const std::string Type<WeaponType, WeaponTypes::Enum::Unknown>::typeNames[TypeData::WEAPON44] =
#endif
  {
    "Gauss_Rifle",
    "Gauss_Rifle_Jim_Raynor",
    "C_10_Canister_Rifle",
    "C_10_Canister_Rifle_Sarah_Kerrigan",
    "Fragmentation_Grenade",
    "Fragmentation_Grenade_Jim_Raynor",
    "Spider_Mines",
    "Twin_Autocannons",
    "Hellfire_Missile_Pack",
    "Twin_Autocannons_Alan_Schezar",
    "Hellfire_Missile_Pack_Alan_Schezar",
    "Arclite_Cannon",
    "Arclite_Cannon_Edmund_Duke",
    "Fusion_Cutter",
    "",
    "Gemini_Missiles",
    "Burst_Lasers",
    "Gemini_Missiles_Tom_Kazansky",
    "Burst_Lasers_Tom_Kazansky",
    "ATS_Laser_Battery",
    "ATA_Laser_Battery",
    "ATS_Laser_Battery_Hero",
    "ATA_Laser_Battery_Hero",
    "ATS_Laser_Battery_Hyperion",
    "ATA_Laser_Battery_Hyperion",
    "Flame_Thrower",
    "Flame_Thrower_Gui_Montag",
    "Arclite_Shock_Cannon",
    "Arclite_Shock_Cannon_Edmund_Duke",
    "Longbolt_Missile",
    "Yamato_Gun",
    "Nuclear_Strike",
    "Lockdown",
    "EMP_Shockwave",
    "Irradiate",
    "Claws",
    "Claws_Devouring_One",
    "Claws_Infested_Kerrigan",
    "Needle_Spines",
    "Needle_Spines_Hunter_Killer",
    "Kaiser_Blades",
    "Kaiser_Blades_Torrasque",
    "Toxic_Spores",
    "Spines",
    "",
    "",
    "Acid_Spore",
    "Acid_Spore_Kukulza",
    "Glave_Wurm",
    "Glave_Wurm_Kukulza",
    "",
    "",
    "Seeker_Spores",
    "Subterranean_Tentacle",
    "Suicide_Infested_Terran",
    "Suicide_Scourge",
    "Parasite",
    "Spawn_Broodlings",
    "Ensnare",
    "Dark_Swarm",
    "Plague",
    "Consume",
    "Particle_Beam",
    "",
    "Psi_Blades",
    "Psi_Blades_Fenix",
    "Phase_Disruptor",
    "Phase_Disruptor_Fenix",
    "",
    "Psi_Assault",
    "Psionic_Shockwave",
    "Psionic_Shockwave_TZ_Archon",
    "",
    "Dual_Photon_Blasters",
    "Anti_Matter_Missiles",
    "Dual_Photon_Blasters_Mojo",
    "Anti_Matter_Missiles_Mojo",
    "Phase_Disruptor_Cannon",
    "Phase_Disruptor_Cannon_Danimoth",
    "Pulse_Cannon",
    "STS_Photon_Cannon",
    "STA_Photon_Cannon",
    "Scarab",
    "Stasis_Field",
    "Psionic_Storm",
    "Warp_Blades_Zeratul",
    "Warp_Blades_Hero",
    "",
    "",
    "",
    "",
    "",
    "Platform_Laser_Battery",
    "Independant_Laser_Battery",
    "",
    "",
    "Twin_Autocannons_Floor_Trap",
    "Hellfire_Missile_Pack_Wall_Trap",
    "Flame_Thrower_Wall_Trap",
    "Hellfire_Missile_Pack_Floor_Trap",
    "Neutron_Flare",
    "Disruption_Web",
    "Restoration",
    "Halo_Rockets",
    "Corrosive_Acid",
    "Mind_Control",
    "Feedback",
    "Optical_Flare",
    "Maelstrom",
    "Subterranean_Spines",
    "",
    "Warp_Blades",
    "C_10_Canister_Rifle_Samir_Duran",
    "C_10_Canister_Rifle_Infested_Duran",
    "Dual_Photon_Blasters_Artanis",
    "Anti_Matter_Missiles_Artanis",
    "C_10_Canister_Rifle_Alexei_Stukov",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "None",
    "Unknown"
  };

  // DEFAULTS
  const int defaultWpnDamageAmt[TypeData::WEAPON44] =
  {
    6, 18, 10, 30, 20, 30, 125, 12, 10, 24, 20, 30, 70, 5, 0, 20, 8, 40, 16, 25, 25, 50, 50, 30, 30, 8, 16, 70,
    150, 20, 260, 600, 0, 0, 250, 5, 10, 50, 10, 20, 20, 50, 4, 5, 0, 30, 20, 40, 9, 18, 5, 10, 15, 40, 500, 110,
    0, 0, 0, 0, 300, 0, 5, 0, 8, 20, 20, 45, 5, 20, 30, 60, 4, 8, 14, 20, 28, 10, 20, 6, 20, 20, 100, 0, 14, 100,
    45, 7, 7, 7, 7, 7, 7, 7, 4, 30, 10, 10, 8, 10, 5, 0, 20, 6, 25, 8, 8, 8, 0, 20, 6, 40, 25, 25, 20, 28, 30, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0
  };
  const int defaultWpnDamageBonus[TypeData::WEAPON44] =
  {
    1, 1, 1, 1, 2, 2, 0, 1, 2, 1, 1, 3, 3, 1, 0, 2, 1, 2, 1, 3, 3, 3, 3, 3, 3, 1, 1, 5, 5, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 3, 3, 1, 0, 0, 1, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1,
    3, 3, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 25, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 2,
    1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
  };
  // ACTUAL
  int wpnDamageAmt[TypeData::WEAPON44], wpnDamageBonus[TypeData::WEAPON44];


  const int wpnDamageCooldowns[TypeData::WEAPON44] =
  {
    15, 15, 22, 22, 30, 22, 22, 22, 22, 22, 22, 37, 37, 15, 15, 22, 30, 22, 30, 30, 30, 30, 30, 22, 22, 22, 22,
    75, 75, 15, 15, 1, 1, 1, 75, 8, 8, 15, 15, 15, 15, 15, 15, 22, 22, 22, 30, 30, 30, 30, 22, 22, 15, 32, 1, 1,
    1, 1, 1, 1, 1, 1, 22, 22, 22, 22, 30, 22, 30, 22, 20, 20, 22, 30, 22, 30, 22, 45, 45, 1, 22, 22, 1, 1, 45, 22,
    30, 22, 22, 22, 22, 22, 22, 22, 9, 22, 22, 22, 22, 22, 8, 22, 22, 64, 100, 22, 22, 22, 1, 37, 15, 30, 22, 22,
    30, 22, 22, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 0, 0
  };
  const int wpnDamageFactor[TypeData::WEAPON44] =
  {
    1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
  };

  const int wpnMinRange[TypeData::WEAPON44] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  const int wpnMaxRange[TypeData::WEAPON44] =
  {
    128, 160, 224, 192, 160, 160, 10, 192, 160, 160, 160, 224, 224, 10, 10, 160, 160, 160, 160, 192, 192, 192, 192,
    192, 192, 32, 32, 384, 384, 224, 320, 3, 256, 256, 288, 15, 15, 15, 128, 160, 25, 25, 2, 32, 128, 64, 256, 256,
    96, 96, 128, 128, 224, 224, 3, 3, 384, 288, 288, 288, 288, 16, 32, 10, 15, 15, 128, 128, 96, 96, 64, 64, 32,
    128, 128, 128, 128, 160, 160, 128, 224, 224, 128, 288, 288, 15, 15, 128, 128, 32, 128, 224, 224, 128, 160, 192,
    160, 160, 64, 160, 160, 288, 192, 192, 192, 256, 320, 288, 320, 192, 128, 15, 192, 192, 128, 128, 192, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 0, 0
  };
  const int wpnSplashRangeInner[TypeData::WEAPON44] =
  {
    0, 0, 0, 0, 0, 0, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 10, 10, 0, 0, 128, 0, 64, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 5, 0, 0, 5, 0, 0, 0, 0, 0, 
    20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  const int wpnSplashRangeMid[TypeData::WEAPON44] =
  {
    0, 0, 0, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 25, 25, 0, 0, 192, 0, 64, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 50, 0, 0, 50, 0, 0, 0, 0, 0, 20, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  const int wpnSplashRangeOuter[TypeData::WEAPON44] =
  {
    0, 0, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 25, 40, 40, 0, 0, 256, 0, 64, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 30, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 30, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 60, 0, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 0, 100, 0, 0, 100, 0, 0, 0, 0, 0, 20, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

#define TARG_AIR      0x01
#define TARG_GROUND   0x02
#define TARG_MECH     0x04
#define TARG_ORGANIC  0x08
#define TARG_NOBUILD  0x10
#define TARG_NOROBOT  0x20
#define TARG_TERRAIN  0x40
#define TARG_ORGMECH  0x80
#define TARG_OWN      0x100

  const int wpnFlags[TypeData::WEAPON44] =
  {
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND | TARG_NOBUILD,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND | TARG_MECH | TARG_NOBUILD,
    TARG_AIR | TARG_GROUND | TARG_TERRAIN,
    TARG_AIR | TARG_GROUND | TARG_TERRAIN,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR,
    TARG_AIR | TARG_GROUND | TARG_NOBUILD,
    TARG_GROUND | TARG_NOBUILD | TARG_NOROBOT | TARG_ORGMECH,
    TARG_AIR | TARG_GROUND | TARG_TERRAIN,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND | TARG_TERRAIN,
    TARG_AIR | TARG_GROUND | TARG_ORGANIC | TARG_NOBUILD | TARG_OWN,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_AIR,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND | TARG_TERRAIN,
    TARG_AIR | TARG_GROUND | TARG_NOBUILD | TARG_TERRAIN,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR,
    TARG_AIR,
    TARG_AIR,
    TARG_GROUND,
    TARG_AIR,
    TARG_AIR,
    TARG_AIR,
    TARG_AIR,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_GROUND,
    TARG_AIR,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND | TARG_TERRAIN,
    TARG_AIR,
    TARG_AIR,
    TARG_AIR | TARG_GROUND | TARG_TERRAIN,
    TARG_AIR | TARG_GROUND | TARG_TERRAIN,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND | TARG_TERRAIN,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_GROUND,
    TARG_AIR,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    TARG_AIR | TARG_GROUND,
    0, 0
  };


  namespace wpnInternalUpgrades
  {
    using namespace UpgradeTypes::Enum;
    const UpgradeType upgrade[TypeData::WEAPON44] =
    {
      Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Vehicle_Weapons, 
      Terran_Vehicle_Weapons, Upgrade_60, Terran_Vehicle_Weapons, Terran_Vehicle_Weapons, Terran_Vehicle_Weapons, Terran_Vehicle_Weapons, 
      Terran_Vehicle_Weapons, Terran_Vehicle_Weapons, Upgrade_60, Upgrade_60, Terran_Ship_Weapons, Terran_Ship_Weapons, Terran_Ship_Weapons, 
      Terran_Ship_Weapons, Terran_Ship_Weapons, Terran_Ship_Weapons, Terran_Ship_Weapons, Terran_Ship_Weapons, Terran_Ship_Weapons, 
      Terran_Ship_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Vehicle_Weapons, Terran_Vehicle_Weapons, Upgrade_60, 
      Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Zerg_Melee_Attacks, Zerg_Melee_Attacks, Zerg_Melee_Attacks, 
      Zerg_Missile_Attacks, Zerg_Missile_Attacks, Zerg_Melee_Attacks, Zerg_Melee_Attacks, Zerg_Melee_Attacks, Upgrade_60, Upgrade_60, 
      Zerg_Flyer_Attacks, Zerg_Flyer_Attacks, Zerg_Flyer_Attacks, Zerg_Flyer_Attacks, Zerg_Flyer_Attacks, Zerg_Missile_Attacks, 
      Zerg_Missile_Attacks, Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, 
      Upgrade_60, Upgrade_60, Upgrade_60, Protoss_Ground_Weapons, Protoss_Ground_Weapons, Protoss_Ground_Weapons, Protoss_Ground_Weapons, 
      Protoss_Ground_Weapons, Protoss_Ground_Weapons, Protoss_Ground_Weapons, Protoss_Ground_Weapons, Protoss_Air_Weapons, Protoss_Air_Weapons, 
      Protoss_Air_Weapons, Protoss_Air_Weapons, Protoss_Air_Weapons, Protoss_Air_Weapons, Protoss_Air_Weapons, Protoss_Air_Weapons, Upgrade_60, 
      Upgrade_60, Scarab_Damage, Upgrade_60, Upgrade_60, Protoss_Ground_Weapons, Protoss_Ground_Weapons, Upgrade_60, Upgrade_60, Upgrade_60, 
      Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Terran_Vehicle_Weapons, Terran_Vehicle_Weapons, 
      Terran_Infantry_Weapons, Terran_Vehicle_Weapons, Protoss_Air_Weapons, Upgrade_60, Upgrade_60, Terran_Ship_Weapons, Zerg_Flyer_Attacks, 
      Upgrade_60, Upgrade_60, Upgrade_60, Upgrade_60, Zerg_Missile_Attacks, Terran_Infantry_Weapons, Protoss_Ground_Weapons, 
      Terran_Infantry_Weapons, Terran_Infantry_Weapons, Protoss_Air_Weapons, Protoss_Air_Weapons, Terran_Infantry_Weapons, 
      Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, 
      Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, 
      Terran_Infantry_Weapons, Terran_Infantry_Weapons, Terran_Infantry_Weapons, None, Unknown
    };
  }
  namespace wpnInternalDamageType
  {
    using namespace DamageTypes::Enum;
    const DamageType damageType[TypeData::WEAPON44] =
    {
      Normal, Normal, Concussive, Concussive, Concussive, Concussive, Explosive, Normal, Explosive, Normal, Explosive, Explosive,
      Explosive, Normal, Normal, Explosive, Normal, Explosive, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Concussive,
      Concussive, Explosive, Explosive, Explosive, Explosive, Explosive, Concussive, Concussive, Ignore_Armor, Normal, Normal, Normal, 
      Explosive, Explosive, Normal, Normal, Normal, Normal, Normal, Explosive, Normal, Normal, Normal, Normal, Concussive, Concussive, 
      Normal, Explosive, Explosive, Normal, Independent, Independent, Independent, Independent, Independent, Independent, Normal, Normal, 
      Normal, Normal, Explosive, Explosive, Normal, Normal, Normal, Normal, Explosive, Normal, Explosive, Normal, Explosive, Explosive, 
      Explosive, Normal, Normal, Normal, Normal, Independent, Ignore_Armor, Normal, Normal, Explosive, Normal, Explosive, Explosive,
      Normal, Normal, Normal, Normal, Normal, Normal, Explosive, Concussive, Explosive, Explosive, Ignore_Armor, Ignore_Armor, Explosive, 
      Explosive, Normal, Ignore_Armor, Independent, Independent, Normal, Normal, Normal, Concussive, Concussive, Normal, Explosive, 
      Concussive, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, None, Unknown
    };
  }
  namespace wpnInternalExplosionType
  {
    using namespace ExplosionTypes::Enum;
    const ExplosionType explosionType[TypeData::WEAPON44] =
    {
      Normal, Normal, Normal, Normal, Normal, Normal, Radial_Splash, Normal, Normal, Normal, Normal, Normal, Normal, Normal, None, 
      Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Enemy_Splash, Enemy_Splash, Radial_Splash, 
      Radial_Splash, Normal, Yamato_Gun, Nuclear_Missile, Lockdown, EMP_Shockwave, Irradiate, Normal, Normal, Normal, Normal, Normal, 
      Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Radial_Splash, Radial_Splash, Normal, Normal, 
      Radial_Splash, Normal, Parasite, Broodlings, Ensnare, Dark_Swarm, Plague, Consume, Normal, None, Normal, Normal, Normal, Normal, 
      Normal, Normal, Enemy_Splash, Enemy_Splash, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, 
      Enemy_Splash, Stasis_Field, Radial_Splash, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, 
      Normal, Normal, Normal, Enemy_Splash, Normal, Air_Splash, Disruption_Web, Restoration, Air_Splash, Corrosive_Acid, Mind_Control, 
      Feedback, Optical_Flare, Maelstrom, Enemy_Splash, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, 
      Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, Normal, None, Unknown
    };
  }
  namespace wpnInternalWhatUses
  {
    using namespace UnitTypes::Enum;
    const UnitType whatUses[TypeData::WEAPON44] =
    {
      Terran_Marine, Hero_Jim_Raynor_Marine, Terran_Ghost, Hero_Sarah_Kerrigan, Terran_Vulture, Hero_Jim_Raynor_Vulture,
      Terran_Vulture_Spider_Mine, Terran_Goliath, Terran_Goliath, Hero_Alan_Schezar, Hero_Alan_Schezar, Terran_Siege_Tank_Tank_Mode,
      Hero_Edmund_Duke_Tank_Mode, Terran_SCV, Terran_SCV, Terran_Wraith, Terran_Wraith, Hero_Tom_Kazansky, Hero_Tom_Kazansky, 
      Terran_Battlecruiser, Terran_Battlecruiser, Hero_Norad_II, Hero_Norad_II, Hero_Hyperion, Hero_Hyperion, Terran_Firebat,
      Hero_Gui_Montag, Terran_Siege_Tank_Siege_Mode, Hero_Edmund_Duke_Siege_Mode, Terran_Missile_Turret, Terran_Battlecruiser, 
      Terran_Ghost, Terran_Ghost, Terran_Science_Vessel, Terran_Science_Vessel, Zerg_Zergling, Hero_Devouring_One, Hero_Infested_Kerrigan,
      Zerg_Hydralisk, Hero_Hunter_Killer, Zerg_Ultralisk, Hero_Torrasque, Zerg_Broodling, Zerg_Drone, Zerg_Drone, None, Zerg_Guardian,
      Hero_Kukulza_Guardian, Zerg_Mutalisk, Hero_Kukulza_Mutalisk, None, None, Zerg_Spore_Colony, Zerg_Sunken_Colony, Zerg_Infested_Terran,
      Zerg_Scourge, Zerg_Queen, Zerg_Queen, Zerg_Queen, Zerg_Defiler, Zerg_Defiler, Zerg_Defiler, Protoss_Probe, Protoss_Probe,
      Protoss_Zealot, Hero_Fenix_Zealot, Protoss_Dragoon, Hero_Fenix_Dragoon, None, Hero_Tassadar, Protoss_Archon, Hero_Tassadar_Zeratul_Archon,
      None, Protoss_Scout, Protoss_Scout, Hero_Mojo, Hero_Mojo, Protoss_Arbiter, Hero_Danimoth, Protoss_Interceptor, Protoss_Photon_Cannon, 
      Protoss_Photon_Cannon, Protoss_Scarab, Protoss_Arbiter, Protoss_High_Templar, Hero_Zeratul, Hero_Dark_Templar, None, None, None, None,
      None, None, Special_Independant_Starport, None, None, Special_Floor_Gun_Trap, Special_Wall_Missile_Trap, Special_Wall_Flame_Trap,
      Special_Floor_Missile_Trap, Protoss_Corsair, Protoss_Corsair, Terran_Medic, Terran_Valkyrie, Zerg_Devourer, Protoss_Dark_Archon, 
      Protoss_Dark_Archon, Terran_Medic, Protoss_Dark_Archon, Zerg_Lurker, None, Protoss_Dark_Templar, Hero_Samir_Duran, Hero_Infested_Duran,
      Hero_Artanis, Hero_Artanis, Hero_Alexei_Stukov, None, None, None, None, None, None, None, None, None, None, None, None, None,
      None, Unknown
    };
  };

  namespace WeaponTypesSet
  {
    using namespace WeaponTypes::Enum;
    const WeaponType::set specialWeaponTypeSet = { Yamato_Gun, Nuclear_Strike, Lockdown, EMP_Shockwave,
      Irradiate, Parasite, Spawn_Broodlings, Ensnare, Dark_Swarm,
      Plague, Consume, Stasis_Field, Psionic_Storm, Disruption_Web,
      Restoration, Mind_Control, Feedback, Optical_Flare, Maelstrom };
    const WeaponType::set normalWeaponTypeSet = { Gauss_Rifle, Gauss_Rifle_Jim_Raynor, C_10_Canister_Rifle,
      C_10_Canister_Rifle_Sarah_Kerrigan, C_10_Canister_Rifle_Samir_Duran,
      C_10_Canister_Rifle_Infested_Duran, C_10_Canister_Rifle_Alexei_Stukov,
      Fragmentation_Grenade, Fragmentation_Grenade_Jim_Raynor, Spider_Mines,
      Twin_Autocannons, Twin_Autocannons_Alan_Schezar, Hellfire_Missile_Pack,
      Hellfire_Missile_Pack_Alan_Schezar, Arclite_Cannon, Arclite_Cannon_Edmund_Duke,
      Fusion_Cutter, Gemini_Missiles, Gemini_Missiles_Tom_Kazansky, Burst_Lasers,
      Burst_Lasers_Tom_Kazansky, ATS_Laser_Battery, ATS_Laser_Battery_Hero,
      ATS_Laser_Battery_Hyperion, ATA_Laser_Battery, ATA_Laser_Battery_Hero,
      ATA_Laser_Battery_Hyperion, Flame_Thrower, Flame_Thrower_Gui_Montag,
      Arclite_Shock_Cannon, Arclite_Shock_Cannon_Edmund_Duke, Longbolt_Missile,
      Claws, Claws_Devouring_One, Claws_Infested_Kerrigan, Needle_Spines,
      Needle_Spines_Hunter_Killer, Kaiser_Blades, Kaiser_Blades_Torrasque,
      Toxic_Spores, Spines, Acid_Spore, Acid_Spore_Kukulza, Glave_Wurm,
      Glave_Wurm_Kukulza, Seeker_Spores, Subterranean_Tentacle,
      Suicide_Infested_Terran, Suicide_Scourge, Particle_Beam, Psi_Blades,
      Psi_Blades_Fenix, Phase_Disruptor, Phase_Disruptor_Fenix, Psi_Assault,
      Psionic_Shockwave, Psionic_Shockwave_TZ_Archon, Dual_Photon_Blasters,
      Dual_Photon_Blasters_Mojo, Dual_Photon_Blasters_Artanis, Anti_Matter_Missiles,
      Anti_Matter_Missiles_Mojo, Anti_Matter_Missiles_Artanis, Phase_Disruptor_Cannon,
      Phase_Disruptor_Cannon_Danimoth, Pulse_Cannon, STS_Photon_Cannon,
      STA_Photon_Cannon, Scarab, Neutron_Flare, Halo_Rockets, Corrosive_Acid,
      Subterranean_Spines, Warp_Blades, Warp_Blades_Hero, Warp_Blades_Zeratul,
      Independant_Laser_Battery, Twin_Autocannons_Floor_Trap, Hellfire_Missile_Pack_Wall_Trap,
      Hellfire_Missile_Pack_Floor_Trap, Flame_Thrower_Wall_Trap };
    
    const WeaponType::set initWeaponTypeSet() {
      WeaponType::set result = { WeaponTypes::None, WeaponTypes::Unknown };
      result.insert(normalWeaponTypeSet.begin(), normalWeaponTypeSet.end());
      result.insert(specialWeaponTypeSet.begin(), specialWeaponTypeSet.end());
      return result;
    }
    const WeaponType::set weaponTypeSet = initWeaponTypeSet();
  }

  namespace wpnInternalTechs
  {
      using namespace TechTypes::Enum;
      const TechType attachedTech[TypeData::WEAPON44] = 
      {
        // Terran
        None, None, None, None, None, None, Spider_Mines, None, None, None, None, None, None, None, None, None, None, None, None,
        None, None, None, None, None, None, None, None, None, None, None, Yamato_Gun, Nuclear_Strike, Lockdown, EMP_Shockwave, Irradiate,
        // Zerg
        None, None, None, None, None, None, None, None, None, None, None, None, None, None,  None, None, None, None, None, None,
        None, Parasite, Spawn_Broodlings, Ensnare, Dark_Swarm, Plague, Consume,
        // Protoss
        None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, 
        None, Stasis_Field, Psionic_Storm, None, None, 
        // Other
        None, None, None, None, None, None, None, None, None, None, None, None, None, 
        // Expansion
        None, Disruption_Web, Restoration, None, None, Mind_Control, Feedback, Optical_Flare, Maelstrom, None, None, None, None, 
        None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None
      };
  }

  // scm-sim: each answer is the game's data once a game's tables were read (TypeData.h), else BWAPI 4.4's own table,
  // whose row for an id is TypeData::weaponRow44 (a mod's weapon past 1.16.1's reads Unknown's).
  using TypeData::weaponRow44;
#define SCM_WEAPON(field, table) do { if ( auto r = TypeData::weapon(this->getID()) ) return r->field; return table[weaponRow44(this->getID())]; } while (0)
  static int weaponFlagsOf(int id)
  {
    if ( auto r = TypeData::weapon(id) )
      return r->flags;
    return wpnFlags[weaponRow44(id)];
  }

  TechType WeaponType::getTech() const
  {
    return wpnInternalTechs::attachedTech[weaponRow44(this->getID())];
  }
  UnitType WeaponType::whatUses() const
  {
    SCM_WEAPON(whatUses, wpnInternalWhatUses::whatUses);
  }
  int WeaponType::damageAmount() const
  {
    SCM_WEAPON(damage, defaultWpnDamageAmt);
  }
  int WeaponType::damageBonus() const
  {
    SCM_WEAPON(bonus, defaultWpnDamageBonus);
  }
  int WeaponType::damageCooldown() const
  {
    SCM_WEAPON(cooldown, wpnDamageCooldowns);
  }
  int WeaponType::damageFactor() const
  {
    SCM_WEAPON(factor, wpnDamageFactor);
  }
  UpgradeType WeaponType::upgradeType() const
  {
    if ( auto r = TypeData::weapon(this->getID()) )
      return UpgradeType(r->upgrade);
    return wpnInternalUpgrades::upgrade[weaponRow44(this->getID())];
  }
  DamageType WeaponType::damageType() const
  {
    if ( auto r = TypeData::weapon(this->getID()) )
      return DamageType(r->damageType);
    return wpnInternalDamageType::damageType[weaponRow44(this->getID())];
  }
  ExplosionType WeaponType::explosionType() const
  {
    if ( auto r = TypeData::weapon(this->getID()) )
      return ExplosionType(r->explosion);
    return wpnInternalExplosionType::explosionType[weaponRow44(this->getID())];
  }
  int WeaponType::minRange() const
  {
    SCM_WEAPON(minRange, wpnMinRange);
  }
  int WeaponType::maxRange() const
  {
    SCM_WEAPON(maxRange, wpnMaxRange);
  }
  int WeaponType::innerSplashRadius() const
  {
    SCM_WEAPON(inner, wpnSplashRangeInner);
  }
  int WeaponType::medianSplashRadius() const
  {
    SCM_WEAPON(median, wpnSplashRangeMid);
  }
  int WeaponType::outerSplashRadius() const
  {
    SCM_WEAPON(outer, wpnSplashRangeOuter);
  }
  bool WeaponType::targetsAir() const
  {
    return !!(weaponFlagsOf(this->getID()) & TARG_AIR);
  }
  bool WeaponType::targetsGround() const
  {
    return !!(weaponFlagsOf(this->getID()) & TARG_GROUND);
  }
  bool WeaponType::targetsMechanical() const
  {
    return !!(weaponFlagsOf(this->getID()) & TARG_MECH);
  }
  bool WeaponType::targetsOrganic() const
  {
    return !!(weaponFlagsOf(this->getID()) & TARG_ORGANIC);
  }
  bool WeaponType::targetsNonBuilding() const
  {
    return !!(weaponFlagsOf(this->getID()) & TARG_NOBUILD);
  }
  bool WeaponType::targetsNonRobotic() const
  {
    return !!(weaponFlagsOf(this->getID()) & TARG_NOROBOT);
  }
  bool WeaponType::targetsTerrain() const
  {
    return !!(weaponFlagsOf(this->getID()) & TARG_TERRAIN);
  }
  bool WeaponType::targetsOrgOrMech() const
  {
    return !!(weaponFlagsOf(this->getID()) & TARG_ORGMECH);
  }
  bool WeaponType::targetsOwn() const
  {
    return !!(weaponFlagsOf(this->getID()) & TARG_OWN);
  }
  const WeaponType::set& WeaponTypes::allWeaponTypes()
  {
    if ( TypeData::use )
      return TypeData::game.allWeapons;
    return WeaponTypesSet::weaponTypeSet;
  }
  const WeaponType::set& WeaponTypes::normalWeaponTypes()
  {
    if ( TypeData::use )
      return TypeData::game.normalWeapons;
    return WeaponTypesSet::normalWeaponTypeSet;
  }
  const WeaponType::set& WeaponTypes::specialWeaponTypes()
  {
    if ( TypeData::use )
      return TypeData::game.specialWeapons;
    return WeaponTypesSet::specialWeaponTypeSet;
  }
#undef SCM_WEAPON

  // scm-sim: BWAPI 4.4's answers that the game's rows start from (TypeData.cpp), and the names.
  UnitType TypeData::bwapiWeaponWhatUses(int id44) { return wpnInternalWhatUses::whatUses[id44]; }

  template<> const std::string &Type<WeaponType, WeaponTypes::Enum::Unknown>::nameOf(int id)
  {
    int i = TypeData::weapon44(id);
    if ( i >= 0 ) return typeNames44[i];
    if ( auto r = TypeData::weapon(id) ) return r->name;
    return typeNames44[TypeData::WEAPON44 - 1];
  }
  template<> std::deque<int> Type<WeaponType, WeaponTypes::Enum::Unknown>::namedIds()
  {
    std::deque<int> ids;
    for ( int i = 0; i <= WeaponTypes::Enum::None; ++i ) ids.push_back(i);
    ids.push_back(WeaponTypes::Enum::Unknown);
    if ( TypeData::use )
      for ( int i = WeaponTypes::Enum::None + 1; i < (int)TypeData::game.weapons.size(); ++i )
        if ( !TypeData::game.weapons[i].name.empty() ) ids.push_back(i);
    return ids;
  }
  template<> int Type<WeaponType, WeaponTypes::Enum::Unknown>::nameGeneration()
  {
    return TypeData::game.generation * 2 + (TypeData::use ? 1 : 0);
  }
}
