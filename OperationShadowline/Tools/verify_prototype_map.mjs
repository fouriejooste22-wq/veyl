/**
 * Verifies the field prototype's map is actually playable.
 *
 * Extracts the map data and the pure geometry/pathfinding code straight out of
 * the HTML (no DOM involved) and checks the things that would silently make the
 * mission impossible: an objective sealed inside a wall, a guard waypoint in the
 * river, or an exfil zone with no route to it across the one bridge.
 *
 * Run: node Tools/verify_prototype_map.mjs
 */
import fs from "node:fs";

const html = fs.readFileSync(new URL("../Prototype/shadowline-field.html", import.meta.url), "utf8");

const from = html.indexOf("var MAP = {");
const to = html.indexOf("/* ---------------------------------------------------------------- state */");
if (from < 0 || to < 0) { console.error("Could not locate the map/geometry block."); process.exit(1); }

const src = html.slice(from, to);
const mod = new Function(src + `
  return { MAP, OBJECTS, ROUTES, solid, wallAt, waterAt, onBridge, inReeds, findPath, walkable, dist };
`)();
const { MAP, OBJECTS, ROUTES, solid, wallAt, waterAt, onBridge, findPath, walkable } = mod;

let pass = 0, fail = 0;
const ok = (c, msg) => { if (c) { pass++; console.log("  pass  " + msg); } else { fail++; console.log("  FAIL  " + msg); } };

console.log("Map: " + MAP.w + " x " + MAP.h + " m, " + MAP.walls.length + " wall rects, "
  + MAP.water.length + " water zones, " + ROUTES.length + " patrol routes\n");

// --- the start point itself ---
const S = MAP.start;
ok(!solid(S.x, S.y), `insertion point (${S.x},${S.y}) is clear`);

// --- deep water blocks, and the bridge overrides it ---
ok(solid(20, 20), "west river centre is impassable");
ok(solid(72, 20), "east river centre is impassable");
ok(solid(20, 46), "main river centre is impassable");
ok(!solid(48, 46), "the bridge crosses the main river");
ok(onBridge(48, 46), "bridge rect covers the river crossing");
ok(!solid(17, 30), "shallow margin is wadeable");
ok(!solid(32, 69), "the reed channel is wadeable, so the exfil is usable");

// --- every guard waypoint must be standable, or the patrol breaks ---
let badWp = [];
ROUTES.forEach((r, i) => r.forEach(([x, y], j) => {
  if (solid(x, y)) badWp.push(`route ${i} waypoint ${j} (${x},${y})`);
}));
ok(badWp.length === 0, "all " + ROUTES.reduce((n, r) => n + r.length, 0)
  + " patrol waypoints are standable" + (badWp.length ? " — " + badWp.join("; ") : ""));

// --- a guard must be able to walk its whole loop ---
let brokenRoutes = [];
ROUTES.forEach((r, i) => {
  for (let j = 0; j < r.length; j++) {
    const a = r[j], b = r[(j + 1) % r.length];
    if (!findPath(a[0], a[1], b[0], b[1])) brokenRoutes.push(`route ${i} leg ${j}`);
  }
});
ok(brokenRoutes.length === 0, "every patrol route is fully traversable"
  + (brokenRoutes.length ? " — " + brokenRoutes.join("; ") : ""));

// --- objectives must be reachable and not sealed in geometry ---
let sealed = [], unreachable = [];
for (const o of OBJECTS) {
  if (solid(o.x, o.y)) sealed.push(`${o.id} (${o.x},${o.y})`);
  else if (!findPath(S.x, S.y, o.x, o.y)) unreachable.push(`${o.id} (${o.x},${o.y})`);
}
ok(sealed.length === 0, "no objective is inside geometry" + (sealed.length ? " — " + sealed.join("; ") : ""));
ok(unreachable.length === 0, "all " + OBJECTS.length + " objectives are reachable from the insertion point"
  + (unreachable.length ? " — " + unreachable.join("; ") : ""));

// --- the mill interior, behind a doorway gap ---
ok(!solid(53, 24), "mill office floor is clear (the notebook's room)");
ok(!!findPath(S.x, S.y, 53, 24), "the mill office is reachable — the doorway gaps work");
ok(!!findPath(48, 55, 32, 69), "the south bank is not cut in two by the channel");
ok(wallAt(48, 21), "the mill's interior divider is solid where it should be");
ok(!solid(48, 25), "the divider's doorway gap is passable");

// --- exfil: the whole mission hinges on this being reachable ---
const ex = MAP.exfil, exc = [ex.x + ex.w / 2, ex.y + ex.h / 2];
ok(!solid(exc[0], exc[1]), `exfil centre (${exc[0]},${exc[1]}) is clear`);
const exPath = findPath(S.x, S.y, exc[0], exc[1]);
ok(!!exPath, "exfil is reachable from the insertion point");
if (exPath) {
  const crossed = exPath.some(([x, y]) => onBridge(x, y));
  ok(crossed, "the route south crosses the bridge — it is the only way over");
  console.log("        route is " + exPath.length + " steps");
}

// --- a full mission route: start -> mill -> all markers -> exfil ---
const legs = [[S.x, S.y], [53, 24], [25, 36], [72, 42], [47, 51], exc];
let legFail = [];
for (let i = 0; i < legs.length - 1; i++) {
  if (!findPath(legs[i][0], legs[i][1], legs[i + 1][0], legs[i + 1][1])) legFail.push(`leg ${i}`);
}
ok(legFail.length === 0, "the full objective route is walkable end to end"
  + (legFail.length ? " — " + legFail.join("; ") : ""));

// --- reeds must actually overlap walkable ground, or concealment is unusable ---
let deadReeds = MAP.reeds.filter(r => {
  for (let y = r.y + 0.5; y < r.y + r.h; y += 1)
    for (let x = r.x + 0.5; x < r.x + r.w; x += 1)
      if (!solid(x, y)) return false;
  return true;
});
ok(deadReeds.length === 0, "every reed bed has standable ground in it"
  + (deadReeds.length ? ` — ${deadReeds.length} unusable` : ""));

// --- walkable area sanity: the map must not be mostly blocked ---
let free = 0, total = 0;
for (let y = 0; y < MAP.h; y++) for (let x = 0; x < MAP.w; x++) { total++; if (walkable(x, y)) free++; }
const pct = (free / total * 100);
ok(pct > 55 && pct < 95, `walkable area is ${pct.toFixed(1)}% — a sane amount of the map`);

console.log(`\n${pass} passed, ${fail} failed.`);
console.log("\nThis checks the map's geometry and connectivity, not how the game feels.");
process.exit(fail ? 1 : 0);
