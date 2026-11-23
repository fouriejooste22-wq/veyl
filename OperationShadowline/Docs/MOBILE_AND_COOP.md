# Mobile, iPad and co-op — what is possible, and what it costs

Asked for: a playable version on mobile and iPad at top specs, with multiplayer up to four people, while keeping the laptop (UE5) version.

---

## 1. What was delivered

**`Prototype/shadowline-field.html`** — a playable, touch-first tactical stealth game that runs in Safari on an iPad or a phone. It is **not** the UE5 game and does not pretend to be. It exists because the stealth loop is the riskiest part of the design and it is far cheaper to find out it is wrong in a browser than in an engine.

What it actually implements:

| System | Status |
|---|---|
| Guard patrols on routes, with pauses and scanning | ✅ playable |
| Vision cones, raycast against walls, blocked by geometry | ✅ |
| Suspicion build-up and decay, four guard states | ✅ |
| Guard-to-guard communication within range | ✅ |
| Concealment from stance, reeds, light pools, pace | ✅ |
| Noise propagation from pace, stance, water, reeds | ✅ |
| Three companions with follow, formation, cone-avoidance, revive | ✅ |
| Four characters with distinct abilities | ✅ |
| Acoustic emitter as a distraction tool | ✅ |
| Site alert escalation and forced extraction | ✅ |
| Nine objectives including three optional | ✅ |
| **All four difficulty presets, with the real knob values** | ✅ |
| Four-player co-op, one character each | ⚠️ **written, not switched on** — see §3 |

The difficulty values are copied from `FShadowlineDifficultyKnobs::FromPreset`, so the prototype and the UE5 project cannot drift apart. Playing Blackout here tells you how Blackout will feel there.

---

## 2. Why there is no native iPad build

**A native iOS/iPadOS build is possible for this project, but not from here.** It needs all of:

| Requirement | Why |
|---|---|
| A **Mac** with Xcode | Apple's toolchain only runs on macOS. No exceptions, no workarounds. |
| An **Apple Developer account** (~$99/yr) | Needed for provisioning profiles and for TestFlight or App Store distribution. |
| The UE5 **iOS platform support** | Installed with the engine, but the build must be driven from the Mac. |
| A **remote build setup**, or the project moved to the Mac | UE can cross-compile from Windows to iOS only with a Mac on the network doing the signing. |

And a design cost that matters more than any of the above:

**The visual target does not survive mobile.** Nanite, Lumen and Virtual Shadow Maps are not available on iOS. A mobile build means the mobile renderer, baked or simple dynamic lighting, aggressive LODs and a different material set — in practice a second art pipeline. That is a real project, not a build-settings change.

### Recommendation

Treat iPad as a **later, separate target**, not a parallel one. Finish the Windows vertical slice first. If a mobile version is still wanted after that, the honest plan is a purpose-built mobile branch with its own art budget — and this browser prototype is a far better guide to whether that is worth doing than a guess.

---

## 3. Co-op: built, switched off

The four-player code is complete in the prototype:

- **Positions**: each player claims one of Lead, Ferreiro, Okonkwo or Varga. Unclaimed positions fall to the companion AI, so a game is never short-handed.
- **Authority**: the player whose peer label sorts first simulates the guards and publishes absolute state about five times a second. Every page computes the same host without anyone announcing it, and a dropped update means a slightly stale guard rather than a desynchronised one.
- **Movement**: each player's own position is authoritative locally and published about ten times a second, well inside the platform's shared send budget.
- **Events**: objectives, emitters, revives and ability use are broadcast; everything is treated as untrusted and clamped on arrival.
- **Degradation**: if the channel is unavailable the game runs solo with no error and no missing features. That is why the prototype is playable right now.

**It is not enabled**, because it needs a capability that opens a live channel between everyone viewing the page, and that is the owner's decision rather than an agent's. Enabling it is one line on the publish — say the word and co-op is live on the same link.

---

## 4. The laptop version is untouched

Everything under `Source/`, `Config/` and the rest of `Docs/` is exactly as it was. The prototype lives in its own `Prototype/` folder and shares no code with the UE5 project — only data: the difficulty values, the region, the mission and the four characters.

The UE5 project remains the target. This is the instrument that tells us whether the design is worth building there.

---

## 5. Verified

`node Tools/verify_prototype_map.mjs` → **23 of 23 checks pass.**

The map's geometry and connectivity are checked for the failures that would quietly make the mission impossible. It caught seven real ones on the first run:

| Bug found | Consequence had it shipped |
|---|---|
| Patrol route 4 ran down the middle of the east river | Guard stuck on frame one, patrol dead |
| Patrol route 0's first waypoint sat inside a crate | Same |
| The west survey marker was inside deep water | **Mission mathematically impossible to complete** |
| The exfil zone was in unswimmable water | **Mission impossible to finish** |
| The southern reed channel was deep water | South bank cut in two, exfil unreachable |
| The main river stopped 6 m short of each map edge | The entire bridge chokepoint could be walked around at the shoreline |
| A cover crate clipped the corrected patrol route | Guard stuck |

That last one is the kind of thing nobody finds by playing for ten minutes, and it would have made the bridge — the map's whole tactical centre — pointless.

**What is not verified:** how the game feels. Guard spacing, suspicion rates, companion follow distance and whether the mission is too hard or too easy are all judgement calls that need a person playing it. That is the one thing I cannot do.
