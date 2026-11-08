// scm-sim: where the game keeps the tables BWAPI reads. What and why: Layout.h; the record: docs/bwapi-plan.md,
// "C1 and C2, as built".
#include "Layout.h"

#include <algorithm>
#include <cstring>
#include <sstream>

namespace BW
{
  Layout layout;

  namespace
  {
    // ---- The game's memory: this module's memory is the game's address space (below 2 GB) --------------------------
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
    // (Made on first use: BWAPI's game object reads the layout while the module's globals are still being made.)
    std::string &said() { static std::string s = "1.16.1's layout (no match started yet)"; return s; }
  }

  // ---- The game's values by type and player -------------------------------------------------------------------------
  int Layout::count(uint32_t table, int type, int player) const
  {
    if ( type < 0 || type >= unitTypes || player < 0 || player >= 12 ) return 0;
    return (int)rd(table + (uint32_t)type * 48 + (uint32_t)player * 4, 4);
  }
  bool Layout::available(int type, int player) const
  {
    if ( type < 0 || type >= unitTypes || player < 0 || player >= 12 ) return false;
    return rd(availability + (uint32_t)player * availabilityRow + (uint32_t)type, 1) != 0;
  }
  namespace
  {
    // A level of `id` in a table kept in two parts (the first `firstRow` ids a player, then the rest `secondRow` a player).
    uint32_t level(uint32_t first, uint32_t firstRow, uint32_t second, uint32_t secondRow, int id, int count, int player)
    {
      if ( id < 0 || id >= count || player < 0 || player >= 12 ) return 0;
      if ( (uint32_t)id < firstRow ) return rd(first + (uint32_t)player * firstRow + (uint32_t)id, 1);
      return rd(second + (uint32_t)player * secondRow + (uint32_t)id - firstRow, 1);
    }
  }
  int Layout::upgradeLevel(int u, int p) const { return (int)level(upgradesSc, upgradeScRow, upgradesBw, upgradeBwRow, u, upgradeTypes, p); }
  int Layout::upgradeMax(int u, int p) const { return (int)level(upgradesScMax, upgradeScRow, upgradesBwMax, upgradeBwRow, u, upgradeTypes, p); }
  bool Layout::researched(int t, int p) const { return level(techSc, techScRow, techBw, techBwRow, t, techTypes, p) != 0; }
  bool Layout::researchAvailable(int t, int p) const { return level(techScAvailable, techScRow, techBwAvailable, techBwRow, t, techTypes, p) != 0; }
  // (A bit an id: 64 upgrades and 48 techs a player, as the game's code reads them; an id past them is not in progress here.)
  bool Layout::isUpgrading(int u, int p) const { return u >= 0 && u < 64 && p >= 0 && p < 12 && (rd(upgrading + (uint32_t)p * 8 + (uint32_t)u / 8, 1) & (1u << (u % 8))) != 0; }
  bool Layout::isResearching(int t, int p) const { return t >= 0 && t < 48 && p >= 0 && p < 12 && (rd(researching + (uint32_t)p * 6 + (uint32_t)t / 8, 1) & (1u << (t % 8))) != 0; }

  namespace LayoutRead
  {
    const std::string &status() { return said(); }

    const std::string &read()
    {
      Layout l;   // 1.16.1's
      std::ostringstream moved;
      int n = 0;
      auto hex = [](uint32_t v) { std::ostringstream s; s << "0x" << std::hex << std::uppercase << v; return s.str(); };
      // An address from the instruction at `operand` (less `less`): a plugin that moved the table wrote it there.
      auto at = [&](const char *name, uint32_t operand, uint32_t less, uint32_t &into)
      {
        uint32_t v = rd(operand, 4) - less;
        if ( !rd(operand, 4) || v < 0x10000 || v >= 0x80000000u ) return;
        if ( v != into ) { moved << (n++ ? "; " : "") << name << " at " << hex(v); }
        into = v;
      };
      // A count or a row from the instruction at `operand` (`size` bytes; `per` of them make one).
      auto num = [&](const char *name, uint32_t operand, uint32_t size, uint32_t per, uint32_t add, uint32_t &into)
      {
        uint32_t raw = rd(operand, size);
        if ( !raw || raw % per ) return;
        uint32_t v = raw / per + add;
        if ( v == 0 || v > 0xFFFF ) return;
        if ( v != into ) { moved << (n++ ? "; " : "") << name << " " << v; }
        into = v;
      };
      // The units: `sub ecx, table` at 0x401EEE and the slot count it is checked against at 0x401F05 (as the page reads
      // them, src/exact/memory.ts OPERANDS), the use count's shift in an id (`shl edx, 11` at 0x401F18).
      at("units", 0x401EF0, 0, l.units);
      num("unit slots", 0x401F07, 4, 1, 0, l.unitSlots);
      { uint32_t s = rd(0x401F1A, 1); if ( s >= 8 && s <= 15 ) { if ( s != l.tagShift ) moved << (n++ ? "; " : "") << "a unit's id shifted by " << s; l.tagShift = s; } }
      // The unit finders: `mov edi, x` at 0x4222B8 and `mov edi, y` at 0x422194 (the finders' search, 0x469B00, called
      // with each), the length `mov eax, [count]` at 0x469B03.
      at("unit finders (x)", 0x4222B9, 0, l.finderX);
      at("unit finders (y)", 0x422195, 0, l.finderY);
      at("the finders' length", 0x469B04, 0, l.finderCount);
      // The bullets: their initialization (0x48A690), the table and its last slot (`mov edx, 99` at 0x48A6C1).
      at("bullets", 0x48A691, 0, l.bullets);
      num("bullet slots", 0x48A6C2, 4, 1, 1, l.bulletSlots);
      // The counts, where the trigger conditions and the AI read them: all (0x4363E3), completed (0x435A96), and the
      // Kill and Deaths conditions (0x46056C, 0x46044C: src/exact/copies.ts COUNT_OPERANDS).
      at("unit counts", 0x4363E6, 0, l.countsAll);
      at("completed counts", 0x435A99, 0, l.countsCompleted);
      at("kills", 0x46056F, 0, l.countsKilled);
      at("deaths", 0x46044F, 0, l.countsDead);
      // Which types a player may make: the requirement evaluator's `imul edi, edi, row` (0x46E24C) and the read after it.
      num("unit types", 0x46E24E, 4, 1, 0, l.availabilityRow);
      at("unit availability", 0x46E258, 0, l.availability);
      // Upgrade levels: the routine at 0x4033D0 reads the current level and the most of one (each part's row, `imul`,
      // and the read after it, whose operand is the part's start less its first id); src/exact/memory.ts readLevels.
      num("upgrades (first part)", 0x4033E6, 1, 1, 0, l.upgradeScRow);
      at("upgrade levels (first part)", 0x4033EA, 0, l.upgradesSc);
      at("most upgrade levels (first part)", 0x40340A, 0, l.upgradesScMax);
      num("upgrades (second part)", 0x4033F2, 1, 1, 0, l.upgradeBwRow);
      at("upgrade levels (second part)", 0x4033F6, (uint32_t)-(int32_t)l.upgradeScRow, l.upgradesBw);
      at("most upgrade levels (second part)", 0x40341C, (uint32_t)-(int32_t)l.upgradeScRow, l.upgradesBwMax);
      // Techs: the players' start (0x4CCCD0) clears researched and available, each part by its length in dwords.
      num("techs (first part)", 0x4CCD03 - 5, 4, 3, 0, l.techScRow);
      at("researched techs (first part)", 0x4CCD03, 0, l.techSc);
      at("available techs (first part)", 0x4CCCE9, 0, l.techScAvailable);
      num("techs (second part)", 0x4CCD0A, 4, 3, 0, l.techBwRow);
      at("researched techs (second part)", 0x4CCD0F, 0, l.techBw);
      at("available techs (second part)", 0x4CCCF5, 0, l.techBwAvailable);
      // What is in progress: where the game's code tests the bits (0x4281BF and 0x428252).
      at("upgrades in progress", 0x4281C2, 0, l.upgrading);
      at("research in progress", 0x428255, 0, l.researching);
      l.unitTypes = (int)l.availabilityRow;
      l.upgradeTypes = (int)(l.upgradeScRow + l.upgradeBwRow);
      l.techTypes = (int)(l.techScRow + l.techBwRow);
      layout = l;
      std::ostringstream s;
      if ( !n ) s << "1.16.1's layout";
      else s << "followed the game's pointers: " << moved.str();
      s << " (" << l.unitSlots << " unit slots, " << l.unitTypes << " unit types, " << l.upgradeTypes << " upgrades, " << l.techTypes << " techs)";
      said() = s.str();
      return said();
    }
  }
}
