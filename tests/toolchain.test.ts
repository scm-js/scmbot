// The toolchain's pieces that need no Emscripten (toolchain/scmbot.mjs): what it rewrites of Visual Studio's C++,
// the source patterns, and the module entry it writes for a client bot.
import { describe, expect, it } from "vitest";
import { entrySource, fixLines, fixableErrors, patternRegex } from "../toolchain/scmbot.mjs";

const fix = (line: string, col: number, message: string) => {
  const lines = [line];
  const changed = fixLines(lines, [{ line: 0, col, message }]) as string[];
  return { text: lines[0], changed: changed.length };
};
const TEMP = "non-const lvalue reference to type 'X' cannot bind to a temporary of type 'X'";

describe("scmbot: Visual Studio's C++ as clang takes it", () => {
  it("a reference declared to hold a temporary becomes &&", () => {
    const line = "        auto & it = std::find(v.begin(), v.end(), b);";
    expect(fix(line, line.indexOf("it ="), TEMP).text).toBe("        auto && it = std::find(v.begin(), v.end(), b);");
    const typed = "    BuildOrder & buildOrder = m.getBuildOrder();";
    expect(fix(typed, typed.indexOf("m.get"), TEMP).text).toBe("    BuildOrder && buildOrder = m.getBuildOrder();");
  });
  it("a temporary handed to a reference parameter goes through scmbot_lvalue", () => {
    const line = "  S().add(name, Strategy(name, race, order));";
    expect(fix(line, line.indexOf("Strategy("), TEMP).text).toBe("  S().add(name, scmbot_lvalue(Strategy(name, race, order)));");
  });
  it("a ?: with a void arm casts the other", () => {
    const line = "void set(U *u) { u ? c = u->weak() : c.reset(); }";
    expect(fix(line, line.indexOf("?"), "right operand to ? is void, but left operand is of type 'weak_ptr<U>'").text)
      .toBe("void set(U *u) { u ? (void)(c = u->weak()) : c.reset(); }");
    const left = "x ? f() : y = 2;";
    expect(fix(left, 2, "left operand to ? is void, but right operand is of type 'int'").text).toBe("x ? f() : (void)(y = 2);");
  });
  it("a lambda handed temporaries takes auto && (or const T &)", () => {
    const line = "    auto f = [&](auto &t, int n) { return t; };";
    expect(fix(line, line.indexOf("["), "lambda-params").text).toBe("    auto f = [&](auto &&t, int n) { return t; };");
    const typed = "  test(path, [&](Position &p) {";
    expect(fix(typed, typed.indexOf("["), "lambda-params").text).toBe("  test(path, [&](const Position &p) {");
  });
  it("a lambda at namespace scope captures nothing", () => {
    const line = "    const auto ok = [&](auto &t) { return vis(t) > 0; };";
    expect(fix(line, line.indexOf("[") + 1, "non-local lambda expression cannot have a capture-default").text).toBe("    const auto ok = [](auto &t) { return vis(t) > 0; };");
  });
  it("finds the errors it can fix, a lambda's at the lambda", () => {
    const err = [
      "/bot/a.cpp:3:10: error: non-local lambda expression cannot have a capture-default",
      "/bot/u.h:9:21: error: no matching function for call to object of type '(lambda at /bot/b.cpp:55:45)'",
      "/bot/c.cpp:1:1: error: something else",
    ].join("\n");
    const found = fixableErrors(err) as Map<string, { line: number; col: number; message: string }[]>;
    expect([...found.keys()]).toEqual(["/bot/a.cpp", "/bot/b.cpp"]);
    expect(found.get("/bot/b.cpp")).toEqual([{ line: 54, col: 44, message: "lambda-params" }]);
  });
  it("an include named in another case than its file's", () => {
    const lines = ['#include "../Common/util.H"'];
    fixLines(lines, [{ line: 0, col: 10, message: "include", found: { name: "../Common/util.H", to: "../common/Util.h" } }]);
    expect(lines[0]).toBe('#include "../common/Util.h"');
    const found = fixableErrors("/bot/a.cpp:1:10: fatal error: 'Util.H' file not found") as Map<string, { name?: string }[]>;
    expect(found.get("/bot/a.cpp")?.[0].name).toBe("Util.H");
  });
  it("leaves a line it cannot read alone", () => {
    expect(fix("int x = 1;", 4, TEMP).changed).toBe(0);
  });
});

describe("scmbot: sources and entry", () => {
  it("source patterns", () => {
    expect(patternRegex("src/**/*.cpp").test("src/a/b/c.cpp")).toBe(true);
    expect(patternRegex("src/**/*.cpp").test("src/c.cpp")).toBe(true);
    expect(patternRegex("src/*.cpp").test("src/a/c.cpp")).toBe(false);
    expect(patternRegex("**/main.cpp").test("x/main.cpp")).toBe(true);
  });
  it("the module entry forwards the events the class has", () => {
    const text = entrySource({ class: "UAB::Module", include: ["Module.h"], setup: ["BOSS::init();"] }) as string;
    expect(text).toContain('#include "Module.h"');
    expect(text).toContain("using Bot = UAB::Module;");
    expect(text).toContain("BOSS::init();");
    expect(text).toContain('extern "C" BWAPI::AIModule *newAIModule()');
    expect(text).toMatch(/void onUnitDestroy\(BWAPI::Unit unit\) override \{ if \(bot\) call_onUnitDestroy\(\*bot, unit\); \}/);
  });
});

/** A bundle as scmbot writes one. */
function bundle(header: Record<string, unknown>, mjs = new TextEncoder().encode("export default 1;"), wasm = new Uint8Array([0, 0x61, 0x73, 0x6d, 1, 0, 0, 0])) {
  const h = new TextEncoder().encode(JSON.stringify({ format: 1, key: "mybot", name: "My Bot", memoryPages: 1792 * 16, commit: "abc", source: "", license: "MIT", toolchain: "t", mjs: mjs.length, wasm: wasm.length, ...header }));
  const out = new Uint8Array(12 + h.length + mjs.length + wasm.length);
  out.set(new TextEncoder().encode("SCMBOT1\n"));
  new DataView(out.buffer).setUint32(8, h.length, true);
  out.set(h, 12); out.set(mjs, 12 + h.length); out.set(wasm, 12 + h.length + mjs.length);
  return out;
}
