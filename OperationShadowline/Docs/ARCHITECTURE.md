# Architecture

## Where files belong

```
veyl/                                   repository root (also holds an unrelated web project)
└── OperationShadowline/                ← the Unreal project
    ├── OperationShadowline.uproject    module + plugin manifest, engine association 5.5
    ├── DEVELOPMENT_LOG.md              append-only record of every stage
    ├── UNFINISHED_FEATURES.md          the honest list of what does not exist yet
    ├── Config/
    │   ├── DefaultEngine.ini           Lumen, Nanite, VSM, volumetrics, RHI/SM6, physics
    │   ├── DefaultGame.ini             project identity, packaging, asset manager
    │   ├── DefaultInput.ini            makes Enhanced Input the default input path
    │   └── DefaultScalability.ini      the Low/Medium/High/Ultra ladder
    ├── Source/
    │   ├── OperationShadowline.Target.cs        game target
    │   ├── OperationShadowlineEditor.Target.cs  editor target
    │   └── OperationShadowline/
    │       ├── OperationShadowline.Build.cs     module dependencies
    │       ├── OperationShadowline.h/.cpp       module impl + LogShadowline category
    │       ├── Core/        types, game mode, player controller
    │       ├── Player/      character, movement component, input config
    │       ├── World/       region definitions (data-driven region identity)
    │       ├── Environment/ sky, clock, weather
    │       ├── Settings/    graphics/display settings
    │       └── UI/          (empty — stage 2)
    ├── Content/                        all .uasset/.umap content (Git LFS)
    │   └── Shadowline/                 everything this project authors lives under here
    ├── Docs/
    │   ├── FEASIBILITY_REPORT.md        what the hardware can and cannot do
    │   ├── SETUP.md                     build steps + milestone 1 instructions
    │   ├── ARCHITECTURE.md              this file
    │   ├── WORLD_AURELIA.md             island, regions, campaign, squad
    │   └── ASSET_PIPELINE.md            licensing policy, naming, asset register
    └── Tools/
        └── check_sources.py             offline source sanity checker
```

**Rule:** nothing this project authors goes directly in `Content/`. Everything goes under `Content/Shadowline/`, so that imported sample content and project content never mix.

---

## Design decisions, and why

### C++ for systems, Blueprints for assembly

Every class here is `Blueprintable` or exposes its tuning as `EditDefaultsOnly` properties. The intent is that **no gameplay tuning value requires a recompile**: speeds, camera distances, traversal heights, fog densities and weather tables are all editor-facing. C++ owns behaviour; Blueprints and data assets own numbers and asset references.

### Region identity is data, not code

`UShadowlineRegionDefinition` holds everything that makes one region of Aurelia look and sound different from the others. `AShadowlineWorldAtmosphere` reads whichever definition is active and blends toward it.

This is the alternative to five separate lighting rigs, and it is why the five-region requirement is tractable: adding a region is authoring one data asset, not building a new sky.

### One actor owns the sky

`AShadowlineWorldAtmosphere` holds the SkyAtmosphere, sun, moon, sky light, volumetric clouds and height fog as **components of a single actor**. The clock, the weather state machine and the material-parameter publishing all live with them.

The reason: a day/night cycle plus dynamic weather plus five regional palettes is three systems writing to the same handful of components. Splitting them across actors and subsystems means three places can disagree about what time it is. Here there is exactly one `DayAlpha`, computed once per tick, and everything reads it.

Materials receive weather through `MPC_Weather`, a material parameter collection — so landscape, foliage and props react to wetness, snow, dust and wind without any per-material C++ coupling.

### Stance rides the engine's crouch path

`UShadowlineCharacterMovement` implements standing/crouched/prone by calling `SetCrouchedHalfHeight` and using the engine's existing crouch machinery at three different heights, rather than adding a custom movement mode.

Capsule resizing, floor adjustment, perch detection and network correction all keep working for free. A custom `MOVE_Custom` prone mode would mean reimplementing all of it.

### Speed comes from stance × gait, not from `MaxWalkSpeed`

`GetMaxSpeed()` is overridden to read a 3×3 table. A stealth game needs a quiet walk in every stance, which a single `MaxWalkSpeed` value cannot express. Slow-walk deliberately overrides sprint when both are held — the quiet option is never lost to a fumbled keypress.

### Traversal is a query, then an action

`ProbeTraversal()` is `const` and side-effect-free: four traces that find an obstacle face, its top, its depth and a clear landing spot. It is called once per tick and cached, so the UI prompt and the jump handler read the same answer.

`TryTraversal()` consumes that result. With a montage assigned it plays root motion warped onto the real geometry; without one it launches the character over the obstacle. **The launch is placeholder behaviour and is listed as such** — it exists so level geometry is testable before animation work starts, not as a shipping feature.

### The settings menu owns input sensitivity, not C++

`Input_Look` passes raw axis values straight through. Sensitivity and inversion belong to Enhanced Input modifiers on `IA_Look`, where the settings menu can change them at runtime without touching gameplay code.

---

## Not yet built

Stages 2 and 4–10 are unimplemented. The current source covers: the module, shared types, the movement/stance system, the player character and camera rig, traversal probing, the region data schema, the sky/clock/weather actor, graphics settings, and a thin game mode and player controller.

`UNFINISHED_FEATURES.md` is the authoritative list of everything else.
