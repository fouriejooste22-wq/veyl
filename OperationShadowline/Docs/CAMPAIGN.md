# Campaign structure

Original fiction. The structure borrows genre conventions — provinces, intel gathering, a named target per province building to a final antagonist — but no name, mark, mission, character or interface design is taken from any existing game.

---

## 1. Shape

**Aurelia is 85 km east–west by 120 km north–south**, divided into **five regions** and **fifteen provinces**.

```
15 province overseers  →  4 regional directors  →  1 final antagonist
```

- A **province** is cleared when its required operations are complete and its **overseer** is resolved.
- A **regional director** unlocks when every province in their region is cleared.
- The **finale** at Meridian Works needs all four directors resolved.

Within a region, province order is almost entirely free. The `PrerequisiteProvinceIds` lists in `UShadowlineProvinceDefinition` are deliberately shallow — a long prerequisite chain turns an open world back into a corridor. Difficulty tiers are advisory: nothing stops the player walking into a tier 5 province on day one and finding out why that was unwise.

## 2. What a confrontation is

Threshold carries no weapons and has no authority to use force. Every confrontation resolves by one of:

- getting the target **on record** saying or doing something
- **lifting** a drive, ledger, manifest or file
- **reaching a place first**, before they destroy or remove what is there
- **putting them on a boat** — extraction, not capture

That constraint is the point. It turns each of the sixteen named antagonists into a design problem — an approach, a timing window, an observation post — rather than a fight. **There is no death screen in this game.** Detection escalates a situation; it does not end a mission. Missions flagged `bUndetectedForBestOutcome` change which outcome they can reach if you are seen.

## 3. Provinces

### Siren Coast — Act I — *Regional Director Priya Raghunathan, Maritime Logistics*

| Province | Tier | Overseer | What they hold |
|---|---|---|---|
| **Port Calloway** | 1 | Harbour Agent Celia Mors | The manifest of every vessel that left in the four days after the blackout |
| **Dunmere Shelf** | 2 | Patrol Lead Yusuf Adeyemi | The patrol log: which villages were cleared, and in what order |
| **The Three Stacks** | 2 | Beacon Keeper Otto Lind | Switched the navigation beacon to a false bearing |
| **The Wreckline** | 3 | Salvage Agent Dmitri Baranov | Pulling drives off eleven wrecks before anyone else can |

### Kolda Range — Act II — *Regional Director Annika Sørvik, Northern Operations*

| Province | Tier | Overseer | What they hold |
|---|---|---|---|
| **Hale Terraces** | 2 | Site Warden Imre Bako | An inventory of nine emptied villages, down to the livestock |
| **Vosk Pass** | 3 | Convoy Marshal Ruben Teixeira | Every convoy schedule on the only road north |
| **Kolda Summit** | 4 | Relay Chief Dagny Holt | The only working transmitter on Aurelia — and she is waiting for someone to come |

### Vaelmire Basin — Act III — *Regional Director Emeka Boateng, Southern Survey*

| Province | Tier | Overseer | What they hold |
|---|---|---|---|
| **Ferrow's Crossing** | 3 | Survey Lead Mirela Dacic | Surveying for something specific in the riverbed |
| **Greenmouth Delta** | 3 | Logistics Chief Pavla Rourke | Moving people out through the delta at night |
| **The Sunken Quarter** | 4 | Containment Officer Halvar Nkemdirim | Posted guards on a flooded ruin |

### Ashen Flats — Act IV — *Regional Director Tove Lindqvist, Legacy Sites*

| Province | Tier | Overseer | What they hold |
|---|---|---|---|
| **The Long Run** | 3 | Pump Chief Marisol Ferrand | Reversed the pipeline flow four days before the blackout |
| **Cinder Reach** | 4 | Field Supervisor Abel Kranz | Running crews into a storm belt on someone else's schedule |
| **Field One** | 5 | Records Custodian Inés Vallejo | Burning the 1987 file, page by page, in a drum behind the office |

### Meridian Works — Act V

| Province | Tier | Overseer | What they hold |
|---|---|---|---|
| **The Research Annex** | 5 | Dr Lotta Reinholm, Chief Analyst | Understands what happened under Meridian — and has been trying to leave for nine days |
| **Meridian Works** | 5 | **Site Director Severin Krejčí** | Has not left the plant since the blackout. He is not hiding what happened. He is finishing it. |

## 4. Operations

**55 operations** across the fifteen provinces:

| | Count |
|---|---|
| Province confrontations | 15 |
| Regional director missions | 4 |
| Finale | 1 |
| Reconnaissance | 10 |
| Investigation | 9 |
| Rescue | 5 |
| Search and discovery | 5 |
| Environmental puzzles | 5 |
| Supply recovery | 2 |

Story operations carry an act. Side operations do not, and are available whenever their province is open.

## 5. The ending

Not a boss fight. The last choice is **expose Aurelia, or contain it**, and the final scene is the team on the water watching the island while the relay finally carries the player's decision off it. What that decision was determines what the relay says, who is waiting at the other end, and which of the four companions is standing on the deck.

## 6. Open design questions

Recorded rather than guessed at:

| Question | Status |
|---|---|
| **What happens to a resolved overseer** | They should persist — reassigned, recalled, or still there and now aware of you. No system exists. |
| **Mission failure** | `bUndetectedForBestOutcome` is the current answer: detection changes the reachable outcome rather than triggering a retry. Untested. |
| **Director mission design** | The four director missions are named and sited but have no objective structure yet. |
