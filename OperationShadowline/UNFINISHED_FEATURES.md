# Unfinished features

Authoritative register of what does **not** exist yet. Nothing is marked done until it has been built and run.

Status key: ⛔ not started · 🔨 partial · ⚠️ written but never compiled · ✅ built and tested

---

## Blocking everything

| Item | Status | Note |
|---|---|---|
| Compile the project once | ⛔ | No engine in the authoring environment. **Every ⚠️ below becomes untrustworthy until this happens.** See `Docs/SETUP.md` §1.4 |
| Confirm GPU supports SM6 | ⛔ | Needed for Nanite, Lumen, VSM. Unknown — target PC not yet inspected |

## Stage 1 — project and environment

| Item | Status |
|---|---|
| Project structure, `.uproject`, build files | ⚠️ |
| Engine config (Lumen, Nanite, VSM, volumetrics) | ⚠️ never loaded by an engine |
| Scalability ladder Low→Ultra | ⚠️ values are informed estimates, **not measured** |
| Content folders, input assets, region asset, map | ⛔ must be authored in the editor (`SETUP.md` §2) |
| Milestone 1: visually impressive valley | ⛔ needs a GPU |

## Stage 2 — UI

**Design specification complete** (`Docs/ui-design-reference.html`, published for review). **No widget built.** `Source/.../UI/` is empty.

| Item | Design | Implementation |
|---|---|---|
| Main menu | ✅ specified | ⛔ |
| Interactive world map (pan, zoom, markers, exploration %) | ✅ specified, interaction prototyped | ⛔ |
| Operations / mission menu with optional objectives | ✅ specified | ⛔ |
| Squad management and standing orders | ✅ specified | ⛔ |
| Loadout (appearance + equipment, weight constraint) | ✅ specified | ⛔ |
| Graphics settings with preset/Custom behaviour | ✅ specified, behaviour prototyped | ⛔ |
| In-game HUD and objective tracker | ✅ specified | ⛔ |
| Pause menu | ⛔ | ⛔ |
| Progression screen | ⛔ | ⛔ |
| Records / collected documents screen | ⛔ | ⛔ |
| CommonUI widget classes, input routing, gamepad focus | ⛔ | ⛔ |
| Settings screen bound to `UShadowlineGameUserSettings` | ✅ specified | ⛔ (backing class is ⚠️) |

### Open design questions

| Question | Note |
|---|---|
| **What happens to a resolved overseer** | They should persist in the world — reassigned, recalled, or still there and now aware of you. No system exists. |
| **Director mission design** | The four regional director missions are named and sited but have no objective structure. |
| **Mission failure state** | Threshold has no fail state that works as a death screen. Detection should escalate, not end the mission. Unresolved — not guessed at. |
| Map render source | Specified as a `SceneCapture2D` orthographic bake per region. Untested; may not hold up at 70 km. |
| Save-header format | The menu's status panel needs act/region/clock/weather/survey% without streaming a level. Schema not designed. |

## Stage 3 — player character

| Item | Status | Note |
|---|---|---|
| Third-person camera rig, shoulder swap, focus mode | ⚠️ | |
| Walk / jog / sprint / slow walk | ⚠️ | |
| Crouch and prone stances | ⚠️ | Prone has **no animation**; capsule resizes but the mesh will look wrong |
| Vault / mantle detection | ⚠️ | Probe logic written |
| Vault / mantle execution | 🔨⚠️ | **Falls back to an unanimated launch.** Needs `AM_Vault_Low`, `AM_Mantle_High` + Motion Warping notifies |
| Swimming | 🔨 | Movement component configured; needs water volumes and swim animations. Untested |
| Climbing (free climb on surfaces) | ⛔ | Only mantling is designed. Full climbing is a separate system |
| Animation Blueprint, locomotion state machine | ⛔ |
| Footstep audio by surface | ⛔ | `GetMovementNoiseLevel()` exists with no consumer |
| Character customisation (face, clothing, equipment, backpack) | ⛔ | Recommend MetaHuman + modular mesh slots |

## Stage 4 — the island

| Item | Status |
|---|---|
| Region data schema | ⚠️ |
| Kolda Range (north) playable section | ⛔ |
| Vaelmire Basin (south) | ⛔ |
| Ashen Flats (west) | ⛔ |
| Siren Coast (east) | ⛔ |
| Meridian Works (centre) | ⛔ |
| World Partition setup, streaming grid tuning | ⛔ per-level editor work |
| Landscape auto-materials, RVT blending | ⛔ |
| Water bodies, rivers, waterfalls | ⛔ |
| Settlements, abandoned buildings, landmarks | ⛔ |

## Stage 5 — lighting, weather, sound

| Item | Status | Note |
|---|---|---|
| Day/night cycle | ⚠️ | Sun/moon/sky-light driven from one clock |
| Weather state machine + blending | ⚠️ | Blends severity; drives fog, sun dimming, MPC params |
| Weather → material parameters (wetness, snow, dust, wind) | ⚠️ | Publishes to `MPC_Weather`; **the materials that read it do not exist** |
| `MPC_Weather` asset | ⛔ `SETUP.md` §2.2 |
| Volumetric cloud coverage reacting to weather | 🔨 | Value is published; cloud material must consume it |
| Precipitation Niagara systems (rain, snow, dust) | ⛔ | Actor activates one if assigned; none authored |
| Wet-surface materials | ⛔ |
| Wind affecting foliage | ⛔ | `WindSpeed` published; foliage WPO not authored |
| Regional ambience beds (MetaSound) | ⛔ | Crossfade logic ⚠️, sounds ⛔ |
| Wind / birds / rain / thunder / water / machinery layers | ⛔ |
| Cinematic colour grading | ⛔ |

## Stage 6 — exploration and missions

**Data schemas written; no mission implemented.**

| Item | Status |
|---|---|
| Campaign designed: 15 provinces, 16 named antagonists, 55 operations | ✅ design only (`Docs/CAMPAIGN.md`) |
| `UShadowlineProvinceDefinition` / `UShadowlineMissionDefinition` | ⚠️ written, never compiled |
| Province and mission assets authored in editor | ⛔ |
| `UShadowlineCampaignSubsystem` (province state, mission state, survey %) | ⛔ — every UI screen needs it |
| Mission / objective runtime framework | ⛔ |
| Reconnaissance, rescue, investigation, supply, search missions | ⛔ |
| Optional objectives | ⛔ |
| Environmental puzzles | ⛔ |
| Non-lethal gadgets (scope, drone, signal sniffer, emitter, door shim) | ⛔ |
| Fictional security systems | ⛔ |
| Collectible documents, hidden locations, landmark discovery | ⛔ |
| Save / load | ⛔ |

## Stage 7 — AI

Nothing exists.

| Item | Status |
|---|---|
| Enemy NPC base, patrol behaviour | ⛔ |
| Perception, suspicion levels, search behaviour | ⛔ |
| NPC-to-NPC communication | ⛔ |
| Day/night behaviour variation | ⛔ |
| Difficulty levels | 🔨⚠️ `FShadowlineDifficultyKnobs` + 4 presets written with real values; **nothing reads them** — the systems they tune do not exist |
| **Player visibility/concealment model** | ⛔ the heart of the stealth loop; nothing yet |
| `StealthVision` trace channel | ⛔ |
| Companion AI (follow, regroup, formation, revive, dialogue) | ⛔ |
| Companion progression and customisation | ⛔ |
| Civilians, wildlife, traffic | ⛔ |

## Stage 8 — vehicles

Nothing exists. `ChaosVehicles` is enabled in the `.uproject` and nothing uses it.

| Item | Status |
|---|---|
| Off-road vehicle, SUV, motorcycle | ⛔ |
| Boats | ⛔ |
| Helicopters | ⛔ explicitly deferred in the brief |
| Suspension, damage visuals, engine audio | ⛔ |

## Stage 9 — campaign

| Item | Status |
|---|---|
| Story and world design | ✅ written (`Docs/WORLD_AURELIA.md`) — design only, nothing implemented |
| Act I–V missions | ⛔ |
| Dialogue system, companion barks | ⛔ |
| Meaningful choices and consequence tracking | ⛔ |
| Cinematics, ending | ⛔ |

## Stage 10 — optimisation and polish

| Item | Status |
|---|---|
| Measure anything, ever | ⛔ |
| Validate the scalability ladder on real hardware | ⛔ |
| Streaming / shader-compilation / draw-call budgets | ⛔ |
| Automated tests | ⛔ |

---

## Known defects in written code

| Where | Issue |
|---|---|
| `ShadowlineCharacter.cpp` — `TryTraversal` | Launch fallback is unanimated placeholder motion |
| `ShadowlineCharacterMovement.cpp` | Prone is a short capsule with no animation support |
| `ShadowlineCharacterMovement.cpp` | Stance/gait are not replicated. Single-player only as written |
| `ShadowlineWorldAtmosphere.cpp` | `ApplyRegionAndWeather` + `PublishMaterialParameters` run every tick; should be dirty-flagged once profiled |
| `ShadowlineWorldAtmosphere.cpp` | Region blending is a hard switch, not a blend, despite `SetRegion`'s name |
| `ShadowlineGameUserSettings.cpp` | Does not expose a hardware-ray-tracing toggle yet, only detection |
| All C++ | **Never compiled.** API names may have moved between engine versions |
