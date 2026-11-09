#!/usr/bin/env node
/**
 * scmbot: a BWAPI 4.4.0 bot (C++, an AI module) built for scm-sim's play page, as one file the page can load
 * (github.com/scm-js/scmbot). Runs inside the toolchain image (toolchain/Dockerfile), where Emscripten is.
 *
 *   scmbot init [project.vcxproj]  a scmbot.json for the bot in /bot (or .): from its Visual Studio project when it has
 *                                  one (the sources, include folders, defines and C++ standard, and those of the static
 *                                  libraries it references), else a skeleton to fill in
 *   scmbot build                   the bot, as its scmbot.json says, into out/<key>.scmbot
 *   scmbot new                     a bot to start from (workers mine, the base trains more), with its scmbot.json
 *   scmbot prepare                 (the image's own build) BWAPI 4.4.0 and the host compiled once, into /scmbot/obj
 *
 * Bot sources are compiled the way MSVC would take them unless scmbot.json says "msvc": false: clang's Microsoft
 * compatibility (late template lookup, a temporary bound to a reference, backslashes in #include, …) and a prelude
 * of what MSVC's library includes by itself and its "secure" C calls (toolchain/msvc-compat.h).
 *
 * The bot is a BWAPI AI module: it exports `gameInit` and `newAIModule` as a module DLL does (see BWAPI's
 * ExampleAIModule/Source/Dll.cpp). BWAPI's own headers are on the include path (<BWAPI.h>).
 *
 * The .scmbot file, little endian:
 *   "SCMBOT1\n", u32 header length, the header (JSON: format, key, name, memoryPages, base, commit, source, license
 *   (text), toolchain, mod when scmbot.json names one, mjs and wasm byte counts), then the module's JavaScript loader,
 *   then its wasm.
 *
 * A bot made for a mod names it in scmbot.json: "mod": { "name", "hash", "link" } (`scmjs mod header --bot` writes it
 * with the mod's header of names). The build carries it into the header, where the play page and the runners read
 * it: "made for <mod>" when the game's mod has that hash. An additive field, which a reader from before skips.
 */
import { spawn } from "node:child_process";
import { cpus } from "node:os";
import { createHash } from "node:crypto";
import { existsSync, mkdirSync, readFileSync, readdirSync, rmSync, statSync, writeFileSync } from "node:fs";
import { dirname, join, relative, resolve } from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";

const HOME = process.env.SCMBOT_HOME || dirname(dirname(fileURLToPath(import.meta.url)));   // native/ (or /scmbot in the image)
const OBJ = process.env.SCMBOT_OBJ || join(HOME, "out/obj/toolchain");
/**
 * What a bundle says built it. 3: the bot's memory starts at 2 GB, above the game's world and a mod's plugins', and
 * BWAPI reads the game's tables where the game's code says they are (a plugin may have moved them); 2: BWAPI's types
 * are read from the game's tables at the start of a match (a mod's too); 1: 1.16.1's types written in. Bundles of 1
 * and 2 keep their memory at 1 GB and play vanilla games and mods without plugins.
 */
export const TOOLCHAIN = "scmbot 3 (BWAPI 4.4.0, types from the game's data, beside a mod's plugins)";
/** Where the bot's part of the memory begins (`-sGLOBAL_BASE`, the bundle's `base`): 2 GB. */
export const BASE_MB = 2048;
/** How the person types this, for the messages that say what to run next (`scmjs bot` passes its own name). */
const CMD = process.env.SCMBOT_COMMAND || "scmbot";
const FORMAT = 1;

/** The compile flags every object gets: BWAPI's in-game headers, the Windows stand-ins, SCM_SIM. */
const BASE = ["-O2", "-fexceptions", "-DNOMINMAX", "-DSCM_SIM", "-include", join(HOME, "shim/prelude.h"), "-Wno-everything",
  ...["shim", "bwapi/include", "bwapi/Util/Source", "bwapi/Storm", "bwapi/BWAPI/Source", "bwapi/BWAPI/Source/BWAPI", "bwapi/Shared", "bwapi/BWAPICore"].map((d) => `-I${join(HOME, d)}`)];

/** BWAPI's in-game part and the host (the same list as native/build.sh). */
function bwapiSources() {
  const B = "bwapi/BWAPI/Source", ls = (d) => readdirSync(join(HOME, d)).filter((f) => f.endsWith(".cpp")).map((f) => `${d}/${f}`);
  return [
    ...ls("bwapi/BWAPILIB/Source"), ...ls("bwapi/BWAPILIB"), ...ls("bwapi/Shared"),
    ...["BWtoBWAPI", "BulletImpl", "CommandOptimizer", "ForceImpl", "GameBullets", "GameCommands", "GameDrawing", "GameEvents", "GameImpl", "GameInternals", "GameUnits", "Map", "PlayerImpl", "RegionImpl", "Server", "UnitImpl", "UnitUpdate"].map((f) => `${B}/BWAPI/${f}.cpp`),
    `${B}/GameUpdate.cpp`, ...ls(`${B}/BW`),
    "bwapi/Util/Source/Util/Exceptions.cpp", "bwapi/Util/Source/Util/Sha1.cpp", "bwapi/Util/Source/Util/MemoryFrame.cpp",
    "bwapi/BWAPICore/APMCounter.cpp", "bwapi/BWAPICore/FPSCounter.cpp", "host/host.cpp",
  ].map((f) => join(HOME, f));
}

/** The link flags of every build (native/link.sh's): the memory imported, BWAPI's data above the game's 2 GB, `botMB` of room. */
function linkFlags(botMB) {
  return ["-O2", "-fexceptions",
    "-sIMPORTED_MEMORY=1", `-sINITIAL_MEMORY=${BASE_MB + botMB}MB`, "-sALLOW_MEMORY_GROWTH=0", `-sGLOBAL_BASE=${BASE_MB * 1048576}`, "-sSTACK_SIZE=8MB",
    "-sMODULARIZE=1", "-sEXPORT_ES6=1", "-sEXPORT_NAME=BwapiModule", "-sENVIRONMENT=node,web,worker",
    "-sEXPORTED_FUNCTIONS=_scm_bwapi_config,_scm_bwapi_frame,_scm_bwapi_end,_scm_bwapi_map_file,_scm_bwapi_turn_sent,_scm_bwapi_lobby_races,_scm_bwapi_connect,_scm_bwapi_capture,_scm_bwapi_multiplayer,_scm_bwapi_shared,_scm_bwapi_match_ended,_scm_bwapi_gamedata,_scm_bwapi_gamedata_size,_scm_bwapi_unit_names,_scm_bwapi_types_status,_scm_bwapi_types_json,_scm_bwapi_layout_status,_scm_bwapi_view_json,_malloc,_free",
    "-sEXPORTED_RUNTIME_METHODS=stringToNewUTF8,UTF8ToString,FS,HEAPU8", "-sFORCE_FILESYSTEM=1"];
}

function run(cmd, args, cwd) {
  return new Promise((resolve) => {
    const child = spawn(cmd, args, { cwd, stdio: ["ignore", "pipe", "pipe"] });
    let err = "";
    child.stdout.on("data", (d) => { err += d; });
    child.stderr.on("data", (d) => { err += d; });
    child.on("close", (code) => resolve({ code, err }));
    child.on("error", (e) => resolve({ code: -1, err: String(e) }));
  });
}

/** What MSVC lets pass and clang does not, as the errors say it, and what to write instead. */
const HINTS = [
  [/right operand to \? is void|left operand to \? is void/, "MSVC takes `c ? a = b : f();` with a void arm; clang does not: write it as if/else"],
  [/non-const lvalue reference to type .* cannot bind to a temporary/, "a temporary bound to a non-const reference (MSVC allows it): `auto &x = f()` → `auto x`, or take the parameter as `const T &`"],
  [/file not found/, "the include's name must match the file's on disk, case and all (Windows does not care, this does), or its folder is missing from \"include\""],
  [/use of undeclared identifier '(GetTickCount|Sleep|QueryPerformanceCounter|OutputDebugString)/, "a Windows call: include <windows.h> (the toolchain has a stand-in)"],
  [/no matching function for call to '(min|max)'/, "std::min/std::max of two different types: give both the same type"],
  [/explicit specialization of .* in class scope/, "a template specialised inside its class: move the specialisation out to namespace scope"],
];
function explain(file, err) {
  const lines = err.split("\n").filter((l) => /error|fatal/.test(l));
  const hints = [...new Set(HINTS.filter(([re]) => lines.some((l) => re.test(l))).map(([, h]) => h))];
  return `${file}:\n${lines.slice(0, 6).join("\n")}${hints.map((h) => `\n  hint: ${h}`).join("")}`;
}

/** From `col` in `text`, where the expression there ends: a `stop` character at its own depth, or an unmatched bracket. */
function expressionEnd(text, col, stops) {
  let depth = 0, quote = null, ternary = 0;
  for (let end = col; end < text.length; end++) {
    const ch = text[end];
    if (quote) { if (ch === "\\") end++; else if (ch === quote) quote = null; continue; }
    if (ch === '"' || ch === "'") quote = ch;
    else if ("([{".includes(ch)) depth++;
    else if (")]}".includes(ch)) { if (depth === 0) return end; depth--; }
    else if (depth === 0 && ch === "?") ternary++;
    else if (depth === 0 && ch === ":" && text[end + 1] !== ":" && text[end - 1] !== ":") { if (ternary) ternary--; else if (stops.includes(":")) return end; }
    else if (depth === 0 && stops.includes(ch)) return end;
  }
  return stops.includes(";") ? text.length : -1;
}

/**
 * What MSVC takes and clang does not, rewritten as clang takes it, line by line where the errors are. Returns what
 * changed (and changes `lines`):
 *  - a temporary bound to a non-const reference: a declaration's `&` becomes `&&` (the temporary lives as long as
 *    the name, as in MSVC); an argument goes through scmbot_lvalue() (it lives to the end of the call);
 *  - `c ? a = b : f()` with one arm void: the other is cast to void;
 *  - a lambda whose `auto &` parameter is handed a temporary: `auto &&`;
 *  - a lambda at namespace scope with a capture default (`[&]`, nothing to capture there): `[]`;
 *  - an #include whose name's case is not the file's (Windows does not mind): the file's (`include` errors carry
 *    the name found, `found`).
 */
function fixLines(lines, errors) {
  const changed = [];
  // From the right of a line, so earlier columns stay where they are.
  for (const { line, col, message, found } of errors.sort((a, b) => b.line - a.line || b.col - a.col)) {
    const text = lines[line];
    if (text === undefined) continue;
    if (/non-const lvalue reference to type .* cannot bind to a temporary/.test(message)) {
      const before = text.slice(0, col);
      // (clang points at the declared name, `auto & it = f();`, or at the initialiser)
      const decl = (/^\w+\s*[=({]/.test(text.slice(col)) ? before.match(/^(.*?[\w>\]])(\s*)&(?!&)(\s*)$/) : null)
        ?? before.match(/^(.*?[\w>\]])(\s*)&(?!&)(\s*\w+\s*[=({]\s*)$/);
      if (decl) { lines[line] = `${decl[1]}${decl[2]}&&${decl[3]}${text.slice(col)}`; changed.push(`${line + 1}: a reference declared && (it holds a temporary)`); continue; }
      const end = expressionEnd(text, col, ",");
      if (end <= col) continue;
      lines[line] = `${text.slice(0, col)}scmbot_lvalue(${text.slice(col, end)})${text.slice(end)}`;
      changed.push(`${line + 1}: a temporary passed by reference, through scmbot_lvalue()`);
      continue;
    }
    if (message === "lambda-params" && text[col] === "[") {
      const close = expressionEnd(text, col + 1, ""), open = close >= 0 ? text.indexOf("(", close) : -1;
      if (open < 0 || text.slice(close + 1, open).trim()) continue;
      const end = expressionEnd(text, open + 1, "");
      if (end < 0) continue;
      const params = text.slice(open + 1, end);
      let now = params.replace(/(^|,)(\s*)auto\s*&(?!&)\s*/g, "$1$2auto &&"), what = "auto & parameters made auto &&";
      // (a typed one, `Position &p`: taken as const, as a temporary can be; if the lambda changes it, that is an error to see)
      if (now === params) { now = params.replace(/(^|,)(\s*)(?!const\b)([A-Za-z_][\w:<>]*)\s*&(?!&)\s*(\w+)/g, "$1$2const $3 &$4"); what = "reference parameters taken as const"; }
      if (now === params) continue;
      lines[line] = text.slice(0, open + 1) + now + text.slice(end);
      changed.push(`${line + 1}: a lambda's ${what} (it is handed temporaries)`);
      continue;
    }
    if (message === "include" && found) {
      const at = text.indexOf(found.name);
      if (at < 0) continue;
      lines[line] = text.slice(0, at) + found.to + text.slice(at + found.name.length);
      changed.push(`${line + 1}: #include "${found.to}", as the file is named`);
      continue;
    }
    // (clang points at the capture default itself, after the [)
    const bracket = text.lastIndexOf("[", col);
    if (/non-local lambda expression cannot have a capture-default/.test(message) && bracket >= 0 && /^\[\s*[&=]\s*\]/.test(text.slice(bracket))) {
      lines[line] = text.slice(0, bracket) + text.slice(bracket).replace(/^\[\s*[&=]\s*\]/, "[]");
      changed.push(`${line + 1}: a lambda outside any function captures nothing ([])`);
      continue;
    }
    const arm = message.match(/(right|left) operand to \? is void/);
    if (arm && text[col] === "?") {
      const colon = expressionEnd(text, col + 1, ":");
      if (colon < 0 || text[colon] !== ":") continue;
      if (arm[1] === "right") {   // the left (true) arm is the one with a value
        lines[line] = `${text.slice(0, col + 1)} (void)(${text.slice(col + 1, colon).trim()}) ${text.slice(colon)}`;
      } else {
        const end = expressionEnd(text, colon + 1, ",;");
        lines[line] = `${text.slice(0, colon + 1)} (void)(${text.slice(colon + 1, end).trim()})${text.slice(end)}`;
      }
      changed.push(`${line + 1}: a ?: with a void arm, the other cast to void`);
    }
  }
  return changed;
}

/** Errors in `err` that fixLines can do something with, by file. */
function fixableErrors(err) {
  const out = new Map();
  const add = (file, line, col, message) => {
    file = resolve(file);
    if (!out.has(file)) out.set(file, []);
    const list = out.get(file), e = { line: +line - 1, col: +col - 1, message };
    const had = list.find((x) => x.line === e.line && x.col === e.col);
    if (had) return had;
    list.push(e);
    return e;
  };
  for (const m of err.matchAll(/^(.+?):(\d+):(\d+): error: (non-const lvalue reference to type .* cannot bind to a temporary.*|(?:right|left) operand to \? is void.*|non-local lambda expression cannot have a capture-default)$/gm)) add(m[1], m[2], m[3], m[4]);
  for (const m of err.matchAll(/^(.+?):(\d+):(\d+): fatal error: '([^']+)' file not found$/gm)) add(m[1], m[2], m[3], "include").name = m[4];
  // A lambda that cannot be called with what it is handed (or made a std::function of): where the lambda is.
  for (const m of err.matchAll(/error: (?:no matching function for call to object of type|no viable conversion from) '(?:const )?\(lambda at (.+?):(\d+):(\d+)\)'/g)) add(m[1], m[2], m[3], "lambda-params");
  return out;
}

/** Whether `object` is newer than everything its .d file says it was made from, under the same flags. */
function upToDate(object, stamp) {
  try {
    if (readFileSync(object + ".flags", "utf8") !== stamp) return false;
    const made = statSync(object).mtimeMs;
    const deps = readFileSync(object + ".d", "utf8").replace(/\\\n/g, " ").replace(/^[^:]+:/, "").trim().split(/(?<!\\)\s+/).map((d) => d.replace(/\\ /g, " "));
    return deps.every((d) => !d || (existsSync(d) && statSync(d).mtimeMs <= made));
  } catch { return false; }
}

const SOURCE_FILE = /\.(c|cc|cpp|cxx|h|hh|hpp|hxx|inl|ipp|tpp)$/i;

/**
 * The bot's sources, mirrored into out/msvc/ to be fixed there (fixLines): a file is copied again when the
 * original changed since, so that its fixes are made afresh. Returns the notes of what was fixed, by file.
 */
function syncMirror(root, mirror) {
  const notesFile = join(mirror, ".fixes.json");
  const notes = existsSync(notesFile) ? JSON.parse(readFileSync(notesFile, "utf8")) : {};
  const walk = (d) => {
    for (const n of readdirSync(d)) {
      if (n === ".git" || (d === root && n === "out")) continue;
      const p = join(d, n);
      if (statSync(p).isDirectory()) { walk(p); continue; }
      if (!SOURCE_FILE.test(n)) continue;
      const rel = relative(root, p), copy = join(mirror, rel);
      if (!existsSync(copy) || statSync(p).mtimeMs > statSync(copy).mtimeMs) {
        mkdirSync(dirname(copy), { recursive: true });
        writeFileSync(copy, readFileSync(p));
        delete notes[rel];
      }
    }
  };
  walk(root);
  writeFileSync(notesFile, JSON.stringify(notes));
  return notes;
}

/**
 * Compiles the bot's `files` (paths under `root`) into out/obj, several at once, each only when it or a header it
 * includes changed since its object was made; returns the objects, or throws with the errors.
 *
 * With `msvc`, what Visual Studio takes and clang does not is fixed (fixLines) in a mirror of the bot's sources,
 * out/msvc/, so that a fix in a header reaches every file that includes it; once a bot needs that, it is built from
 * the mirror (the author's files are never changed). What was fixed is said each build.
 */
async function compile(files, root, flags, std, label, msvc, dir = join(root, "out/obj")) {
  const out = join(root, "out"), mirror = join(out, "msvc");
  mkdirSync(dir, { recursive: true });
  const objects = files.map((f) => join(dir, createHash("sha1").update(relative(root, f)).digest("hex").slice(0, 12) + "-" + f.replace(/^.*[\\/]/, "").replace(/\.\w+$/, ".o")));
  const inMirror = (p) => p.startsWith(root + "/") && !p.startsWith(out + "/") ? join(mirror, relative(root, p)) : p;
  const fromMirror = (text) => text.split(mirror + "/").join(root + "/");
  let notes = {};
  let mirrored = msvc && existsSync(mirror);
  if (mirrored) notes = syncMirror(root, mirror);
  const failed = new Map();
  let built = 0;
  for (let round = 0; round < 6; round++) {
    const args = [`-std=${std}`, "-ferror-limit=0", ...BASE, ...(mirrored ? flags.map((f) => f.startsWith("-I") ? `-I${inMirror(f.slice(2))}` : f.startsWith("/") ? inMirror(f) : f) : flags)];
    const stamp = args.join("\n");
    const todo = files.map((_, i) => i).filter((i) => round === 0 ? true : failed.has(i));
    failed.clear();
    let next = 0, done = 0;
    const worker = async () => {
      while (next < todo.length) {
        const i = todo[next++], o = objects[i], src = mirrored ? inMirror(files[i]) : files[i];
        if (!upToDate(o, stamp)) {
          built++;
          const r = await run("em++", [...args, "-c", src, "-MD", "-MF", o + ".d", "-o", o]);
          if (r.code !== 0) failed.set(i, r.err); else writeFileSync(o + ".flags", stamp);
        }
        done++;
        if (process.stdout.isTTY) process.stdout.write(`\r${label}: ${done}/${todo.length}${round ? ` (again, with fixes: ${round})` : ""}`);
      }
    };
    await Promise.all(Array.from({ length: Math.max(1, cpus().length) }, worker));
    if (process.stdout.isTTY) process.stdout.write("\r\x1b[K");
    if (!failed.size || !msvc) break;
    // What can be fixed: in the mirror (made now, if this is the first need of it).
    const fixable = fixableErrors([...failed.values()].join("\n"));
    if (!fixable.size) break;
    if (!mirrored) { notes = syncMirror(root, mirror); mirrored = true; }
    let any = false;
    // (an include's file, found in its folder's case: beside the file, or in an include folder)
    const includeDirs = flags.filter((f) => f.startsWith("-I")).map((f) => f.slice(2));
    for (const [file, errors] of fixable) for (const e of errors) {
      if (e.message !== "include") continue;
      const original = file.startsWith(mirror + "/") ? join(root, relative(mirror, file)) : file, want = e.name.replace(/\\/g, "/");
      for (const base of [dirname(original), ...includeDirs]) {
        const p = onDisk(resolve(base, want));
        if (p && p !== resolve(base, want)) { e.found = { name: e.name, to: relative(base, p).split("\\").join("/") }; break; }
      }
    }
    for (const [file, errors] of fixable) {
      const copy = file.startsWith(mirror + "/") ? file : inMirror(file);
      if (!existsSync(copy) || !copy.startsWith(mirror + "/")) continue;
      const lines = readFileSync(copy, "utf8").split("\n"), changed = fixLines(lines, errors);
      if (!changed.length) continue;
      writeFileSync(copy, lines.join("\n"));
      const rel = relative(mirror, copy);
      notes[rel] = [...(notes[rel] ?? []), ...changed];
      any = true;
    }
    writeFileSync(join(mirror, ".fixes.json"), JSON.stringify(notes));
    if (!any) break;
    if (round === 0) for (let i = 0; i < files.length; i++) if (!failed.has(i)) failed.set(i, "");   // (from the mirror now: all of them, as their flags changed)
  }
  const fixedFiles = Object.entries(notes).filter(([, n]) => n.length);
  if (fixedFiles.length) {
    const count = fixedFiles.reduce((n, [, l]) => n + l.length, 0);
    console.log(`${count} lines compiled as Visual Studio reads them, in ${fixedFiles.length} files (copies in out/msvc/, your files unchanged):`);
    for (const [f, l] of fixedFiles.slice(0, 12)) console.log(`  ${f}: ${l.length > 3 ? `${l.length} lines, ${[...new Set(l.map((x) => x.replace(/^\d+: /, "")))].join("; ")}` : l.join("; ")}`);
    if (fixedFiles.length > 12) console.log(`  (and ${fixedFiles.length - 12} more files: out/msvc/.fixes.json)`);
  }
  if (failed.size) {
    const list = [...failed].map(([i, err]) => explain(relative(root, files[i]), fromMirror(err)));
    throw new Error(`${failed.size} of ${files.length} files did not compile:\n\n${list.slice(0, 8).join("\n\n")}${list.length > 8 ? `\n\n(and ${list.length - 8} more)` : ""}`);
  }
  console.log(`${label}: ${files.length} files${built < files.length ? ` (${built} compiled, the rest unchanged)` : " compiled"}`);
  return objects;
}

/** A pattern's expression: `*` and `?` within one name, `**` for any number of folders. */
function patternRegex(pattern) {
  const parts = pattern.split("/"), last = parts.length - 1;
  let s = "";
  parts.forEach((p, i) => {
    if (p === "**") { s += "(?:[^/]+/)*"; return; }
    s += p.replace(/[.+^${}()|[\]\\]/g, "\\$&").replace(/\*/g, "[^/]*").replace(/\?/g, "[^/]") + (i < last ? "/" : "");
  });
  return new RegExp(`^${s}$`);
}

/** Paths under `root` (its out/ left out) matching a pattern, relative, with forward slashes. */
function glob(root, pattern) {
  const re = patternRegex(pattern), out = [];
  const walk = (dir) => {
    for (const name of readdirSync(dir)) {
      if (name === "out" && dir === root) continue;
      const path = join(dir, name), rel = relative(root, path).split("\\").join("/");
      if (statSync(path).isDirectory()) walk(path);
      else if (re.test(rel)) out.push(rel);
    }
  };
  walk(root);
  return out.sort();
}

const KEY = /^[a-z0-9_-]{1,32}$/;

/** The bot's scmbot.json, checked. */
function readConfig(dir) {
  const file = join(dir, "scmbot.json");
  if (!existsSync(file)) throw new Error(`no scmbot.json in ${dir} (${CMD} init writes one to start from)`);
  const c = JSON.parse(readFileSync(file, "utf8"));
  const list = (v, what) => { if (v === undefined) return []; if (!Array.isArray(v) || v.some((x) => typeof x !== "string")) throw new Error(`scmbot.json: ${what} is a list of strings`); return v; };
  if (typeof c.key !== "string" || !KEY.test(c.key)) throw new Error("scmbot.json: key is the bot's short name: a to z, 0 to 9, - and _, at most 32");
  if (typeof c.name !== "string" || !c.name.trim() || c.name.length > 32) throw new Error("scmbot.json: name is how the bot is shown, at most 32 characters");
  // The bot's room above the game's world (BWAPI's data and the bot's heap): botMB; or memoryMB, as scmbot 1 and 2 had
  // it, the whole memory when the bot sat above the game's 1 GB (the room is memoryMB less 1024).
  if (c.botMB !== undefined && c.memoryMB !== undefined) throw new Error("scmbot.json: give botMB (the bot's room) or memoryMB (as before: 1024 and the room), not both");
  const botMB = c.botMB ?? (c.memoryMB !== undefined ? c.memoryMB - 1024 : 768);
  if (!Number.isInteger(botMB) || botMB < 64 || botMB > 2047) throw new Error(c.botMB !== undefined
    ? "scmbot.json: botMB is the bot's room above the game's 2 GB: BWAPI's data and the bot's heap (64 to 2047; 768 if unsure)"
    : "scmbot.json: memoryMB is the game's 1024 MB and the bot's room above it (1088 to 3071): write botMB, the room alone, instead");
  if (typeof c.license !== "string" || !existsSync(join(dir, c.license))) throw new Error("scmbot.json: license names the bot's licence file (it goes with the build)");
  return {
    key: c.key, name: c.name.trim(), botMB, license: c.license,
    sources: list(c.sources, "sources"), exclude: list(c.exclude, "exclude"), include: list(c.include, "include"),
    defines: list(c.defines, "defines"), flags: list(c.flags, "flags"), prelude: typeof c.prelude === "string" ? c.prelude : null,
    std: typeof c.std === "string" && /^c\+\+(14|17|20)$/.test(c.std) ? c.std : "c++17",
    files: readFiles(dir, c.files ?? c.embed),
    source: typeof c.source === "string" ? c.source : "", commit: typeof c.commit === "string" ? c.commit : "",
    msvc: c.msvc !== false,
    entry: c.entry ? readEntry(c.entry) : null,
    mod: c.mod === undefined ? null : readMod(c.mod),
  };
}

/** scmbot.json's "mod": the mod the bot is made for, as `scmjs mod header --bot` writes it. */
function readMod(m) {
  if (!m || typeof m !== "object" || typeof m.name !== "string" || !m.name.trim() || m.name.length > 64 || typeof m.hash !== "string" || !/^[0-9a-f]{16}$/.test(m.hash))
    throw new Error("scmbot.json: mod is the mod the bot is made for: { \"name\": its name, \"hash\": its hash, sixteen hex digits (scmjs mod info shows it; scmjs mod header --bot writes it) }");
  if (m.link !== undefined && (typeof m.link !== "string" || !/^https?:\/\//.test(m.link))) throw new Error("scmbot.json: mod.link is the address of the mod's makers (http or https), or left out");
  return { name: m.name.trim(), hash: m.hash, ...(m.link ? { link: m.link } : {}) };
}

/**
 * The files the bot reads, put with it where it looks for them: "files": [{ "from": "bin/MyBot_Config.txt", "to":
 * "MyBot_Config.txt" }]. `to` is from StarCraft's folder (the folder a bot is run in), as on Windows: "bwapi-data/AI/x".
 */
function readFiles(dir, v) {
  if (v === undefined) return [];
  if (!Array.isArray(v)) throw new Error("scmbot.json: files is a list of { from, to }");
  return v.map((e) => {
    if (!e || typeof e.from !== "string" || typeof e.to !== "string") throw new Error("scmbot.json: each of files is { from: its path here, to: its path from StarCraft's folder }");
    if (!existsSync(join(dir, e.from))) throw new Error(`scmbot.json: files: no ${e.from}`);
    return { from: e.from, to: e.to.replace(/\\/g, "/").replace(/^(\.\/|\/)+/, "") };
  });
}

/** Data files the sources name in a string ("UAlbertaBot_Config.txt", "bwapi-data/AI/x.json") that are in the bot's folder, once. */
function namedFiles(root, sources) {
  const named = new Set();
  for (const f of sources) for (const m of readFileSync(join(root, f), "utf8").matchAll(/"((?:[\w.-]+[\\/]{1,2})*[\w.-]+\.(?:txt|json|cfg|ini|xml|ya?ml|dat|csv))"/g)) named.add(m[1].replace(/\\+/g, "/"));
  if (!named.size) return [];
  const all = [];
  const walk = (d, depth) => {
    if (depth > 6) return;
    for (const n of readdirSync(d)) {
      if (n === ".git" || n === "out" || n === "node_modules") continue;
      const p = join(d, n);
      if (statSync(p).isDirectory()) walk(p, depth + 1); else all.push(relative(root, p).split("\\").join("/"));
    }
  };
  walk(root, 0);
  const out = [];
  for (const to of named) {
    if (to.startsWith("bwapi-data/read/") || to.startsWith("bwapi-data/write/")) continue;   // (what it learns: kept by the page)
    const base = to.replace(/^.*\//, "").toLowerCase(), found = all.filter((p) => p.toLowerCase().endsWith(base) && p.replace(/^.*\//, "").toLowerCase() === base);
    if (found.length === 1 && statSync(join(root, found[0])).size < 4 << 20) out.push({ from: found[0], to });
  }
  return out;
}

function readEntry(e) {
  if (typeof e !== "object" || typeof e.class !== "string" || !/^[A-Za-z_][\w:]*$/.test(e.class)) throw new Error("scmbot.json: entry.class is the bot's class (namespace and all: UAlbertaBot::UAlbertaBotModule)");
  const strings = (v, what) => { if (v === undefined) return []; if (!Array.isArray(v) || v.some((x) => typeof x !== "string")) throw new Error(`scmbot.json: entry.${what} is a list of strings`); return v; };
  return { class: e.class, include: strings(e.include, "include"), setup: strings(e.setup, "setup") };
}

async function prepare() {
  await compile(bwapiSources(), HOME, [], "c++17", "BWAPI 4.4.0 and the host", false, OBJ);
}

/**
 * The module entry for a bot written as a BWAPI client program (its own main, a loop over the game's events, as
 * UAlbertaBot is): an AI module that makes the bot's class at each game's start and hands it the events it has
 * methods for. scmbot.json: "entry": { "class": "UAlbertaBot::UAlbertaBotModule", "include": ["UAlbertaBotModule.h"],
 * "setup": ["SparCraft::init();", "BOSS::init();"] } (setup: what its main ran once before the first game).
 */
function entrySource(e) {
  const events = [["onStart", ""], ["onEnd", "bool isWinner", "isWinner"], ["onFrame", ""], ["onSendText", "std::string text", "text"],
    ["onReceiveText", "BWAPI::Player player, std::string text", "player, text"], ["onPlayerLeft", "BWAPI::Player player", "player"],
    ["onNukeDetect", "BWAPI::Position target", "target"], ...["Discover", "Evade", "Show", "Hide", "Create", "Destroy", "Morph", "Renegade", "Complete"].map((n) => [`onUnit${n}`, "BWAPI::Unit unit", "unit"]),
    ["onSaveGame", "std::string name", "name"]];
  const has = events.map(([m]) => `  template <class T> static auto has_${m}(int) -> decltype(&T::${m}, std::true_type{});\n  template <class T> static std::false_type has_${m}(...);`).join("\n");
  // (each through a template, so that a method the class does not have is never looked for)
  const call = events.map(([m, params, args = ""]) =>
    `  template <class T> void call_${m}(T &b${params ? ", " + params : ""}) { if constexpr (decltype(has_${m}<T>(0))::value) b.${m}(${args}); }`).join("\n");
  const forward = events.filter(([m]) => m !== "onStart").map(([m, params, args = ""]) =>
    `    void ${m}(${params}) override { if (bot) call_${m}(*bot${args ? ", " + args : ""}); }`).join("\n");
  return `// Written by scmbot (scmbot.json's "entry"): the bot's class as a BWAPI AI module.
#include <BWAPI.h>
#include <memory>
#include <type_traits>
${e.include.map((h) => `#include "${h}"`).join("\n")}

namespace scmbot_entry
{
  using Bot = ${e.class};
${has}
${call}
  class Module : public BWAPI::AIModule
  {
    std::unique_ptr<Bot> bot;
  public:
    void onStart() override
    {
      static bool setUp = false;
      if (!setUp) { ${e.setup.join(" ")} setUp = true; }
      bot = std::make_unique<Bot>();
      call_onStart(*bot);
    }
${forward}
  };
}

extern "C" void gameInit(BWAPI::Game *game) { BWAPI::BroodwarPtr = game; }
extern "C" BWAPI::AIModule *newAIModule() { return new scmbot_entry::Module(); }
`;
}

async function build(dir) {
  dir = resolve(dir);
  const c = readConfig(dir);
  if (!existsSync(OBJ) || !readdirSync(OBJ).length) { console.log("BWAPI is not compiled yet here: compiling it first"); await prepare(); }
  const excluded = new Set(c.exclude.flatMap((p) => glob(dir, p)));
  const files = [...new Set(c.sources.flatMap((p) => glob(dir, p)))].filter((f) => !excluded.has(f) && /\.(cpp|cc|cxx|c)$/.test(f));
  if (!files.length) throw new Error("scmbot.json: sources match no file");
  const out = join(dir, "out");
  mkdirSync(out, { recursive: true });
  const sources = files.map((f) => join(dir, f));
  if (c.entry) { const f = join(out, "scmbot-entry.cpp"); writeFileSync(f, entrySource(c.entry)); sources.push(f); }
  console.log(`${c.name} (${c.key}): ${files.length} source files${c.entry ? `, and a module entry for ${c.entry.class}` : ""}`);
  // As Visual Studio reads a bot: its extensions, _MSC_VER (bots guard what needs BWAPI's Windows build with it),
  // the standard library's parts it pulls in by itself, std::random_shuffle.
  const msvc = c.msvc ? ["-fms-extensions", "-fms-compatibility", "-fms-compatibility-version=19.30", "-fdelayed-template-parsing", "-D_MSC_VER=1930", "-D_LIBCPP_ENABLE_CXX17_REMOVED_RANDOM_SHUFFLE", "-include", join(HOME, "toolchain/msvc-compat.h")] : [];
  const flags = [...msvc, ...c.include.map((i) => `-I${join(dir, i)}`), ...c.defines.map((d) => `-D${d}`), ...(c.prelude ? ["-include", join(dir, c.prelude)] : []), ...c.flags];
  const objects = await compile(sources, dir, flags, c.std, c.name, c.msvc);
  const bwapi = readdirSync(OBJ).filter((f) => f.endsWith(".o")).map((f) => join(OBJ, f));
  const mjs = join(out, `${c.key}.mjs`), wasm = join(out, `${c.key}.wasm`);
  const embed = c.files.flatMap((e) => ["--embed-file", `${join(dir, e.from)}@/${e.to}`]);
  const linked = await run("em++", [...linkFlags(c.botMB), ...embed, ...bwapi, ...objects, "-o", mjs]);
  if (linked.code !== 0) {
    const missing = [...new Set((linked.err.match(/undefined symbol: [^\n(]+/g) ?? []).map((m) => m.slice(18).trim()))];
    const hints = [];
    if (missing.some((m) => /gameInit|newAIModule/.test(m))) hints.push("no module entry: an AI module defines gameInit and newAIModule (BWAPI's ExampleAIModule/Source/Dll.cpp); a client program can have scmbot write one: \"entry\" in scmbot.json (" + CMD + " init fills it in when it finds the client's loop)");
    if (missing.some((m) => /BWAPIClient|BWAPI::Client/.test(m))) hints.push("a client program's own main is among the sources: leave it out (\"exclude\") and use \"entry\"");
    if (missing.some((m) => /BWTA|BWEM/.test(m)) && !hints.length) hints.push("a map analysis library's sources are not among the bot's: add their folder to \"sources\"");
    if (missing.length && !hints.length) hints.push("these are defined in no source given: a file left out of \"sources\", or a library the bot links on Windows (its .lib): its sources go in too");
    throw new Error(`the link failed${missing.length ? `, ${missing.length} names undefined: ${missing.slice(0, 10).join("; ")}${missing.length > 10 ? "; …" : ""}` : `:\n${linked.err.slice(-2000)}`}${hints.map((h) => `\n  hint: ${h}`).join("")}`);
  }
  const js = readFileSync(mjs), bin = readFileSync(wasm);
  const header = Buffer.from(JSON.stringify({ format: FORMAT, key: c.key, name: c.name, memoryPages: (BASE_MB + c.botMB) * 16, base: BASE_MB * 1048576, commit: c.commit, source: c.source, license: readFileSync(join(dir, c.license), "utf8"), toolchain: TOOLCHAIN, ...(c.mod ? { mod: c.mod } : {}), mjs: js.length, wasm: bin.length }));
  const head = Buffer.alloc(12);
  head.write("SCMBOT1\n", 0, "latin1");
  head.writeUInt32LE(header.length, 8);
  const bundle = join(out, `${c.key}.scmbot`);
  writeFileSync(bundle, Buffer.concat([head, header, js, bin]));
  console.log(`\nout/${c.key}.scmbot: ${(statSync(bundle).size / 1048576).toFixed(1)} MB${c.mod ? `, made for ${c.mod.name} (${c.mod.hash})` : ""}.\nOn the play page, Options → Community bots → Add a bot from a file…, then choose it for a computer.`);
}

/** Every .vcxproj under `root` (not in out/ or .git/). */
function projects(root) {
  const out = [];
  const walk = (d, depth) => {
    if (depth > 5) return;
    for (const name of readdirSync(d)) {
      if (name === ".git" || name === "out" || name === "node_modules") continue;
      const p = join(d, name);
      if (statSync(p).isDirectory()) walk(p, depth + 1);
      else if (name.endsWith(".vcxproj")) out.push(p);
    }
  };
  walk(root, 0);
  return out;
}

/** The nearest folder from `dir` up to `root` holding a file `match` takes. */
function findUp(dir, root, match) {
  for (let d = dir; d.startsWith(root); d = dirname(d)) {
    if (readdirSync(d).some(match)) return d;
    if (d === root) break;
  }
  return null;
}

/** `path` as it is on disk, its letters' case found as Windows would (or null). */
function onDisk(path) {
  if (existsSync(path)) return path;
  const parent = dirname(path);
  if (parent === path) return null;
  const found = onDisk(parent);
  if (!found) return null;
  const want = path.slice(parent.length + 1).toLowerCase();
  let names;
  try { names = readdirSync(found); } catch { return null; }
  const name = names.find((n) => n.toLowerCase() === want);
  return name ? join(found, name) : null;
}

/**
 * What a Visual Studio project builds: its sources, the folders they and its headers are in and the project's
 * include folders, defines, C++ standard, and the same of the static libraries it uses (by reference, or by a .lib
 * it links that a project here builds: UAlbertaBot's BOSS and SparCraft).
 */
function readProject(file, root, all, seen = new Set()) {
  const none = { sources: [], include: [], defines: [], std: null, application: false };
  if (seen.has(file)) return none;
  seen.add(file);
  const xml = readFileSync(file, "utf8"), here = dirname(file);
  // (Visual Studio's own macros for the project's and the solution's folders; a path with any other is left out)
  const solution = findUp(here, root, (n) => n.endsWith(".sln")) ?? dirname(here);
  const expand = (p) => p.replace(/\$\((ProjectDir|MSBuildProjectDirectory|MSBuildThisFileDirectory)\)/g, here + "/").replace(/\$\(SolutionDir\)/g, solution + "/");
  const local = (p) => { p = expand(p); if (/[$%]\(/.test(p)) return null; const f = onDisk(resolve(here, p.replace(/\\/g, "/"))); return f && f.startsWith(root) ? relative(root, f).split("\\").join("/") || "." : null; };
  const values = (tag) => [...xml.matchAll(new RegExp(`<${tag}>([^<]*)</${tag}>`, "g"))].map((m) => m[1]);
  const sources = [...xml.matchAll(/<ClCompile\s+Include="([^"]+)"/g)].map((m) => local(m[1])).filter(Boolean);
  const headers = [...xml.matchAll(/<ClInclude\s+Include="([^"]+)"/g)].map((m) => local(m[1])).filter(Boolean);
  const include = [...new Set([
    ...values("AdditionalIncludeDirectories").flatMap((v) => v.split(";")).map((v) => v.trim()).filter(Boolean).map(local).filter(Boolean),
    ...[...sources, ...headers].map((f) => dirname(f)).filter((d) => d !== "."),
  ])];
  const NOT = /^(WIN32|_WIN32|_WINDOWS|_DEBUG|DEBUG|NDEBUG|_USRDLL|_CONSOLE|_LIB|_CRT_SECURE_NO_WARNINGS|_SCL_SECURE_NO_WARNINGS|NOMINMAX|.*_EXPORTS|%\(.*\))$/;
  const defines = [...new Set(values("PreprocessorDefinitions").flatMap((v) => v.split(";")).map((v) => v.trim()).filter((v) => v && !NOT.test(v)))];
  // (the newest standard any of its configurations asks for: Release is usually the one that is played)
  const stds = values("LanguageStandard").map((l) => /20|latest/.test(l) ? 20 : /17/.test(l) ? 17 : /14/.test(l) ? 14 : 0);
  const std = stds.length && Math.max(...stds) ? `c++${Math.max(...stds)}` : null;
  const application = values("ConfigurationType").includes("Application");
  const refs = [...xml.matchAll(/<ProjectReference\s+Include="([^"]+)"/g)].map((m) => onDisk(resolve(here, m[1].replace(/\\/g, "/")))).filter(Boolean);
  const libs = values("AdditionalDependencies").flatMap((v) => v.split(";")).map((v) => v.trim().replace(/^.*[\\/]/, "").replace(/\.lib$/i, "").toLowerCase()).filter((l) => l && !/^bwapi|^bwta|%\(|^kernel32|^user32/.test(l));
  for (const p of all) if (libs.includes(p.replace(/^.*[\\/]/, "").replace(/\.vcxproj$/, "").toLowerCase())) refs.push(p);
  for (const ref of [...new Set(refs)]) {
    const r = readProject(ref, root, all, seen);
    sources.push(...r.sources); include.push(...r.include.filter((d) => !include.includes(d))); defines.push(...r.defines.filter((d) => !defines.includes(d)));
  }
  return { sources: [...new Set(sources)], include, defines, std, application };
}

/** In a client program's main: the class whose object gets the game's events, its header, and the init calls before. */
function clientEntry(root, sources) {
  for (const f of sources) {
    const text = readFileSync(join(root, f), "utf8");
    if (!/BWAPIClient/.test(text)) continue;
    const call = text.match(/(\w+)\s*(?:\.|->)\s*onFrame\s*\(\s*\)/);
    if (!call) continue;
    const decl = text.match(new RegExp(`([A-Za-z_][\\w:]*)\\s*(?:\\*\\s*)?\\b${call[1]}\\b\\s*(?:;|=|\\{|\\()`));
    if (!decl || /^(auto|return)$/.test(decl[1])) continue;
    const bare = decl[1].replace(/^.*::/, "");
    const ns = [...text.matchAll(/using\s+namespace\s+(\w+)\s*;/g)].map((m) => m[1]).filter((n) => n !== "BWAPI" && n !== "std");
    const header = headersDeclaring(root, bare);
    const cls = decl[1].includes("::") || !ns.length ? decl[1] : `${ns[0]}::${bare}`;
    const setup = [...text.matchAll(/^\s*([A-Za-z_][\w:]*::init\s*\(\s*\)\s*;)/gm)].map((m) => m[1].replace(/\s+/g, ""));
    const includes = [...text.matchAll(/#include\s+"([^"]+)"/g)].map((m) => m[1].replace(/^.*[\\/]/, "")).filter((h) => setup.some((s) => h.toLowerCase().startsWith(s.split("::")[0].toLowerCase())));
    return { main: f, entry: { class: cls, include: [...new Set([...header.slice(0, 1), ...includes])], setup } };
  }
  return null;
}

/** Headers under `root` declaring `class <name>`, by file name only. */
function headersDeclaring(root, name) {
  const out = [], re = new RegExp(`\\b(class|struct)\\s+${name}\\b[^;]*\\{`);
  const walk = (d, depth) => {
    if (depth > 6) return;
    for (const n of readdirSync(d)) {
      if (n === ".git" || n === "out") continue;
      const p = join(d, n);
      if (statSync(p).isDirectory()) walk(p, depth + 1);
      else if (/\.(h|hpp)$/i.test(n) && re.test(readFileSync(p, "utf8"))) out.push(n);
    }
  };
  walk(root, 0);
  return out;
}

/** The bot's folder's name (in the image, the folder outside that is mounted at /bot). */
const folderName = (root) => (root === "/bot" && process.env.SCMBOT_FOLDER) || root.replace(/^.*[\\/]/, "");

const LICENSES = ["LICENSE", "LICENSE.md", "LICENSE.txt", "License.md", "License.txt", "license.md", "license.txt", "COPYING", "COPYING.txt"];

function init(dir, chosen) {
  const root = resolve(dir), file = join(root, "scmbot.json");
  if (existsSync(file)) throw new Error(`${file} is there already: edit it, or remove it to start again`);
  const license = LICENSES.find((f) => existsSync(join(root, f))) ?? "LICENSE";
  const all = projects(root);
  const candidates = chosen ? [onDisk(resolve(root, chosen))].filter(Boolean) : all;
  if (chosen && !candidates.length) throw new Error(`no project ${chosen}`);
  const defines = (x) => x.r.sources.some((f) => /newAIModule/.test(readFileSync(join(root, f), "utf8")));
  // A bot's project: the one whose sources define the module entry, else a client program, else the largest.
  const read = candidates.map((p) => ({ p, r: readProject(p, root, all) })).filter((x) => x.r.sources.length);
  read.forEach((x) => { x.module = defines(x); x.client = !x.module && clientEntry(root, x.r.sources); });
  read.sort((a, b) => Number(b.module) - Number(a.module) || Number(!!b.client) - Number(!!a.client) || b.r.sources.length - a.r.sources.length);
  const best = read[0];
  const base = (best ? best.p.replace(/^.*[\\/]/, "").replace(/\.vcxproj$/, "") : folderName(root)) || "mybot";
  const key = base.toLowerCase().replace(/[^a-z0-9_-]+/g, "-").replace(/^-+|-+$/g, "").slice(0, 32) || "mybot";
  const git = readGit(root);
  const config = { key, name: base.slice(0, 32), license, sources: ["src/**/*.cpp"], exclude: [], include: ["src"], defines: [], std: "c++17", botMB: 768, source: git.url, commit: git.commit };
  if (best) {
    config.sources = best.client ? best.r.sources.filter((f) => f !== best.client.main) : best.r.sources;
    Object.assign(config, { include: best.r.include, defines: best.r.defines, std: best.r.std ?? "c++17" });
    if (best.client) config.entry = best.client.entry;
    const files = namedFiles(root, best.r.sources);
    if (files.length) config.files = files;
  }
  // (its fields in the order a person reads them)
  const ordered = { key: config.key, name: config.name, license: config.license, source: config.source, commit: config.commit, ...config };
  writeFileSync(file, JSON.stringify(ordered, null, 2) + "\n");
  const say = [];
  if (best) say.push(`scmbot.json written from ${relative(root, best.p)}: ${config.sources.length} sources, ${config.include.length} include folders${config.defines.length ? `, defines ${config.defines.join(" ")}` : ""}.`);
  else say.push("scmbot.json written. No Visual Studio project here: name the bot's sources and include folders in it.");
  if (read.length > 1) say.push(`(${read.length} projects here; for another: ${CMD} init <its .vcxproj>)`);
  if (config.files) say.push(`Files it reads, put with it: ${config.files.map((f) => f.to).join(", ")} (check "files").`);
  if (best?.client) say.push(`It is a client program: ${best.client.main} is left out, and the module entry makes ${best.client.entry.class} for each game${best.client.entry.setup.length ? `, after ${best.client.entry.setup.join(" ")} once` : ""}. Check "entry".`);
  else if (best && !best.module) say.push("note: no source defines newAIModule, and no client loop was found: the bot needs a module entry (see \"entry\" in the guide).");
  if (!existsSync(join(root, license))) say.push(`note: no licence file found: name the bot's in "license" (it goes with every build).`);
  say.push(`next: ${CMD} build`);
  console.log(say.join("\n"));
}

/** The repository's address and commit, when the folder is a git clone (read from .git, no git needed). */
function readGit(root) {
  try {
    const git = join(root, ".git");
    const headRef = readFileSync(join(git, "HEAD"), "utf8").trim();
    let commit = headRef;
    if (headRef.startsWith("ref: ")) {
      const ref = headRef.slice(5);
      commit = existsSync(join(git, ref)) ? readFileSync(join(git, ref), "utf8").trim()
        : (readFileSync(join(git, "packed-refs"), "utf8").split("\n").find((l) => l.endsWith(" " + ref)) ?? "").split(" ")[0];
    }
    const cfg = readFileSync(join(git, "config"), "utf8");
    const url = (cfg.match(/\[remote "origin"\][^[]*?url\s*=\s*(\S+)/) ?? [])[1] ?? "";
    return { url: url.replace(/^git@github\.com:/, "https://github.com/").replace(/\.git$/, ""), commit: /^[0-9a-f]{40}$/.test(commit) ? commit : "" };
  } catch { return { url: "", commit: "" }; }
}

/** A bot to start from: a protoss, terran or zerg that sends its workers to minerals and trains more. */
function starter(dir) {
  const root = resolve(dir);
  if (existsSync(join(root, "scmbot.json"))) throw new Error("scmbot.json is there already: this folder has a bot");
  mkdirSync(join(root, "src"), { recursive: true });
  const write = (f, text) => { if (!existsSync(join(root, f))) writeFileSync(join(root, f), text); };
  write("src/MyBot.h", `#pragma once
#include <BWAPI.h>

class MyBot : public BWAPI::AIModule
{
public:
  void onStart() override;
  void onFrame() override;
  void onEnd(bool isWinner) override;

private:
  int supplyOrdered = -1000;
};
`);
  write("src/MyBot.cpp", `#include "MyBot.h"

using namespace BWAPI;

void MyBot::onStart()
{
  Broodwar->sendText("gl hf");
}

void MyBot::onFrame()
{
  // The game asks the bot every frame; commands go out every few frames (latency), so not every frame needs any.
  if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) return;
  Player me = Broodwar->self();
  UnitType supply = me->getRace().getSupplyProvider();
  // Supply first when it runs short (and not again until the last one has had time to start).
  bool shortOfSupply = me->supplyTotal() < 400 && me->supplyTotal() - me->supplyUsed() <= 4 && Broodwar->getFrameCount() > supplyOrdered + 400;
  for (auto &unit : me->getUnits())
  {
    if (!unit->exists() || !unit->isCompleted()) continue;
    if (unit->getType().isWorker())
    {
      if (shortOfSupply && supply.isBuilding() && me->minerals() >= supply.mineralPrice() && !unit->isCarryingMinerals())
      {
        TilePosition at = Broodwar->getBuildLocation(supply, unit->getTilePosition());
        if (at != TilePositions::Invalid && unit->build(supply, at)) { supplyOrdered = Broodwar->getFrameCount(); shortOfSupply = false; }
      }
      else if (unit->isIdle())
        unit->gather(unit->getClosestUnit(Filter::IsMineralField));
    }
    else if (unit->getType().isResourceDepot() && unit->isIdle())
    {
      if (shortOfSupply && !supply.isBuilding() && me->minerals() >= supply.mineralPrice())
      {
        if (unit->train(supply)) { supplyOrdered = Broodwar->getFrameCount(); shortOfSupply = false; }   // (zerg: an overlord from a larva)
      }
      else if (me->minerals() >= 50 && !shortOfSupply)
        unit->train(me->getRace().getWorker());
    }
  }
}

void MyBot::onEnd(bool isWinner)
{
}
`);
  write("src/Dll.cpp", `// The two functions BWAPI looks for in an AI module.
#include <BWAPI.h>
#include "MyBot.h"

extern "C" void gameInit(BWAPI::Game *game) { BWAPI::BroodwarPtr = game; }
extern "C" BWAPI::AIModule *newAIModule() { return new MyBot(); }
`);
  const name = folderName(root) || "mybot";
  const key = name.toLowerCase().replace(/[^a-z0-9_-]+/g, "-").replace(/^-+|-+$/g, "").slice(0, 32) || "mybot";
  writeFileSync(join(root, "scmbot.json"), JSON.stringify({ key, name: name.slice(0, 32), license: "LICENSE", source: "", commit: "", sources: ["src/**/*.cpp"], include: ["src"], std: "c++17", botMB: 512 }, null, 2) + "\n");
  console.log(`a bot to start from: src/MyBot.cpp (workers mine, the base trains more), scmbot.json.${existsSync(join(root, "LICENSE")) ? "" : "\nAdd a LICENSE file (any licence you choose): it goes with every build."}\nnext: ${CMD} build`);
}

const COMMANDS = [
  ["new", "a bot to start from, in this folder"],
  ["init [project.vcxproj]", "scmbot.json for the bot in this folder, read from its Visual Studio project"],
  ["build", "out/<key>.scmbot, to add on the play page"],
];
const USAGE = `${CMD}: build a BWAPI 4.4.0 bot for scm-sim's play page.

${COMMANDS.map(([c, what]) => `  ${`${CMD} ${c}`.padEnd(CMD.length + 26)}${what}`).join("\n")}

Guide: https://github.com/scm-js/scmbot`;

/** The command line (not when a test imports this file for fixLines and the rest). */
async function main() {
const [command = "build", where] = process.argv.slice(2);
const here = existsSync("/bot") ? "/bot" : ".";
try {
  if (command === "prepare") await prepare();
  else if (command === "build") await build(where ?? here);
  else if (command === "init") init(here, where);
  else if (command === "new") starter(here);
  else if (command === "help" || command === "--help" || command === "-h") console.log(USAGE);
  else throw new Error(`no command ${command}\n\n${USAGE}`);
} catch (e) { console.error(`${CMD}: ${e instanceof Error ? e.message : e}`); process.exit(1); }
}
if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) await main();

export { entrySource, fixLines, fixableErrors, patternRegex, readMod };
