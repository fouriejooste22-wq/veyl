# Aurelia — world and campaign design

Original fiction. No character, faction, location, logo or plot element here is drawn from any existing game, film or franchise.

---

## 1. The island

**Aurelia** is a large volcanic island in a temperate-to-subtropical ocean, roughly 70 km north to south. A dormant central caldera gave it fertile valleys, deep ore seams and geothermal heat — which is why anyone settled it, and why a resource corporation eventually bought most of it.

Its geography is the reason the five regions look nothing alike: the Kolda massif in the north throws a rain shadow west across the Ashen Flats and dumps everything it catches on the southern basin.

### The five regions

| Region | Name | Character |
|---|---|---|
| North | **Kolda Range** | Granite massif, snow above 1,800 m, black pine, terraced villages on the valley shoulders, a single switchback road. Cold blue shadows, hard raking light, wind that never stops. |
| South | **Vaelmire Basin** | Flooded rainforest basin. Buttressed trees, hanging moss, a river delta broken by waterfalls. Settlements abandoned to the water and reclaimed by it. Green-gold light through canopy; permanent haze; sudden violent rain. |
| West | **Ashen Flats** | Volcanic desert in the Kolda rain shadow. Grey-black dunes over basalt, wind-carved mesas, an abandoned extraction field of pipe runs and sun-bleached prefabs. Bleached white light, long dust plumes. |
| East | **Siren Coast** | Limestone cliffs, pale beaches, a working fishing port and three offshore stacks. The only part of Aurelia that still looks inhabited. Warm light, wet rock, seabirds. |
| Centre | **Meridian Works** | The caldera floor, industrialised: geothermal plant, ore processing, research annex, rail spur, worker housing. Sodium lighting, steam, corrugated steel, rust. The place every story thread eventually points at. |

**Design rule:** a player dropped anywhere on Aurelia with no HUD should know which region they are in within two seconds, from light and silhouette alone. That is what `UShadowlineRegionDefinition` exists to enforce — each region's fog colour, atmosphere tint, sun intensity and weather table are deliberately far apart.

---

## 2. Premise

Aurelia went quiet eleven days ago. Not gradually — all of it, inside four hours. Satellite relay, maritime radio, the undersea cable, the civil aviation beacon. No distress call, no weather event, no seismic trigger large enough to explain it.

**THRESHOLD** is an international disaster-response and reconnaissance body: the organisation that goes in when nobody knows yet whether a place needs doctors, engineers or a diplomat. It does not have an army. It has four people, a boat, and a mandate to find out what happened and who is still alive.

The player leads **Shadowline** — a four-person reconnaissance team put ashore on the Siren Coast at night.

What they find: the island is not empty. It is being *tidied*. **Vantrell Continental**, which owns the Meridian Works and most of the land, has private security on the ground sweeping settlements, pulling drives from server rooms and burning paper. The civilians are not gone; they have been moved. And Vantrell's people are as surprised by the comms blackout as THRESHOLD is — they did not cause it, they are exploiting it.

The real cause is underneath Meridian Works, and the people who understood it are the ones Vantrell is most eager to find first.

### Tone

Investigative, not militaristic. The player is a witness with a camera and a lockpick, not an occupying force. Tension comes from *not being seen* and from *not knowing* — never from firepower. Non-lethal by design and by mandate: THRESHOLD has no authority to shoot anyone, and the game's systems reflect that.

---

## 3. Squad

| | Role | Personality | Arc |
|---|---|---|---|
| **Player** | Team lead | Silent protagonist; characterised by player choice | — |
| **Nadia Ferreiro** | Hydrologist and field engineer | Dry, impatient, allergic to speculation. Wants data. | Learns that some questions cannot be answered with instruments |
| **Tomas "Kip" Okonkwo** | Communications and drone operator | Youngest, talks to fill silence, brilliant and underconfident | Stops needing permission |
| **Sela Varga** | Ex-mountain rescue, medic | Oldest, quietest, has done this before in worse places | The team's conscience; will say no to the player |

Companions comment on the environment, disagree with each other, and occasionally disagree with the player's decision — in dialogue, not in mechanics. Each has two or three moments where their reaction to a choice is the point of the scene.

---

## 4. Mission types

| Type | What the player does |
|---|---|
| **Reconnaissance** | Reach an overlook, photograph a site, leave without being detected. The purest expression of the game. |
| **Rescue** | Find people. Move them, or mark them for extraction. Noise brings a sweep. |
| **Investigation** | Reconstruct what happened from the state of a place — not from a cutscene. |
| **Supply recovery** | Find and move cached relief materiel; route planning as a puzzle. |
| **Search and discovery** | Open-world, unmarked: wrecks, hidden valleys, survivor camps, old shrines, Vantrell's own abandoned equipment. |
| **Environmental puzzles** | Power, water and access as physical systems: reroute a substation, drain a flooded stair, lower a bridge. Fictional security panels, not real-world procedures. |

### The stealth loop

Avoid, observe, move on. The player's tools are observational and non-lethal: a spotting scope, a small drone, a signal sniffer, a distraction emitter, a door shim. The design target is that **a mission completed without a single guard ever becoming suspicious is the best outcome and feels like it.**

---

## 5. Campaign shape

Five acts, one per region, roughly in this order — though the island is open after Act 1.

| Act | Region | Question the act answers |
|---|---|---|
| **I — Landfall** | Siren Coast | Where did everyone go? *(They were moved. By whom, and in what hurry.)* |
| **II — The Shadow Line** | Kolda Range | Why did the north go dark first? *(The relay station. And what the villagers saw before it did.)* |
| **III — Standing Water** | Vaelmire Basin | What is Vantrell looking for? *(A person, not a thing.)* |
| **IV — Ash Record** | Ashen Flats | What did they already try, and bury? *(The first extraction field, and why it was abandoned in 1987.)* |
| **V — Meridian** | Meridian Works | What is actually down there? |

### Meaningful choices

Three or four decisions that change outcomes rather than dialogue flavour:

- Whether to transmit what you find immediately (fast, and makes you a target) or carry it out physically (slow, and some of it won't survive the trip)
- Which of two survivor groups gets the one extraction window
- Whether Kip's intercepted Vantrell traffic gets used as evidence or as leverage
- At the end: expose Aurelia, or contain it

The ending is cinematic and quiet — not a boss fight. The last scene is the team on the water, watching the island, and the player seeing the consequence of their choices in what the relay finally carries.

---

## 6. Environmental storytelling rules

The island tells the story; characters confirm it.

- **No information in a cutscene that could have been in a room.** A kitchen with three days of uneaten food says more than a line of dialogue.
- **Every abandoned building answers one question** and raises another.
- **Collectible documents are primary sources** — a shift roster, a geological survey with a page missing, a child's drawing of the mountain — not lore dumps.
- **The blackout's cause is visible on the terrain** from Act II onward, to anyone who looks up at the right moment. Most players will not notice until Act V. That is the intent.
