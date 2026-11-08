#include "UnitTarget.h"

#include "Offsets.h"

#include <BWAPI/UnitImpl.h>
#include <BW/CUnit.h>

#include <Debug.h>

namespace BW
{
  //---------------------------------------------- CONSTRUCTOR -----------------------------------------------
  UnitTarget::UnitTarget() {}

  UnitTarget::UnitTarget(const BWAPI::Unit target)
  {
    BW::CUnit* bwTarget = static_cast<BWAPI::UnitImpl*>(target)->getOriginalRawData;
    u16 unitID = static_cast<u16>((bwTarget - BWDATA::UnitNodeTable.data()) + 1);
    if (unitID <= layout.unitSlots) // scm-sim: the game's slots and its id's shift (Layout.h)
    {
      this->targetID = static_cast<u16>(unitID | (bwTarget->uniquenessIdentifier << layout.tagShift));
    }
  }
  UnitTarget::UnitTarget(const BW::CUnit* target)
  {
    u16 unitID = static_cast<u16>((target - BWDATA::UnitNodeTable.data()) + 1);
    if (unitID <= layout.unitSlots) // scm-sim: the game's slots and its id's shift (Layout.h)
    {
      this->targetID = static_cast<u16>(unitID | (target->uniquenessIdentifier << layout.tagShift));
    }
  }
  //-------------------------------------------------- GETTARGET ---------------------------------------------
  u16 UnitTarget::getTarget() const
  {
    return this->targetID;
  }
  //----------------------------------------------------------------------------------------------------------

}
