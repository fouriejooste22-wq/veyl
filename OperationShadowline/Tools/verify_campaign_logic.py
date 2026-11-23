#!/usr/bin/env python3
"""
Verifies the ALGORITHM in UShadowlineCampaignSubsystem, not the C++.

The campaign's derived state - province state, mission state, survey percentages, whether
the finale is open - is pure logic over stored progress plus definition data. That logic can
be modelled exactly and tested without an engine, which is worth doing: a progression bug
here is the kind that surfaces forty hours into someone's playthrough and cannot be
reproduced.

This file transcribes the derivation rules from
Source/OperationShadowline/Core/ShadowlineCampaignSubsystem.cpp and asserts them against
hand-computed expectations.

WHAT THIS PROVES: the rules are self-consistent and behave as intended at the edges.
WHAT IT DOES NOT PROVE: that the C++ compiles, or that the transcription is faithful.
If the C++ changes, this file must change with it, by hand.

Run:  python3 -I Tools/verify_campaign_logic.py
"""

from __future__ import annotations

import sys
from dataclasses import dataclass, field

# --- mirrors of the C++ enums -------------------------------------------------

LOCKED, OPEN, ACTIVE, CLEARED = "Locked", "Open", "Active", "Cleared"
M_LOCKED, M_AVAILABLE, M_ACTIVE, M_COMPLETE = "Locked", "Available", "Active", "Complete"

DIRECTOR_REGIONS = ["North", "South", "West", "East"]   # Meridian ("Central") has none


@dataclass
class ProvinceDef:
    id: str
    region: str
    documents: int = 0
    landmarks: int = 0
    caches: int = 0
    prerequisites: list[str] = field(default_factory=list)

    def total_discoverables(self) -> int:
        return max(0, self.documents) + max(0, self.landmarks) + max(0, self.caches)


@dataclass
class MissionDef:
    id: str
    province: str
    act: int = 0
    mission_type: str = "Reconnaissance"
    prerequisites: list[str] = field(default_factory=list)
    objectives: list[tuple[str, bool]] = field(default_factory=list)  # (event, required)

    def is_confrontation(self) -> bool:
        return self.mission_type in ("Confrontation", "Director", "Finale")


@dataclass
class ProvinceProgress:
    documents_found: int = 0
    landmarks_found: int = 0
    caches_found: int = 0
    overseer_resolved: bool = False


@dataclass
class MissionProgress:
    completed_objectives: list[str] = field(default_factory=list)
    complete: bool = False
    detected_this_attempt: bool = False


class Campaign:
    """Transcription of UShadowlineCampaignSubsystem's derivation rules."""

    def __init__(self, provinces: list[ProvinceDef], missions: list[MissionDef]):
        self.province_defs = {p.id: p for p in provinces}
        self.mission_defs = {m.id: m for m in missions}
        self.provinces: dict[str, ProvinceProgress] = {}
        self.missions: dict[str, MissionProgress] = {}
        self.resolved_directors: set[str] = set()

    # -- stored progress access (defaults for unknown ids) --

    def pp(self, pid: str) -> ProvinceProgress:
        return self.provinces.get(pid, ProvinceProgress())

    def mp(self, mid: str) -> MissionProgress:
        return self.missions.get(mid, MissionProgress())

    # -- derived state --

    def missions_in(self, pid: str) -> list[str]:
        return [m.id for m in self.mission_defs.values() if m.province == pid]

    def province_state_ignoring_prereqs(self, pid: str) -> str:
        d = self.province_defs.get(pid)
        if d is None:
            return LOCKED

        p = self.pp(pid)

        # Only story missions and confrontations gate clearing; side operations never do.
        all_gating_done = True
        for m in self.mission_defs.values():
            if m.province != pid:
                continue
            if (m.act > 0 or m.is_confrontation()) and not self.mp(m.id).complete:
                all_gating_done = False
                break

        if p.overseer_resolved and all_gating_done:
            return CLEARED

        if p.documents_found or p.landmarks_found or p.caches_found or p.overseer_resolved:
            return ACTIVE
        for mid in self.missions_in(pid):
            m = self.mp(mid)
            if m.complete or m.completed_objectives:
                return ACTIVE

        return OPEN

    def prerequisites_met(self, d: ProvinceDef) -> bool:
        return all(
            self.province_state_ignoring_prereqs(req) == CLEARED
            for req in d.prerequisites
        )

    def province_state(self, pid: str) -> str:
        d = self.province_defs.get(pid)
        if d is None:
            return LOCKED
        intrinsic = self.province_state_ignoring_prereqs(pid)
        # Prerequisites gate opening, not progress already made.
        if intrinsic == OPEN and not self.prerequisites_met(d):
            return LOCKED
        return intrinsic

    def mission_state(self, mid: str) -> str:
        d = self.mission_defs.get(mid)
        if d is None:
            return M_LOCKED
        m = self.mp(mid)
        if m.complete:
            return M_COMPLETE
        if m.completed_objectives:
            return M_ACTIVE
        if d.province and self.province_state(d.province) == LOCKED:
            return M_LOCKED
        if any(not self.mp(r).complete for r in d.prerequisites):
            return M_LOCKED
        return M_AVAILABLE

    def required_objectives_satisfied(self, mid: str) -> int:
        d = self.mission_defs.get(mid)
        if d is None:
            return 0
        done = set(self.mp(mid).completed_objectives)
        return sum(1 for ev, req in d.objectives if req and ev in done)

    def province_survey_fraction(self, pid: str) -> float:
        d = self.province_defs.get(pid)
        if d is None:
            return 0.0
        total = d.total_discoverables()
        if total <= 0:
            return 1.0          # nothing to find is fully surveyed by definition
        p = self.pp(pid)
        found = (min(p.documents_found, d.documents)
                 + min(p.landmarks_found, d.landmarks)
                 + min(p.caches_found, d.caches))
        return max(0.0, min(found / total, 1.0))

    def island_survey_fraction(self) -> float:
        # Weighted by content, not an average of percentages.
        found = available = 0
        for pid, d in self.province_defs.items():
            total = d.total_discoverables()
            if total <= 0:
                continue
            p = self.pp(pid)
            available += total
            found += (min(p.documents_found, d.documents)
                      + min(p.landmarks_found, d.landmarks)
                      + min(p.caches_found, d.caches))
        return 0.0 if available <= 0 else max(0.0, min(found / available, 1.0))

    def provinces_cleared(self) -> int:
        return sum(1 for pid in self.province_defs if self.province_state(pid) == CLEARED)

    def overseers_resolved(self) -> int:
        return sum(1 for pid, p in self.provinces.items()
                   if p.overseer_resolved and pid in self.province_defs)

    def director_available(self, region: str) -> bool:
        if region in ("Central", "None"):
            return False
        ids = [p.id for p in self.province_defs.values() if p.region == region]
        if not ids:
            return False
        return all(self.province_state(i) == CLEARED for i in ids)

    def finale_available(self) -> bool:
        return all(r in self.resolved_directors for r in DIRECTOR_REGIONS)

    # -- mutations --

    def complete_objective(self, mid: str, event: str) -> None:
        if not event or mid not in self.mission_defs:
            return
        m = self.missions.setdefault(mid, MissionProgress())
        if event not in m.completed_objectives:
            m.completed_objectives.append(event)

    def record_detection(self, mid: str) -> None:
        if mid in self.mission_defs:
            self.missions.setdefault(mid, MissionProgress()).detected_this_attempt = True

    def complete_mission(self, mid: str) -> None:
        d = self.mission_defs.get(mid)
        if d is None:
            return
        m = self.missions.setdefault(mid, MissionProgress())
        if m.complete:
            return
        m.complete = True
        m.completed_undetected = not m.detected_this_attempt
        if d.mission_type == "Confrontation" and d.province:
            self.provinces.setdefault(d.province, ProvinceProgress()).overseer_resolved = True

    def record_discovery(self, pid: str, kind: str) -> None:
        d = self.province_defs.get(pid)
        if d is None:
            return
        p = self.provinces.setdefault(pid, ProvinceProgress())
        # Clamped, so a double-fire cannot push survey past 100%.
        if kind == "Document":
            if p.documents_found >= d.documents:
                return
            p.documents_found += 1
        elif kind == "Landmark":
            if p.landmarks_found >= d.landmarks:
                return
            p.landmarks_found += 1
        elif kind == "Cache":
            if p.caches_found >= d.caches:
                return
            p.caches_found += 1


# --- fixture: a small slice shaped like the real campaign ---------------------

def build() -> Campaign:
    provinces = [
        ProvinceDef("calloway", "East", documents=3, landmarks=2, caches=1),
        ProvinceDef("stacks", "East", documents=2, landmarks=1, caches=0,
                    prerequisites=["calloway"]),
        ProvinceDef("hale", "North", documents=2, landmarks=1, caches=1),
        ProvinceDef("barren", "West"),                      # nothing to find at all
        ProvinceDef("meridian", "Central", documents=4, landmarks=1, caches=0,
                    prerequisites=["stacks", "hale"]),
    ]
    missions = [
        MissionDef("cal_recon", "calloway", act=1, mission_type="Reconnaissance",
                   objectives=[("reach_office", True), ("photograph", True), ("rota", False)]),
        MissionDef("cal_side", "calloway", act=0, mission_type="Puzzle",
                   objectives=[("power", True)]),
        MissionDef("cal_con", "calloway", act=1, mission_type="Confrontation",
                   prerequisites=["cal_recon"],
                   objectives=[("locate", True), ("record", True)]),
        MissionDef("stk_recon", "stacks", act=0, mission_type="Reconnaissance",
                   objectives=[("lighthouse", True)]),
        MissionDef("mer_finale", "meridian", act=5, mission_type="Finale",
                   objectives=[("control_floor", True), ("decide", True)]),
    ]
    return Campaign(provinces, missions)


# --- assertions ---------------------------------------------------------------

CHECKS: list[tuple[str, callable]] = []


def check(name):
    def deco(fn):
        CHECKS.append((name, fn))
        return fn
    return deco


def eq(actual, expected, what):
    assert actual == expected, f"{what}: expected {expected!r}, got {actual!r}"


def close(actual, expected, what, tol=1e-9):
    assert abs(actual - expected) < tol, f"{what}: expected {expected}, got {actual}"


@check("fresh campaign: no prerequisites means Open, unmet prerequisites mean Locked")
def _():
    c = build()
    eq(c.province_state("calloway"), OPEN, "calloway")
    eq(c.province_state("hale"), OPEN, "hale")
    eq(c.province_state("stacks"), LOCKED, "stacks (needs calloway)")
    eq(c.province_state("meridian"), LOCKED, "meridian (needs stacks + hale)")


@check("a single discovery moves a province from Open to Active")
def _():
    c = build()
    c.record_discovery("calloway", "Document")
    eq(c.province_state("calloway"), ACTIVE, "calloway after one document")


@check("clearing needs the overseer AND every gating mission; side operations never gate")
def _():
    c = build()
    c.complete_mission("cal_recon")
    c.complete_mission("cal_con")          # a Confrontation also resolves the overseer
    eq(c.pp("calloway").overseer_resolved, True, "overseer auto-resolved by confrontation")
    eq(c.province_state("calloway"), CLEARED, "calloway with both story missions done")
    eq(c.mission_state("cal_side"), M_AVAILABLE, "side operation still available")
    eq(c.province_state("calloway"), CLEARED, "side operation does not block clearing")


@check("clearing a prerequisite opens its dependent")
def _():
    c = build()
    eq(c.province_state("stacks"), LOCKED, "stacks before")
    c.complete_mission("cal_recon")
    c.complete_mission("cal_con")
    eq(c.province_state("stacks"), OPEN, "stacks after calloway cleared")
    eq(c.province_state("meridian"), LOCKED, "meridian still needs hale")


@check("an overseer resolved without the gating missions is Active, not Cleared")
def _():
    c = build()
    c.provinces.setdefault("calloway", ProvinceProgress()).overseer_resolved = True
    eq(c.province_state("calloway"), ACTIVE, "overseer alone is not enough")


@check("a province with progress is never retroactively locked")
def _():
    c = build()
    # Reaching a locked province out of order and finding something there.
    c.record_discovery("stacks", "Document")
    eq(c.province_state("stacks"), ACTIVE, "stacks with progress despite unmet prerequisite")


@check("mission locks follow their province, but work already done outranks the lock")
def _():
    c = build()
    eq(c.mission_state("stk_recon"), M_LOCKED, "mission in a locked province")
    c.complete_objective("stk_recon", "lighthouse")
    eq(c.mission_state("stk_recon"), M_ACTIVE, "objective done outranks the province lock")


@check("mission prerequisites gate independently of the province")
def _():
    c = build()
    eq(c.mission_state("cal_con"), M_LOCKED, "confrontation before its recon")
    c.complete_mission("cal_recon")
    eq(c.mission_state("cal_con"), M_AVAILABLE, "confrontation after its recon")


@check("required objectives are counted; optional ones are not")
def _():
    c = build()
    c.complete_objective("cal_recon", "rota")          # optional
    eq(c.required_objectives_satisfied("cal_recon"), 0, "optional does not count")
    c.complete_objective("cal_recon", "reach_office")  # required
    eq(c.required_objectives_satisfied("cal_recon"), 1, "required counts")
    c.complete_objective("cal_recon", "reach_office")  # idempotent
    eq(c.required_objectives_satisfied("cal_recon"), 1, "no double count")


@check("survey fraction: partial, clamped, and empty provinces count as complete")
def _():
    c = build()
    close(c.province_survey_fraction("calloway"), 0.0, "calloway empty")
    c.record_discovery("calloway", "Document")
    c.record_discovery("calloway", "Document")
    close(c.province_survey_fraction("calloway"), 2 / 6, "calloway 2 of 6")

    for _ in range(10):                               # over-fire every kind
        c.record_discovery("calloway", "Document")
        c.record_discovery("calloway", "Landmark")
        c.record_discovery("calloway", "Cache")
    close(c.province_survey_fraction("calloway"), 1.0, "calloway clamped at 1.0")
    eq(c.pp("calloway").documents_found, 3, "document count clamped to the definition")

    close(c.province_survey_fraction("barren"), 1.0, "province with nothing to find")
    close(c.province_survey_fraction("nonexistent"), 0.0, "unknown province")


@check("island survey is weighted by content, not an average of percentages")
def _():
    c = build()
    # calloway 6, stacks 3, hale 4, meridian 5, barren 0 -> 18 discoverables.
    # Fill the smallest province completely: 3 of 18, NOT 1/4 of the provinces.
    for _ in range(2):
        c.record_discovery("stacks", "Document")
    c.record_discovery("stacks", "Landmark")
    close(c.province_survey_fraction("stacks"), 1.0, "stacks complete")
    close(c.island_survey_fraction(), 3 / 18, "island weighted by content")
    assert c.island_survey_fraction() < 0.25, "a small province must not count as a quarter"


@check("completing undetected is read from observed state, not asserted by the caller")
def _():
    c = build()
    c.complete_mission("cal_recon")
    eq(c.mp("cal_recon").completed_undetected, True, "clean run")

    c2 = build()
    c2.record_detection("cal_recon")
    c2.complete_mission("cal_recon")
    eq(c2.mp("cal_recon").completed_undetected, False, "detected run")


@check("regional director needs every province in the region cleared")
def _():
    c = build()
    eq(c.director_available("East"), False, "East before")
    c.complete_mission("cal_recon")
    c.complete_mission("cal_con")
    eq(c.director_available("East"), False, "East with stacks still open")
    c.complete_mission("stk_recon")                   # side op: does not clear stacks
    eq(c.province_state("stacks"), ACTIVE, "stacks active, not cleared")
    eq(c.director_available("East"), False, "stacks has no confrontation to clear it")
    c.provinces.setdefault("stacks", ProvinceProgress()).overseer_resolved = True
    eq(c.province_state("stacks"), CLEARED, "stacks cleared once overseer resolved")
    eq(c.director_available("East"), True, "East available")


@check("Meridian has no regional director")
def _():
    c = build()
    eq(c.director_available("Central"), False, "Central never offers a director")


@check("finale needs all four directors resolved")
def _():
    c = build()
    eq(c.finale_available(), False, "none resolved")
    for r in ["North", "South", "West"]:
        c.resolved_directors.add(r)
    eq(c.finale_available(), False, "three of four")
    c.resolved_directors.add("East")
    eq(c.finale_available(), True, "all four")


@check("unknown ids answer safely rather than throwing")
def _():
    c = build()
    eq(c.province_state("nope"), LOCKED, "unknown province")
    eq(c.mission_state("nope"), M_LOCKED, "unknown mission")
    eq(c.required_objectives_satisfied("nope"), 0, "unknown mission objectives")
    c.record_discovery("nope", "Document")            # must not raise
    c.complete_mission("nope")
    c.complete_objective("nope", "x")
    eq(c.provinces_cleared(), 0, "nothing cleared")


@check("stale save data for a removed province cannot inflate the overseer count")
def _():
    c = build()
    c.provinces["deleted_province"] = ProvinceProgress(overseer_resolved=True)
    eq(c.overseers_resolved(), 0, "unregistered province ignored")
    c.provinces.setdefault("hale", ProvinceProgress()).overseer_resolved = True
    eq(c.overseers_resolved(), 1, "only the real one counts")


def main() -> int:
    passed = failed = 0
    for name, fn in CHECKS:
        try:
            fn()
        except AssertionError as exc:
            print(f"  FAIL  {name}\n          {exc}")
            failed += 1
        else:
            print(f"  pass  {name}")
            passed += 1

    print(f"\n{passed} passed, {failed} failed, {len(CHECKS)} checks.")
    print("\nThis verifies the ALGORITHM in ShadowlineCampaignSubsystem.cpp, transcribed by")
    print("hand into Python. It does NOT prove the C++ compiles, nor that the transcription")
    print("is faithful. Tests/ShadowlineCampaignTests.cpp runs the same cases against the")
    print("real C++ once there is an engine to run them in.")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
