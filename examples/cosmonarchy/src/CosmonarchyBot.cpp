// A bot made for Cosmonarchy BW, as small as it can be and still play: its Masons mine, the Ministry trains more of
// them, a Mason puts up a Stockade (and more of them, up to four, while minerals pile up), the Stockades train
// Mavericks, and every six Mavericks attack-move to where the enemy is: the buildings of theirs it has seen, else the
// start locations one after another.
//
// The mod's units are named in Cosmonarchy_BW.h, which `scmjs mod header` writes from the mod itself. In this folder:
//   scmjs mod header "<the Cosmonarchy release>" --out . --bot     (the header, and the mod's name and hash in scmbot.json)
//   scmbot build                                                   (out/cosmonarchy-example.scmbot)
// Built for another version of the mod, a renamed unit is an error here, and the play page and `scmjs bot run` say
// "made for Cosmonarchy BW" only when the game's mod has the hash this bot was built against.
#include <BWAPI.h>
#include <map>
#include "Cosmonarchy_BW.h"

using namespace BWAPI;
namespace cosmo = Cosmonarchy_BW;

namespace
{
  // The mod's names for the four types this bot uses (BWAPI's own Terran_SCV and Terran_Barracks mean nothing here).
  const UnitType Worker = cosmo::UnitTypes::Terran_Mason;
  const UnitType Hall = cosmo::UnitTypes::Terran_Ministry;
  const UnitType Barracks = cosmo::UnitTypes::Terran_Stockade;
  const UnitType Soldier = cosmo::UnitTypes::Terran_Maverick;
  const int WORKERS = 14, WAVE = 6, STOCKADES = 4;
}

class CosmonarchyBot : public AIModule
{
  Position home;
  std::map<Unit, Position> enemyBuildings;   // the enemy's buildings as last seen
  size_t nextStart = 0;                      // the start location to look at next, while no building is known
  int buildOrdered = -10000;                 // when the Stockade was last asked for
  bool attacking = false;

public:
  void onStart() override
  {
    Broodwar->sendText("made for %s (%s)", cosmo::MOD_NAME, cosmo::MOD_HASH);
    Broodwar->setCommandOptimizationLevel(2);
    home = Position(Broodwar->self()->getStartLocation());
    if (Broodwar->self()->getRace() != Races::Terran) Broodwar->sendText("I play Cosmonarchy's terrans only: my workers will mine, nothing more");
  }

  void onFrame() override
  {
    // Commands go out every few frames (the latency): asking more often only repeats them.
    if (Broodwar->isPaused() || Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) return;
    Player me = Broodwar->self();
    Unitset workers, soldiers;
    Unitset barracks;
    Unit hall = nullptr;
    int stockades = 0;           // standing or going up
    bool underway = false;       // one going up (or a Mason on its way to build one)
    for (Unit u : me->getUnits())
    {
      UnitType t = u->getType();
      if (t.isWorker() && u->isCompleted()) workers.insert(u);   // (Masons; another race's workers mine too)
      else if (t == Hall && u->isCompleted()) hall = u;
      else if (t == Barracks) { ++stockades; if (u->isCompleted()) barracks.insert(u); else underway = true; }
      else if (t == Soldier && u->isCompleted()) soldiers.insert(u);
      if (t == Worker && u->getBuildType() == Barracks && !u->getBuildUnit()) underway = true;
    }

    // Idle Masons mine.
    for (Unit w : workers)
      if (w->isIdle()) w->gather(w->getClosestUnit(Filter::IsMineralField));

    // The Ministry trains Masons up to WORKERS.
    if (hall && hall->isIdle() && (int)workers.size() < WORKERS && me->minerals() >= Worker.mineralPrice())
      hall->train(Worker);

    // A Stockade, and another while minerals pile up, one at a time, by a Mason that is not carrying anything (asked
    // again if it has not started in 20 seconds).
    bool wanted = stockades == 0 || (stockades < STOCKADES && me->minerals() >= 2 * Barracks.mineralPrice());
    if (me->getRace() == Races::Terran && wanted && !underway && me->minerals() >= Barracks.mineralPrice() && Broodwar->getFrameCount() > buildOrdered + 480)
    {
      Unit builder = Broodwar->getClosestUnit(home, Filter::IsOwned && Filter::GetType == Worker && Filter::IsGatheringMinerals && !Filter::IsCarryingMinerals);
      TilePosition at = builder ? Broodwar->getBuildLocation(Barracks, builder->getTilePosition()) : TilePositions::Invalid;
      if (at != TilePositions::Invalid && builder->build(Barracks, at)) buildOrdered = Broodwar->getFrameCount();
    }

    // Each idle Stockade trains a Maverick while there are minerals to spare after the Masons.
    int purse = me->minerals() - ((int)workers.size() < WORKERS ? Worker.mineralPrice() : 0);
    for (Unit b : barracks)
      if (b->isIdle() && purse >= Soldier.mineralPrice() && b->train(Soldier)) purse -= Soldier.mineralPrice();

    // Every WAVE Mavericks go, and from then on every idle one: to a building of the enemy's, else to look for one.
    Unitset idle;
    for (Unit s : soldiers) if (s->isIdle()) idle.insert(s);
    if ((attacking || (int)idle.size() >= WAVE) && !idle.empty())
    {
      attacking = true;
      idle.attack(target());
    }
  }

  // Where to attack: the nearest enemy building seen, else the next start location that is not ours.
  Position target()
  {
    Position best = Positions::None;
    for (auto &seen : enemyBuildings)
      if (best == Positions::None || home.getApproxDistance(seen.second) < home.getApproxDistance(best)) best = seen.second;
    if (best != Positions::None) return best;
    const TilePosition::list &starts = Broodwar->getStartLocations();
    for (size_t i = 0; i < starts.size(); ++i)
    {
      TilePosition t = starts[(nextStart + i) % starts.size()];
      if (t == Broodwar->self()->getStartLocation()) continue;
      nextStart = (nextStart + i + 1) % starts.size();
      return Position(t) + Position(64, 48);
    }
    return home;
  }

  void onUnitShow(Unit unit) override
  {
    if (unit->getType().isBuilding() && Broodwar->self()->isEnemy(unit->getPlayer())) enemyBuildings[unit] = unit->getPosition();
  }

  void onUnitDestroy(Unit unit) override { enemyBuildings.erase(unit); }

  void onEnd(bool isWinner) override
  {
    Broodwar->sendText(isWinner ? "gg: a win for %s" : "gg: a loss for %s", cosmo::MOD_NAME);
  }
};

// The two functions BWAPI looks for in an AI module.
extern "C" void gameInit(Game *game) { BroodwarPtr = game; }
extern "C" AIModule *newAIModule() { return new CosmonarchyBot(); }
