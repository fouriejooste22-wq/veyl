# Asset pipeline and licensing policy

Rule 9 of the project's development rules is "use original or properly licensed assets." This document is how that is kept honest, and the record of what has actually come in.

---

## 1. Allowed sources

| Source | Licence | Commercial use | Notes |
|---|---|---|---|
| **Fab / Quixel Megascans** | Epic's Fab Standard Licence; Megascans library free for UE users | ✅ within Unreal Engine projects | The backbone of this project's environment art |
| **Epic sample projects** (Third Person template, City Sample, Electric Dreams, Valley of the Ancient) | Free, redistributable inside UE projects | ✅ | Master materials, vegetation, Manny/Quinn meshes |
| **MetaHuman** | Free; licensed for use in Unreal Engine projects | ✅ in UE only | The answer to the brief's facial-customisation requirement |
| **Fab marketplace (paid)** | Per-product; almost always perpetual commercial | ✅ | Keep the receipt and the licence text |
| **CC0 / public domain** (e.g. Poly Haven, ambientCG) | CC0 | ✅ | Still record the source |
| **Freesound** | Per-sound: CC0, CC-BY, or CC-BY-NC | ⚠️ **check each one** | CC-BY needs attribution; **CC-BY-NC cannot be used** in a commercial project |
| **Original work** (sculpted, photographed, recorded, composed) | Yours | ✅ | Preferred for anything that defines the game's identity |

## 2. Forbidden, without exception

- Assets ripped or extracted from any shipped game
- Anything from *Ghost Recon*, *Far Cry*, *Metal Gear*, *Splinter Cell* or any other franchise — **including names, logos, insignia, HUD designs, weapon model names and mission titles.** Aurelia is inspired by the open-world stealth genre; it copies nothing from it.
- Assets of unclear provenance ("found on a forum", "from an asset pack site with no licence page")
- AI-generated assets where the generator's terms do not grant commercial rights
- Real-world military unit insignia, real company logos, real people's likenesses
- Fonts without a desktop/embedding licence (a very common and very expensive mistake)

## 3. Naming conventions

| Type | Prefix | Example |
|---|---|---|
| Static mesh | `SM_` | `SM_Kolda_Rock_Granite_01` |
| Skeletal mesh | `SK_` | `SK_ShadowlinePlayer` |
| Material | `M_` | `M_Landscape_Kolda_Auto` |
| Material instance | `MI_` | `MI_Rock_Granite_Wet` |
| Material parameter collection | `MPC_` | `MPC_Weather` |
| Texture | `T_` + suffix | `T_Granite_01_D`, `_N`, `_ORM` |
| Blueprint | `BP_` | `BP_ShadowlineCharacter` |
| Data asset | `DA_` | `DA_Region_KoldaRange` |
| Animation montage | `AM_` | `AM_Vault_Low` |
| Animation sequence | `AS_` | `AS_Jog_Fwd` |
| Niagara system | `NS_` | `NS_Rain_Heavy` |
| MetaSound source | `MS_` | `MS_Ambience_Kolda_Day` |
| Sound cue / wave | `SC_` / `A_` | `A_Footstep_Gravel_01` |
| Input action / context | `IA_` / `IMC_` | `IA_Move`, `IMC_Shadowline_Default` |
| Level | `L_` | `L_Aurelia_Sandbox` |
| Widget | `WBP_` | `WBP_MainMenu` |

## 4. Texture budget

| Use | Max resolution |
|---|---|
| Hero props the player gets close to | 4096 |
| Standard environment props | 2048 |
| Terrain layers | 2048 (with Runtime Virtual Texture) |
| Small / distant props | 1024 |
| UI | As authored, no mips |

Pack roughness/metallic/AO into one ORM texture. Use BC7 for colour, BC5 for normals. `r.Streaming.PoolSize` is set to 2000 MB in `DefaultEngine.ini` — if the editor warns about texture streaming pool overflow, that number or the budget above is wrong.

## 5. Git LFS

`.gitattributes` routes all binary Unreal assets through Git LFS. **Run `git lfs install` once before your first commit of content.** A `.uasset` committed without LFS bloats the repository permanently and is painful to remove.

## 6. Asset register

Every third-party asset gets a row here when it enters the project. If it is not in this table, it is not cleared for use.

| Asset | Source | Licence | Attribution required | Date added |
|---|---|---|---|---|
| _(none yet — no content has been imported)_ | | | | |

## 7. Attribution file

When the first CC-BY asset is added, create `Docs/ATTRIBUTION.md` and surface its contents in the game's Settings → Credits screen. Some licences require the attribution to be visible to players, not just present in the repository.
