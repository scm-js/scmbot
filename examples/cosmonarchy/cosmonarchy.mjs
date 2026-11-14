// A bot made for Cosmonarchy BW, in JavaScript: the same bot as CosmonarchyBot.cpp beside it. Its Masons mine, the
// Ministry trains more of them, a Mason puts up a Stockade (and more of them, up to four, while minerals pile up), the
// Stockades train Mavericks, and every six Mavericks attack the enemy's base.
//
// It is one plain file with no imports: drop it on the play page ("Add a bot from a file"), or run it with
//   scmjs bot run cosmonarchy.mjs <a map> --mod "<the Cosmonarchy release>"
// In a mod's game the bot is told the mod (`game.init.mod`) and gets its tables with every type called by the mod's
// own name — "Terran_Mason", or just "Mason" — so it needs no header. (`scmjs mod header --js` writes the same names
// as a module, for an editor's completion, when you want them.)

const WORKERS = 14, WAVE = 6, STOCKADES = 4;

export default {
  name: "Cosmonarchy Example (JavaScript)",

  onStart(game) {
    const mod = game.init.mod;
    if (mod?.name !== "Cosmonarchy BW") game.log(`made for Cosmonarchy BW, playing ${mod ? mod.name : "the game's own units"}: the names below may mean nothing here`);
    if (game.race.name !== "Terran") game.log("I play Cosmonarchy's terrans only: my workers will mine, nothing more");
  },

  onFrame(game) {
    game.mine();
    if (game.race.name !== "Terran") return;

    // Masons up to fourteen (counting the ones in training), from the Ministry.
    game.want("Terran_Mason", WORKERS);

    // A Stockade, and one more while minerals pile up, one at a time, up to four.
    const stockades = game.count("Terran_Stockade");
    const more = stockades === 0 || (stockades < STOCKADES && game.minerals >= 2 * game.info("Terran_Stockade").mineralPrice);
    if (more && game.planned("Terran_Stockade") === 0) game.build("Terran_Stockade");

    // Mavericks from every Stockade, with what is left.
    game.want("Terran_Maverick", 200);

    // Every six at home go; once out, a Maverick with nothing to do goes on to the next building of theirs.
    const target = game.enemyBase();
    const atHome = game.army.of("Terran_Maverick").filter((u) => !u.memory.out);
    if (atHome.length >= WAVE && target) {
      for (const u of atHome) u.memory.out = true;
      atHome.attack(target);
    }
    game.army.filter((u) => u.memory.out).idle.attack(target);
  },

  onEnd(result, game) {
    game.log(result.result === "victory" ? "gg: a win for Cosmonarchy BW" : "gg");
  },
};
