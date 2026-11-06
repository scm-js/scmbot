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
| `memoryMB` | The whole memory: the game's 1024 MB and the bot's above it, 1088–3072. 1792 is enough for McRave. |
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

## What is different from Windows

- One BWAPI bot a game: every build puts its memory at the same place.
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
