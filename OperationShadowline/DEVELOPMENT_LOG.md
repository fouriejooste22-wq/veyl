# Development log — Operation Shadowline

Append-only. Newest entry at the top. Every entry records what was done, what was verified, and what was **not**.

---

## 2026-11-24 (later) — Mountain rebuild: Hale Terraces, Kolda Range

### The ask

"Make it beautiful and cool mountain terrain and stuff", continue unsupervised, and no more of the old build.

### What was chosen

Mountains map straight onto the project's own fiction, so rather than inventing a setting the prototype moved to **Hale Terraces in the Kolda Range** — Act II, "Inventory", against Site Warden Imre Bako, exactly as `Docs/CAMPAIGN.md` describes it. Nine villages emptied in a night, a warden counting what was left behind, and an inventory that is itself the evidence. This also delivers the brief's original milestone 1: mountains, realistic terrain, trees, atmospheric lighting.

The top-down build was deleted. It is in git history if it is ever wanted.

### Created

`Prototype/shadowline-3d.html`, rebuilt from the ground up — 1,959 lines, same artifact link.

| | |
|---|---|
| Terrain | Hash-based value noise, fractal and ridged, baked to a 1 m heightmap and bilinearly sampled. A winding valley between ridges, 116 m of relief inside a 128 × 128 m playfield, climbing to 150 m+ peaks in the surrounding scenery |
| Movement | **Slope-based walkability** — anything steeper than about 49° cannot be climbed, so 56% of the map is impassable and the valley, the road and the terraces are genuinely the ways through |
| Village | Four terraces cut into the east shoulder with ramps between them, retaining walls, nine stone buildings, a chapel with a tower and spire |
| Road | A ten-node switchback from the valley floor to the north pass, carved into the heightfield and laid with a ribbon mesh |
| Snow line | Snow above 30 m (1,920 m on the in-game altimeter), shed from steep ground so crests read as bare rock |
| Treeline | Pines thin out and stop at 34 m, which is what makes a treeline read |
| Lighting | Low golden-hour sun with PCF soft shadows on a player-following frustum, strong sky fill so shadows read blue |
| Weather | Billboarded drifting snow, drifting valley mist planes, far-reaching blue haze |
| Pathfinding | Binary-heap A* with a climb penalty, so patrols prefer the road and the terraces over scrambling up the shoulder |

Stealth additions the mountain made possible: snow above the snow line strips concealment, pines give it back, crusted snow carries noise further, the loose road surface is louder, and climbing is slower than walking.

### Verified

`node Tools/verify_prototype_map.mjs` → **18 of 18**, rewritten for the new terrain. Then five render passes in headless Chromium with software WebGL, **zero runtime errors throughout**.

Between them they caught eight real defects:

| Found | Consequence |
|---|---|
| **A patrol looped two metres from the insertion point** | The team came ashore on top of a guard and was pinned in nine seconds. Found by watching a render end instantly |
| The village inventory was inside the office's own footprint | Objective unreachable — mission uncompletable |
| Two survey stakes sat on terrace retaining steps | Same |
| Two shrines were in the tarn or on an unclimbable slope | Same |
| Seven patrol waypoints were in a house, in the lake, or on a retaining wall | Guards stuck on frame one |
| The ramp to terrace 4 cut through the chapel's north wall | Building on a 5 m step |
| **The altimeter multiplied metres by ten** | Elevation read 54 m when the player was at 5.4 m |
| **The camera boom pulled in against uphill terrain** | On a mountain the ground behind you is almost always uphill, so the camera jammed against the character for the whole climb. Now it rises over the slope and only buildings shorten the boom |

The spawn-adjacent patrol and the camera boom are the two that matter: neither is visible in code review, and both made the game unplayable. There is now a check asserting no patrol waypoint sits within sight of the insertion point.

### Honest about what it is

Real-time 3D with fractal terrain, dynamic shadows, a working snow line and a full stealth simulation. Still low-poly and untextured — no normal maps, no global illumination, no Nanite or Lumen. A browser on a tablet with no art budget cannot be those things, and the photorealistic target is still the UE5 project.

### NOT verified

- **How it runs on real hardware.** Every frame figure came from a software rasteriser
- **How it plays.** Climb pacing, guard spacing on terraces, whether the pass is too far
- Co-op remains written and switched off

---

## 2026-11-24 — 3D rebuild of the prototype

### The criticism was correct

The user's response to the top-down prototype: "looks like something for a 2 yr old, where is the 3d". Fair. The original brief said explicitly *do not build a simple prototype that looks like a basic Unity or browser game*, and a top-down canvas build with circles for people is exactly that. Rebuilt in real 3D.

### Created

`Prototype/shadowline-3d.html` — WebGL third-person stealth, ~2,080 lines. Three.js r137 (UMD), pinned, loaded from jsDelivr.

| | |
|---|---|
| Terrain | 96 × 72 m playfield inside a 288 × 264 m heightfield, carved riverbeds, levelled mill floor and bridge approaches, two-octave vertex colouring |
| Horizon | Ground climbs into forested valley walls outside the playfield, with an instanced far treeline, so the skyline is landscape rather than a cut edge |
| Sky | Vertex-coloured gradient dome, fog matched to the horizon colour |
| Lighting | Low warm directional sun with PCF soft shadow maps (frustum follows the player), hemisphere fill, seven unshadowed lamp point lights with additive ground pools |
| Water | Animated swell injected into a standard material, so it keeps real lighting and a sun specular |
| Vegetation | Instanced trees (trunk + two canopy tiers), instanced reeds with a wind sway in the vertex shader, instanced stones |
| Characters | Articulated figures — hips, torso, chest, head, two arms, two legs, boots, packs, guard caps and vests — with a procedural walk cycle, stance poses and head look |
| Weather | ~900 instanced rain streaks recycled around the camera |
| Controls | Left thumbstick, drag-to-look, pinch zoom, full keyboard and mouse |
| Quality | Low → Ultra, rebuilding shadow resolution, pixel ratio, vegetation density, rain and draw distance |

The simulation underneath is the same verified model: guard perception, suspicion across four states, communication range, concealment, noise, companion AI, nine objectives, and the four difficulty presets with the real values from `FShadowlineDifficultyKnobs::FromPreset`.

### Verified, with a browser

Three.js was pulled from the npm registry to confirm the CDN path and the API surface: `three@0.137.5/build/three.min.js` exists as a UMD build, and every symbol used is present. `CapsuleGeometry` is **not** in r137 — correctly avoided.

The page was then run in headless Chromium with software WebGL (SwiftShader) and screenshotted across five iterations. **Zero runtime errors in every pass.** The frame counter reads 4 FPS there because it is a CPU rasteriser with no GPU; it says nothing about real hardware.

### What the screenshots caught

Looking at the render, rather than trusting the code, found four things no amount of reading would have:

| Problem | Cause | Fix |
|---|---|---|
| Everything washed out pale | **Three r137 has no automatic colour management.** A hex set on a material is treated as already-linear, then `outputEncoding` converts linear → sRGB on output, so every authored colour rendered roughly twice as bright as intended | Convert every material, vertex and light colour from sRGB to linear once, up front |
| Camera spawned inside a tree trunk | Tree scatter excluded the mill yard but not the landing point | Clear a 9.5 m radius around the insertion point |
| Black void above the horizon | Flat background colour, fog colour darker than the sky | Gradient sky dome, fog matched to the horizon |
| Canopies formed a ceiling overhead | Tree canopies were 6–12 m across at that spacing | Halved the scale range, roughly doubled the spacing |

The colour-management one is the significant find: it is invisible in code review and it was the reason three consecutive lighting passes looked wrong. The first two "fixes" were over-corrections chasing a symptom.

### Honest about what it is

It is **real-time 3D with real dynamic shadows, atmospherics and a full stealth simulation**. It is **not** Ghost Recon, and it is not photorealistic: low-poly stylised geometry, untextured materials, no normal maps, no Nanite, no Lumen, no global illumination. WebGL on a tablet with no asset budget cannot be those things.

The photorealistic target still lives in the UE5 project, which is still uncompiled and still waiting on the laptop.

### NOT verified

- **How it runs on a real iPad.** Every frame number here came from a software rasteriser
- **How it plays.** Camera distance, walk speed, guard spacing — all judgement calls needing a person
- Co-op is still written and still switched off, pending the `room` capability

---

## 2026-11-23 (overnight) — Playable field prototype: the stealth loop, on a tablet

### What was asked, and the choices made unsupervised

Asked for a playable mobile/iPad version at top specs, four-player multiplayer, keeping the laptop version. The user went to sleep with "choose for me the best".

| Decision | Choice | Why |
|---|---|---|
| Native iPad build? | **No** | Needs a Mac, Xcode and an Apple Developer account, none of which exist here. And Nanite/Lumen/VSM do not run on iOS, so it is a second art pipeline, not a build setting. Recorded in `Docs/MOBILE_AND_COOP.md`. |
| What to build instead | **A browser game that is genuinely playable on an iPad** | Touch-first, runs in Safari, no install. And it is not throwaway: the stealth loop is the riskiest part of the design, and testing it in a browser is far cheaper than discovering it is wrong in an engine. |
| Genre/view | **Top-down tactical** rather than attempted 3D | Reads perfectly on touch, suits co-op where each player drives one squad member, and achieves real visual quality in canvas instead of a poor 3D imitation. |
| Co-op shape | **Four players, one character each** | Maps exactly onto the fiction: Lead, Ferreiro, Okonkwo, Varga. Unclaimed positions fall to the companion AI, so a game is never short-handed. |
| Laptop project | **Untouched** | The prototype is in its own `Prototype/` folder and shares no code — only data. |

### Created

`Prototype/shadowline-field.html` — ~1,900 lines. Ferrow's Crossing, the Act III mission "Standing Water", playable end to end.

Implements: guard patrols with pauses and idle scanning; vision cones raycast against geometry; suspicion build-up and decay across four guard states; guard-to-guard alerting within communication range; concealment from stance, reeds, light pools and pace; noise propagation modified by water and reeds; three companions with formation-following, vision-cone avoidance and revive; four characters with distinct abilities; an acoustic emitter as a distraction; site alert escalation to forced extraction; nine objectives including three optional; and all four difficulty presets.

**The difficulty values are the ones from `FShadowlineDifficultyKnobs::FromPreset`.** Playing Blackout in the browser tells you how Blackout will feel in the engine.

`Docs/MOBILE_AND_COOP.md` — the honest native-mobile requirements and cost, the co-op architecture, and what was verified.
`Tools/verify_prototype_map.mjs` — map playability verification.

### Verified

`node Tools/verify_prototype_map.mjs` → **23 of 23 pass.** It extracts the map data and the pure geometry and pathfinding code straight out of the HTML and checks connectivity.

**It caught seven mission-breaking bugs on its first run**, two of which made the mission mathematically impossible to complete:

| Bug | Consequence |
|---|---|
| Patrol route 4 ran down the middle of the east river | Guard stuck on frame one |
| Route 0's first waypoint sat inside a crate | Same |
| The west survey marker was inside deep water | **Objective unreachable — mission uncompletable** |
| The exfil zone was in unswimmable water | **Mission unfinishable** |
| The southern reed channel was deep water | South bank cut in two |
| The main river stopped 6 m short of each map edge | **The entire bridge chokepoint could be walked around at the shoreline** |
| A cover crate clipped the corrected route 4 | Guard stuck |

That last river bug is the one worth noting: it would not show up in a short playtest, and it silently destroyed the tactical point of the map's centrepiece.

A performance pass followed, because this has to run on a tablet: wall lookups moved to a baked 0.5 m occupancy grid (vision rays call `wallAt` tens of thousands of times a second), vision cones cached and rebuilt at 8 Hz instead of 60, and off-screen geometry plus distant reed detail culled entirely.

### Blocked, needing the user

**Four-player co-op is written and complete but not switched on.** It needs the `room` capability, which opens a live channel between everyone viewing the page, and auto mode correctly refused to grant that without the owner's say-so.

The code degrades exactly as designed — `claude.use("room")` returns null, the game runs solo with companion AI, no error and no missing features — which is why the prototype is playable right now. Enabling co-op is one line on the publish.

### NOT verified

- **How it feels.** Guard spacing, suspicion rates, companion follow distance, whether the mission is too hard. All judgement calls needing a person to play it.
- Co-op has never run with two live players, since the channel is off.
- Nothing about the UE5 project changed, and it is still uncompiled.

### Next

1. The user plays it and says what is wrong with the stealth loop
2. Their answer on enabling co-op
3. Still waiting on the laptop: compile, and run `Shadowline.Campaign` automation tests

---

## 2026-11-23 (later still) — Campaign subsystem, save schema, companions, and the first real tests

### Context

The user is on an iPad and asked to continue with whatever does not need the laptop. Everything here is pure logic and data, so none of it needed an engine.

### Created

| File | What it does |
|---|---|
| `Core/ShadowlineCampaignSubsystem.*` | The single owner of campaign state, and what every UI screen binds to |
| `Core/ShadowlineSaveGame.*` | Two-file save schema (cheap header + full body) and `UShadowlineSaveLibrary` |
| `Core/ShadowlineTypes.h` (extended) | `EShadowlineMissionState`, `EShadowlineDiscovery`, `FShadowlineMissionProgress`, `FShadowlineProvinceProgress`, `FShadowlineCampaignProgress` |
| `Companions/ShadowlineCompanionDefinition.*` | Companion identity, personality brief, specialisation tiers with accumulating capabilities |
| `Companions/ShadowlineDialogueSet.*` | Barks with region/weather/day-night/act filters, priority selection, random tie-breaking |
| `Tests/ShadowlineCampaignTests.cpp` | **Six automation test groups, ~50 assertions** against the real C++ |
| `Tools/verify_campaign_logic.py` | The same rules transcribed to Python and tested **now** |

### Verified — and this time something substantive was actually checked

`python3 -I Tools/verify_campaign_logic.py` → **17 of 17 checks pass.**

This is the first real verification in the project. The campaign's derived state is pure logic over stored progress plus definition data, so it can be modelled exactly and tested without an engine. Cases covered:

- fresh campaign gating; prerequisites met and unmet
- a single discovery moving a province Open → Active
- clearing requiring the overseer **and** every gating mission, with side operations never gating
- clearing a prerequisite opening its dependent
- an overseer resolved alone reading Active, not Cleared
- a province with progress never being retroactively locked
- mission locks following their province, with work already done outranking the lock
- mission prerequisites gating independently of the province
- required objectives counted, optional ones not, and idempotent on a repeated event
- survey fraction: partial, clamped at 1.0, and a province with nothing to find reading 100%
- island survey **weighted by content**, with an explicit assertion that one small province cannot count as a quarter of the island
- undetected completion read from observed state, not from a caller's argument
- regional director requiring every province in the region cleared
- Meridian never offering a regional director
- the finale requiring all four directors
- unknown ids answering safely rather than throwing
- stale save data for a removed province not inflating the overseer count

`Tools/check_sources.py` → clean across 17 headers, 17 sources, 4 config files.

**What this proves:** the rules are self-consistent and behave correctly at the edges.
**What it does not prove:** that the C++ compiles, or that the Python transcription is faithful to it. `Tests/ShadowlineCampaignTests.cpp` is the real check and runs on the laptop — it needs no GPU and no content, so it is now step 1.5 of `SETUP.md`, before launching the editor.

### Design decisions

- **Nothing derived is ever stored.** `FShadowlineCampaignProgress` holds only facts that cannot be computed; province state, mission state, survey percentages and finale availability are all derived. Storing "province 7 is cleared" next to the facts that make it cleared is how a save file and the game's rules drift apart, and it produces exactly the progression bug that surfaces forty hours in and cannot be reproduced.
- **Progress always outranks a lock.** A province or mission the player has worked in is never retroactively locked, even if its prerequisites are unmet. Players reach places out of order; taking their progress away is worse than letting the sequence break.
- **Side operations never gate clearing.** Only story missions and confrontations do, so nobody is forced to hoover up every cache to move the campaign on.
- **Island survey is weighted by content, not an average of percentages.** A province holding forty discoverables counts for more than one holding four. There is a test asserting this specifically, because the naive implementation is an easy mistake with a dishonest result.
- **`CompleteMission` reads detection from stored state**, not from an argument. No caller can claim a clean run the stealth systems did not observe.
- **Prerequisite checks deliberately skip the recursive branch**, so a cycle in the data cannot hang the game. Shallow chains stay a design rule rather than something enforced in code.
- **Two save files per slot.** The menu reads a small header for five slots without deserialising a campaign. `SaveSlot` writes body then header, and reports failure loudly if the header write fails, because a stale header describing a newer body is worse than a failed save.
- **Saves refuse to load from a newer version** rather than guessing.
- **`UShadowlineDialogueSet` is stateless.** Cooldowns and once-ever tracking belong to the speaking component, because the asset is shared between every instance.
- **Companion tiers accumulate** — tier 3 grants what tiers 1 and 2 granted.

### Tooling improvement

`check_sources.py` now knows that files under `Tests/` have no header of their own, and conversely **fails** a test file that is not guarded by `#if WITH_DEV_AUTOMATION_TESTS` — which would otherwise compile test code into a shipping build. The checker caught its own gap when the test file was added.

### NOT verified

- **Still never compiled.** Eight more files joined the first-build surface
- The automation tests have never run — they are written, not executed
- Nothing calls `RegisterDefinitions` yet; no province or mission asset exists
- `UShadowlineSaveLibrary` has never written a file
- No companion or dialogue asset authored, so `SelectBarkIndex` has never chosen a line

### Next

Still blocked on the laptop for the engine. Available here:

1. `UShadowlineSquadComponent` — follow, regroup, formation, spacing, revive: the stage 7 companion behaviour, whose decision logic is testable the same way
2. The player visibility model — the largest unbuilt system, and pure maths
3. Authoring the fifteen province and fifty-five mission data assets as a commandlet or a Python script the editor can run, rather than fifteen hours of clicking

---

## 2026-11-23 (later still) — Campaign expanded: 15 provinces, confrontation ladder, difficulty system

### What the user asked for

A bigger, more detailed map with more regions; more missions building up to a boss; difficulty levels; more detail generally, including in-game photos in the menu; "Ghost Recon style".

### What was delivered, and the three honest limits stated up front

| Asked | Delivered | Limit |
|---|---|---|
| Bigger, more detailed map, more regions | Island expanded to 85 × 120 km, 5 regions subdivided into **15 provinces**, generated province areas, roads, contours, rivers, dunes, plant grid, compass rose, four toggleable marker layers | Still a web mockup, not an Unreal widget |
| More missions building to a boss | **55 operations**: 15 province confrontations → 4 regional directors → 1 final antagonist | Designed and data-modelled; none implemented |
| Difficulty levels | 4 presets + Custom, **19 knobs**, in `FShadowlineDifficultyKnobs` with real values | Nothing reads the knobs yet — the AI they tune does not exist |
| In-game photos in the menu | Procedural concept vistas per biome (alpine, coast, jungle, desert, industrial) with layered ridgelines, atmospheric haze, sun glow, foreground silhouettes and weather | **Not screenshots.** Nothing has been rendered in an engine, so no capture exists. Every image carries a visible "concept art — not a capture" label |
| Ghost Recon style | Genre structure adopted: provinces, intel gathering, a named target per province, a regional tier above them | **No asset, name, mark, mission, character or interface design copied.** Copying would make the project unusable |
| "Create the game for me" | Not possible | UE5 does not run on iPadOS or in this container. Code, data and design can be built here; engine assembly needs the laptop |

### Created

**C++ data schemas** — the campaign is now modelled in code, not only in prose:

| File | What it carries |
|---|---|
| `Core/ShadowlineTypes.h` (extended) | `EShadowlineMissionType` (9 types), `EShadowlineDifficulty` (5), `EShadowlineProvinceState` (4), `FShadowlineObjective`, `FShadowlineDifficultyKnobs` (19 knobs) |
| `Core/ShadowlineTypes.cpp` (new) | `FShadowlineDifficultyKnobs::FromPreset` — the four presets' real values |
| `World/ShadowlineProvinceDefinition.*` | Province identity, tier, overseer, mission list, discoverable counts, prerequisites, map seed point and tint |
| `Core/ShadowlineMissionDefinition.*` | Mission identity, type, act, briefing, objective array, prerequisites, `bUndetectedForBestOutcome`, map location |
| `Settings/ShadowlineDifficultyProfile.*` | A difficulty as an asset, so designers can author profiles and a mission can override one for a sequence |

**Docs**: `Docs/CAMPAIGN.md` — the full province table, all sixteen named antagonists, operation counts by type, the ending, and the open design questions. `Docs/WORLD_AURELIA.md` extended with the province breakdown.

**Interface reference** rebuilt to nine screens: menu, world map, campaign ladder, operations, difficulty, squad, loadout, graphics, HUD.

### Design decisions

- **Province areas are generated, not drawn.** Each province carries a `MapSeedPoint`; the map assigns every point on the island to its nearest seed. Moving one seed reshapes its neighbours. No province outline is authored anywhere, which means rebalancing the map is editing a number rather than redrawing a shape.
- **A confrontation is never a fight.** Threshold has no weapons and no authority to use force, so each of the sixteen antagonists resolves by getting them on record, lifting a drive, reaching a place first, or putting them on a boat. This keeps the brief's "avoid real-world combat procedures" constraint and makes each one a design problem instead.
- **Difficulty is mostly interface.** Six of the nineteen knobs are assists rather than opposition. The largest felt difference between Standard and Blackout is not that guards see further — it is losing the target marker, the tracker, the concealment readout and the compass, and having to read the world instead. Far cheaper to build than smarter AI, and a bigger difference.
- **Nothing is locked behind a difficulty**, and lowering it carries no penalty. A stealth game that punishes approachability gets abandoned.
- **Consequences, not deaths.** A downed companion self-recovers, needs reviving, or on Blackout is extracted for the rest of the act. Threshold loses people to evacuation.
- **Prerequisites stay shallow.** Province order inside a region is almost free, so the open world does not become a corridor.
- **The ending is still not a boss fight.** The user asked for a boss; the answer is sixteen named antagonists and a final confrontation that resolves as a choice — expose Aurelia or contain it — rather than combat. This keeps the brief's non-lethal mandate intact while giving the campaign the build-up they wanted.

### Verified

- ✅ `python3 -I Tools/check_sources.py` → exit 0 across 13 headers, 12 sources, 4 config files
- ✅ The interface page's script parses
- ✅ Published; map pan/zoom/province selection, layer toggles, difficulty preset switching and the graphics preset/group interplay all exercised in the published page

### NOT verified

- **Still never compiled.** The new data assets add four more files to the first-build surface
- No mission, province or difficulty asset has been authored in the editor
- **Nothing reads `FShadowlineDifficultyKnobs`.** The values are real and the schema is sound, but the systems they tune — NPC perception, patrol placement, HUD assists — do not exist. This is stage 7
- The 55 operations are titles and objective lists, not implemented missions

### Next

Blocked on the laptop for anything engine-side. Available here:

1. Companion dialogue data schema, which stage 7 needs
2. The save-game header schema the menu's status panel reads
3. `UShadowlineCampaignSubsystem` — province state, mission state, difficulty, survey percentage: the thing every screen binds to

---

## 2026-11-23 (later) — Stage 2 begun: interface design reference

### Context change

The user is on an **iPad 11-inch**, not at their laptop. **Unreal Engine does not run on iPadOS at all** — there is no Unreal Editor for iPad, and Remote Control requires the Claude desktop app on a computer, so this session cannot reach the laptop either. Every engine-dependent task is therefore blocked until the user is at their laptop. Laptop specs remain unknown.

The user deferred the scope and workflow decisions, so these were taken:

| Decision | Choice | Reasoning |
|---|---|---|
| Scope | **One excellent vertical slice** — a ~2 km Kolda Range valley | A stunning valley is achievable in weeks and is the thing that makes the rest fundable or recruitable. 500 km² of empty terrain reads worse than one finished place. |
| Workflow | **Claude writes, user builds** (Path A), with Remote Control offered once they are at the laptop | The only workflow available from an iPad. |
| Immediate work | **Stage 2 UI design**, not more systems code | It is the one stage whose deliverable is reviewable on an iPad, and it is work that must precede any widget being built. |

### Created

`Docs/ui-design-reference.html` — the interface specification for seven screens: main menu, world map, operations, squad, loadout, graphics settings and in-game HUD. Published as a viewable page so it can be reviewed from the iPad.

It is a **design reference, not the game**, and says so prominently on the page. It exists to settle layout, hierarchy and interaction before any UMG widget is built.

Content is drawn from the project's own data, not invented for the mockup: the five regions and their descriptions from `WORLD_AURELIA.md`, the squad and their specialisations, the five acts, and the ten scalability groups with their actual console variables from `DefaultScalability.ini`.

Two parts are functionally interactive, because the feel of them is what needed agreeing:

- **The world map** — pan, scroll-zoom, pinch-zoom, tap-to-select-region, zoom clamped to fit..6×, pan clamped so the island cannot be dragged off-screen, a live scale readout, and markers that reveal labels past a zoom threshold. The island is drawn procedurally (sum-of-sines coastline with three carved bays, regional wedges clipped to the coast, elevation banding north, rivers south, dune hatching west).
- **The settings screen** — preset and per-group interaction mirrors `UShadowlineGameUserSettings::RefreshQualityPreset`: setting a preset writes every group, changing any single group drops the preset to Custom, and the Custom button is inert because Custom is a state rather than an action. Selecting Low reports Lumen as off, which is what `GlobalIlluminationQuality@0` actually does.

Each screen carries build notes recording the implementation intent and, where relevant, the open design question.

### Design decisions

- **Visual direction: survey instrument, not sci-fi HUD.** Desaturated slate, bone text, a single muted amber for active state and a cool cyan for data. Archivo Narrow for headings (institutional and condensed), IBM Plex Sans for body, IBM Plex Mono for every readout — grid references, times, console variables. The deliberate avoidance is the near-black-with-acid-green look that every tactical game UI defaults to.
- **Committed single dark theme.** A game interface is dark in every viewer's theme; the page sets `color-scheme: dark` and defines every colour explicitly rather than offering a light variant that would misrepresent the design.
- **Loadout merges appearance and equipment** onto one screen, because in a stealth game carried volume changes silhouette, and silhouette is a stealth variable. No grid inventory: weight against capacity is the only constraint.
- **HUD is deliberately sparse** — no minimap, no health bar. The only continuous readout is concealment state.
- **Settings are honest about hardware.** The bar reports the detected RHI and whether hardware ray tracing is available; options the GPU cannot do are disabled with the reason shown rather than silently ignored.

### Verified

- ✅ The page's script parses (`new Function` over the extracted script body)
- ✅ Published and renders; map pan/zoom/selection and the preset/group interplay were exercised in the published page

### NOT verified

- The design has had no user review yet — that is what it is for
- **No UMG widget has been built.** This is a specification; `Source/.../UI/` is still empty
- Nothing about the Unreal project has changed, and it is still uncompiled

### Open design question recorded

**Mission failure.** Threshold has no fail state that works as a death screen — detection should escalate, not end the mission. Logged in `UNFINISHED_FEATURES.md` rather than guessed at.

### Next

Still blocked on the laptop for anything engine-side. Available without it:

1. User reviews the interface reference and says what is wrong with it
2. Stage 2 continues: `UShadowlineUISubsystem`, the settings-screen data binding, and the CommonUI widget class skeletons — all authorable here
3. Companion data asset schema (dialogue, specialisation tiers), which Stage 7 needs and which is pure data design

---

## 2026-11-23 — Stage 1: environment inspection and project establishment

### Environment inspected

Measured, not assumed (`lscpu`, `free`, `df`, filesystem probes, version checks):

- Ubuntu 24.04.5 LTS, kernel 6.18.44, KVM guest
- Intel Xeon @ 2.10 GHz, 4 cores / 4 threads
- 15 GiB RAM, no swap
- 61 GB disk available
- **No GPU** — no `/dev/dri`, no `nvidia-smi`, no PCI display device; only `lavapipe` software Vulkan
- **No display server** — `DISPLAY` and `WAYLAND_DISPLAY` unset
- clang 18.1.3, gcc 13.3.0, cmake 3.28.3, ninja 1.11.1, Python 3.13.18 — all present
- **.NET SDK: absent** (UnrealBuildTool cannot run)
- **Unreal Engine: absent.** Searched for `UnrealEditor*`, `UE_5*`, `GenerateProjectFiles.sh`, `UnrealBuildTool*` — no matches

**Conclusion: UE5 cannot be built or run in this environment.** Four independent blockers (no GPU, no display, insufficient disk, no .NET + infeasible build budget). Full reasoning in `Docs/FEASIBILITY_REPORT.md`.

Checked for an attached machine via Remote Control — none available. All engine work is currently the user's to execute on their own PC.

### Created

| What | Where |
|---|---|
| Project manifest, 14 plugins | `OperationShadowline.uproject` |
| Game + editor targets | `Source/*.Target.cs` |
| Module rules (minimal dependency set) | `Source/OperationShadowline/OperationShadowline.Build.cs` |
| Module impl + `LogShadowline` category | `Source/OperationShadowline/OperationShadowline.{h,cpp}` |
| Shared enums + traversal probe struct | `Core/ShadowlineTypes.h` |
| Game mode, player controller | `Core/ShadowlineGameMode.*`, `Core/ShadowlinePlayerController.*` |
| Stance/gait movement component | `Player/ShadowlineCharacterMovement.*` |
| Player character + camera rig + traversal | `Player/ShadowlineCharacter.*` |
| Enhanced Input config data asset | `Player/ShadowlineInputConfig.h` |
| Region definition data asset | `World/ShadowlineRegionDefinition.*` |
| Sky / clock / weather actor | `Environment/ShadowlineWorldAtmosphere.*` |
| Graphics settings (presets, motion blur, benchmark) | `Settings/ShadowlineGameUserSettings.*` |
| Engine config: Lumen, Nanite, VSM, volumetrics, SM6 | `Config/DefaultEngine.ini` |
| Scalability ladder Low→Ultra, Lumen off at Low | `Config/DefaultScalability.ini` |
| Enhanced Input as the default input path | `Config/DefaultInput.ini` |
| Project identity, packaging, asset manager | `Config/DefaultGame.ini` |
| Git LFS routing for all binary assets | `.gitattributes` |
| Offline source checker | `Tools/check_sources.py` |
| Feasibility, setup, architecture, world, asset-pipeline docs | `Docs/` |
| Unfinished-feature register | `UNFINISHED_FEATURES.md` |

~2,600 lines of C++ / C# / INI.

### Verified

Only what is actually verifiable without an engine:

- ✅ `OperationShadowline.uproject` is valid JSON; every declared module has a matching `Source/<Name>/` folder and `Build.cs`
- ✅ All 4 `.ini` files parse: every setting sits inside a section and contains `=`
- ✅ All 10 headers / 8 sources: balanced braces, parens and brackets (comment- and string-aware)
- ✅ Every reflected header includes its `.generated.h`, as the **last** include
- ✅ Every `UCLASS`/`USTRUCT` has `GENERATED_BODY()`
- ✅ Every `UPROPERTY`/`UFUNCTION` is followed by a declaration
- ✅ Every project `#include` resolves to a real file
- ✅ Every `.cpp` includes its own header
- ✅ `python3 -I Tools/check_sources.py` → exit 0

### NOT verified

**The project has never been compiled. No frame has ever been rendered. Nothing here is claimed to work.**

The checker above is a text analyser. It cannot see Unreal's headers, so it cannot validate a single engine API call.

### Engine API calls to verify on first build

These are the calls I am least certain about — setter names on rendering components move between engine versions. If the first build fails, look here first.

| File | Call | Risk |
|---|---|---|
| `ShadowlineWorldAtmosphere.cpp` | `UDirectionalLightComponent::SetAtmosphereSunLight` | Medium — may be a direct `bAtmosphereSunLight` property assignment instead |
| `ShadowlineWorldAtmosphere.cpp` | `SetDynamicShadowDistanceMovableLight`, `SetLightSourceAngle`, `SetDynamicShadowCascades` | Medium |
| `ShadowlineWorldAtmosphere.cpp` | `USkyAtmosphereComponent::SetRayleighScattering` / `SetRayleighScatteringScale` | Medium |
| `ShadowlineWorldAtmosphere.cpp` | `UExponentialHeightFogComponent::SetFogInscatteringColor`, `SetVolumetricFogExtinctionScale` | Medium — UE5 renamed some fog members toward `...Luminance` |
| `ShadowlineWorldAtmosphere.cpp` | `UNiagaraComponent::SetFloatParameter`, `GetAsset` | Low–medium |
| `ShadowlineCharacterMovement.cpp` | `SetCrouchedHalfHeight` | Low — added in 5.3, should be present in 5.5 |
| `ShadowlineGameUserSettings.cpp` | `Scalability::FQualityLevels::GetSingleQualityLevel`, `ResolutionQuality`, `LastCPUBenchmarkResult` | Low–medium — `ScalabilityQuality` and the benchmark fields are protected members |
| `ShadowlineCharacter.cpp` | `UMotionWarpingComponent::AddOrUpdateWarpTargetFromLocationAndRotation` | Low |

If a setter does not exist, the property behind it almost always does — assign it directly and mark the component dirty.

### Decisions and the reasoning

- **Stance rides the engine's crouch path** (`SetCrouchedHalfHeight` at three heights) instead of a custom `MOVE_Custom` mode. Keeps capsule resizing, floor adjustment, perch detection and network correction working for free.
- **Speed is a 3×3 stance × gait table**, not `MaxWalkSpeed`. A stealth game needs a quiet walk in every stance. Slow-walk beats sprint when both are held, so the quiet option is never lost to a fumbled key.
- **Region identity is a data asset**, not code. Adding a region is authoring one `UShadowlineRegionDefinition`, not building a sixth lighting rig. This is what makes the five-region requirement tractable.
- **One actor owns the whole sky.** Clock, weather and five regional palettes are three systems writing to the same components; across separate actors they can disagree about what time it is. There is exactly one `DayAlpha`, computed once per tick.
- **Low quality does not require Lumen.** `GlobalIlluminationQuality@0` falls back to screen-space GI. This is the single decision that makes the project viable on hardware without SM6.
- **Dependencies trimmed to what the code uses.** AIModule, ChaosVehicles, MetasoundEngine, CommonUI and GameplayTags were removed from `Build.cs` until a stage needs them — every unused dependency is build time and another way the first build fails.
- **Traversal is a const query, then an action.** `ProbeTraversal()` is side-effect-free and cached once per tick, so the UI prompt and the jump handler cannot disagree.
- **Project lives in `OperationShadowline/`**, not the repository root. The root already held an unrelated `index.html` web project, which was left untouched.

### Defects found and fixed during this session

| Defect | Fix |
|---|---|
| Traversal montage set `MOVE_Flying` with nothing to restore it — an interrupted montage would leave the character permanently flying | Added `OnTraversalMontageEnded` bound via `Montage_SetEndDelegate`, restoring `MOVE_Falling` on both completion **and** interruption; added a re-entry guard |
| `ShadowlineAtmosphereParams::Wetness` (an `FName`) collided with the `Wetness` float member — member lookup would have won and passed a float where an `FName` was expected | Renamed the constant to `SurfaceWetness`; replaced `using namespace` with a namespace alias so every parameter name is explicitly qualified |
| Sun elevation was computed by two different formulas, and the one in `PublishMaterialParameters` was wrong at night | Cached `DayAlpha` as a member, computed once in `ApplyTimeOfDay`; everything now reads it |
| `ApplyQualityPreset` discarded the user's resolution-scale setting | Preserve and restore `ResolutionQuality` around `SetFromSingleQualityLevel` |
| `SCENE_QUERY_STAT` used in a trace — its defining header has moved between versions | Replaced with a plain `FName` trace tag |
| `GetMesh()->GetAnimInstance()` and `GetLocalPlayer()` used without their headers | Added `Components/SkeletalMeshComponent.h`, `Engine/LocalPlayer.h` |
| `.uproject` had the MetaSound plugin as `MetaSound`; the plugin is `Metasound` | Corrected casing |

### Next

**Blocked on the user.** Stage 1's remaining work needs a real engine:

1. Build in Visual Studio (`Docs/SETUP.md` §1) and report the first compile errors
2. Author the content the C++ expects — `MPC_Weather`, input assets, region asset, map (`SETUP.md` §2)
3. Build the Kolda Range valley for milestone 1 (`SETUP.md` §3)

Open question for the user: **what are the actual specs of the PC this will be built on?** The whole scalability plan, and whether Ultra is even a target, depends on the answer. 32 GB RAM is the practical floor for UE5 open-world editing.

Stage 2 (UI) can proceed here in parallel without an engine, since Slate/UMG widget hierarchies and the settings-menu wiring are authorable as code and Blueprints.
