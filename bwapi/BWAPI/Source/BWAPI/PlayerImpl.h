#pragma once
#include <Util/Types.h>
#include <string>
#include <vector>

#include <BWAPI/Player.h>
#include <BWAPI/Client/PlayerData.h>
#include <BWAPI/Unitset.h>

#include "UnitImpl.h"
#include "ForceImpl.h"

namespace BWAPI
{
  // Forwards
  class ForceInterface;
  typedef ForceInterface* Force;
  class UnitType;
  class UpgradeType;
  class TechType;
  class PlayerType;
  class Race;

  /** Represents one player in game. Note that there is always player 12 who owns resources. */
  class PlayerImpl : public PlayerInterface
  {
    public:
      virtual int         getID() const override;
      virtual std::string getName() const override;
      virtual Race        getRace() const override;
      virtual PlayerType  getType() const override;
      virtual Force       getForce() const override;

      virtual bool isAlly(const Player player) const override;
      virtual bool isEnemy(const Player player) const override;
      virtual bool isNeutral() const override;

      virtual bool isVictorious() const override;
      virtual bool isDefeated() const override;
      virtual bool leftGame() const override;

      virtual const Unitset &getUnits() const override;
      virtual TilePosition  getStartLocation() const override;

      virtual int  minerals() const override;
      virtual int  gas() const override;

      virtual int  gatheredMinerals() const override;
      virtual int  gatheredGas() const override;
      virtual int  repairedMinerals() const override;
      virtual int  repairedGas() const override;
      virtual int  refundedMinerals() const override;
      virtual int  refundedGas() const override;
      virtual int  spentMinerals() const override;
      virtual int  spentGas() const override;

      virtual int  supplyTotal(Race race = Races::None) const override;
      virtual int  supplyUsed(Race race = Races::None) const override;

      virtual int  allUnitCount(UnitType unit) const override;
      virtual int  visibleUnitCount(UnitType unit) const override;
      virtual int  completedUnitCount(UnitType unit) const override;
      virtual int  deadUnitCount(UnitType unit) const override;
      virtual int  killedUnitCount(UnitType unit) const override;

      virtual int  getUpgradeLevel(UpgradeType upgrade) const override;
      virtual bool hasResearched(TechType tech) const override;
      virtual bool isResearching(TechType tech) const override;
      virtual bool isUpgrading(UpgradeType upgrade) const override;

      virtual BWAPI::Color getColor() const override;

      virtual int getUnitScore() const override;
      virtual int getKillScore() const override;
      virtual int getBuildingScore() const override;
      virtual int getRazingScore() const override;
      virtual int getCustomScore() const override;

      virtual bool isObserver() const override;

      virtual int  getMaxUpgradeLevel(UpgradeType upgrade) const override;
      virtual bool isResearchAvailable(TechType tech) const override;
      virtual bool isUnitAvailable(UnitType unit) const override;

      //Internal BWAPI commands:
      /**
       * Constructs new player
       * @param id 0-based Index of the player (11 for resources as it is player 12)
       */
      PlayerImpl(u8 index);
      int getIndex() const;      // Gets 0-based index of the player. (11 for neutral)


      void setID(int newID);

      /**
       * Updates localData according to bw values. (Should be called every frame before local data updates are
       * performed
       */
      void updateData();
      void onGameEnd();

      void setParticipating(bool isParticipating = true);
      void resetResources();
      
    // data members
      ForceImpl*  force = nullptr;
      PlayerData  data = PlayerData();
      PlayerData* self = &data;
      Unitset     units;

      s32 _repairedMinerals;
      s32 _repairedGas;
      s32 _refundedMinerals;
      s32 _refundedGas;

      bool wasSeenByBWAPIPlayer = false;

      // scm-sim: what `self` (PlayerData: the client protocol, at BWAPI 4.4's widths) has no place for. By the game's
      // type id, as wide as the game's data (BW/Layout.h: a mod's types past 1.16.1's), and read where 4.4 had no number
      // for an id (docs/bwapi-plan.md, "C1 and C2, as built").
      struct ScmsimWide
      {
        std::vector<int> all, visible, completed, dead, killed;   // units
        std::vector<int> upgradeLevel, maxUpgradeLevel;           // upgrades
        std::vector<char> upgrading, researched, researching, researchAvailable, unitAvailable;
        static int at(const std::vector<int> &v, int id) { return id >= 0 && id < (int)v.size() ? v[id] : 0; }
        static bool on(const std::vector<char> &v, int id) { return id >= 0 && id < (int)v.size() && v[id] != 0; }
        /// Sized for the game's types; the per-frame counts zeroed.
        void size(int units, int upgrades, int techs);
      } scmsimWide;
    private :
      int id = -1;
      u8 index;  /**< Order of the player, is used to load player's information from the memory */
  };
};
