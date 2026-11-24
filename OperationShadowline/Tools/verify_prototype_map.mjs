/**
 * Verifies the Kolda Range prototype map is actually playable.
 *
 * The terrain is fractal and movement is blocked by slope, so a patrol route or
 * an objective can very easily end up somewhere nobody can stand or reach. This
 * extracts the terrain, collision and pathfinding code straight out of the HTML
 * (no renderer involved) and checks connectivity for real.
 *
 * Run: node Tools/verify_prototype_map.mjs
 */
import fs from "node:fs";
const html = fs.readFileSync(new URL("../Prototype/shadowline-3d.html", import.meta.url), "utf8");

const from = html.indexOf("/* ---------------------------------------------------------------- helpers */");
const to = html.indexOf("   RENDERER AND WORLD");
if (from < 0 || to < 0) { console.error("Could not locate the terrain block."); process.exit(1); }
const src = html.slice(from, html.lastIndexOf("/* ===", to));

const M = new Function(src + `
  buildHeightmap(); buildBuildingList(); WALLS = buildingRects(); buildBlockGrid();
  return { MAP, TERRACES, RAMPS, ROAD, TARN, EXFIL, START, OBJECTS, ROUTES, BUILDINGS,
           SNOWLINE, TREELINE, MAXSLOPE, terrainH, slopeAt, solid, walkable, findPath,
           onRoad, onTerrace, inTarn, wallAt, dist };
`)();

let pass = 0, fail = 0;
const ok = (c, m) => { if (c) { pass++; console.log("  pass  " + m); } else { fail++; console.log("  FAIL  " + m); } };

// elevation profile
let lo = 1e9, hi = -1e9;
for (let z = 0; z <= M.MAP.h; z += 2) for (let x = 0; x <= M.MAP.w; x += 2) {
  const h = M.terrainH(x, z); if (h < lo) lo = h; if (h > hi) hi = h;
}
console.log(`Playfield ${M.MAP.w} x ${M.MAP.h} m, elevation ${lo.toFixed(1)} to ${hi.toFixed(1)} m`);
console.log(`Snow line ${M.SNOWLINE} m, treeline ${M.TREELINE} m, max walkable slope ${M.MAXSLOPE}\n`);

ok(hi - lo > 45, `the valley has real relief (${(hi - lo).toFixed(0)} m of it)`);
ok(hi > M.SNOWLINE + 10, "ground rises well above the snow line, so there is snow to see");
ok(lo < 8, "there is a valley floor to walk along");

// start and exfil
ok(!M.solid(M.START.x, M.START.y), `insertion point (${M.START.x},${M.START.y}) is standable`);
const ex = M.EXFIL, exc = [ex.x + ex.w / 2, ex.y + ex.h / 2];
ok(!M.solid(exc[0], exc[1]), `the pass notch (${exc[0]},${exc[1]}) is standable`);
const exPath = M.findPath(M.START.x, M.START.y, exc[0], exc[1]);
ok(!!exPath, "the north pass is reachable from the insertion point");
if (exPath) console.log(`        valley-to-pass route is ${exPath.length} steps, climbing `
  + `${(M.terrainH(exc[0],exc[1]) - M.terrainH(M.START.x,M.START.y)).toFixed(0)} m`);

// terraces
// Sample across each terrace rather than only its centre: a building standing
// on the middle of a shelf is correct, not a fault.
let badT = [];
M.TERRACES.forEach((T, i) => {
  let open = 0, tot = 0, reached = false;
  for (let z = T.z + 2; z < T.z + T.d - 2; z += 3)
    for (let x = T.x + 1.5; x < T.x + T.w - 1.5; x += 2) {
      tot++;
      if (!M.solid(x, z)) {
        open++;
        if (!reached && M.findPath(M.START.x, M.START.y, x, z)) reached = true;
      }
    }
  const frac = open / tot;
  if (frac < 0.55) badT.push(`terrace ${i} only ${(frac*100).toFixed(0)}% standable`);
  else if (!reached) badT.push(`terrace ${i} unreachable`);
  else console.log(`        terrace ${i}: ${(frac*100).toFixed(0)}% open, reachable`);
});
ok(badT.length === 0, "all four terraces are walkable and reachable" + (badT.length ? " — " + badT.join("; ") : ""));

// buildings sit on their terrace, not in mid-air or buried
let badB = [];
M.BUILDINGS.forEach(b => {
  const c = [b.x + b.w / 2, b.z + b.d / 2];
  const corners = [[b.x,b.z],[b.x+b.w,b.z],[b.x,b.z+b.d],[b.x+b.w,b.z+b.d]].map(p => M.terrainH(p[0],p[1]));
  const spread = Math.max(...corners) - Math.min(...corners);
  if (spread > 2.6) badB.push(`${b.kind} at ${c} sits on a ${spread.toFixed(1)} m step`);
});
ok(badB.length === 0, "every building sits on reasonably level ground" + (badB.length ? " — " + badB.join("; ") : ""));

// objectives
let sealed = [], unreach = [];
for (const o of M.OBJECTS) {
  if (M.solid(o.x, o.z)) sealed.push(`${o.id} (${o.x},${o.z})`);
  else if (!M.findPath(M.START.x, M.START.y, o.x, o.z)) unreach.push(`${o.id} (${o.x},${o.z})`);
}
ok(sealed.length === 0, "no objective is in unstandable ground" + (sealed.length ? " — " + sealed.join("; ") : ""));
ok(unreach.length === 0, `all ${M.OBJECTS.length} objectives are reachable` + (unreach.length ? " — " + unreach.join("; ") : ""));

// patrol routes
let badWp = [], badLeg = [];
M.ROUTES.forEach((r, i) => {
  r.forEach(([x, z], j) => { if (M.solid(x, z)) badWp.push(`route ${i} wp ${j} (${x},${z})`); });
  for (let j = 0; j < r.length; j++) {
    const a = r[j], b = r[(j + 1) % r.length];
    if (!M.solid(a[0],a[1]) && !M.solid(b[0],b[1]) && !M.findPath(a[0], a[1], b[0], b[1])) badLeg.push(`route ${i} leg ${j}`);
  }
});
ok(badWp.length === 0, `all ${M.ROUTES.reduce((n,r)=>n+r.length,0)} patrol waypoints are standable` + (badWp.length ? " — " + badWp.join("; ") : ""));
ok(badLeg.length === 0, "every patrol route is traversable" + (badLeg.length ? " — " + badLeg.join("; ") : ""));

// Nothing may patrol within sight of where the team comes ashore: being seen
// before the player can take their hands off the controls is not difficulty.
let tooNear = [];
M.ROUTES.forEach((r, i) => r.forEach(([x, z], j) => {
  const d = M.dist(x, z, M.START.x, M.START.y);
  if (d < 28) tooNear.push(`route ${i} wp ${j} is ${d.toFixed(0)} m from the insertion point`);
}));
ok(tooNear.length === 0, "no patrol waypoint sits within sight of the insertion point"
  + (tooNear.length ? " — " + tooNear.join("; ") : ""));

// the road must actually be a usable route up the mountain
let roadBad = [];
for (let i = 0; i < M.ROAD.length; i++) {
  const [x, z] = M.ROAD[i];
  if (M.solid(x, z)) roadBad.push(`road node ${i} (${x},${z})`);
}
ok(roadBad.length === 0, "every road node is standable" + (roadBad.length ? " — " + roadBad.join("; ") : ""));

// the full mission, leg by leg
const byId = Object.fromEntries(M.OBJECTS.map(o => [o.id, [o.x, o.z]]));
const legs = [[M.START.x, M.START.y], byId.nb, byId.s1, byId.s2, byId.s3, exc];
let legFail = [];
for (let i = 0; i < legs.length - 1; i++)
  if (!M.findPath(legs[i][0], legs[i][1], legs[i+1][0], legs[i+1][1])) legFail.push(`leg ${i}`);
ok(legFail.length === 0, "the whole mission route is walkable end to end" + (legFail.length ? " — " + legFail.join("; ") : ""));

// the mountain must still feel like a mountain: plenty of it unclimbable
let free = 0, total = 0, steep = 0;
for (let z = 0; z < M.MAP.h; z++) for (let x = 0; x < M.MAP.w; x++) {
  total++;
  if (M.walkable(x, z)) free++;
  if (M.slopeAt(x + 0.5, z + 0.5) > M.MAXSLOPE) steep++;
}
const pct = free / total * 100;
ok(pct > 35 && pct < 80, `${pct.toFixed(1)}% of the map is walkable — a valley, not a plain`);
ok(steep / total > 0.12, `${(steep/total*100).toFixed(1)}% is too steep to climb, so the routes matter`);

// snow should be above the treeline, not below it
ok(M.TREELINE > M.SNOWLINE - 12 && M.TREELINE <= M.SNOWLINE + 8, "treeline and snow line are consistent");

console.log(`\n${pass} passed, ${fail} failed.`);
process.exit(fail ? 1 : 0);
