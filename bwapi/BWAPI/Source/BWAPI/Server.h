#pragma once
#include <windows.h>

#include <vector>
#include <unordered_map>

namespace BWAPI
{
  // Forwards
  struct GameData;
  struct GameTable;
  class Event;
  class ForceInterface;
  typedef ForceInterface* Force;
  class PlayerInterface;
  typedef PlayerInterface* Player;
  class UnitInterface;
  typedef UnitInterface* Unit;

  class Server
  {
  public:
    Server();
    ~Server();

    Server(const Server &other) = delete;
    Server(Server &&other) = delete;
    
    void      update();
    bool      isConnected() const;
    int       addEvent(const BWAPI::Event& e);
    int       addString(const char* text);
    void      clearAll();

    int       getForceID(Force force);
    Force     getForce(int id) const;
    
    int       getPlayerID(Player player);
    Player    getPlayer(int id) const;
    
    int       getUnitID(Unit unit);
    Unit      getUnit(int id) const;

    GameData  *data = nullptr;
#ifdef SCM_SIM
    // scm-sim: the page stands in for a client (it reads GameData and writes commands where a client process would).
    void scmsimConnect() { connected = true; }
#endif
  private:
    void onMatchStart();
    void checkForConnections();
    void initializeSharedMemory();
    void updateSharedMemory();
    void callOnFrame();
    void processCommands();
    void setWaitForResponse(bool wait);

    HANDLE pipeObjectHandle = nullptr;
    HANDLE mapFileHandle = nullptr;
    HANDLE gameTableFileHandle = nullptr;
    GameTable* gameTable = nullptr;
    int gameTableIndex = -1;
    bool connected = false;
    bool localOnly = false;

    std::vector<Force> forceVector;
    std::unordered_map<Force, int> forceLookup;

    std::vector<Player> playerVector;
    std::unordered_map<Player, int> playerLookup;

    std::vector<Unit> unitVector;
    std::unordered_map<Unit, int> unitLookup;

    PSID pEveryoneSID = nullptr;
    PACL pACL = nullptr;
    PSECURITY_DESCRIPTOR pSD = nullptr;
  };
}
