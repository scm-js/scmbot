#include "PlayerImpl.h"
#include "GameImpl.h"
#include "UnitImpl.h"

#include <algorithm>
#include <initializer_list>
#include <string>
#include <Util/Convenience.h>

#include <BW/Offsets.h>

#include <BWAPI/PlayerType.h>

#include <Debug.h>

namespace BWAPI
{
  //------------------------------------------- THE GAME'S TYPES ---------------------------------------------
  // scm-sim: as wide as the game's data (PlayerImpl.h); the per-frame counts are zeroed with `self`'s
  void PlayerImpl::ScmsimWide::size(int units, int upgrades, int techs)
  {
    for (auto *v : { &all, &visible, &completed, &dead, &killed }) if ( (int)v->size() != units ) v->assign(units, 0);
    for (auto *v : { &upgradeLevel, &maxUpgradeLevel }) if ( (int)v->size() != upgrades ) v->assign(upgrades, 0);
    for (auto *v : { &upgrading }) if ( (int)v->size() != upgrades ) v->assign(upgrades, 0);
    for (auto *v : { &researched, &researching, &researchAvailable }) if ( (int)v->size() != techs ) v->assign(techs, 0);
    if ( (int)unitAvailable.size() != units ) unitAvailable.assign(units, 0);
  }
  //--------------------------------------------- CONSTRUCTOR ------------------------------------------------
  PlayerImpl::PlayerImpl(u8 index)
    : index(index)
  {
    resetResources();
    self->color = index < 12 ? BW::BWDATA::Game.playerColorIndex[index] : Colors::Black.getID();  // scm-sim: clang wants one type
  }
  //--------------------------------------------- SET ID -----------------------------------------------------
  void PlayerImpl::setID(int newID)
  {
    id = newID;
  }
  //--------------------------------------------- GET INDEX --------------------------------------------------
  int PlayerImpl::getIndex() const
  {
    return index;
  }
  //--------------------------------------------- GET NAME ---------------------------------------------------
  std::string PlayerImpl::getName() const
  {
    if ( index == 11 )
      return std::string("Neutral");
    return std::string(BW::BWDATA::Players[index].szName);
  }
  //--------------------------------------------- GET RACE ---------------------------------------------------
  BWAPI::Race PlayerImpl::getRace() const
  {
    BroodwarImpl.setLastError();
    if ( this->index < BW::PLAYABLE_PLAYER_COUNT )
    {
      Race rlast = BroodwarImpl.lastKnownRaceBeforeStart[this->index];
      if (  rlast != Races::Zerg          &&
            rlast != Races::Terran        &&
            rlast != Races::Protoss       &&
            !this->wasSeenByBWAPIPlayer   && 
            !BroodwarImpl.isFlagEnabled(Flag::CompleteMapInformation) )
      {
        BroodwarImpl.setLastError(Errors::Access_Denied);
        return Races::Unknown;
      }
    }
    return BWAPI::Race( BW::BWDATA::Players[index].nRace );
  }
  //--------------------------------------------- GET TYPE ---------------------------------------------------
  BWAPI::PlayerType PlayerImpl::getType() const
  {
    return BWAPI::PlayerType((int)(BW::BWDATA::Players[index].nType));
  }
  //--------------------------------------------- GET FORCE --------------------------------------------------
  Force PlayerImpl::getForce() const
  {
    return force;
  }
  //--------------------------------------------- IS ALLIES WITH ---------------------------------------------
  bool PlayerImpl::isAlly(const Player player) const
  {
    if ( !player || this->isNeutral() || player->isNeutral() || this->isObserver() || player->isObserver() )
      return false;
    return BW::BWDATA::Game.playerAlliances[index][ static_cast<PlayerImpl*>(player)->getIndex() ] != 0;
  }
  //--------------------------------------------- IS ALLIES WITH ---------------------------------------------
  bool PlayerImpl::isEnemy(const Player player) const
  {
    if ( !player || this->isNeutral() || player->isNeutral() || this->isObserver() || player->isObserver() )
      return false;
    return BW::BWDATA::Game.playerAlliances[index][ static_cast<PlayerImpl*>(player)->getIndex() ] == 0;
  }
  //--------------------------------------------- IS NEUTRAL -------------------------------------------------
  bool PlayerImpl::isNeutral() const
  {
    return index == 11;
  }
  //--------------------------------------------- GET START POSITION -----------------------------------------
  TilePosition PlayerImpl::getStartLocation() const
  {
    // Clear last error
    BroodwarImpl.setLastError();

    // Return None if there is no start location
    if (index >= BW::PLAYABLE_PLAYER_COUNT || BW::BWDATA::Game.startPositions[index] == BW::Positions::Origin)
      return TilePositions::None;

    // Return unknown and set Access_Denied if the start location
    // should not be made available.
    if ( !BroodwarImpl.isReplay() &&
       BroodwarImpl.self()->isEnemy(const_cast<PlayerImpl*>(this)) &&
       !BroodwarImpl.isFlagEnabled(Flag::CompleteMapInformation) )
    {
      BroodwarImpl.setLastError(Errors::Access_Denied);
      return TilePositions::Unknown;
    }
    // return the start location as a tile position
    return TilePosition(BW::BWDATA::Game.startPositions[index] - BW::Position((TILE_SIZE * 4) / 2, (TILE_SIZE * 3) / 2));
  }
  //--------------------------------------------- IS VICTORIOUS ----------------------------------------------
  bool PlayerImpl::isVictorious() const
  {
    if ( index >= 8 ) 
      return false;
    return BW::BWDATA::PlayerVictory[index] == 3;
  }
  //--------------------------------------------- IS DEFEATED ------------------------------------------------
  bool PlayerImpl::isDefeated() const
  {
    if ( index >= 8 ) 
      return false;
    return BW::BWDATA::PlayerVictory[index] == 1 ||
           BW::BWDATA::PlayerVictory[index] == 2 ||
           BW::BWDATA::PlayerVictory[index] == 4 ||
           BW::BWDATA::PlayerVictory[index] == 6;
  }
  //--------------------------------------------- UPDATE -----------------------------------------------------
  void PlayerImpl::updateData()
  { 
    self->color = index < BW::PLAYER_COUNT ? BW::BWDATA::Game.playerColorIndex[index] : Colors::Black.getID();  // scm-sim: clang wants one type
  
    // Get upgrades, tech, resources
    if ( this->isNeutral() || 
      index >= BW::PLAYER_COUNT ||
         (!BroodwarImpl.isReplay() && 
          BroodwarImpl.self()->isEnemy(this) && 
          !BroodwarImpl.isFlagEnabled(Flag::CompleteMapInformation)) )
    {
      self->minerals           = 0;
      self->gas                = 0;
      self->gatheredMinerals   = 0;
      self->gatheredGas        = 0;
      self->repairedMinerals   = 0;
      self->repairedGas        = 0;
      self->refundedMinerals   = 0;
      self->refundedGas        = 0;

      // Reset values
      MemZero(self->upgradeLevel);
      MemZero(self->hasResearched);
      MemZero(self->isUpgrading);
      MemZero(self->isResearching);
    
      MemZero(self->maxUpgradeLevel);
      MemZero(self->isResearchAvailable);
      MemZero(self->isUnitAvailable);
      scmsimWide.size(BW::layout.unitTypes, BW::layout.upgradeTypes, BW::layout.techTypes);
      for (auto *v : { &scmsimWide.upgradeLevel, &scmsimWide.maxUpgradeLevel }) std::fill(v->begin(), v->end(), 0);
      for (auto *v : { &scmsimWide.upgrading, &scmsimWide.researched, &scmsimWide.researching, &scmsimWide.researchAvailable, &scmsimWide.unitAvailable }) std::fill(v->begin(), v->end(), 0);

      if (!this->isNeutral() && index < BW::PLAYER_COUNT)
      {
        // set upgrade level for visible enemy units
        // scm-sim: every upgrade of the game's data, where the game keeps the levels (BW/Layout.h); 4.4's in `self`
        for(int i = 0; i < BW::layout.upgradeTypes; ++i)
        {
          for(UnitType t : UpgradeType(i).whatUses())
          {
            if ( this->completedUnitCount(t) > 0 )
            {
              if ( i < BW::UPGRADE_TYPE_COUNT ) self->upgradeLevel[i] = BW::layout.upgradeLevel(i, index);
              scmsimWide.upgradeLevel[i] = BW::layout.upgradeLevel(i, index);
            }
          }
        }
      }
    }
    else
    {
      this->wasSeenByBWAPIPlayer = true;

      // set resources
      self->minerals           = BW::BWDATA::Game.minerals[index];
      self->gas                = BW::BWDATA::Game.gas[index];
      self->gatheredMinerals   = BW::BWDATA::Game.cumulativeMinerals[index];
      self->gatheredGas        = BW::BWDATA::Game.cumulativeGas[index];
      self->repairedMinerals   = this->_repairedMinerals;
      self->repairedGas        = this->_repairedGas;
      self->refundedMinerals   = this->_refundedMinerals;
      self->refundedGas        = this->_refundedGas;

      // scm-sim: the levels, techs and availability where the game keeps them (BW/Layout.h: a plugin may have moved and
      // widened them), every id of the game's data: 4.4's ids in `self`, all of them in `scmsimWide`
      scmsimWide.size(BW::layout.unitTypes, BW::layout.upgradeTypes, BW::layout.techTypes);
      // set upgrade level
      for(int i = 0; i < BW::layout.upgradeTypes; ++i)
      {
        int level = BW::layout.upgradeLevel(i, index), most = BW::layout.upgradeMax(i, index);
        if ( i < BW::UPGRADE_TYPE_COUNT ) { self->upgradeLevel[i] = level; self->maxUpgradeLevel[i] = most; }
        scmsimWide.upgradeLevel[i] = level; scmsimWide.maxUpgradeLevel[i] = most;
      }

      // set abilities researched
      for(int i = 0; i < BW::layout.techTypes; ++i)
      {
        bool researched = (TechType(i).whatResearches() == UnitTypes::None ? true : BW::layout.researched(i, index));
        bool available  = BW::layout.researchAvailable(i, index);
        if ( i < BW::TECH_TYPE_COUNT ) { self->hasResearched[i] = researched; self->isResearchAvailable[i] = available; }
        scmsimWide.researched[i] = researched; scmsimWide.researchAvailable[i] = available;
      }

      // set upgrades in progress
      for (int i = 0; i < BW::layout.upgradeTypes; ++i)
      {
        bool on = BW::layout.isUpgrading(i, index);
        if ( i < BW::UPGRADE_TYPE_COUNT ) self->isUpgrading[i] = on;
        scmsimWide.upgrading[i] = on;
      }
      
      // set research in progress
      for (int i = 0; i < BW::layout.techTypes; ++i)
      {
        bool on = BW::layout.isResearching(i, index);
        if ( i < BW::TECH_TYPE_COUNT ) self->isResearching[i] = on;
        scmsimWide.researching[i] = on;
      }

      for (int i = 0; i < BW::layout.unitTypes; ++i)
      {
        bool on = BW::layout.available(i, index);
        if ( i < BW::UNIT_TYPE_COUNT ) self->isUnitAvailable[i] = on;
        scmsimWide.unitAvailable[i] = on;
      }

      self->hasResearched[Wire::techType(TechTypes::Enum::Nuclear_Strike)] = self->isUnitAvailable[UnitTypes::Enum::Terran_Nuclear_Missile];
    }

    // Get Scores, supply
    if ( (!BroodwarImpl.isReplay() && 
          BroodwarImpl.self()->isEnemy(this) && 
          !BroodwarImpl.isFlagEnabled(Flag::CompleteMapInformation)) ||
          index >= BW::PLAYER_COUNT)
    {
      MemZero(self->supplyTotal);
      MemZero(self->supplyUsed);
      MemZero(self->deadUnitCount);
      MemZero(self->killedUnitCount);
      std::fill(scmsimWide.dead.begin(), scmsimWide.dead.end(), 0); // scm-sim: the game's types (PlayerImpl.h)
      std::fill(scmsimWide.killed.begin(), scmsimWide.killed.end(), 0);

      self->totalUnitScore      = 0;
      self->totalKillScore      = 0;
      self->totalBuildingScore  = 0;
      self->totalRazingScore    = 0;
      self->customScore         = 0;
    }
    else
    {
      // set supply
      for (u8 i = 0; i < BW::RACE_COUNT; ++i)
      {
        self->supplyTotal[i]  = BW::BWDATA::Game.supplies[i].available[index];
        if (self->supplyTotal[i] > BW::BWDATA::Game.supplies[i].max[index])
          self->supplyTotal[i]  = BW::BWDATA::Game.supplies[i].max[index];
        self->supplyUsed[i]   = BW::BWDATA::Game.supplies[i].used[index];
      }
      // set total unit counts
      // scm-sim: where the game keeps the deaths and kills (BW/Layout.h), every type of its data
      scmsimWide.size(BW::layout.unitTypes, BW::layout.upgradeTypes, BW::layout.techTypes);
      for (int i = 0; i < BW::layout.unitTypes; ++i)
      {
        int dead = BW::layout.count(BW::layout.countsDead, i, index), killed = BW::layout.count(BW::layout.countsKilled, i, index);
        if ( i < BW::UNIT_TYPE_COUNT ) { self->deadUnitCount[i] = dead; self->killedUnitCount[i] = killed; }
        scmsimWide.dead[i] = dead; scmsimWide.killed[i] = killed;
      }
      // set macro dead unit counts
      self->deadUnitCount[Wire::unitType(UnitTypes::AllUnits)]    = BW::BWDATA::Game.allUnitsLost[index] + BW::BWDATA::Game.allBuildingsLost[index];
      self->deadUnitCount[Wire::unitType(UnitTypes::Men)]         = BW::BWDATA::Game.allUnitsLost[index];
      self->deadUnitCount[Wire::unitType(UnitTypes::Buildings)]   = BW::BWDATA::Game.allBuildingsLost[index];
      self->deadUnitCount[Wire::unitType(UnitTypes::Factories)]   = BW::BWDATA::Game.allFactoriesLost[index];

      // set macro kill unit counts
      self->killedUnitCount[Wire::unitType(UnitTypes::AllUnits)]  = BW::BWDATA::Game.allUnitsKilled[index] + BW::BWDATA::Game.allBuildingsRazed[index];
      self->killedUnitCount[Wire::unitType(UnitTypes::Men)]       = BW::BWDATA::Game.allUnitsKilled[index];
      self->killedUnitCount[Wire::unitType(UnitTypes::Buildings)] = BW::BWDATA::Game.allBuildingsRazed[index];
      self->killedUnitCount[Wire::unitType(UnitTypes::Factories)] = BW::BWDATA::Game.allFactoriesRazed[index];
      
      // set score counts
      self->totalUnitScore      = BW::BWDATA::Game.allUnitScore[index];
      self->totalKillScore      = BW::BWDATA::Game.allKillScore[index];
      self->totalBuildingScore  = BW::BWDATA::Game.allBuildingScore[index];
      self->totalRazingScore    = BW::BWDATA::Game.allRazingScore[index];
      self->customScore         = BW::BWDATA::Game.customScore[index];
    }

    if (index < BW::PLAYER_COUNT && (BW::BWDATA::Players[index].nType == PlayerTypes::PlayerLeft ||
        BW::BWDATA::Players[index].nType == PlayerTypes::ComputerLeft ||
       (BW::BWDATA::Players[index].nType == PlayerTypes::Neutral && !isNeutral())))
    {
      self->leftGame = true;
    }
  }
  //----------------------------------------------------------------------------------------------------------
  void PlayerImpl::onGameEnd()
  {
    this->units.clear();
    this->clientInfo.clear();
    this->interfaceEvents.clear();

    self->leftGame = false;
    this->wasSeenByBWAPIPlayer = false;
  }
  void PlayerImpl::setParticipating(bool isParticipating)
  {
    self->isParticipating = isParticipating;
  }
  void PlayerImpl::resetResources()
  {
    _repairedMinerals = 0;
    _repairedGas      = 0;
    _refundedMinerals = 0;
    _refundedGas      = 0;
  }
};
