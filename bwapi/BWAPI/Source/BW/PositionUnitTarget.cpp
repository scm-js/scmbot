#include "PositionUnitTarget.h"
#include "CUnit.h"

#include <Debug.h>
#include "Offsets.h"

namespace BW
{
  //---------------------------------------------- CONSTRUCTOR -----------------------------------------------
  PositionUnitTarget::PositionUnitTarget(const Position& position)
      : position(position)
  {
  }
  PositionUnitTarget::PositionUnitTarget(int x, int y)
    : position(static_cast<s16>(x), static_cast<s16>(y))
  {
  }
  //---------------------------------------------- CONSTRUCTOR -----------------------------------------------
  PositionUnitTarget::PositionUnitTarget(const UnitTarget& target)
      : target(target)
  {
    int index = (target.getTarget() & layout.slotMask()) - 1; // scm-sim: the game's id (Layout.h)
    CUnit* unit = &BWDATA::UnitNodeTable[index];
    this->position = unit->position;
  }
  //----------------------------------------------------------------------------------------------------------
}

