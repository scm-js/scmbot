// scm-sim: BWAPI's types from the game's own data. What and why: TypeData.h; the record: docs/bwapi-plan.md, "C3".
#include "TypeData.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <sstream>

#include <BWAPI/Race.h>
#include <BWAPI/UnitSizeType.h>
#include <BWAPI/DamageType.h>
#include <BWAPI/ExplosionType.h>
#include <BWAPI/Order.h>

namespace BWAPI
{
  namespace TypeData
  {
    Tables game;
    bool use = false;
    static std::vector<std::string> givenUnitNames;
    static std::string said = "BWAPI 4.4's own types (no game's tables read yet)";

    void setUnitNames(const std::vector<std::string> &names) { givenUnitNames = names; }
    const std::string &status() { return said; }

    namespace
    {
      // ---- The game's memory: this module's memory is the game's address space -------------------------------------
      uint64_t memoryEnd()
      {
#ifdef __wasm__
        return (uint64_t)__builtin_wasm_memory_size(0) * 65536u;
#else
        return 0;
#endif
      }
      bool readable(uint32_t at, uint64_t bytes) { return at >= 0x10000 && (uint64_t)at + bytes <= std::min<uint64_t>(memoryEnd(), 0x80000000u); }
      uint32_t rd(uint32_t at, uint32_t size)
      {
        if ( !readable(at, size) ) return 0;
        uint32_t v = 0;
        std::memcpy(&v, reinterpret_cast<const void *>(static_cast<uintptr_t>(at)), size);
        return v;
      }

      /// A column of a table as the game's pointer table has it: its address, the size of one value, the values it
      /// holds; `first` the id its first value is (a column that holds only a range of ids), `parts` the values an id.
      struct Col
      {
        uint32_t base = 0, size = 0, values = 0;
        int first = 0, parts = 1;
        uint32_t get(int id, int part = 0) const
        {
          if ( !base || id < first || id - first >= (int)values ) return 0;
          return rd(base + (uint32_t)(((id - first) * parts + part) * (int)size), size);
        }
      };
      /// A column's values an id, the id its file starts at, and (a column of several values an id) the size of one value.
      struct Shape { int parts, first, width = 0; };

      /**
       * A table through its pointer table (GPTP's `DatLoad`, 12 bytes a column in file order: address, size of a
       * value, number of values; ended by a zero address), as the game's table loader (0x4D2E80) reads it. A plugin
       * that moves a column or widens it writes the new address and size there, so it is read where it is. A column of
       * several values an id (a unit's extents, its placement box) is described one of two ways: 1.16.1's (the size of
       * one value, as many values as there are parts in all) and GPTP's (the size of an id's record, one an id: its
       * extents 8 bytes, 3,000 of them in Cosmonarchy); told apart by the size.
       */
      bool readTable(const char *name, uint32_t descriptor, const std::vector<Shape> &shape, std::vector<Col> &cols, int &entries, std::string &why)
      {
        cols.assign(shape.size(), Col());
        for ( size_t k = 0; k < shape.size(); ++k )
        {
          uint32_t e = descriptor + 12 * (uint32_t)k, base = rd(e, 4), size = rd(e + 4, 4), count = rd(e + 8, 4);
          bool record = shape[k].parts > 1 && shape[k].width && size == (uint32_t)(shape[k].parts * shape[k].width);
          if ( !base || (!record && size != 1 && size != 2 && size != 4) || !count || count > 0x1000000 || !readable(base, (uint64_t)size * count) )
          {
            why = std::string(name) + ": the game's pointer table has no readable column " + std::to_string(k);
            return false;
          }
          cols[k].base = base; cols[k].parts = shape[k].parts;
          if ( record ) { cols[k].size = (uint32_t)shape[k].width; cols[k].values = count; }
          else { cols[k].size = size; cols[k].values = count / shape[k].parts; }
        }
        entries = (int)cols[0].values;
        // A column with every id starts at 0; one of a range (the game's own layout) where its file says.
        for ( size_t k = 0; k < shape.size(); ++k )
          cols[k].first = (int)cols[k].values >= entries ? 0 : shape[k].first;
        if ( entries <= 0 || entries > 0xFF00 )
        {
          why = std::string(name) + ": " + std::to_string(entries) + " entries";
          return false;
        }
        return true;
      }

      // The pointer tables, as 1.16.1 has them, and the shape of each table's columns (src/mods/live.ts, LIVE_TABLES).
      const uint32_t UNITS_DAT = 0x513c30, WEAPONS_DAT = 0x513868, FLINGY_DAT = 0x515a38, UPGRADES_DAT = 0x5136e0, TECHDATA_DAT = 0x5137d8;
      std::vector<Shape> unitShape()
      {
        std::vector<Shape> s(54, Shape{ 1, 0 });
        s[3] = { 1, 106 };   // infestation: the buildings only
        s[36] = { 2, 0, 2 };    // StarEdit's placement box: width, height
        s[37] = { 2, 106, 2 };  // add-on position: the buildings only
        s[38] = { 4, 0, 2 };    // extents: left, up, right, down
        return s;
      }
      namespace U { enum { Flingy = 0, Subunit1 = 1, ShieldEnabled = 6, Shields = 7, HitPoints = 8, GroundWeapon = 17, GroundHits = 18, AirWeapon = 19, AirHits = 20,
        Flags = 22, SeekRange = 23, SightRange = 24, ArmorUpgrade = 25, Size = 26, Armor = 27, Placement = 36, Extents = 38, Ore = 40, Gas = 41, Time = 42,
        GroupFlags = 44, SupplyProvided = 45, SupplyRequired = 46, SpaceRequired = 47, SpaceProvided = 48, BuildScore = 49, DestroyScore = 50 }; }
      namespace W { enum { Label = 0, TargetFlags = 3, MinRange = 4, MaxRange = 5, Upgrade = 6, DamageType = 7, Explosion = 10, Inner = 11, Median = 12, Outer = 13,
        Damage = 14, Bonus = 15, Cooldown = 16, Factor = 17 }; }
      namespace F { enum { Speed = 1, Acceleration = 2, HaltDistance = 3, TurnRadius = 4, MovementControl = 6 }; }
      namespace G { enum { OreBase = 0, OreFactor = 1, GasBase = 2, GasFactor = 3, TimeBase = 4, TimeFactor = 5, Label = 8, Race = 9, MaxRepeats = 10 }; }
      namespace T { enum { Ore = 0, Gas = 1, Time = 2, Energy = 3, Label = 7, Race = 8 }; }

      // ---- Strings ------------------------------------------------------------------------------------------------
      /**
       * A string of the game's `stat_txt` (1-based; "" for none): the table the game loaded (its pointer at 0x6D1238),
       * 1.16.1's form (16-bit count and offsets) or the 32-bit one GPTP's string extender puts at the same pointer, told
       * apart by the first offset (src/exact/card.ts, statString).
       */
      std::string statString(uint32_t id)
      {
        uint32_t table = rd(0x6d1238, 4);
        if ( !id || !table ) return "";
        uint32_t wide = rd(table, 4), first32 = rd(table + 4, 4), at;
        if ( wide > 0 && wide < 0x100000 && first32 == 4 + 4 * wide )
        {
          if ( id > wide ) return "";
          at = table + rd(table + 4 * id, 4);
        }
        else
        {
          if ( id > rd(table, 2) ) return "";
          at = table + rd(table + 2 * id, 2);
        }
        std::string s;
        for ( uint32_t i = 0; i < 512; ++i )
        {
          uint32_t c = rd(at + i, 1);
          if ( !c ) break;
          s += (char)c;
        }
        return s;
      }
      /// A name in BWAPI's style: "Terran Marine" → "Terran_Marine" (letters and digits, the rest one underscore).
      std::string identifier(const std::string &s)
      {
        std::string out;
        for ( char ch : s )
        {
          unsigned char c = (unsigned char)ch;
          if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ) out += (char)c;
          else if ( !out.empty() && out.back() != '_' ) out += '_';
        }
        while ( !out.empty() && out.back() == '_' ) out.pop_back();
        return out;
      }

      // ---- Requirement lists (src/exact/card.ts, readRequirementLists) --------------------------------------------
      bool plausible(uint32_t at) { return at >= 0x10000 && at < 0x80000000u; }
      /**
       * One of the game's requirement lists, by id: where its evaluator loads it from (a graft writes its own list's
       * address in that operand), else 1.16.1's place. Words from the list + 2: an id, its opcodes, 0xFFFF, the next …;
       * an id of 0xFFFF ends it. An upgrade's sections a level (0xFF20, 0xFF21) are kept with the upgrade's words.
       */
      std::map<int, std::vector<uint16_t>> readLists(uint32_t operand, uint32_t fallback)
      {
        std::map<int, std::vector<uint16_t>> out;
        uint32_t base = rd(operand, 4);
        if ( !plausible(base) ) base = fallback;
        uint32_t at = base + 2;
        std::vector<uint16_t> *last = nullptr;
        for ( int guard = 0; guard < 0x4000; ++guard )
        {
          uint16_t id = (uint16_t)rd(at, 2);
          at += 2;
          if ( id == 0xFFFF || !readable(at, 2) ) break;
          std::vector<uint16_t> words;
          for ( uint16_t w = (uint16_t)rd(at, 2); w != 0xFFFF && words.size() < 512; w = (uint16_t)rd(at, 2) ) { words.push_back(w); at += 2; }
          at += 2;
          if ( id >= 0xFF00 && last ) { last->push_back(id); last->insert(last->end(), words.begin(), words.end()); }
          else { last = &(out[id] = words); }
        }
        return out;
      }
      /// What one list says, as BWAPI asks it.
      struct Parsed
      {
        std::vector<int> current;   // every "current unit is", alternatives too, in order
        std::vector<int> owns[3];   // what must be owned (the first of alternatives), at level 1, 2, 3
        int tech = -1;              // a tech researched (0xFF25)
        bool hangar = false;        // made in a hangar (0xFF0E: an interceptor, a scarab)
        bool researched = false;    // asks that its tech be researched (0xFF0F, a tech's use list)
      };
      Parsed parse(const std::vector<uint16_t> &w)
      {
        Parsed p;
        int level = 0;
        bool alternative = false;
        for ( size_t i = 0; i < w.size(); ++i )
        {
          uint16_t x = w[i];
          if ( x == 0xFF20 ) { level = 1; alternative = false; continue; }
          if ( x == 0xFF21 ) { level = 2; alternative = false; continue; }
          if ( x == 0xFF01 ) { alternative = true; continue; }
          if ( x == 0xFF0E ) p.hangar = true;
          if ( x == 0xFF0F ) p.researched = true;
          bool alt = alternative;
          alternative = false;
          if ( x < 0xFF00 ) { if ( !alt ) p.owns[level].push_back(x); continue; }
          int operand = (x == 0xFF02 || x == 0xFF03 || x == 0xFF04 || x == 0xFF25) && i + 1 < w.size() ? w[++i] : -1;
          if ( x == 0xFF02 ) p.current.push_back(operand);
          else if ( (x == 0xFF03 || x == 0xFF04) && !alt ) p.owns[level].push_back(operand);
          else if ( x == 0xFF25 && !alt && p.tech < 0 ) p.tech = operand;
        }
        return p;
      }

      int raceOf(int r) { return r == 0 ? Races::Enum::Zerg : r == 1 ? Races::Enum::Terran : r == 2 ? Races::Enum::Protoss : Races::Enum::None; }
    }

    std::string load()
    {
      std::string why;
      std::vector<Col> u, w, f, g, t;
      int nu = 0, nw = 0, nf = 0, ng = 0, nt = 0;
      if ( !readTable("units.dat", UNITS_DAT, unitShape(), u, nu, why) ||
           !readTable("weapons.dat", WEAPONS_DAT, std::vector<Shape>(24, Shape{ 1, 0 }), w, nw, why) ||
           !readTable("flingy.dat", FLINGY_DAT, std::vector<Shape>(7, Shape{ 1, 0 }), f, nf, why) ||
           !readTable("upgrades.dat", UPGRADES_DAT, std::vector<Shape>(12, Shape{ 1, 0 }), g, ng, why) ||
           !readTable("techdata.dat", TECHDATA_DAT, std::vector<Shape>(11, Shape{ 1, 0 }), t, nt, why) )
      {
        use = false;
        return said = "BWAPI 4.4's own types: " + why;
      }
      // Rows are filled with BWAPI 4.4's answers switched off: what the game's data does not say comes from them.
      use = false;
      Tables d;
      d.generation = game.generation + 1;
      const int noUnit = UnitTypes::Enum::None, noWeapon = WeaponTypes::Enum::None, noUpgrade = UpgradeTypes::Enum::None;
      auto isUnit = [&](int id) { return id >= 0 && id < nu && id != noUnit; };
      auto isWeapon = [&](int id) { return id >= 0 && id < nw && id != noWeapon; };
      auto isUpgrade = [&](int id) { return id >= 0 && id < ng && id != noUpgrade; };

      // Weapons.
      d.weapons.resize(nw);
      for ( int id = 0; id < nw; ++id )
      {
        WeaponRow &r = d.weapons[id];
        r.gameName = identifier(statString(w[W::Label].get(id)));
        r.name = id < WeaponTypes::Enum::None ? "" : r.gameName;
        r.damage = w[W::Damage].get(id); r.bonus = w[W::Bonus].get(id); r.cooldown = w[W::Cooldown].get(id); r.factor = w[W::Factor].get(id);
        r.upgrade = w[W::Upgrade].get(id); r.damageType = w[W::DamageType].get(id); r.explosion = w[W::Explosion].get(id);
        r.minRange = w[W::MinRange].get(id); r.maxRange = w[W::MaxRange].get(id);
        r.inner = w[W::Inner].get(id); r.median = w[W::Median].get(id); r.outer = w[W::Outer].get(id);
        r.flags = w[W::TargetFlags].get(id);
      }

      // Units.
      d.units.resize(nu);
      for ( int id = 0; id < nu; ++id )
      {
        UnitRow &r = d.units[id];
        // Names: the host's names table where it has one (a mod's rez\unitnames.tbl), else the game's strings (a unit's
        // name is stat_txt's string id + 1 for 1.16.1's types).
        std::string given = id < (int)givenUnitNames.size() ? givenUnitNames[id] : "";
        r.gameName = identifier(!given.empty() ? given : id < noUnit ? statString(id + 1) : "");
        if ( id > noUnit ) r.name = r.gameName;
        int group = u[U::GroupFlags].get(id);
        r.race = group & 1 ? Races::Enum::Zerg : group & 2 ? Races::Enum::Terran : group & 4 ? Races::Enum::Protoss : Races::Enum::None;
        r.maxHP = (int)(u[U::HitPoints].get(id) >> 8);
        r.maxSP = u[U::ShieldEnabled].get(id) ? u[U::Shields].get(id) : 0;
        r.armor = u[U::Armor].get(id); r.ore = u[U::Ore].get(id); r.gas = u[U::Gas].get(id); r.time = u[U::Time].get(id);
        r.supplyRequired = u[U::SupplyRequired].get(id); r.supplyProvided = u[U::SupplyProvided].get(id);
        r.spaceRequired = u[U::SpaceRequired].get(id); r.spaceProvided = u[U::SpaceProvided].get(id);
        r.buildScore = u[U::BuildScore].get(id); r.destroyScore = u[U::DestroyScore].get(id);
        r.size = u[U::Size].get(id);
        r.flags = u[U::Flags].get(id);
        // The tiles a building takes are its placement box's (the game places it by them). Other units have no tile
        // size in the game: BWAPI 4.4's own for its types, the placement box rounded up for a mod's.
        r.tileWidth = (u[U::Placement].get(id, 0) + 31) / 32; r.tileHeight = (u[U::Placement].get(id, 1) + 31) / 32;
        if ( !(r.flags & 1) && id < noUnit ) { r.tileWidth = UnitType(id).tileWidth(); r.tileHeight = UnitType(id).tileHeight(); }
        r.left = u[U::Extents].get(id, 0); r.up = u[U::Extents].get(id, 1); r.right = u[U::Extents].get(id, 2); r.down = u[U::Extents].get(id, 3);
        r.seekRange = u[U::SeekRange].get(id); r.sightRange = u[U::SightRange].get(id);
        r.armorUpgrade = u[U::ArmorUpgrade].get(id);
        // BWAPI's "produces units" is its own flag, not units.dat's bit (which the table sets for a bunker and not for a
        // starport): BWAPI 4.4's for its types; a mod's: worked out below from what it builds.
        const unsigned produces = 0x80000000u;
        r.flags = (r.flags & ~produces) | (id < noUnit && UnitType(id).canProduce() ? produces : 0);
        // A unit whose weapon is on its turret (a tank, a goliath) is given the turret's, as BWAPI 4.4 gives it.
        int sub = u[U::Subunit1].get(id);
        r.groundWeapon = u[U::GroundWeapon].get(id); r.groundHits = u[U::GroundHits].get(id);
        r.airWeapon = u[U::AirWeapon].get(id); r.airHits = u[U::AirHits].get(id);
        if ( r.groundWeapon == noWeapon && isUnit(sub) ) { r.groundWeapon = u[U::GroundWeapon].get(sub); r.groundHits = u[U::GroundHits].get(sub); }
        if ( r.airWeapon == noWeapon && isUnit(sub) ) { r.airWeapon = u[U::AirWeapon].get(sub); r.airHits = u[U::AirHits].get(sub); }
        // Movement: flingy.dat's, for a unit that moves (BWAPI gives a turret, a building that cannot lift off and the
        // like none). A unit the animation script moves (flingy.dat's movement control not 0: a marine, a zergling)
        // has no top speed in the table: BWAPI 4.4's measured one for its types.
        int flingy = u[U::Flingy].get(id);
        bool moves = (r.flags & 0x08000000) || (r.flags & 0x20);   // AutoAttackAndMove, FlyingBuilding
        if ( flingy < nf && !(r.flags & 0x10) )                    // a turret (Subunit) moves with its unit
        {
          r.topSpeed = moves ? f[F::Speed].get(flingy) / 256.0 : 0;
          r.acceleration = f[F::Acceleration].get(flingy);
          r.haltDistance = f[F::HaltDistance].get(flingy);
          r.turnRadius = f[F::TurnRadius].get(flingy);
          if ( moves && f[F::MovementControl].get(flingy) != 0 && id < noUnit ) r.topSpeed = UnitType(id).topSpeed();
        }
      }

      // Upgrades and techs.
      d.upgrades.resize(ng);
      for ( int id = 0; id < ng; ++id )
      {
        UpgradeRow &r = d.upgrades[id];
        r.gameName = identifier(statString(g[G::Label].get(id)));
        r.name = id < noUpgrade ? "" : r.gameName;
        r.race = raceOf(g[G::Race].get(id));
        r.oreBase = g[G::OreBase].get(id); r.oreFactor = g[G::OreFactor].get(id); r.gasBase = g[G::GasBase].get(id); r.gasFactor = g[G::GasFactor].get(id);
        r.timeBase = g[G::TimeBase].get(id); r.timeFactor = g[G::TimeFactor].get(id); r.maxRepeats = g[G::MaxRepeats].get(id);
      }
      d.techs.resize(nt);
      for ( int id = 0; id < nt; ++id )
      {
        TechRow &r = d.techs[id];
        r.gameName = identifier(statString(t[T::Label].get(id)));
        r.name = id < TechTypes::Enum::None ? "" : r.gameName;
        r.race = raceOf(t[T::Race].get(id));
        r.ore = t[T::Ore].get(id); r.gas = t[T::Gas].get(id); r.time = t[T::Time].get(id); r.energy = t[T::Energy].get(id);
        // The executable charges energy only for its spells: a tech BWAPI 4.4 says costs none (Lurker Aspect, the
        // melds) costs none, whatever the table holds for it.
        if ( id < TechTypes::Enum::None && TechType(id).energyCost() == 0 ) r.energy = 0;
      }

      // The requirement lists: the operands the game's evaluators load them from (src/exact/card.ts, REQUIREMENTS).
      std::vector<bool> listed(nu, false);
      std::vector<std::vector<int>> unitBy(nu);
      std::vector<bool> hangar(nu, false);
      for ( auto &l : readLists(0x46e138, 0x514178) )
      {
        if ( !isUnit(l.first) ) continue;
        listed[l.first] = true;
        UnitRow &r = d.units[l.first];
        Parsed p = parse(l.second);
        unitBy[l.first] = p.current;
        hangar[l.first] = p.hangar;
        if ( !p.current.empty() && isUnit(p.current[0]) ) { r.whatBuilds = UnitType(p.current[0]); r.whatBuildsCount = 1; r.requiredUnits[r.whatBuilds] = 1; }
        for ( int o : p.owns[0] ) if ( isUnit(o) ) r.requiredUnits[UnitType(o)] = 1;
        if ( p.tech >= 0 && p.tech < nt ) r.requiredTech = TechType(p.tech);
      }
      // For the inverses (researchesWhat, upgradesWhat): every type a list names as the one doing it, as BWAPI 4.4 has
      // a Lair and a Hive research Burrowing; whatResearches and whatUpgrades are the first.
      std::vector<std::vector<int>> upgradeBy(ng), techBy(nt), techUsers(nt);
      std::vector<bool> upgradeListed(ng, false);
      for ( auto &l : readLists(0x46d5d6, 0x5145c0) )
      {
        if ( !isUpgrade(l.first) ) continue;
        UpgradeRow &r = d.upgrades[l.first];
        Parsed p = parse(l.second);
        upgradeListed[l.first] = !p.current.empty();
        upgradeBy[l.first] = p.current;
        if ( !p.current.empty() && isUnit(p.current[0]) ) r.whatUpgrades = UnitType(p.current[0]);
        for ( int level = 0; level < 3; ++level )
          if ( !p.owns[level].empty() && isUnit(p.owns[level][0]) ) r.required[level] = UnitType(p.owns[level][0]);
      }
      for ( auto &l : readLists(0x46d568, 0x514908) )
      {
        if ( l.first < 0 || l.first >= nt || l.first == TechTypes::Enum::None ) continue;
        TechRow &r = d.techs[l.first];
        Parsed p = parse(l.second);
        techBy[l.first] = p.current;
        if ( !p.current.empty() && isUnit(p.current[0]) ) r.whatResearches = UnitType(p.current[0]);
        if ( !p.owns[0].empty() && isUnit(p.owns[0][0]) ) r.requiredUnit = UnitType(p.owns[0][0]);
      }
      std::vector<bool> researchable(nt, false);
      for ( auto &l : readLists(0x46d4f8, 0x514a48) )
      {
        if ( l.first < 0 || l.first >= nt || l.first == TechTypes::Enum::None ) continue;
        Parsed p = parse(l.second);
        researchable[l.first] = p.researched;
        for ( int c : p.current ) if ( isUnit(c) ) { d.techs[l.first].whatUses.insert(UnitType(c)); techUsers[l.first].push_back(c); }
      }
      // A tech whose use list does not ask for it to be researched (Feedback, Parasite, Defensive Matrix, the melds), or
      // that has none, is never researched: nothing researches it and it costs nothing (its research list and the
      // table's prices are left from before the release; BWAPI says None and 0 too, and so takes it as researched).
      for ( int id = 0; id < nt; ++id )
      {
        if ( id == TechTypes::Enum::None || researchable[id] ) continue;
        TechRow &r = d.techs[id];
        r.whatResearches = UnitTypes::None; r.requiredUnit = UnitTypes::None; techBy[id].clear();
        r.ore = r.gas = r.time = 0;
      }
      // A merge is made by its tech's user, two of them: the executable makes an Archon of two of Archon Warp's users,
      // a Dark Archon of two of Dark Archon Meld's.
      const std::pair<int, int> merges[] = { { UnitTypes::Enum::Protoss_Archon, TechTypes::Enum::Archon_Warp }, { UnitTypes::Enum::Protoss_Dark_Archon, TechTypes::Enum::Dark_Archon_Meld } };
      for ( auto &m : merges )
      {
        if ( !isUnit(m.first) || m.second >= nt || d.units[m.first].whatBuilds != UnitTypes::None ) continue;
        UnitType::set &users = d.techs[m.second].whatUses;
        UnitType by = UnitTypes::None;
        for ( UnitType x : users ) if ( by == UnitTypes::None || x.getID() < by.getID() ) by = x;
        if ( by == UnitTypes::None ) continue;
        d.units[m.first].whatBuilds = by; d.units[m.first].whatBuildsCount = 2; d.units[m.first].requiredUnits[by] = 2;
        listed[m.first] = true;
      }
      // A type of 1.16.1's that no list makes is made by the executable's own code (an egg, a cocoon, larva, siege
      // mode): BWAPI 4.4's answer, which is that code's.
      for ( int id = 0; id < nu && id < noUnit; ++id )
      {
        if ( listed[id] ) continue;
        UnitType x(id);
        auto wb = x.whatBuilds();
        d.units[id].whatBuilds = wb.first; d.units[id].whatBuildsCount = wb.second;
        d.units[id].requiredUnits = x.requiredUnits();
        d.units[id].requiredTech = x.requiredTech();
      }

      // What follows from the rows: each unit's upgrades and abilities, and the inverses.
      for ( int id = 0; id < nu; ++id )
      {
        if ( id == noUnit ) continue;
        UnitRow &r = d.units[id];
        // An upgrade counts when something can do it (it has a list): the armor "upgrade" of a unit with none is an
        // upgrade nothing researches.
        auto add = [&](int x) { if ( isUpgrade(x) && upgradeListed[x] ) r.upgrades.insert(UpgradeType(x)); };
        add(r.armorUpgrade);
        for ( int weapon : { r.groundWeapon, r.airWeapon } )
          if ( isWeapon(weapon) ) add(d.weapons[weapon].upgrade);
        // A unit with no weapon whose attack is what it makes in its hangar (a carrier's interceptors, a reaver's
        // scarabs): their weapon upgrades.
        if ( !isWeapon(r.groundWeapon) && !isWeapon(r.airWeapon) )
          for ( int x = 0; x < nu; ++x )
            if ( hangar[x] && std::find(unitBy[x].begin(), unitBy[x].end(), id) != unitBy[x].end() )
              for ( int weapon : { d.units[x].groundWeapon, d.units[x].airWeapon } )
                if ( isWeapon(weapon) ) add(d.weapons[weapon].upgrade);
        if ( r.maxSP > 0 ) add(UpgradeTypes::Enum::Protoss_Plasma_Shields);
        // The upgrades the executable's code applies by unit type (speed, range, energy, capacity), as BWAPI 4.4 lists them.
        if ( id < noUnit )
          for ( UpgradeType x : bwapiUnitUpgrades(id) ) if ( x.getID() > UpgradeTypes::Enum::Protoss_Plasma_Shields ) r.upgrades.insert(x);
        // Nuclear Strike is BWAPI's own tech (no table holds it).
        if ( id < noUnit && bwapiUnitAbilities(id).contains(TechTypes::Nuclear_Strike) ) r.abilities.insert(TechTypes::Nuclear_Strike);
        if ( isUnit(r.whatBuilds) && listed[id] ) d.units[r.whatBuilds].buildsWhat.insert(UnitType(id));
        for ( UpgradeType x : r.upgrades ) if ( isUpgrade(x) ) d.upgrades[x].whatUses.insert(UnitType(id));
      }
      for ( int id = 0; id < nt; ++id )
      {
        if ( id == TechTypes::Enum::None ) continue;
        for ( UnitType x : d.techs[id].whatUses ) if ( isUnit(x) ) d.units[x].abilities.insert(TechType(id));
        for ( int x : techBy[id] ) if ( isUnit(x) ) d.units[x].researchesWhat.insert(TechType(id));
      }
      for ( int id = 0; id < ng; ++id )
        for ( int x : upgradeBy[id] ) if ( id != noUpgrade && isUnit(x) ) d.units[x].upgradesWhat.insert(UpgradeType(id));
      // A mod's type produces units (BWAPI's canProduce) when a unit that is not a building is made by it.
      for ( int x = noUnit + 1; x < nu; ++x )
        for ( int y = 0; y < nu; ++y )
          if ( y != noUnit && d.units[y].whatBuilds == UnitType(x) && !(d.units[y].flags & 1) ) { d.units[x].flags |= 0x80000000u; break; }
      // A weapon's user, as BWAPI 4.4 names one (the regular unit before a hero, a unit before its turret): the first unit that carries it (its
      // own or its turret's), else the first its tech's list names (a spell), else the first hero that carries it, else
      // BWAPI 4.4's.
      for ( int id = 0; id < nw; ++id )
      {
        if ( id == noWeapon ) continue;
        WeaponRow &r = d.weapons[id];
        auto hero = [&](int x) { return (d.units[x].flags & 0x40) != 0; };
        auto carries = [&](int x) { return x != noUnit && (d.units[x].groundWeapon == id || d.units[x].airWeapon == id); };
        for ( int x = 0; x < nu && r.whatUses == UnitTypes::None; ++x )
          if ( carries(x) && !hero(x) && !(d.units[x].flags & 0x10) ) r.whatUses = UnitType(x);
        TechType tech = WeaponType(id).getTech();
        if ( r.whatUses == UnitTypes::None && tech.getID() >= 0 && tech.getID() < nt && tech != TechTypes::None )
          for ( int x : techUsers[tech] ) if ( r.whatUses == UnitTypes::None && !hero(x) ) r.whatUses = UnitType(x);
        for ( int x = 0; x < nu && r.whatUses == UnitTypes::None; ++x )
          if ( carries(x) && !(d.units[x].flags & 0x10) ) r.whatUses = UnitType(x);
        for ( int x = 0; x < nu && r.whatUses == UnitTypes::None; ++x )
          if ( carries(x) ) r.whatUses = UnitType(x);
        if ( r.whatUses == UnitTypes::None && id < noWeapon ) r.whatUses = bwapiWeaponWhatUses(id);
      }

      // The sets: BWAPI 4.4's for the ids it knew, and a mod's named ids past them.
      for ( int id = 0; id < nu; ++id )
        if ( id < noUnit ? UnitTypes::allUnitTypes().contains(UnitType(id)) : id != noUnit && !d.units[id].name.empty() ) d.allUnits.insert(UnitType(id));
      d.allUnits.insert(UnitTypes::None); d.allUnits.insert(UnitTypes::Unknown);
      for ( int id = 0; id < nw; ++id )
      {
        if ( id == noWeapon ) continue;
        bool carried = false;
        for ( int x = 0; x < nu && !carried; ++x ) carried = x != noUnit && (d.units[x].groundWeapon == id || d.units[x].airWeapon == id);
        bool special = id < noWeapon && WeaponTypes::specialWeaponTypes().contains(WeaponType(id));
        if ( id < noWeapon && !WeaponTypes::allWeaponTypes().contains(WeaponType(id)) ) continue;
        if ( id > noWeapon && d.weapons[id].name.empty() ) continue;
        (special ? d.specialWeapons : d.normalWeapons).insert(WeaponType(id));
        d.allWeapons.insert(WeaponType(id));
        (void)carried;
      }
      d.allWeapons.insert(WeaponTypes::None); d.allWeapons.insert(WeaponTypes::Unknown);
      for ( int id = 0; id < ng; ++id )
        if ( id < noUpgrade ? UpgradeTypes::allUpgradeTypes().contains(UpgradeType(id)) : id != noUpgrade && !d.upgrades[id].name.empty() ) d.allUpgrades.insert(UpgradeType(id));
      d.allUpgrades.insert(UpgradeTypes::None); d.allUpgrades.insert(UpgradeTypes::Unknown);
      for ( int id = 0; id < nt; ++id )
        if ( id < TechTypes::Enum::None ? TechTypes::allTechTypes().contains(TechType(id)) : id != TechTypes::Enum::None && !d.techs[id].name.empty() ) d.allTechs.insert(TechType(id));
      d.allTechs.insert(TechTypes::None); d.allTechs.insert(TechTypes::Nuclear_Strike); d.allTechs.insert(TechTypes::Unknown);

      game = std::move(d);
      use = true;
      std::ostringstream s;
      s << "the game's types: " << nu << " units, " << nw << " weapons, " << nf << " flingy, " << ng << " upgrades, " << nt << " techs";
      if ( !givenUnitNames.empty() ) s << "; " << givenUnitNames.size() << " unit names given";
      return said = s.str();
    }

    // ---- The vanilla check: every answer BWAPI 4.4 had written down, beside the game's ---------------------------
    namespace
    {
      std::string num(double v) { std::ostringstream s; s << v; return s.str(); }
      template<class S> std::string ids(const S &set)
      {
        std::vector<int> v;
        for ( auto x : set ) v.push_back(x.getID());
        std::sort(v.begin(), v.end());
        std::string s = "[";
        for ( size_t i = 0; i < v.size(); ++i ) s += (i ? "," : "") + std::to_string(v[i]);
        return s + "]";
      }
      std::string quote(const std::string &s)
      {
        std::string o = "\"";
        for ( char c : s ) { if ( c == '"' || c == '\\' ) o += '\\'; if ( (unsigned char)c >= 0x20 ) o += c; }
        return o + "\"";
      }
      typedef std::vector<std::pair<const char *, std::string>> Props;
      Props unitProps(UnitType u)
      {
        auto wb = u.whatBuilds();
        std::string req = "[";
        for ( auto &p : u.requiredUnits() ) req += (req.size() > 1 ? "," : "") + std::string("[") + std::to_string(p.first.getID()) + "," + std::to_string(p.second) + "]";
        req += "]";
        return {
          { "name", quote(use && unit(u) ? unit(u)->gameName : u.getName()) }, { "race", num(u.getRace().getID()) }, { "whatBuilds", "[" + num(wb.first.getID()) + "," + num(wb.second) + "]" },
          { "requiredUnits", req }, { "requiredTech", num(u.requiredTech().getID()) }, { "cloakingTech", num(u.cloakingTech().getID()) },
          { "abilities", ids(u.abilities()) }, { "upgrades", ids(u.upgrades()) }, { "armorUpgrade", num(u.armorUpgrade().getID()) },
          { "maxHitPoints", num(u.maxHitPoints()) }, { "maxShields", num(u.maxShields()) }, { "maxEnergy", num(u.maxEnergy()) }, { "armor", num(u.armor()) },
          { "mineralPrice", num(u.mineralPrice()) }, { "gasPrice", num(u.gasPrice()) }, { "buildTime", num(u.buildTime()) },
          { "supplyRequired", num(u.supplyRequired()) }, { "supplyProvided", num(u.supplyProvided()) }, { "spaceRequired", num(u.spaceRequired()) },
          { "spaceProvided", num(u.spaceProvided()) }, { "buildScore", num(u.buildScore()) }, { "destroyScore", num(u.destroyScore()) },
          { "size", num(u.size().getID()) }, { "tileWidth", num(u.tileWidth()) }, { "tileHeight", num(u.tileHeight()) },
          { "dimensionLeft", num(u.dimensionLeft()) }, { "dimensionUp", num(u.dimensionUp()) }, { "dimensionRight", num(u.dimensionRight()) },
          { "dimensionDown", num(u.dimensionDown()) }, { "seekRange", num(u.seekRange()) }, { "sightRange", num(u.sightRange()) },
          { "groundWeapon", num(u.groundWeapon().getID()) }, { "maxGroundHits", num(u.maxGroundHits()) }, { "airWeapon", num(u.airWeapon().getID()) },
          { "maxAirHits", num(u.maxAirHits()) }, { "topSpeed", num(u.topSpeed()) }, { "acceleration", num(u.acceleration()) },
          { "haltDistance", num(u.haltDistance()) }, { "turnRadius", num(u.turnRadius()) },
          { "canProduce", num(u.canProduce()) }, { "canAttack", num(u.canAttack()) }, { "canMove", num(u.canMove()) }, { "isFlyer", num(u.isFlyer()) },
          { "regeneratesHP", num(u.regeneratesHP()) }, { "isSpellcaster", num(u.isSpellcaster()) }, { "hasPermanentCloak", num(u.hasPermanentCloak()) },
          { "isInvincible", num(u.isInvincible()) }, { "isOrganic", num(u.isOrganic()) }, { "isMechanical", num(u.isMechanical()) },
          { "isRobotic", num(u.isRobotic()) }, { "isDetector", num(u.isDetector()) }, { "isResourceContainer", num(u.isResourceContainer()) },
          { "isResourceDepot", num(u.isResourceDepot()) }, { "isRefinery", num(u.isRefinery()) }, { "isWorker", num(u.isWorker()) },
          { "requiresPsi", num(u.requiresPsi()) }, { "requiresCreep", num(u.requiresCreep()) }, { "isTwoUnitsInOneEgg", num(u.isTwoUnitsInOneEgg()) },
          { "isBurrowable", num(u.isBurrowable()) }, { "isCloakable", num(u.isCloakable()) }, { "isBuilding", num(u.isBuilding()) },
          { "isAddon", num(u.isAddon()) }, { "isFlyingBuilding", num(u.isFlyingBuilding()) }, { "isNeutral", num(u.isNeutral()) },
          { "isHero", num(u.isHero()) }, { "isPowerup", num(u.isPowerup()) }, { "isBeacon", num(u.isBeacon()) }, { "isFlagBeacon", num(u.isFlagBeacon()) },
          { "isSpecialBuilding", num(u.isSpecialBuilding()) }, { "isSpell", num(u.isSpell()) }, { "producesCreep", num(u.producesCreep()) },
          { "producesLarva", num(u.producesLarva()) }, { "isMineralField", num(u.isMineralField()) }, { "isCritter", num(u.isCritter()) },
          { "canBuildAddon", num(u.canBuildAddon()) }, { "buildsWhat", ids(u.buildsWhat()) }, { "researchesWhat", ids(u.researchesWhat()) },
          { "upgradesWhat", ids(u.upgradesWhat()) },
        };
      }
      Props weaponProps(WeaponType x)
      {
        return {
          { "name", quote(use && weapon(x) ? weapon(x)->gameName : x.getName()) }, { "getTech", num(x.getTech().getID()) }, { "whatUses", num(x.whatUses().getID()) },
          { "damageAmount", num(x.damageAmount()) }, { "damageBonus", num(x.damageBonus()) }, { "damageCooldown", num(x.damageCooldown()) },
          { "damageFactor", num(x.damageFactor()) }, { "upgradeType", num(x.upgradeType().getID()) }, { "damageType", num(x.damageType().getID()) },
          { "explosionType", num(x.explosionType().getID()) }, { "minRange", num(x.minRange()) }, { "maxRange", num(x.maxRange()) },
          { "innerSplashRadius", num(x.innerSplashRadius()) }, { "medianSplashRadius", num(x.medianSplashRadius()) }, { "outerSplashRadius", num(x.outerSplashRadius()) },
          { "targetsAir", num(x.targetsAir()) }, { "targetsGround", num(x.targetsGround()) }, { "targetsMechanical", num(x.targetsMechanical()) },
          { "targetsOrganic", num(x.targetsOrganic()) }, { "targetsNonBuilding", num(x.targetsNonBuilding()) }, { "targetsNonRobotic", num(x.targetsNonRobotic()) },
          { "targetsTerrain", num(x.targetsTerrain()) }, { "targetsOrgOrMech", num(x.targetsOrgOrMech()) }, { "targetsOwn", num(x.targetsOwn()) },
        };
      }
      Props upgradeProps(UpgradeType x)
      {
        return {
          { "name", quote(use && upgrade(x) ? upgrade(x)->gameName : x.getName()) }, { "race", num(x.getRace().getID()) },
          { "mineralPrice", "[" + num(x.mineralPrice(1)) + "," + num(x.mineralPrice(2)) + "," + num(x.mineralPrice(3)) + "]" },
          { "gasPrice", "[" + num(x.gasPrice(1)) + "," + num(x.gasPrice(2)) + "," + num(x.gasPrice(3)) + "]" },
          { "upgradeTime", "[" + num(x.upgradeTime(1)) + "," + num(x.upgradeTime(2)) + "," + num(x.upgradeTime(3)) + "]" },
          { "whatUpgrades", num(x.whatUpgrades().getID()) }, { "whatUses", ids(x.whatUses()) }, { "maxRepeats", num(x.maxRepeats()) },
          { "whatsRequired", "[" + num(x.whatsRequired(1).getID()) + "," + num(x.whatsRequired(2).getID()) + "," + num(x.whatsRequired(3).getID()) + "]" },
        };
      }
      Props techProps(TechType x)
      {
        return {
          { "name", quote(use && tech(x) ? tech(x)->gameName : x.getName()) }, { "race", num(x.getRace().getID()) }, { "mineralPrice", num(x.mineralPrice()) }, { "gasPrice", num(x.gasPrice()) },
          { "researchTime", num(x.researchTime()) }, { "energyCost", num(x.energyCost()) }, { "whatResearches", num(x.whatResearches().getID()) },
          { "getWeapon", num(x.getWeapon().getID()) }, { "targetsUnit", num(x.targetsUnit()) }, { "targetsPosition", num(x.targetsPosition()) },
          { "whatUses", ids(x.whatUses()) }, { "getOrder", num(x.getOrder().getID()) }, { "requiredUnit", num(x.requiredUnit().getID()) },
        };
      }
      std::string object(const Props &p)
      {
        std::string s = "{";
        for ( size_t i = 0; i < p.size(); ++i ) s += (i ? "," : "") + quote(p[i].first) + ":" + p[i].second;
        return s + "}";
      }
      std::string kind(const char *name, int count, const std::function<Props(int)> &props)
      {
        std::string s = quote(name) + ":[";
        bool was = use;
        for ( int id = 0; id < count; ++id )
        {
          use = false; std::string bwapi = object(props(id));
          use = was; std::string data = object(props(id));
          s += (id ? "," : "") + std::string("{\"id\":") + std::to_string(id) + ",\"bwapi\":" + bwapi + ",\"game\":" + data + "}";
        }
        use = was;
        return s + "]";
      }
    }

    std::string dump()
    {
      std::string s = "{\"status\":" + quote(said) + ",\"use\":" + (use ? "true" : "false") + ",\"counts\":{\"units\":" + std::to_string(game.units.size()) +
        ",\"weapons\":" + std::to_string(game.weapons.size()) + ",\"upgrades\":" + std::to_string(game.upgrades.size()) + ",\"techs\":" + std::to_string(game.techs.size()) + "},";
      s += kind("units", UnitTypes::Enum::None, [](int id) { return unitProps(UnitType(id)); }) + ",";
      s += kind("weapons", WeaponTypes::Enum::None, [](int id) { return weaponProps(WeaponType(id)); }) + ",";
      s += kind("upgrades", UpgradeTypes::Enum::None, [](int id) { return upgradeProps(UpgradeType(id)); }) + ",";
      s += kind("techs", TechTypes::Enum::None, [](int id) { return techProps(TechType(id)); }) + ",";
      bool was = use;
      use = false;
      std::string sets = "\"sets\":{\"bwapi\":{\"units\":" + ids(UnitTypes::allUnitTypes()) + ",\"weapons\":" + ids(WeaponTypes::allWeaponTypes()) +
        ",\"normalWeapons\":" + ids(WeaponTypes::normalWeaponTypes()) + ",\"upgrades\":" + ids(UpgradeTypes::allUpgradeTypes()) + ",\"techs\":" + ids(TechTypes::allTechTypes()) + "}";
      use = was;
      sets += ",\"game\":{\"units\":" + ids(UnitTypes::allUnitTypes()) + ",\"weapons\":" + ids(WeaponTypes::allWeaponTypes()) +
        ",\"normalWeapons\":" + ids(WeaponTypes::normalWeaponTypes()) + ",\"upgrades\":" + ids(UpgradeTypes::allUpgradeTypes()) + ",\"techs\":" + ids(TechTypes::allTechTypes()) + "}}";
      // A mod's types past 1.16.1's, as the game's data gives them (the names and what a bot reads first).
      std::string extra = ",\"beyond\":[";
      bool first = true;
      for ( int id = UnitTypes::Enum::None + 1; id < (int)game.units.size(); ++id )
      {
        if ( game.units[id].name.empty() ) continue;
        UnitType u(id);
        extra += (first ? "" : ",") + std::string("{\"id\":") + std::to_string(id) + ",\"name\":" + quote(u.getName()) + ",\"mineralPrice\":" + num(u.mineralPrice()) +
          ",\"gasPrice\":" + num(u.gasPrice()) + ",\"maxHitPoints\":" + num(u.maxHitPoints()) + ",\"whatBuilds\":" + num(u.whatBuilds().first.getID()) + "}";
        first = false;
      }
      extra += "]";
      return s + sets + extra + "}";
    }
  }
}
