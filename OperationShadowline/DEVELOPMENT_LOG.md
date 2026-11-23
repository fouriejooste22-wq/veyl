# Development log — Operation Shadowline

Append-only. Newest entry at the top. Every entry records what was done, what was verified, and what was **not**.

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
