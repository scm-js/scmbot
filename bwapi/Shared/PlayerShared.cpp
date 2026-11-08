#include "PlayerImpl.h"

#include <BWAPI/Color.h>
#include <BWAPI/WeaponType.h>

namespace BWAPI
{
  //--------------------------------------------- GET ID -----------------------------------------------------
  int PlayerImpl::getID() const
  {
    return id;
  }
  //--------------------------------------------- GET UNITS --------------------------------------------------
  const Unitset &PlayerImpl::getUnits() const
  {
    return this->units;
  }
  //--------------------------------------------- MINERALS ---------------------------------------------------
  int PlayerImpl::minerals() const
  {
    return self->minerals;
  }
  //--------------------------------------------- GAS --------------------------------------------------------
  int PlayerImpl::gas() const
  {
    return self->gas;
  }
  //--------------------------------------------- GATHERED MINERALS ------------------------------------------
  int PlayerImpl::gatheredMinerals() const
  {
    return self->gatheredMinerals;
  }
  //--------------------------------------------- GATHERED GAS -----------------------------------------------
  int PlayerImpl::gatheredGas() const
  {
    return self->gatheredGas;
  }
  //--------------------------------------------- REPAIRED MINERALS ------------------------------------------
  int PlayerImpl::repairedMinerals() const
  {
    return self->repairedMinerals;
  }
  //--------------------------------------------- REPAIRED GAS -----------------------------------------------
  int PlayerImpl::repairedGas() const
  {
    return self->repairedGas;
  }
  //--------------------------------------------- REFUNDED MINERALS ------------------------------------------
  int PlayerImpl::refundedMinerals() const
  {
    return self->refundedMinerals;
  }
  //--------------------------------------------- REFUNDED GAS -----------------------------------------------
  int PlayerImpl::refundedGas() const
  {
    return self->refundedGas;
  }
  //--------------------------------------------- SPENT MINERALS ---------------------------------------------
  int PlayerImpl::spentMinerals() const
  {
    return self->gatheredMinerals + self->refundedMinerals - self->minerals - self->repairedMinerals;
  }
  //--------------------------------------------- SPENT GAS --------------------------------------------------
  int PlayerImpl::spentGas() const
  {
    return self->gatheredGas + self->refundedGas - self->gas - self->repairedGas;
  }
  //--------------------------------------------- SUPPLY TOTAL -----------------------------------------------
  int PlayerImpl::supplyTotal(Race race) const
  {
    if ( race == Races::None )  // Get current race's supply if None is specified
      race = this->getRace();

    if (static_cast<unsigned>(race) < std::extent<decltype(self->supplyTotal)>::value)
      return self->supplyTotal[race];
    return 0;
  }
  //--------------------------------------------- SUPPLY USED ------------------------------------------------
  int PlayerImpl::supplyUsed(Race race) const
  {
    if ( race == Races::None )  // Get current race's supply if None is specified
      race = this->getRace();

    if (static_cast<unsigned>(race) < std::extent<decltype(self->supplyUsed)>::value)
      return self->supplyUsed[race];
    return 0;
  }
  //--------------------------------------------- ALL UNIT COUNT ---------------------------------------------
  int PlayerImpl::allUnitCount(UnitType unit) const
  {
    return Wire::unitType(unit) >= 0 ? self->allUnitCount[Wire::unitType(unit)] : 0; // scm-sim: 4.4's numbering (PlayerData.h)
  }
  //--------------------------------------------- VISIBLE UNIT COUNT -----------------------------------------
  int PlayerImpl::visibleUnitCount(UnitType unit) const
  {
    return Wire::unitType(unit) >= 0 ? self->visibleUnitCount[Wire::unitType(unit)] : 0; // scm-sim: 4.4's numbering (PlayerData.h)
  }
  //--------------------------------------------- COMPLETED UNIT COUNT ---------------------------------------
  int PlayerImpl::completedUnitCount(UnitType unit) const
  {
    return Wire::unitType(unit) >= 0 ? self->completedUnitCount[Wire::unitType(unit)] : 0; // scm-sim: 4.4's numbering (PlayerData.h)
  }
  //--------------------------------------------- DEAD UNIT COUNT --------------------------------------------
  int PlayerImpl::deadUnitCount(UnitType unit) const
  {
    return Wire::unitType(unit) >= 0 ? self->deadUnitCount[Wire::unitType(unit)] : 0; // scm-sim: 4.4's numbering (PlayerData.h)
  }
  //--------------------------------------------- KILLED UNIT COUNT ------------------------------------------
  int PlayerImpl::killedUnitCount(UnitType unit) const
  {
    return Wire::unitType(unit) >= 0 ? self->killedUnitCount[Wire::unitType(unit)] : 0; // scm-sim: 4.4's numbering (PlayerData.h)
  }
  //--------------------------------------------------- SCORE ------------------------------------------------
  int PlayerImpl::getUnitScore() const
  {
    return self->totalUnitScore;
  }
  int PlayerImpl::getKillScore() const
  {
    return self->totalKillScore;
  }
  int PlayerImpl::getBuildingScore() const
  {
    return self->totalBuildingScore;
  }
  int PlayerImpl::getRazingScore() const
  {
    return self->totalRazingScore;
  }
  int PlayerImpl::getCustomScore() const
  {
    return self->customScore;
  }
  //--------------------------------------------- GET UPGRADE LEVEL ------------------------------------------
  int PlayerImpl::getUpgradeLevel(UpgradeType upgrade) const
  {
    return Wire::upgradeType(upgrade) >= 0 ? self->upgradeLevel[Wire::upgradeType(upgrade)] : 0; // scm-sim: 4.4's numbering
  }
  //--------------------------------------------- HAS RESEARCHED ---------------------------------------------
  bool PlayerImpl::hasResearched(TechType tech) const
  {
    return Wire::techType(tech) >= 0 ? self->hasResearched[Wire::techType(tech)] : false; // scm-sim: 4.4's numbering
  }
  //--------------------------------------------- IS RESEARCHING ---------------------------------------------
  bool PlayerImpl::isResearching(TechType tech) const
  {
    return Wire::techType(tech) >= 0 ? self->isResearching[Wire::techType(tech)] : false; // scm-sim: 4.4's numbering
  }
  //--------------------------------------------- IS UPGRADING -----------------------------------------------
  bool PlayerImpl::isUpgrading(UpgradeType upgrade) const
  {
    return Wire::upgradeType(upgrade) >= 0 ? self->isUpgrading[Wire::upgradeType(upgrade)] : false; // scm-sim: 4.4's numbering
  }
  //-------------------------------------------------- COLOUR ------------------------------------------------
  BWAPI::Color PlayerImpl::getColor() const
  {
    return BWAPI::Color(self->color);
  }
  //------------------------------------------------- OBSERVER -----------------------------------------------
  bool PlayerImpl::isObserver() const
  {
    return !self->isParticipating;
  }
  //------------------------------------------------ AVAILABILITY --------------------------------------------
  int PlayerImpl::getMaxUpgradeLevel(UpgradeType upgrade) const
  {
    return Wire::upgradeType(upgrade) >= 0 ? self->maxUpgradeLevel[Wire::upgradeType(upgrade)] : 0; // scm-sim: 4.4's numbering
  }
  bool PlayerImpl::isResearchAvailable(TechType tech) const
  {
    return Wire::techType(tech) >= 0 ? self->isResearchAvailable[Wire::techType(tech)] : false; // scm-sim: 4.4's numbering
  }
  bool PlayerImpl::isUnitAvailable(UnitType unit) const
  {
    return Wire::unitType(unit) >= 0 ? self->isUnitAvailable[Wire::unitType(unit)] : false; // scm-sim: 4.4's numbering
  }
  //--------------------------------------------- LEFT GAME --------------------------------------------------
  bool PlayerImpl::leftGame() const
  {
    return self->leftGame;
  }
}

