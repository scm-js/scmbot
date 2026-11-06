// scm-sim's side of BWAPI in wasm: what BWAPI.dll's Windows layer (DLLMain, Config, Detours, AutoMenuManager, storm
// imports) gave the in-game code, answered for a wasm module whose memory is the game's address space.
// The calls out to the page go through `Module.scm` (set by the host before the module starts: src/bwapi/).
#include <emscripten.h>
#include <sys/stat.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <windows.h>
#include <storm.h>

#include <BWAPI.h>
#include "BWAPI/GameImpl.h"
#include "BWAPI/AutoMenuManager.h"
#include <BW/Offsets.h>
#include <BWAPI/Client/GameData.h>
#include <Util/StringUtil.h>

EM_JS(void, scm_print, (const char *text), { Module.scm.print(UTF8ToString(text)); });
EM_JS(void, scm_error, (const char *text), { Module.scm.error(UTF8ToString(text)); });
EM_JS(void, scm_call, (unsigned address), { Module.scm.call(address >>> 0); });
EM_JS(double, scm_ms, (), { return Module.scm.now(); });
EM_JS(void, scm_client, (), { Module.scm.client(); });
EM_JS(void, scm_command, (const void *bytes, int length), { Module.scm.command(HEAPU8.slice(bytes, bytes + length)); });
EM_JS(void, scm_leave, (), { Module.scm.leave(); });
EM_JS(void, scm_chat, (const char *text, int toAllies), { Module.scm.chat(UTF8ToString(text), toAllies !== 0); });

// ---- Config (Config.cpp read bwapi.ini; here the page hands the text over) ---------------------------------------
static std::map<std::string, std::string> config;
static std::string ckey(const char *section, const char *item) { return Util::to_lower_copy(std::string(section)) + "\n" + Util::to_lower_copy(std::string(item)); }

std::string screenshotFmt = "gif";
bool isCorrectVersion = true;
bool showWarn = false;
bool serverEnabled = false;
unsigned gdwProcNum = 0;

std::string LoadConfigString(const char *pszKey, const char *pszItem, const char *pszDefault)
{
  auto it = config.find(ckey(pszKey, pszItem));
  if (it != config.end()) return it->second;
  return pszDefault ? pszDefault : "";
}
std::string LoadConfigStringUCase(const char *pszKey, const char *pszItem, const char *pszDefault) { return Util::to_upper_copy(LoadConfigString(pszKey, pszItem, pszDefault)); }
int LoadConfigInt(const char *pszKey, const char *pszItem, const int iDefault)
{
  auto it = config.find(ckey(pszKey, pszItem));
  if (it == config.end() || it->second.empty()) return iDefault;
  return std::atoi(it->second.c_str());
}
void WriteConfig(const char *pszKey, const char *pszItem, const std::string &value) { config[ckey(pszKey, pszItem)] = value; }
void WriteConfig(const char *pszKey, const char *pszItem, int value) { config[ckey(pszKey, pszItem)] = std::to_string(value); }
void InitPrimaryConfig() {}

// ---- Detours.cpp's globals that the game code reads ------------------------------------------------------------
DWORD lastTurnTime = 0;
DWORD lastTurnFrame = 0;
std::string gDesiredReplayName;
bool detourCreateWindow = false;

// ---- DLLMain.cpp ------------------------------------------------------------------------------------------------
void BWAPIError(const char *format, ...)
{
  char buffer[1024];
  va_list ap; va_start(ap, format); vsnprintf(buffer, sizeof buffer, format, ap); va_end(ap);
  scm_error(buffer);
}
void BWAPIError(DWORD dwErrCode, const char *format, ...)
{
  char buffer[1024];
  int n = snprintf(buffer, sizeof buffer, "(%lu) ", (unsigned long)dwErrCode);
  va_list ap; va_start(ap, format); vsnprintf(buffer + n, sizeof buffer - n, format, ap); va_end(ap);
  scm_error(buffer);
}

/** Commands handed to the page one by one instead of into the turn buffer (`scm_bwapi_capture`): the bot is not the
 *  game's local player, and the page gives its commands to its own player's turn. */
static bool captureCommands = false;

/** The command into the game's turn buffer, as BWAPI.dll did: what does not fit this turn is dropped (BWAPI would
 *  have sent the turn and waited; the page sends one turn a frame). */
void __fastcall QueueGameCommand(void *pBuffer, size_t dwLength)
{
  if (!pBuffer || !dwLength) return;
  if (captureCommands) { scm_command(pBuffer, (int)dwLength); return; }
  if (dwLength + BW::BWDATA::sgdwBytesInCmdQueue <= BW::BWDATA::TurnBuffer.size())
  {
    memcpy(&BW::BWDATA::TurnBuffer[BW::BWDATA::sgdwBytesInCmdQueue], pBuffer, dwLength);
    BW::BWDATA::sgdwBytesInCmdQueue += dwLength;
    return;
  }
  BWAPIError("a command of %u bytes did not fit in this turn and was dropped", (unsigned)dwLength);
}

// ---- Windows ----------------------------------------------------------------------------------------------------
extern "C" DWORD GetTickCount() { return (DWORD)(unsigned long long)scm_ms(); }
BOOL QueryPerformanceFrequency(LARGE_INTEGER *f) { if (f) f->QuadPart = 1000000; return TRUE; }
BOOL QueryPerformanceCounter(LARGE_INTEGER *c) { if (c) c->QuadPart = (LONGLONG)(scm_ms() * 1000.0); return TRUE; }
DWORD GetModuleFileNameA(HMODULE, LPSTR buffer, DWORD size)
{
  const char *path = "C:\\StarCraft\\StarCraft.exe";
  strncpy(buffer, path, size); if (size) buffer[size - 1] = 0;
  return (DWORD)strlen(buffer);
}

// The bot is linked into the same module: "loading" the AI module finds its two exports.
extern "C" void gameInit(BWAPI::Game *game) __attribute__((weak));
extern "C" BWAPI::AIModule *newAIModule() __attribute__((weak));
static int theModule;
HMODULE LoadLibraryA(LPCSTR name) { return name && *name && newAIModule ? (HMODULE)&theModule : nullptr; }
FARPROC GetProcAddress(HMODULE h, LPCSTR name)
{
  if (h != (HMODULE)&theModule || !name) return nullptr;
  if (!strcmp(name, "gameInit")) return (FARPROC)gameInit;
  if (!strcmp(name, "newAIModule")) return (FARPROC)newAIModule;
  return nullptr;
}
BOOL FreeLibrary(HMODULE) { return TRUE; }

// ---- storm.dll ---------------------------------------------------------------------------------------------------
BOOL STORMAPI SEvtDispatch(DWORD, DWORD, int type, PS_EVT pEvent)
{
  if (type == 4 && pEvent && pEvent->pData) scm_print((const char *)pEvent->pData);
  return TRUE;
}
BOOL STORMAPI SNetGetProviderCaps(PCAPS caps) { if (caps) { caps->maxmessagesize = 512; caps->dwCallDelay = 1; caps->dwPlayerCount = 8; } return TRUE; }
/** A chat line in a game between players (GameImpl::vSendTextEx: two bytes, then the text): to the page. */
BOOL STORMAPI SNetSendMessage(int to, void *data, size_t length)
{
  const char *b = (const char *)data;
  if (length > 2 && b[0] == 0 && b[1] == 1) { std::string text(b + 2, strnlen(b + 2, length - 2)); scm_chat(text.c_str(), to != -1); }
  return TRUE;
}
BOOL STORMAPI SNetSendServerChatCommand(const char *) { return TRUE; }
int STORMAPI SStrCopy(char *dest, const char *source, int size)
{
  int n = 0;
  if (size <= 0) return 0;
  while (n < size - 1 && source[n]) { dest[n] = source[n]; ++n; }
  dest[n] = 0;
  return n;
}

// ---- The auto-menu: the page starts games itself ----------------------------------------------------------------
namespace BWAPI
{
  AutoMenuManager::AutoMenuManager() {}
  void AutoMenuManager::reloadConfig() { autoMenuSaveReplay = ""; }
  void AutoMenuManager::chooseNewRandomMap() {}
  void AutoMenuManager::onMenuFrame() {}
  const char *AutoMenuManager::interceptFindFirstFile(const char *lpFileName) { return lpFileName; }
  void AutoMenuManager::pressDialogKey(BW::dialog *) {}

  // GameMenu.cpp's, without the menus.
  void GameImpl::loadAutoMenuData()
  {
    this->autoMenuManager.reloadConfig();
    this->seedOverride = LoadConfigInt("starcraft", "seed_override", std::numeric_limits<decltype(this->seedOverride)>::max());
    this->speedOverride = LoadConfigInt("starcraft", "speed_override", std::numeric_limits<decltype(this->speedOverride)>::min());
    this->wantDropPlayers = LoadConfigStringUCase("starcraft", "drop_players", "ON") == "ON";
  }
}

// A call into the game the page makes for BWAPI (`Module.scm.call`): it runs the game's routine at that address.
namespace BWAPI
{
  /** BWAPI's isMultiplayer: the page's game between players (GameImpl.cpp, SCM_SIM). */
  bool scmsimMultiplayer = false;
  /** The game runs on other pages too: BWAPI changes nothing of the game's memory (its speed, frame skip, screen words). */
  bool scmsimShared = false;
  void scmsim_call(unsigned address) { scm_call(address); }
  /** BWAPI woke its client (Server::callOnFrame): the page's client reads GameData and writes commands now. */
  void scmsim_client_frame() { scm_client(); }
  /** The bot called leaveGame (GameImpl.cpp, SCM_SIM): the page takes its player out of the game. */
  void scmsim_leave() { scm_leave(); }
}

// ---- The map file (Map::calculateMapHash reads it whole through storm; the page hands its bytes over) -------------
static std::vector<char> mapFile;
static int mapHandle;
BOOL STORMAPI SFileOpenFileEx(HANDLE, const char *, DWORD, HANDLE *phFile)
{
  if (phFile) *phFile = nullptr;
  if (mapFile.empty() || !phFile) return FALSE;
  *phFile = &mapHandle; return TRUE;
}
LONG STORMAPI SFileGetFileSize(HANDLE h, LPDWORD high) { if (high) *high = 0; return h == &mapHandle ? (LONG)mapFile.size() : 0; }
BOOL STORMAPI SFileReadFile(HANDLE h, void *buffer, DWORD size, LPDWORD read, LONG)
{
  DWORD n = h == &mapHandle ? (DWORD)std::min<size_t>(size, mapFile.size()) : 0;
  if (n) memcpy(buffer, mapFile.data(), n);
  if (read) *read = n;
  return n == size;
}
BOOL STORMAPI SFileCloseFile(HANDLE) { return TRUE; }
void *STORMAPI SMemAlloc(size_t amount, char *, int, char defaultValue) { void *p = malloc(amount); if (p) memset(p, defaultValue, amount); return p; }
BOOL STORMAPI SMemFree(void *location, char *, int, char) { free(location); return TRUE; }

// ---- WMode.cpp (the /wmode chat command) ----------------------------------------------------------------------------
bool wmode = false;
void SetWMode(int, int, bool) {}

// ---- What the page calls --------------------------------------------------------------------------------------------
extern "C"
{
  /** bwapi.ini's text: `[section]` lines and `key = value` lines; `;` starts a comment. */
  EMSCRIPTEN_KEEPALIVE void scm_bwapi_config(const char *ini)
  {
    // Bots keep files between games in bwapi-data/read and write (relative to StarCraft's folder, the process's
    // working directory). Here they are in the module's in-memory file system; the page copies them in and out.
    mkdir("bwapi-data", 0777); mkdir("bwapi-data/read", 0777); mkdir("bwapi-data/write", 0777); mkdir("bwapi-data/AI", 0777);
    std::string section, s(ini ? ini : "");
    size_t at = 0;
    while (at <= s.size())
    {
      size_t end = s.find('\n', at); if (end == std::string::npos) end = s.size();
      std::string line = s.substr(at, end - at); at = end + 1;
      size_t semi = line.find(';'); if (semi != std::string::npos) line.erase(semi);
      Util::trim(line, Util::is_whitespace_or_newline);
      if (line.empty()) continue;
      if (line.front() == '[' && line.back() == ']') { section = line.substr(1, line.size() - 2); continue; }
      size_t eq = line.find('='); if (eq == std::string::npos) continue;
      std::string key = line.substr(0, eq), value = line.substr(eq + 1);
      Util::trim(key, Util::is_whitespace_or_newline); Util::trim(value, Util::is_whitespace_or_newline);
      config[ckey(section.c_str(), key.c_str())] = value;
    }
  }
  /** The map's file as the game opened it (its SHA-1 is BWAPI's map hash). */
  EMSCRIPTEN_KEEPALIVE void scm_bwapi_map_file(const char *bytes, int length) { mapFile.assign(bytes, bytes + length); }
  /** Each slot's race as picked in the lobby, before the game rolled the random ones (0 zerg, 1 terran, 2 protoss,
   *  6 random, anything else none). BWAPI.dll noted them around the game's own race roll; it hides a random pick from
   *  the bot until one of that player's units is seen. */
  EMSCRIPTEN_KEEPALIVE void scm_bwapi_lobby_races(const unsigned char *races)
  {
    for (int i = 0; i < BW::PLAYABLE_PLAYER_COUNT; ++i)
      BWAPI::BroodwarImpl.lastKnownRaceBeforeStart[i] = races[i] <= 2 || races[i] == 6 ? BWAPI::Race(races[i]) : BWAPI::Races::None;
  }
  /** BWAPI is to call the game one between players (what a bot sees in a LAN game): the page's game between players. */
  EMSCRIPTEN_KEEPALIVE void scm_bwapi_multiplayer(int on) { BWAPI::scmsimMultiplayer = on != 0; }
  /** The game runs on other pages too (a game between people): BWAPI writes nothing into the game's memory. */
  EMSCRIPTEN_KEEPALIVE void scm_bwapi_shared(int on) { BWAPI::scmsimShared = on != 0; }
  /** Commands to the page (`Module.scm.command`) rather than into the turn buffer. */
  EMSCRIPTEN_KEEPALIVE void scm_bwapi_capture(int on) { captureCommands = on != 0; }
  /** The game has just sent a turn (storm #128, SNetSendTurn, where BWAPI.dll's _SNetSendTurn noted it for its latency). */
  EMSCRIPTEN_KEEPALIVE void scm_bwapi_turn_sent() { lastTurnTime = GetTickCount(); lastTurnFrame = BWAPI::BroodwarImpl.getFrameCount(); }
  /** At the gate of each frame (0x4D974E, where BWAPI.dll's _nextFrameHook was called): BWAPI's whole update. */
  EMSCRIPTEN_KEEPALIVE int scm_bwapi_frame() { BWAPI::BroodwarImpl.update(); return BW::BWDATA::isGamePaused; }
  /** From now on the page is BWAPI's client (as a bot process connected over the pipe would be): GameData is filled
   *  every frame and the page's `client()` is called where the client process was woken. Before the first frame. */
  EMSCRIPTEN_KEEPALIVE void scm_bwapi_connect() { BWAPI::BroodwarImpl.server.scmsimConnect(); }
  /** Where GameData is (the block a client reads), and its size. */
  EMSCRIPTEN_KEEPALIVE BWAPI::GameData *scm_bwapi_gamedata() { return BWAPI::BroodwarImpl.server.data; }
  EMSCRIPTEN_KEEPALIVE int scm_bwapi_gamedata_size() { return (int)sizeof(BWAPI::GameData); }
  /** BWAPI has ended the match itself (its update saw the bot's player win or lose and called the bot's onEnd). */
  EMSCRIPTEN_KEEPALIVE int scm_bwapi_match_ended() { return BWAPI::BroodwarImpl.scmsimMatchEnded() ? 1 : 0; }
  /** The game is over and left (Detours' _SNetLeaveGame / the end of the match). */
  EMSCRIPTEN_KEEPALIVE void scm_bwapi_end() { BWAPI::BroodwarImpl.onGameEnd(); }
}
