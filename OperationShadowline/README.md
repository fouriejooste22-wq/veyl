# Operation Shadowline

Open-world third-person tactical stealth on the fictional island of **Aurelia**. Original IP, built in Unreal Engine 5.5.

> **Status: Stage 1 of 10. The project has never been compiled.**
> It was authored in an environment with no Unreal Engine, no GPU and no display server. Expect compile errors on the first build — see `DEVELOPMENT_LOG.md` for the specific API calls to check first. Nothing here is claimed to work.

## Start here

| Document | What it is |
|---|---|
| **[`Docs/FEASIBILITY_REPORT.md`](Docs/FEASIBILITY_REPORT.md)** | What the hardware can and cannot do, what to install, realistic timelines |
| **[`Docs/SETUP.md`](Docs/SETUP.md)** | Build steps, the content the C++ expects, and how to build the first valley |
| **[`Docs/ARCHITECTURE.md`](Docs/ARCHITECTURE.md)** | Where every file belongs and why the systems are shaped this way |
| **[`Docs/WORLD_AURELIA.md`](Docs/WORLD_AURELIA.md)** | The island, its five regions, the squad, the campaign |
| **[`Docs/ASSET_PIPELINE.md`](Docs/ASSET_PIPELINE.md)** | Licensing policy, naming conventions, asset register |
| **[`DEVELOPMENT_LOG.md`](DEVELOPMENT_LOG.md)** | What was done, what was verified, what was not |
| **[`UNFINISHED_FEATURES.md`](UNFINISHED_FEATURES.md)** | The honest list of what does not exist yet |

## What exists

C++ for: the stance/gait movement system (standing, crouched, prone × slow, normal, sprint), the third-person camera rig with shoulder swap and focus mode, traversal probing for vault and mantle, the region data schema, the sky/clock/weather actor, and the graphics settings with a Low→Ultra ladder that does not require Lumen at Low.

Config for: Lumen, Nanite, Virtual Shadow Maps, volumetric fog and clouds, TSR, SM6 targeting, and the full scalability ladder.

No content. No maps, meshes, materials, animations or sounds — those are authored in the editor (`Docs/SETUP.md` §2).

## Checks

```bash
python3 -I Tools/check_sources.py
```

A text-level sanity check: include correctness, reflection macros, brace balance, config syntax. **It is not a compiler** and a clean run says nothing about whether the project builds.

## Licence

Original work. Third-party assets are governed by `Docs/ASSET_PIPELINE.md`; nothing is copied from any existing game or franchise.
