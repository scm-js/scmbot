/**
 * Starter: the example bot. It is written to be read — a whole bot that plays all three races and beats the game's own
 * computer more often than not, in a few dozen lines. Copy it and change it.
 *
 * A bot written this way exports `onFrame(game)`. The game calls it every few frames; the bot looks at `game` and gives
 * its orders, and the layer underneath takes care of the bookkeeping: counting what is on its way, keeping money aside
 * for buildings, finding builders and sites. Each line asks for something, and what is asked for first is paid for
 * first — so the order of the lines below is the bot's order of priorities.
 *
 * The page and `scmjs bot run` play this file as it is. Its types (for an editor's completion) are those of the npm
 * package @scm-js/bot.
 */
/** Each race's first fighting building and the unit it makes. The game's names are forgiving: "Marine" will do. */
const ARMY = {
  Terran: { building: "Barracks", unit: "Marine" },
  Protoss: { building: "Gateway", unit: "Zealot" },
  Zerg: { building: "Spawning_Pool", unit: "Zergling" },
};

/** How many fighters go out together. */
const WAVE = 12;

/** @type {import("@scm-js/bot").EasyBot} */
export default {
  name: "Starter",
  version: "2",

  onFrame(game) {
    const { building, unit } = ARMY[game.race.name];
    const zerg = game.race.name === "Zerg";

    // 1. The economy. Idle workers go to the minerals, a supply building (or overlord) goes up before supply runs
    // out, and workers are trained up to 18. `want` counts what is already on its way, so it is safe every frame.
    game.mine();
    game.keepSupply();
    game.want(game.race.worker, 18);

    // 2. Production: two of the fighting building (a zerg needs one pool; its units come from larvae).
    game.want(building, zerg ? 1 : 2);

    // 3. Growth. A new base before the minerals at ours run out — for zerg also whenever money piles up, since each
    // hatchery brings more larvae. For the others, piling money means one more production building, up to six.
    const mineralsLeft = game.ownBases.reduce((sum, base) => sum + game.mineralsLeft(base), 0);
    const ready = game.done(building).length > 0;
    if (game.planned(game.race.hall) === 0 && (mineralsLeft < 4000 || (zerg && ready && game.minerals > 400))) game.expand();
    if (!zerg && ready && game.minerals > 400 && game.planned(building) === 0 && game.count(building) < 6) game.build(building);

    // 4. The army: as many fighters as there is money for.
    game.want(unit, 200);

    // 5. Attack. A dozen at home leave together for the enemy's base; once out, a fighter with nothing left to do
    // goes on to the next thing of theirs it knows of (`enemyBase` searches the map when it knows of nothing).
    const target = game.enemyBase();
    const atHome = game.army.filter((u) => !u.memory.out);
    if (atHome.length >= WAVE && target) {
      for (const u of atHome) u.memory.out = true;
      atHome.attack(target);
      game.log(`a wave of ${atHome.length} goes to ${Math.round(target.x)},${Math.round(target.y)}`);
    }
    game.army.filter((u) => u.memory.out).idle.attack(target);
  },
};
