// scm-sim: BWAPI's unit, weapon, upgrade and tech types from the game's own data (scm-sim's docs/bwapi-plan.md, "C3").
//
// BWAPI 4.4 knows every type by tables written into its source for 1.16.1 (UnitType.cpp and its neighbours). Here
// those tables are still there, as BWAPI 4.4 had them, but once a game has started the types answer from the game's
// tables in memory instead: the arrays the game filled from units.dat, weapons.dat, flingy.dat, upgrades.dat and
// techdata.dat (a map's unit settings included), found through the game's pointer tables (GPTP's `DatLoad`: for each
// column, its address, the size of one value and how many values there are), so a table a plugin has moved or widened
// is read where it is and at its width; the requirement lists the game evaluates (found through the operand each
// evaluator loads its list from, where a graft writes its own); and the names of the game's string table, with a
// names table the host hands over for types past 1.16.1's (a mod's `rez\unitnames.tbl`).
//
// What the game's data does not hold stays BWAPI's: what the executable's code decides by type (which unit is a
// refinery, a critter, which tech cloaks which unit, a spell's order and weapon, the speed and range upgrades), the
// names of the types the enum has, and BWAPI's own sentinels.
#pragma once
#include <map>
#include <string>
#include <vector>
#include <BWAPI/UnitType.h>
#include <BWAPI/WeaponType.h>
#include <BWAPI/UpgradeType.h>
#include <BWAPI/TechType.h>
#include <BWAPI/Race.h>
#include <BWAPI/UnitSizeType.h>

namespace BWAPI
{
  namespace TypeData
  {
    /// How long BWAPI 4.4's tables are: 1.16.1's types, then its sentinels (None and the rest).
    const int UNIT44 = 234, WEAPON44 = 132, UPGRADE44 = 63, TECH44 = 47;

    /// An id as BWAPI 4.4 numbered it, or -1 for an id it had no number for (a mod's type past 1.16.1's). This is the
    /// row of BWAPI 4.4's tables and the number GameData (its client protocol) carries.
    inline int unit44(int id)
    {
      if ( id >= 0 && id <= UnitTypes::Enum::None ) return id;
      if ( id >= UnitTypes::Enum::AllUnits && id <= UnitTypes::Enum::Unknown ) return id - UnitTypes::Enum::AllUnits + UnitTypes::Enum::None + 1;
      return -1;
    }
    inline int weapon44(int id)
    {
      if ( id >= 0 && id <= WeaponTypes::Enum::None ) return id;
      return id == WeaponTypes::Enum::Unknown ? WEAPON44 - 1 : -1;
    }
    inline int upgrade44(int id)
    {
      if ( id >= 0 && id <= UpgradeTypes::Enum::None ) return id;
      return id == UpgradeTypes::Enum::Unknown ? UPGRADE44 - 1 : -1;
    }
    inline int tech44(int id)
    {
      if ( id >= 0 && id <= TechTypes::Enum::None ) return id;
      if ( id == TechTypes::Enum::Nuclear_Strike ) return TECH44 - 2;
      return id == TechTypes::Enum::Unknown ? TECH44 - 1 : -1;
    }
    /// The row of BWAPI 4.4's tables an id reads: its own, or Unknown's.
    inline int unitRow44(int id) { int i = unit44(id); return i < 0 ? UNIT44 - 1 : i; }
    inline int weaponRow44(int id) { int i = weapon44(id); return i < 0 ? WEAPON44 - 1 : i; }
    inline int upgradeRow44(int id) { int i = upgrade44(id); return i < 0 ? UPGRADE44 - 1 : i; }
    inline int techRow44(int id) { int i = tech44(id); return i < 0 ? TECH44 - 1 : i; }

    // `name`: a type past BWAPI 4.4's enum is called by it (the enum's own keep BWAPI's names); `gameName`: what the
    // game's strings call every type, in BWAPI's style (the names check of the vanilla dump).
    struct UnitRow
    {
      std::string name, gameName;
      int race = Races::Enum::None;
      int maxHP = 0, maxSP = 0, armor = 0, ore = 0, gas = 0, time = 0;
      int supplyRequired = 0, supplyProvided = 0, spaceRequired = 0, spaceProvided = 0, buildScore = 0, destroyScore = 0;
      int size = UnitSizeTypes::Enum::None, tileWidth = 0, tileHeight = 0, left = 0, up = 0, right = 0, down = 0;
      int seekRange = 0, sightRange = 0, groundWeapon = WeaponTypes::Enum::None, groundHits = 0, airWeapon = WeaponTypes::Enum::None, airHits = 0;
      int armorUpgrade = UpgradeTypes::Enum::None, acceleration = 0, haltDistance = 0, turnRadius = 0;
      double topSpeed = 0;
      unsigned flags = 0;
      UnitType whatBuilds = UnitTypes::None;
      int whatBuildsCount = 0;
      std::map<UnitType, int> requiredUnits;
      TechType requiredTech = TechTypes::None;
      TechType::set abilities, researchesWhat;
      UpgradeType::set upgrades, upgradesWhat;
      UnitType::set buildsWhat;
    };
    struct WeaponRow
    {
      std::string name, gameName;
      int damage = 0, bonus = 0, cooldown = 0, factor = 0, upgrade = UpgradeTypes::Enum::None, damageType = 0, explosion = 0;
      int minRange = 0, maxRange = 0, inner = 0, median = 0, outer = 0, flags = 0;
      UnitType whatUses = UnitTypes::None;
    };
    struct UpgradeRow
    {
      std::string name, gameName;
      int race = Races::Enum::None, oreBase = 0, oreFactor = 0, gasBase = 0, gasFactor = 0, timeBase = 0, timeFactor = 0, maxRepeats = 0;
      UnitType whatUpgrades = UnitTypes::None;
      UnitType required[3] = { UnitTypes::None, UnitTypes::None, UnitTypes::None };
      UnitType::set whatUses;
    };
    struct TechRow
    {
      std::string name, gameName;
      int race = Races::Enum::None, ore = 0, gas = 0, time = 0, energy = 0;
      UnitType whatResearches = UnitTypes::None, requiredUnit = UnitTypes::None;
      UnitType::set whatUses;
    };

    /// The game's tables, once read (`load`); empty before.
    struct Tables
    {
      std::vector<UnitRow> units;
      std::vector<WeaponRow> weapons;
      std::vector<UpgradeRow> upgrades;
      std::vector<TechRow> techs;
      UnitType::set allUnits, macroUnits;
      WeaponType::set allWeapons, normalWeapons, specialWeapons;
      UpgradeType::set allUpgrades;
      TechType::set allTechs;
      /// Bumped by every load: names and the sizes worked out from the tables are looked again.
      int generation = 0;
    };
    extern Tables game;
    /// Whether the types answer from the game's tables (true once a game's tables were read). `dump` turns it off for a
    /// moment to show BWAPI 4.4's own answers beside them.
    extern bool use;

    inline const UnitRow *unit(int id) { return use && id >= 0 && id < (int)game.units.size() && id != UnitTypes::Enum::None ? &game.units[id] : nullptr; }
    inline const WeaponRow *weapon(int id) { return use && id >= 0 && id < (int)game.weapons.size() && id != WeaponTypes::Enum::None ? &game.weapons[id] : nullptr; }
    inline const UpgradeRow *upgrade(int id) { return use && id >= 0 && id < (int)game.upgrades.size() && id != UpgradeTypes::Enum::None ? &game.upgrades[id] : nullptr; }
    inline const TechRow *tech(int id) { return use && id >= 0 && id < (int)game.techs.size() && id != TechTypes::Enum::None ? &game.techs[id] : nullptr; }

    /// The names a mod's data set gives its unit types (`rez\unitnames.tbl`, a string a type), for types past 1.16.1's.
    void setUnitNames(const std::vector<std::string> &names);
    /// Reads the game's tables, at the start of a game. Returns what it read, in words; on a table it cannot read the
    /// types stay BWAPI 4.4's and the words say why.
    std::string load();
    /// Every type BWAPI 4.4 knew, as its own tables answer and as the game's data answers, as JSON (the vanilla check).
    std::string dump();
    /// What `load` said last.
    const std::string &status();

    /// BWAPI 4.4's answers for the types, by its own tables (each type's own file): what the game's rows start from
    /// where the game's data says nothing (the parts the executable's code decides).
    UpgradeType::set bwapiUnitUpgrades(int id44);
    TechType::set bwapiUnitAbilities(int id44);
    UnitType bwapiWeaponWhatUses(int id44);
  }
}
