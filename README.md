# scmbot

Build your own [BWAPI](https://github.com/bwapi/bwapi) 4.4.0 bot (the C++ AI modules of the bot
tournaments) into one `.scmbot` file, and play against it on [scmjs.dev/play](https://scmjs.dev/play/).
BWAPI runs beside the game there as it does in StarCraft 1.16.1, so a bot plays the way it plays
in the tournaments.

## What you need

- Docker (Docker Desktop on Windows or macOS).
- Your bot's source: an AI module (a DLL with `newAIModule`), or a client program (a `.exe` with
  its own `main`, as UAlbertaBot is). Its Visual Studio project is read if it has one.

The toolchain is a Docker image, `ghcr.io/scm-js/scmbot`: Emscripten, and BWAPI 4.4.0 compiled
once, for Intel and ARM machines alike. The first run downloads it, which takes a few GB. After
that a build takes seconds, and a rebuild compiles only the files that changed.

## Three commands

Run them in your bot's folder. `scmbot` stands for

```sh
docker run --rm -it -v "$PWD":/bot -u "$(id -u):$(id -g)" ghcr.io/scm-js/scmbot
```

or, in PowerShell on Windows, `docker run --rm -it -v "${PWD}:/bot" ghcr.io/scm-js/scmbot`.
Run `docker pull ghcr.io/scm-js/scmbot` now and then for the newest toolchain, the one the play page
expects. `scmbot new` names the bot after its folder, which `docker run` hides from it: add
`-e SCMBOT_FOLDER=MyBot` for that one.

With a clone of this repository, `toolchain/scmbot` (on Windows, `toolchain\scmbot.cmd`) is the
same and shorter. It downloads the image made from the clone's files, and builds the image itself
when you have changed them.

```sh
scmbot init      # scmbot.json, read from the bot's Visual Studio project
scmbot build     # out/<key>.scmbot
```

`init` finds the project (`.vcxproj`) whose sources define `newAIModule`, or a client program's
project, and writes down its sources, include folders, defines and C++ standard, with those of the
static libraries it links (UAlbertaBot's BOSS and SparCraft are found this way). It also notes the
licence file, the repository and its commit, and any data file the sources open by name
(`UAlbertaBot_Config.txt`) that is in the folder. Read what it wrote: it is a starting point.

With no bot yet:

```sh
scmbot new       # src/MyBot.cpp and scmbot.json: workers mine, the base trains more, supply is built
scmbot build
```

## Playing it

On the play page, **Options → Community bots → Add a bot from a file…**, or drop the `.scmbot` file
on the page. Your bot then appears in the setup screen's computer rows as "*name* (your file)". In
a lobby of a game between people the host can add it the same way: the host's page runs it, and
the other pages see a player.

The bot is kept in this browser until you remove it in Options. Adding a file with the same `key`
replaces the old build, so rebuilding and adding it again is the whole loop. Your own bots need no
account; the catalogue's bots do.

A bot from a file runs as part of the page, with the page's access to this browser. Add only files
you built or trust.

## scmbot.json

| Field | What it is |
| --- | --- |
| `key` | The bot's short name: `a`–`z`, `0`–`9`, `-` and `_`, at most 32. The page keeps one bot a key. |
| `name` | How the bot is shown, at most 32 characters. |
| `license` | The bot's licence file. Its text goes into every build and is shown with the bot. |
| `source`, `commit` | Where the source is and which commit was built (shown with the bot). |
| `sources` | The `.cpp` files: paths or patterns (`src/**/*.cpp`). |
| `exclude` | Patterns left out of `sources`. |
| `include` | Include folders. BWAPI's own headers (`<BWAPI.h>`) are always there. |
| `defines` | Preprocessor definitions, as `NAME` or `NAME=value`. |
| `std` | `c++14`, `c++17` (the default) or `c++20`. |
| `botMB` | The bot's room: BWAPI's data and the bot's heap, above the game's 2 GB; 64–2047 (768 if unsure, enough for McRave). An older `memoryMB` (the game's 1024 MB and the room) is still read: the room is `memoryMB` less 1024. |
| `files` | Files the bot reads, put where it looks for them: `[{ "from": "bin/x.txt", "to": "x.txt" }]`. `to` is a path from StarCraft's folder, as on Windows. |
| `entry` | For a client program: see below. |
| `prelude` | A header included before every source, for anything else the bot assumed of its compiler. |
| `flags` | Further compiler flags. |
| `msvc` | `false` to compile as plain clang, with none of what the next section describes. |

## Code written for Visual Studio

Most bots were only ever compiled by Visual Studio, which accepts some C++ that the standard and
clang do not. scmbot compiles a bot as Visual Studio would read it: Microsoft's language
extensions, `_MSC_VER` defined (bots use it to mean "built with BWAPI on Windows"), the standard
headers MSVC's library includes by itself, its `localtime_s` and `sprintf_s` family, `<windows.h>`
for the calls BWAPI bots use, backslashes in `#include` paths, and `std::random_shuffle`. 

What clang will not take even so, scmbot rewrites in a copy of the bot's sources (`out/msvc/`)
and says which lines it changed. Your files are never changed:

- a temporary bound to a non-const reference: `auto & it = m.find(k);` is declared `auto &&`, and
  a temporary passed to a `T &` parameter is passed through a function that makes it an lvalue
  for the call;
- a `?:` with one arm `void`: the other arm is cast to `void`;
- a lambda with `auto &` parameters that is handed temporaries takes `auto &&` (a typed
  `T &` parameter is taken as `const T &`);
- a lambda at namespace scope with a capture default (`[&]`) captures nothing.
- an `#include` whose name differs in case from the file's (`"util.H"` for `Util.h`) names the file
  as it is.

UAlbertaBot and McRave build from their unchanged repositories this way.

An error scmbot cannot fix is printed with the file and line, and often a hint about what to
change.

## A client bot

A client program connects to BWAPI from its own `main` and runs the game's events through a loop.
The page loads AI modules, so scmbot writes the module: `entry` names the bot's class, the headers
that declare it, and what `main` ran once before the first game.

```json
"entry": {
  "class": "UAlbertaBot::UAlbertaBotModule",
  "include": ["UAlbertaBotModule.h", "SparCraft.h", "BOSS.h"],
  "setup": ["SparCraft::init();", "BOSS::init();"]
}
```

A new object of the class is made at each game's start, and given each event it has a method for
(`onStart`, `onFrame`, `onUnitCreate`, …). `init` fills this in when it finds the client's loop,
and leaves the file with `main` out of `sources`.

## Learning between games

What the bot writes to `bwapi-data/write/` during a game is kept by the page and given back in
`bwapi-data/read/` at the start of the next, as the tournaments do: a file up to 1 MB, 8 MB a bot.
**Options → Community bots → Forget** clears it.

## Types from the game, and mods

BWAPI 4.4 knows the unit, weapon, upgrade and tech types from tables written into its source for
StarCraft 1.16.1. Here BWAPI reads them from the game when the match starts instead: the game's
own tables (a mod's `units.dat`, `weapons.dat`, `flingy.dat`, `upgrades.dat` and `techdata.dat`,
and a map's unit settings), the requirement lists the game checks, and its names. On a mod that
makes an SCV cost 65 minerals, `UnitTypes::Terran_SCV.mineralPrice()` is 65 and `canTrain` waits
for 65. On the game's own data a few answers change from BWAPI 4.4's, where its tables were wrong:
mostly speeds it rounded, the ranges at which units start to attack, and which upgrades apply to
which unit.

What the game's code decides by type rather than by its tables stays BWAPI's: which unit is a
refinery or a critter, which tech cloaks which unit, a spell's order, the speed of a unit its
animation moves. The types BWAPI's enum names keep their names (`Terran_Marine`) whatever a mod
calls them.

A mod may have more types than 1.16.1's 228 units, so `UnitType(494)` can be a real type, and
`UnitTypes::allUnitTypes()` lists the mod's. To make room, BWAPI's own markers moved to the top
of the 16-bit range: `UnitTypes::AllUnits`, `Men`, `Buildings`, `Factories` and `Unknown` are
65531–65535, `WeaponTypes::Unknown`, `UpgradeTypes::Unknown` and `TechTypes::Unknown` 65535, and
`TechTypes::Nuclear_Strike` 65534. `None` stays where the game has it (unit 228, weapon 130,
upgrade 61, tech 44). A bot that uses the names needs only a rebuild; one that wrote the numbers
(`UnitType(233)`) should use the names.

**Rebuild your bot.** A `.scmbot` made before this (its header says `scmbot 1 (BWAPI 4.4.0)`)
still plays, with 1.16.1's types written in. Run `docker pull ghcr.io/scm-js/scmbot` and
`scmbot build` again for the types from the game.

## Beside a mod's plugins

Some mods carry plugins: code that runs inside the game (GPTP's, in Cosmonarchy, say). Such a
plugin can move the game's tables and make them bigger: Cosmonarchy's has room for 4,095 units
where the game had 1,700, 2,048 bullets, and counts, deaths and kills for its 3,000 unit types.

**The memory.** The game and its plugins are 32-bit Windows code and keep everything below 2 GB;
the plugins' heap grows up to there. A bot built with scmbot 3 (`scmbot 3 (BWAPI 4.4.0, types from
the game's data, beside a mod's plugins)` in its header) keeps its own memory above 2 GB, so the
page gives a game with it 2 GB and the bot's room: 2.75 GB for McRave's 768 MB. A bundle built
before keeps its memory at 1 GB, where the plugins keep theirs: it still plays vanilla games and
mods without plugins, and the page refuses it beside a mod's plugins, saying to build it again.

**What BWAPI reads.** BWAPI 4.4 reads the game at 1.16.1's addresses. Built with scmbot 3, it reads
the game's tables where the game's own code says they are, at the start of the match: the units
and their number, the unit finders, the bullets, the counts, deaths and kills, which units a player
may make, the upgrade levels and the techs. Where nothing moved these are 1.16.1's places, and a
bot sees what it saw. Beside the plugins, `allUnitCount`, `completedUnitCount`, `deadUnitCount`,
`killedUnitCount`, `getUpgradeLevel`, `hasResearched` and `isUnitAvailable` answer for the mod's
own types too (`UnitType(494)`), and a unit's id in a command is the plugin's (12 bits of slot,
not 11).

**What a bot sees of a mod.** BWAPI's types are the mod's (above). A bot written for the game's
own units still names them by 1.16.1's numbers, which mean the mod's units in the same places: in
Cosmonarchy the terran worker is still type 7 and its hall type 106, so `getRace().getWorker()`
trains it. A plan made for vanilla units is not a plan for a mod's; a bot for a mod is written
against its types.

**GameData**, the block a client process reads, keeps BWAPI 4.4's widths: it lists 1,700 unit
slots, 100 bullets and counts for 234 types. A mod's type past 4.4's numbers is named in a unit's
fields by its own id (by 4.4's `Unknown` where its id would read as one of 4.4's markers), and has
no count there.

## What is different from Windows

- One BWAPI bot a game: every build puts its memory at the same place (2 GB).
- Threads cannot be started, and there is no network.
- The bot's clock is the real one. A bot that stops searching after so many milliseconds plays a
  little differently each game, as it does in the game.
- BWAPI's map analysis libraries that come as source (BWEM, BWEB) build with the bot. BWTA2 comes
  as a Windows library and does not.
- The page runs the bot inside the game's frame. A slow `onFrame` slows the game for everyone in it.

## What is in this repository

| Folder | What it is | Licence |
| --- | --- | --- |
| `toolchain/` | `scmbot.mjs` (the three commands), the `Dockerfile`, `msvc-compat.h`, and the `scmbot` / `scmbot.cmd` wrappers | MIT |
| `shim/` | Stand-ins for the Windows headers BWAPI and bots include | MIT |
| `host/` | What BWAPI.dll's Windows side does, for BWAPI running beside the play page's engine | MIT |
| `bwapi/` | BWAPI 4.4.0's in-game source ([github.com/bwapi/bwapi](https://github.com/bwapi/bwapi), tag v4.4.0), cut to the part that runs in the game; each change is marked `scm-sim:` | LGPL-3 (`bwapi/COPYING`, `bwapi/LICENSE.md`) |

These files are kept in step with [scm-sim](https://scmjs.dev/play/), whose page loads the bots: a
`.scmbot` file is built against the page's memory layout and its loader, which is why the
Emscripten image is pinned. The image is published from this repository whenever they change,
tagged `latest` and with a hash of the files (`toolchain/scmbot` pulls the one for its clone).

Every `.scmbot` file carries your bot's licence and BWAPI's (LGPL-3), and the page shows both with
the bot.

No StarCraft game data is in this repository or in the image. The play page reads the game's files
from your own copy of StarCraft.

## Licence

MIT (`LICENSE`) for everything except `bwapi/`, which is BWAPI's and stays under the LGPL-3.
