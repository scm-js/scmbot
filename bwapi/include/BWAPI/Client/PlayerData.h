#pragma once

#include <BWAPI/UnitType.h>
#include <BWAPI/UpgradeType.h>
#include <BWAPI/TechType.h>

namespace BWAPI
{
  // scm-sim: GameData, this struct among it, is BWAPI 4.4's client protocol: its per-type arrays keep 4.4's widths, and
  // a type is filed under the number BWAPI 4.4 gave it. The types come from the game's data now and BWAPI's own
  // sentinels moved above any table's width (UnitType.h), so these say where each goes: the same id for the game's own
  // types and its "none" (unit 228, weapon 130, upgrade 61, tech 44), 4.4's numbers for the sentinels (AllUnits 229 …
  // Unknown 233, Nuclear_Strike 45 …), and -1 for a type 4.4 had no place for (a mod's past 1.16.1's). Such a type is
  // counted in BWAPI's own objects at the data set's width (PlayerImpl's `scmsimWide`, C2); GameData has no place for
  // its counts, and a field naming it carries its id when that is past 4.4's numbers and 4.4's Unknown when it would
  // read as one of 4.4's sentinels (a mod's unit 229 to 233, upgrade 62, tech 45 and 46): docs/bwapi-plan.md, C2.
  namespace Wire
  {
    const int UNITS = 234, UPGRADES = 63, TECHS = 47, WEAPONS = 132;
    inline int unitType(int id)
    {
      if ( id >= 0 && id <= UnitTypes::Enum::None ) return id;
      if ( id >= UnitTypes::Enum::AllUnits && id <= UnitTypes::Enum::Unknown ) return id - UnitTypes::Enum::AllUnits + UnitTypes::Enum::None + 1;
      return -1;
    }
    inline int upgradeType(int id) { return id >= 0 && id <= UpgradeTypes::Enum::None ? id : id == UpgradeTypes::Enum::Unknown ? UPGRADES - 1 : -1; }
    inline int techType(int id)
    {
      if ( id >= 0 && id <= TechTypes::Enum::None ) return id;
      return id == TechTypes::Enum::Nuclear_Strike ? TECHS - 2 : id == TechTypes::Enum::Unknown ? TECHS - 1 : -1;
    }
    /// A type id as GameData carries it in a field: 4.4's number, or the id itself where 4.4 had none and it is past
    /// 4.4's numbers, else 4.4's Unknown (never one of 4.4's sentinels).
    inline int unitValue(int id) { int w = unitType(id); return w >= 0 ? w : id >= UNITS || id < 0 ? id : UNITS - 1; }
    inline int upgradeValue(int id) { int w = upgradeType(id); return w >= 0 ? w : id >= UPGRADES || id < 0 ? id : UPGRADES - 1; }
    inline int techValue(int id) { int w = techType(id); return w >= 0 ? w : id >= TECHS || id < 0 ? id : TECHS - 1; }
  }
#define SCM_UNIT_TYPES Wire::UNITS
#define SCM_UPGRADE_TYPES Wire::UPGRADES
#define SCM_TECH_TYPES Wire::TECHS

  struct PlayerData
  {
    char name[25];
    int  race;
    int  type;
    int  force;
    bool isAlly[12];
    bool isEnemy[12];
    bool isNeutral;
    int  startLocationX;
    int  startLocationY;
    bool isVictorious;
    bool isDefeated;
    bool leftGame;
    bool isParticipating;

    int minerals;
    int gas;
    int gatheredMinerals;
    int gatheredGas;
    int repairedMinerals;
    int repairedGas;
    int refundedMinerals;
    int refundedGas;
    int supplyTotal[3];
    int supplyUsed[3];

    int allUnitCount[SCM_UNIT_TYPES];
    int visibleUnitCount[SCM_UNIT_TYPES];
    int completedUnitCount[SCM_UNIT_TYPES];
    int deadUnitCount[SCM_UNIT_TYPES];
    int killedUnitCount[SCM_UNIT_TYPES];

    int  upgradeLevel[SCM_UPGRADE_TYPES];
    bool hasResearched[SCM_TECH_TYPES];
    bool isResearching[SCM_TECH_TYPES];
    bool isUpgrading[SCM_UPGRADE_TYPES];

    int color;

    int totalUnitScore;
    int totalKillScore;
    int totalBuildingScore;
    int totalRazingScore;
    int customScore;

    int   maxUpgradeLevel[SCM_UPGRADE_TYPES];
    bool  isResearchAvailable[SCM_TECH_TYPES];
    bool  isUnitAvailable[SCM_UNIT_TYPES];
  };
}
