// scm-sim: where the game keeps the tables BWAPI reads (scm-sim's docs/bwapi-plan.md, "C1 and C2, as built").
//
// BWAPI 4.4 reads the game at 1.16.1's fixed addresses (Offsets.h). A plugin can move a table and widen it: GPTP's
// limit extender gives the units a table of 4,095 slots of its own, the unit finders and the bullets theirs, and its
// DatExtend gives the unit counts, the deaths and kills, the unit availability and the upgrade and tech levels tables as
// wide as the mod's data. Such a plugin rewrites the operand of every instruction that uses the table, so the game's
// own code says where each one is now. `read()` takes that word from the instruction BWAPI's table is used by (the
// same instructions the play page follows: src/exact/memory.ts `OPERANDS`, `LEVEL_OPERANDS`, src/exact/copies.ts
// `COUNT_OPERANDS`), at the start of each match; a value that is not plausible (outside 64 KB to 2 GB, a count of none
// or past 65,535) is taken as 1.16.1's. With nothing moved every entry is 1.16.1's own, and BWAPI reads what it read.
#pragma once
#include <cstdint>
#include <string>

namespace BW
{
  class  CUnit;
  class  CBullet;
  struct unitFinder;

  struct Layout
  {
    /// The unit table, its slots and the shift of a slot's use count in the 16-bit id a command names a unit by.
    uint32_t units = 0x0059CCA8, unitSlots = 1700, tagShift = 11;
    /// The unit finders (two sorted lists, x and y), and where their length is.
    uint32_t finderX = 0x0066FF78, finderY = 0x006769B8, finderCount = 0x0066FF74;
    /// The bullets.
    uint32_t bullets = 0x0064B2E8, bulletSlots = 100;
    /// The per-type unit counts, `[type * 48 + player * 4]`: all, completed, killed (kills), dead (deaths).
    uint32_t countsAll = 0x00582324, countsCompleted = 0x00584DE4, countsKilled = 0x005878A4, countsDead = 0x0058A364;
    /// Which types each player may make, `[player * row + type]`.
    uint32_t availability = 0x0057F27C, availabilityRow = 228;
    /// The upgrade levels: 0–45 in one part a player (`row` ids), 46 on in another; the current levels and the most.
    uint32_t upgradesSc = 0x0058D2B0, upgradesScMax = 0x0058D088, upgradeScRow = 46;
    uint32_t upgradesBw = 0x0058F32C, upgradesBwMax = 0x0058F278, upgradeBwRow = 15;
    /// The techs: researched and available, 0–23 in one part, 24 on in another.
    uint32_t techSc = 0x0058CF44, techScAvailable = 0x0058CE24, techScRow = 24;
    uint32_t techBw = 0x0058F140, techBwAvailable = 0x0058F050, techBwRow = 20;
    /// What is being upgraded and researched: a bit an id, 8 and 6 bytes a player.
    uint32_t upgrading = 0x0058F3E0, researching = 0x0058F230;
    /// The data set's widths as the game's code has them: unit types (the availability row), upgrades and techs (the
    /// two parts' rows together).
    int unitTypes = 228, upgradeTypes = 61, techTypes = 44;

    /// The mask of a slot in a unit's 16-bit id.
    uint32_t slotMask() const { return (1u << tagShift) - 1; }
    CUnit *unitTable() const { return reinterpret_cast<CUnit *>(static_cast<uintptr_t>(units)); }
    CBullet *bulletTable() const { return reinterpret_cast<CBullet *>(static_cast<uintptr_t>(bullets)); }
    unitFinder *finders(bool y) const { return reinterpret_cast<unitFinder *>(static_cast<uintptr_t>(y ? finderY : finderX)); }
    int finderLength() const { return *reinterpret_cast<const int *>(static_cast<uintptr_t>(finderCount)); }

    // The game's values by type and player, 0 where the table has no such place.
    int count(uint32_t table, int type, int player) const;
    bool available(int type, int player) const;
    int upgradeLevel(int upgrade, int player) const;
    int upgradeMax(int upgrade, int player) const;
    bool researched(int tech, int player) const;
    bool researchAvailable(int tech, int player) const;
    bool isUpgrading(int upgrade, int player) const;
    bool isResearching(int tech, int player) const;
  };

  /// The layout of this match (1.16.1's until `read`).
  extern Layout layout;
  namespace LayoutRead
  {
    /// Reads where the game's code says its tables are; at the start of each match, before BWAPI's tables are made.
    /// Returns the layout's words, as `status` keeps them.
    const std::string &read();
    /// What `read` found, in words: "1.16.1's layout", or which tables moved and where.
    const std::string &status();
  }
}
