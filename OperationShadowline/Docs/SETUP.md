# Setup — from a fresh clone to the first playable valley

Everything in this document runs **on your PC**, not in the Claude session. Work through it in order. If a step fails, paste the exact error back into the session and I will fix it.

---

## Part 1 — First build (30–90 minutes, mostly waiting)

### 1.1 Prerequisites

Install in this order (details and versions in `FEASIBILITY_REPORT.md` §4):

1. Visual Studio 2022 with the **Game development with C++** workload
2. Epic Games Launcher → Unreal Engine **5.5**
3. Git and **Git LFS**

### 1.2 Clone and initialise LFS

```bat
git clone https://github.com/fouriejooste22-wq/veyl.git
cd veyl
git checkout claude/operation-shadowline-dev-owvo5a
git lfs install
git lfs pull
cd OperationShadowline
```

The Unreal project is the `OperationShadowline/` subfolder. The repository root also holds an unrelated `index.html` web project, which this work does not touch.

### 1.3 Generate Visual Studio project files

Right-click `OperationShadowline.uproject` → **Generate Visual Studio project files**.

If that entry is missing from the context menu, run the engine's registration tool once:

```bat
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealVersionSelector.exe" /fileassociations
```

### 1.4 Build

Open `OperationShadowline.sln`. Set:

- Solution Configuration: **Development Editor**
- Solution Platform: **Win64**

Build the `OperationShadowline` project (**Ctrl+Shift+B**).

> ⚠️ **Expect compile errors on this first build.** The C++ in this repository was written against the UE 5.5 API but has never been compiled — there is no engine in the environment where it was authored. This is stated plainly in the feasibility report and tracked in `DEVELOPMENT_LOG.md`.
>
> Paste the **first 10 errors** from the Output window into the session. Do not try to fix them yourself unless you want to; diagnosing them is the fastest-moving task I have.
>
> The files most likely to need a correction are `ShadowlineWorldAtmosphere.cpp` (the sky-component setter names move between engine versions) and `ShadowlineGameUserSettings.cpp` (`Scalability::FQualityLevels` members).

### 1.5 Run the automation tests — do this before anything else

The campaign logic has automation tests, and they need **no GPU and no content**. They are the fastest way to find out whether the C++ actually works.

In the editor: **Window → Test Automation → Shadowline.Campaign → Start Tests**.

Or headless, which is quicker:

```bat
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
  "%CD%\OperationShadowline.uproject" ^
  -ExecCmds="Automation RunTests Shadowline.Campaign; Quit" ^
  -unattended -nopause -nullrhi -testexit="Automation Test Queue Empty"
```

Six test groups, roughly fifty assertions: province gating, mission gating, survey maths, the confrontation ladder, detection and difficulty, and robustness against bad data. **Paste the result back either way** — passing tells us the campaign rules are sound on real hardware, and failing tells us exactly which rule is wrong.

### 1.6 Launch

Once it builds, launch the editor from Visual Studio (**F5**) or by double-clicking the `.uproject`. The project has **no content yet** — no maps, no meshes, no input assets. The next part creates them.

---

## Part 2 — Content the C++ expects

The C++ classes are deliberately inert until these assets exist and are assigned. Create them in this order.

### 2.1 Folder structure

In the Content Browser, create:

```
Content/Shadowline/
├── Maps/
├── Input/
│   └── Actions/
├── World/
│   └── Regions/
├── Characters/
│   └── Player/
├── Materials/
│   └── Collections/
├── Audio/
├── Effects/
└── UI/
```

### 2.2 Material parameter collection

`Content/Shadowline/Materials/Collections/MPC_Weather`
→ **Right-click → Materials → Material Parameter Collection**

Add these **scalar** parameters, spelled exactly like this — `ShadowlineWorldAtmosphere.cpp` looks them up by name:

| Name | Default |
|---|---|
| `CloudCoverage` | 0.25 |
| `RainIntensity` | 0.0 |
| `Wetness` | 0.0 |
| `SnowCoverage` | 0.0 |
| `DustIntensity` | 0.0 |
| `WindSpeed` | 1.0 |
| `NightAlpha` | 0.0 |

Any material that should react to weather reads from this collection. A wet surface, for example: lerp base roughness toward ~0.1 and darken base colour slightly by `Wetness`.

### 2.3 Enhanced Input assets

Create one **Input Action** per row in `Content/Shadowline/Input/Actions/`:

| Asset | Value Type | Suggested binding |
|---|---|---|
| `IA_Move` | Axis2D | WASD (2D Vector modifiers), left stick |
| `IA_Look` | Axis2D | Mouse XY, right stick |
| `IA_Jump` | Digital (bool) | Space, gamepad A |
| `IA_Sprint` | Digital (bool) | Left Shift, L3 |
| `IA_SlowWalk` | Digital (bool) | Left Alt, L1 |
| `IA_Crouch` | Digital (bool) | C, B |
| `IA_Prone` | Digital (bool) | Z, hold B |
| `IA_SwapShoulder` | Digital (bool) | Middle mouse, R3 |
| `IA_Focus` | Digital (bool) | Right mouse, L2 |
| `IA_CameraDistance` | Axis1D (float) | Mouse wheel |

Then create `Content/Shadowline/Input/IMC_Shadowline_Default` (**Input Mapping Context**) and add every action with its keys.

On `IA_Move`, add modifiers so the axes land correctly:
- W → `Swizzle Input Axis Values` (YXZ)
- S → `Swizzle Input Axis Values` (YXZ) + `Negate`
- A → `Negate`
- D → no modifier

On `IA_Look`, add `Negate` on the Y axis if you want non-inverted mouse pitch. Put mouse sensitivity here as a `Scalar` modifier — the settings menu changes it at this layer, which is why there is no sensitivity value in C++.

Finally create `Content/Shadowline/Input/DA_ShadowlineInput`:
**Right-click → Miscellaneous → Data Asset → `ShadowlineInputConfig`**, and assign every field.

### 2.4 Player character Blueprint

`Content/Shadowline/Characters/Player/BP_ShadowlineCharacter`
→ **Blueprint Class → All Classes → `ShadowlineCharacter`**

Set on the Blueprint:
- **Mesh** → a skeletal mesh (see §2.7 for a free, licence-clean option)
- **Anim Class** → your animation Blueprint
- **InputConfig** → `DA_ShadowlineInput`
- Mesh transform: rotate Z by **-90°**, translate Z by **-90°** (the usual correction so a UE-skeleton mesh's feet sit on the capsule base)

Leave `VaultMontage` and `MantleMontage` empty for now. The character falls back to an unanimated launch over obstacles, which is intentional placeholder behaviour and is listed in `UNFINISHED_FEATURES.md`.

### 2.5 Region definition

`Content/Shadowline/World/Regions/DA_Region_KoldaRange`
→ **Data Asset → `ShadowlineRegionDefinition`**

For the milestone-1 valley, the northern mountains region (see `WORLD_AURELIA.md` for the intended look of all five):

| Field | Value |
|---|---|
| Region | `Kolda Range` |
| Display Name | Kolda Range |
| Noon Sun Intensity | 68000 |
| Clear Fog Density | 0.012 |
| Heavy Fog Density | 0.28 |
| Fog Inscattering Colour | (0.42, 0.55, 0.70) |
| Clear Cloud Coverage | 0.35 |
| Atmosphere Tint | (0.16, 0.31, 0.60) |
| Mean Weather Duration | 40 |
| Base Wind Speed | 2.0 |
| Weather Table | Clear 3, Overcast 3, Fog 2, Snow 2, Storm 1 |

### 2.6 The map

`Content/Shadowline/Maps/L_Aurelia_Sandbox`
→ **File → New Level → Open World** (this gives you World Partition and a Landscape already set up)

Then:

1. **Delete** the template's `DirectionalLight`, `SkyLight`, `SkyAtmosphere`, `VolumetricCloud` and `ExponentialHeightFog` actors.
2. Drag in a **`ShadowlineWorldAtmosphere`** actor. It owns all of those as components and will fight any duplicates left in the level.
3. On it, set:
   - `RegionDefinitions` → add key `Kolda Range` → `DA_Region_KoldaRange`
   - `WeatherParameters` → `MPC_Weather`
   - `TimeOfDay` → `16.5` (low afternoon sun — the most flattering light for a first screenshot)
   - `MinutesPerSecond` → `0.5`
4. Place a **PlayerStart**.
5. **World Settings** → GameMode Override → `ShadowlineGameMode` (or a `BP_ShadowlineGameMode` whose Default Pawn is `BP_ShadowlineCharacter`).
6. Drag `ShadowlineWorldAtmosphere`'s `TimeOfDay` in the Details panel while the viewport is live — the sun should move. **That is your first real test that the C++ works.**

---

## Part 3 — Milestone 1: making the valley beautiful

This is art direction, and it is the part only you can do. The order matters more than the tools.

### 3.1 Terrain (do this first, and badly, then refine)

**Landscape mode** → sculpt a valley roughly 2 × 2 km:
- Landscape resolution **2017×2017**, scale **100** → 2.0 km per side
- Two ridgelines running roughly north–south, a floor that falls gently to the south
- **Critical for believability:** erosion. Sculpt → **Erosion** and **Hydro Erosion** brushes, several light passes. Hand-sculpted hills read as fake precisely because they lack erosion channels. This single step does more for realism than any material.
- Keep slopes under ~50° where the player should walk — that is the `SetWalkableFloorAngle(50.f)` set in `ShadowlineCharacterMovement`.

### 3.2 Assets — free and licence-clean

Do not use placeholder cubes, and do not grab assets of uncertain provenance. Two sources cover almost everything, legitimately:

| Source | Licence | What to take |
|---|---|---|
| **Fab / Quixel Megascans** | Free for UE users; usable in commercial UE projects | Rock formations, cliffs, boulders, ground surfaces, debris, tree bark/atlases |
| **Epic's own sample projects** (City Sample, Electric Dreams, Valley of the Ancient) | Free, redistributable within UE projects | Vegetation, master materials, decals |

See `ASSET_PIPELINE.md` for the licensing rules this project holds itself to, and for the record you should keep as assets come in.

For a character mesh with a clean licence: Epic's **MetaHuman** (free, licensed for UE projects) or the **Quinn/Manny** meshes shipped in the Third Person template. Both solve §2.4 immediately. MetaHuman is also the sane answer to the brief's facial-customisation requirement — do not model a face system from scratch.

### 3.3 Landscape material

Build an auto-material with **4–5 layers** blended by slope and altitude:

1. **Rock** — on slopes steeper than ~35° (`LandscapeLayerBlend` driven by world normal Z)
2. **Scree / gravel** — the transition band below rock faces
3. **Grass / alpine meadow** — the valley floor
4. **Snow** — above an altitude threshold, **multiplied by `SnowCoverage` from `MPC_Weather`** so snowfall actually accumulates
5. **Mud / wet soil** — near water, modulated by `Wetness`

Use **Virtual Heightfield Mesh** or Nanite-enabled landscape for the far distance. Add `Runtime Virtual Textures` for the colour and normal so foliage and rocks can blend into the ground — the thing that most clearly separates a professional-looking landscape from a hobby one.

### 3.4 Foliage (the single biggest visual win)

**Foliage mode**, painted in this order:

1. **Trees** — pines for this region. 2–3 species, ~3 scale variants each. Density low enough to see through; a wall of trees reads as a wall.
2. **Large shrubs and ferns** — breaks up tree trunk bases
3. **Grass** — via the landscape material's **Grass Output** node, not hand-painted. This is what keeps the density sane.
4. **Small rocks and detritus** — scattered at the base of cliffs and along erosion channels

Settings that matter:
- **Cull distances**, every type. Grass ~3000, shrubs ~8000, trees ~25000.
- **Wind**: foliage materials need `SimpleGrassWind` or a WPO node driven by `WindSpeed` from `MPC_Weather`. Static vegetation kills realism instantly.
- Enable **Nanite** on tree trunks and rocks. Leave leaf cards as traditional LODs with masked materials — Nanite and heavy masked foliage interact badly.

### 3.5 Composition — treat it as a photograph

The thing that will make this look premium is not any single setting:

- **One clear focal point.** A distinctive peak, a ruined tower, a bridge. The player's eye needs somewhere to go.
- **Layered depth.** Foreground rocks, midground trees, background mountains, atmospheric haze between each. `ShadowlineWorldAtmosphere`'s height fog gives you the aerial perspective for free — use it.
- **Low sun.** 16:30 or 07:30, never noon. Long shadows and raking light across terrain do most of the work.
- **A `PostProcessVolume`** (Unbound) with a restrained film grade: slight S-curve contrast, a touch of desaturation in the shadows, **no** heavy LUT.

### 3.6 Verify, honestly

In the editor:

- `Stat FPS`, `Stat Unit` — know your frame cost before adding more
- **View Mode → Lit → Nanite Visualization → Overview** — confirm Nanite is actually on where you think it is
- **View Mode → Lumen → Lumen Scene** — confirm GI is doing something
- `r.Shadow.Virtual.Visualize 1` — confirm VSM is active

If any of the above shows a feature is off, your GPU or settings do not support it. Report what you see; do not assume it is working because the config file asks for it.

---

## Part 4 — Reporting back

After each attempt, tell the session:

1. **Compile errors** — the first 10, verbatim
2. **What you see** — a screenshot is worth more than a description
3. **Frame time** — from `Stat Unit`: Frame / Game / Draw / GPU
4. **What felt wrong** — movement too floaty, camera too close, light too flat

I will update `DEVELOPMENT_LOG.md` with results and `UNFINISHED_FEATURES.md` as items close.
