# Technical Feasibility Report — Operation Shadowline

**Date:** 2026-11-23
**Stage:** 1 — environment inspection
**Verdict:** The project is viable. **The machine this session runs on cannot build or run Unreal Engine 5.** All engine work must happen on your own PC.

---

## 1. What this session actually inspected

This session does not run on your PC. It runs in an ephemeral Linux container in Anthropic's cloud. Everything below was measured with `lscpu`, `free`, `df`, `uname`, direct filesystem probes and version checks — not assumed.

| Property | Measured value |
|---|---|
| OS | Ubuntu 24.04.5 LTS, kernel 6.18.44, x86_64 |
| Virtualisation | KVM guest (full virtualisation) |
| CPU | Intel Xeon @ 2.10 GHz, **4 cores / 4 threads**, family 6 model 207 (Emerald Rapids class) |
| CPU features | AVX-512 (f/dq/bw/vl/vnni/bf16/fp16), AMX, SHA-NI, AES-NI |
| RAM | **15 GiB**, **no swap** |
| Disk | 252 GB volume, **61 GB available** (a fixed per-session allowance) |
| GPU | **None.** No `/dev/dri`, no `nvidia-smi`, no PCI display device |
| Vulkan | `lavapipe` only — a CPU software rasteriser at `/opt/lavapipe` |
| Display server | **None.** `DISPLAY` and `WAYLAND_DISPLAY` are both empty |

### Toolchain present

| Tool | Status |
|---|---|
| clang / clang++ | 18.1.3 ✅ |
| gcc / g++ | 13.3.0 ✅ |
| cmake | 3.28.3 ✅ |
| make | 4.3 ✅ |
| ninja | 1.11.1 ✅ |
| Python | 3.13.18 ✅ |
| Node | 22.23.5 ✅ |
| **.NET SDK** | ❌ **Not installed** |
| **Unreal Engine** | ❌ **Not installed anywhere on the filesystem** |
| mono / msbuild | ❌ Not installed |

I searched the filesystem for `UnrealEditor*`, `UE_5*`, `GenerateProjectFiles.sh` and `UnrealBuildTool*`. There are no matches. **No engine is installed here, and I will not pretend otherwise.**

---

## 2. Why Unreal Engine 5 cannot run in this container

Four independent blockers. Any one of them alone is sufficient.

### Blocker A — No GPU (fatal, unfixable)

The container has no graphics hardware at all. The only Vulkan implementation available is `lavapipe`, a CPU software rasteriser.

Every headline feature in your brief depends on GPU capability this machine does not have:

| Feature | Requires |
|---|---|
| **Nanite** | Shader Model 6 (DX12 or Vulkan SM6), GPU compute |
| **Lumen** (software tracing) | SM6, mesh distance fields, heavy GPU compute |
| **Lumen** (hardware tracing) | Ray-tracing-capable GPU |
| **Virtual Shadow Maps** | SM6 |
| **Volumetric clouds / fog** | GPU compute |
| **TSR** | SM6 |

`lavapipe` does not provide an SM6-class feature set, and even where it technically completes a frame it does so at a rate measured in seconds per frame. There is no workaround — not a flag, not a fallback renderer. **Photorealistic rendering on a machine with no GPU is not a tuning problem; it is a hardware absence.**

### Blocker B — No display server

The Unreal Editor is a desktop GUI application. `DISPLAY` and `WAYLAND_DISPLAY` are unset and no X or Wayland compositor is running. The editor has nowhere to draw.

### Blocker C — Disk space (fatal at current allowance)

A UE5 Linux source build needs, approximately:

| Item | Size |
|---|---|
| Git clone of `EpicGames/UnrealEngine` | ~20 GB |
| `Setup.sh` binary dependencies | ~15 GB |
| Intermediate object files + linked binaries | ~60–150 GB |
| Derived Data Cache (shader compilation) | ~20–60 GB |
| **Minimum realistic total** | **~100 GB; comfortably 150–250 GB** |

**61 GB is available.** The build would fail partway through with `no space left on device`.

### Blocker D — Missing .NET SDK and build budget

`UnrealBuildTool` and `AutomationTool` are .NET applications. No .NET SDK is installed, so `GenerateProjectFiles.sh` cannot run. That is installable — but it does not help, because:

- Epic's own guidance for a source build assumes a high-core-count workstation. On **4 cores at 2.1 GHz**, a full engine compile realistically takes **10–25 hours**.
- Several link steps want **16 GB+ of RAM each**. This container has **15 GiB with no swap**, so linking would likely be OOM-killed.
- Access to `github.com/EpicGames/UnrealEngine` is a **private repository** requiring an Epic Games account linked to a GitHub account. This session's GitHub access is scoped to `fouriejooste22-wq/veyl` only.

Also worth knowing: **Epic does not ship a prebuilt Linux binary of UE5 through the Epic Games Launcher.** Linux is source-build-only. Windows is the fully supported path.

---

## 3. What this container *is* genuinely good for

This is not a dead end. Roughly 70% of the work in your 10-stage plan is text and structure, and all of it can be done here, correctly, and committed to git:

| Work | Viable here |
|---|---|
| C++ gameplay systems (authoring, review, refactoring) | ✅ |
| Project structure, modules, `.uproject`, `Build.cs`, `Target.cs` | ✅ |
| Config: Lumen / Nanite / VSM / scalability ladder / input | ✅ |
| World, region, mission and campaign design documents | ✅ |
| Data-asset schemas, save format, UI information architecture | ✅ |
| Asset licensing policy, Git LFS setup, CI scripts | ✅ |
| Development log, task tracking, unfinished-feature register | ✅ |
| **Compiling C++ against engine headers** | ❌ needs the engine |
| **Opening the editor, building terrain, placing foliage** | ❌ needs a GPU |
| **Seeing a single rendered frame** | ❌ needs a GPU |

**Consequence for honesty:** the C++ in this repository is written against the UE 5.5 API but **has never been compiled.** Expect to fix some compile errors on first build. I have flagged every API call I am less than certain about in `DEVELOPMENT_LOG.md`. I will not tell you a system works until it has been built and run.

---

## 4. What you need to install on your own PC

### Required software (Windows — the recommended path)

| # | Software | Notes |
|---|---|---|
| 1 | **Windows 10 22H2 or Windows 11 (64-bit)** | DX12 required for Nanite/Lumen |
| 2 | **Epic Games Launcher** → **Unreal Engine 5.5** | ~50 GB installed. Free. `.uproject` here targets 5.5 |
| 3 | **Visual Studio 2022 Community** (free) | With these workloads/components: **Game development with C++**, **Desktop development with C++**, **MSVC v143 build tools**, **Windows 11 SDK (10.0.22621+)**, **.NET 8 SDK**, **.NET Framework 4.6.2 targeting pack**, **C++ profiling tools**, **Unreal Engine installer** component |
| 4 | **Git** + **Git LFS** | LFS is mandatory: `.gitattributes` in this project routes `.uasset`/`.umap` through it |
| 5 | **Latest GPU driver** | NVIDIA 550+ / AMD Adrenalin 24.x+ |

> Install Visual Studio **before** the engine, so the engine installer finds the toolchain.

### Hardware: what each quality tier realistically needs

The scalability ladder in `Config/DefaultScalability.ini` was written so Low does **not** require Lumen (it falls back to screen-space GI), which is what makes the low tier achievable at all.

| Tier | GPU | VRAM | CPU | RAM | Storage |
|---|---|---|---|---|---|
| **Low** (1080p, ~45 fps, no Lumen) | GTX 1660 Super / RX 5600 XT | 6 GB | 4-core | 16 GB | SATA SSD |
| **Medium** (1080p, 60 fps, Lumen SW) | RTX 3060 / RX 6600 XT | 8 GB | 6-core | 16 GB | NVMe |
| **High** (1440p, 60 fps) | RTX 4070 / RX 7800 XT | 12 GB | 8-core | 32 GB | NVMe |
| **Ultra** (4K, 60 fps, HW Lumen) | RTX 4080 / 7900 XTX or better | 16 GB | 8-core+ | 32–64 GB | Fast NVMe |

### Hardware for *developing* (stricter than playing)

The editor costs far more than the game. For open-world work with World Partition and Lumen:

| | Minimum to work at all | Comfortable |
|---|---|---|
| CPU | 6-core | 12-core+ (shader compilation is embarrassingly parallel) |
| RAM | **32 GB** | **64 GB** |
| VRAM | 8 GB | 12–16 GB |
| Disk | 250 GB free | 1 TB+ NVMe |

**16 GB of RAM is below the practical floor for UE5 open-world editing.** The editor will run, but it will swap constantly while cooking and building navigation. If your PC has 16 GB, treat a RAM upgrade as the single highest-value change you can make.

---

## 5. Honest schedule expectations

Your brief describes a game of roughly *Ghost Recon Wildlands* scope. That game was made by a studio of several hundred people over about four years. One person with an AI assistant is not going to reproduce that, and I am not going to pretend the ten stages add up to it.

What *is* achievable, and worth doing:

| Horizon | Realistic outcome |
|---|---|
| **Weeks 1–2** | Project builds; third-person character moves well; one small, genuinely beautiful valley with real terrain, Megascans rock/tree assets, Lumen lighting, working day/night cycle. **This is milestone 1 and it is very achievable.** |
| **Months 1–3** | One region (~2×2 km) with World Partition streaming, weather, a stealth loop against simple enemy AI, a working UI shell |
| **Months 3–9** | Two or three regions, companion AI, vehicles, a short campaign of hand-built missions |
| **Years** | All five regions of Aurelia at the fidelity your brief describes |

The honest framing: **build a vertical slice that is excellent, not a whole island that is empty.** A single stunning 2 km valley with tight movement and a real stealth loop is a far better outcome than 500 km² of placeholder terrain — and it is the thing that would make the rest fundable or recruitable.

---

## 5b. Console targets — asked and answered

**Can this be put on a PS4? No.** Three independent blockers:

1. **UE5 does not support PS4.** Epic dropped last-generation consoles in Unreal Engine 5; PS4 and Xbox One were UE4 platforms. This project is UE 5.5, so PS4 is not a build target that exists for it.
2. **The hardware could not run it.** A base PS4 is roughly 1.8 TFLOPS with a 1.6 GHz Jaguar CPU. Nanite, Lumen and Virtual Shadow Maps all require Shader Model 6-class hardware. The Low tier in `DefaultScalability.ini` already drops Lumen entirely, and that floor is still well above a PS4.
3. **A retail PS4 will not install your own software.** Publishing to PlayStation requires a PlayStation Partners account (company registration, approved pitch), a leased devkit, Sony's SDK, and passing certification. Unreal's console platform code is not in the public engine download — licensed developers obtain it through the platform holder's portal.

### What this means in practice

| Want | Path |
|---|---|
| Play the game | **Windows PC.** No approval, no platform fee. Already the project's target. |
| Play it on a TV with a controller | PC to the TV via HDMI. A DualShock 4 works over USB or Bluetooth, and Steam Input maps it automatically. |
| Stream it to the PS4 | Not possible. Remote Play runs PS4 → PC, never PC → PS4. |
| Ship it to other people | itch.io (free) or Steam (one-off fee per app). |
| Reach a console eventually | **PS5**, which UE5 does support — but still needs an approved PlayStation developer account. A conversation for after there is a playable vertical slice to show, not before. |

### The decision this forces

If PS4 specifically were the goal, the correct engine would be **UE4, not UE5** — and that means giving up Nanite, Lumen and the photorealistic target in section 3 of the brief. **Recorded decision: stay on UE5 and target Windows.** The visual quality is the point of the project; a PS4 build is not worth trading it for. Cheap to reverse now, expensive later.

## 6. Two paths forward

### Path A — You run the engine, I write the code (default)

I write C++, config, data assets, design docs and tooling here and commit them. You pull, build in Visual Studio, open the editor, do the art and level work, and report compile errors and results back. I fix and iterate.

Works today. The split is natural: the engine work needs your GPU, the code work does not.

### Path B — Attach your PC to this session (much faster loop)

Claude Code supports **Remote Control**: you share one folder from your own computer with this session, and then I can run commands *on your machine* — invoke `UnrealBuildTool`, compile the project, run the editor's commandlets, read the build log and fix errors directly.

To enable it: **Claude desktop app → Settings → Claude Code → Remote Control**, share the folder you want to keep the project in.

I checked for attached machines at the start of this session and there are none. With Path B I could compile and iterate against your real engine install, which turns "here is code you must test" into "I built it and here is the error I just fixed."

---

## 7. Stage 1 status

| Item | Status |
|---|---|
| Inspect development environment | ✅ Done — above |
| Feasibility report | ✅ Done — this document |
| Software install list | ✅ Done — section 4 |
| Establish UE5 project structure | ✅ Done — see `Docs/ARCHITECTURE.md` |
| Create initial project | ✅ Created, **never compiled** (no engine here) |
| First playable environment | ⛔ **Blocked on a GPU.** Step-by-step build instructions are in `Docs/SETUP.md`; the work is yours to execute in the editor until Path B is enabled |

**No part of this project has been compiled or run. Nothing in it is claimed to work.**
