# Development log — Operation Shadowline

Append-only. Newest entry at the top. Every entry records what was done, what was verified, and what was **not**.

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
