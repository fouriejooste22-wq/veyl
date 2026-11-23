// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.
//
// Automation tests for the campaign derivation logic.
//
// These mirror Tools/verify_campaign_logic.py, which checks the same rules as a hand
// transcription in Python because the project was authored without an engine. This file is
// the real check: it runs the actual C++.
//
// Run from the editor:  Window > Test Automation > Shadowline.Campaign
// Or headless:
//   UnrealEditor-Cmd.exe <path>\OperationShadowline.uproject ^
//     -ExecCmds="Automation RunTests Shadowline.Campaign; Quit" ^
//     -unattended -nopause -nullrhi -testexit="Automation Test Queue Empty"
//
// -nullrhi matters: these tests need no GPU, so they run anywhere the engine builds.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Core/ShadowlineCampaignSubsystem.h"
#include "Core/ShadowlineMissionDefinition.h"
#include "Core/ShadowlineTypes.h"
#include "World/ShadowlineProvinceDefinition.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace ShadowlineTestFixture
{
	/** Region names here match the real enum; the Python model uses short labels. */
	static constexpr EShadowlineRegion East = EShadowlineRegion::EasternCoast;
	static constexpr EShadowlineRegion North = EShadowlineRegion::NorthernMountains;
	static constexpr EShadowlineRegion West = EShadowlineRegion::WesternDesert;
	static constexpr EShadowlineRegion South = EShadowlineRegion::SouthernJungle;
	static constexpr EShadowlineRegion Central = EShadowlineRegion::CentralIndustrial;

	static UShadowlineProvinceDefinition* MakeProvince(
		FName Id, EShadowlineRegion Region, int32 Docs, int32 Landmarks, int32 Caches,
		const TArray<FName>& Prerequisites = {})
	{
		UShadowlineProvinceDefinition* P =
			NewObject<UShadowlineProvinceDefinition>(GetTransientPackage());
		P->ProvinceId = Id;
		P->Region = Region;
		P->DocumentCount = Docs;
		P->LandmarkCount = Landmarks;
		P->CacheCount = Caches;
		P->PrerequisiteProvinceIds = Prerequisites;
		return P;
	}

	static UShadowlineMissionDefinition* MakeMission(
		FName Id, FName ProvinceId, int32 Act, EShadowlineMissionType Type,
		const TArray<FName>& Prerequisites = {},
		const TArray<TPair<FName, bool>>& Objectives = {})
	{
		UShadowlineMissionDefinition* M =
			NewObject<UShadowlineMissionDefinition>(GetTransientPackage());
		M->MissionId = Id;
		M->ProvinceId = ProvinceId;
		M->Act = Act;
		M->MissionType = Type;
		M->PrerequisiteMissionIds = Prerequisites;
		for (const TPair<FName, bool>& Pair : Objectives)
		{
			FShadowlineObjective Objective;
			Objective.CompletionEvent = Pair.Key;
			Objective.bRequired = Pair.Value;
			M->Objectives.Add(Objective);
		}
		return M;
	}

	/**
	 * The same small slice the Python model uses:
	 * calloway(6 discoverables) -> stacks(3); hale(4); barren(0); meridian(5, needs both).
	 */
	static UShadowlineCampaignSubsystem* BuildCampaign()
	{
		UShadowlineCampaignSubsystem* Campaign =
			NewObject<UShadowlineCampaignSubsystem>(GetTransientPackage());

		TArray<UShadowlineProvinceDefinition*> Provinces;
		Provinces.Add(MakeProvince(TEXT("calloway"), East, 3, 2, 1));
		Provinces.Add(MakeProvince(TEXT("stacks"), East, 2, 1, 0, { TEXT("calloway") }));
		Provinces.Add(MakeProvince(TEXT("hale"), North, 2, 1, 1));
		Provinces.Add(MakeProvince(TEXT("barren"), West, 0, 0, 0));
		Provinces.Add(MakeProvince(TEXT("meridian"), Central, 4, 1, 0,
			{ TEXT("stacks"), TEXT("hale") }));

		TArray<UShadowlineMissionDefinition*> Missions;
		Missions.Add(MakeMission(TEXT("cal_recon"), TEXT("calloway"), 1,
			EShadowlineMissionType::Reconnaissance, {},
			{ { TEXT("reach_office"), true }, { TEXT("photograph"), true }, { TEXT("rota"), false } }));
		Missions.Add(MakeMission(TEXT("cal_side"), TEXT("calloway"), 0,
			EShadowlineMissionType::Puzzle, {}, { { TEXT("power"), true } }));
		Missions.Add(MakeMission(TEXT("cal_con"), TEXT("calloway"), 1,
			EShadowlineMissionType::Confrontation, { TEXT("cal_recon") },
			{ { TEXT("locate"), true }, { TEXT("record"), true } }));
		Missions.Add(MakeMission(TEXT("stk_recon"), TEXT("stacks"), 0,
			EShadowlineMissionType::Reconnaissance, {}, { { TEXT("lighthouse"), true } }));
		Missions.Add(MakeMission(TEXT("mer_finale"), TEXT("meridian"), 5,
			EShadowlineMissionType::Finale, {},
			{ { TEXT("control_floor"), true }, { TEXT("decide"), true } }));

		Campaign->RegisterDefinitions(Provinces, Missions);
		return Campaign;
	}

	/** Clears calloway the way play would: both story missions, confrontation last. */
	static void ClearCalloway(UShadowlineCampaignSubsystem* Campaign)
	{
		Campaign->CompleteMission(TEXT("cal_recon"));
		Campaign->CompleteMission(TEXT("cal_con"));
	}
}

using namespace ShadowlineTestFixture;

// ---------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShadowlineProvinceGatingTest,
	"Shadowline.Campaign.ProvinceGating",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShadowlineProvinceGatingTest::RunTest(const FString& Parameters)
{
	UShadowlineCampaignSubsystem* C = BuildCampaign();

	TestEqual(TEXT("calloway opens with no prerequisites"),
		C->GetProvinceState(TEXT("calloway")), EShadowlineProvinceState::Open);
	TestEqual(TEXT("hale opens with no prerequisites"),
		C->GetProvinceState(TEXT("hale")), EShadowlineProvinceState::Open);
	TestEqual(TEXT("stacks is locked behind calloway"),
		C->GetProvinceState(TEXT("stacks")), EShadowlineProvinceState::Locked);
	TestEqual(TEXT("meridian is locked behind stacks and hale"),
		C->GetProvinceState(TEXT("meridian")), EShadowlineProvinceState::Locked);

	C->RecordDiscovery(TEXT("calloway"), EShadowlineDiscovery::Document);
	TestEqual(TEXT("one discovery makes a province Active"),
		C->GetProvinceState(TEXT("calloway")), EShadowlineProvinceState::Active);

	ClearCalloway(C);
	TestTrue(TEXT("a Confrontation resolves its province's overseer"),
		C->GetProgress().Provinces[TEXT("calloway")].bOverseerResolved);
	TestEqual(TEXT("calloway clears once its story missions are done"),
		C->GetProvinceState(TEXT("calloway")), EShadowlineProvinceState::Cleared);
	TestEqual(TEXT("clearing a prerequisite opens its dependent"),
		C->GetProvinceState(TEXT("stacks")), EShadowlineProvinceState::Open);
	TestEqual(TEXT("meridian still needs hale"),
		C->GetProvinceState(TEXT("meridian")), EShadowlineProvinceState::Locked);

	// Side operations must never gate clearing.
	TestEqual(TEXT("a side operation stays available in a cleared province"),
		C->GetMissionState(TEXT("cal_side")), EShadowlineMissionState::Available);
	TestEqual(TEXT("an incomplete side operation does not un-clear a province"),
		C->GetProvinceState(TEXT("calloway")), EShadowlineProvinceState::Cleared);

	// An overseer alone is not enough.
	UShadowlineCampaignSubsystem* C2 = BuildCampaign();
	C2->ResolveOverseer(TEXT("calloway"));
	TestEqual(TEXT("overseer resolved without the story missions is Active"),
		C2->GetProvinceState(TEXT("calloway")), EShadowlineProvinceState::Active);

	// Progress must never be retroactively locked away.
	UShadowlineCampaignSubsystem* C3 = BuildCampaign();
	C3->RecordDiscovery(TEXT("stacks"), EShadowlineDiscovery::Document);
	TestEqual(TEXT("a province with progress is not locked by an unmet prerequisite"),
		C3->GetProvinceState(TEXT("stacks")), EShadowlineProvinceState::Active);

	return true;
}

// ---------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShadowlineMissionGatingTest,
	"Shadowline.Campaign.MissionGating",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShadowlineMissionGatingTest::RunTest(const FString& Parameters)
{
	UShadowlineCampaignSubsystem* C = BuildCampaign();

	TestEqual(TEXT("a mission in a locked province is locked"),
		C->GetMissionState(TEXT("stk_recon")), EShadowlineMissionState::Locked);

	C->CompleteObjective(TEXT("stk_recon"), TEXT("lighthouse"));
	TestEqual(TEXT("an objective already done outranks the province lock"),
		C->GetMissionState(TEXT("stk_recon")), EShadowlineMissionState::Active);

	TestEqual(TEXT("the confrontation is locked behind its recon"),
		C->GetMissionState(TEXT("cal_con")), EShadowlineMissionState::Locked);
	C->CompleteMission(TEXT("cal_recon"));
	TestEqual(TEXT("the confrontation opens once its recon is done"),
		C->GetMissionState(TEXT("cal_con")), EShadowlineMissionState::Available);
	TestEqual(TEXT("a completed mission reads Complete"),
		C->GetMissionState(TEXT("cal_recon")), EShadowlineMissionState::Complete);

	// Objective counting: optional objectives must not count, and must be idempotent.
	UShadowlineCampaignSubsystem* C2 = BuildCampaign();
	C2->CompleteObjective(TEXT("cal_recon"), TEXT("rota"));
	TestEqual(TEXT("an optional objective does not count toward required"),
		C2->GetRequiredObjectivesSatisfied(TEXT("cal_recon")), 0);
	C2->CompleteObjective(TEXT("cal_recon"), TEXT("reach_office"));
	TestEqual(TEXT("a required objective counts"),
		C2->GetRequiredObjectivesSatisfied(TEXT("cal_recon")), 1);
	C2->CompleteObjective(TEXT("cal_recon"), TEXT("reach_office"));
	TestEqual(TEXT("raising the same event twice does not double-count"),
		C2->GetRequiredObjectivesSatisfied(TEXT("cal_recon")), 1);

	return true;
}

// ---------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShadowlineSurveyTest,
	"Shadowline.Campaign.Survey",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShadowlineSurveyTest::RunTest(const FString& Parameters)
{
	UShadowlineCampaignSubsystem* C = BuildCampaign();

	TestEqual(TEXT("a fresh province is 0% surveyed"),
		C->GetProvinceSurveyFraction(TEXT("calloway")), 0.f);

	C->RecordDiscovery(TEXT("calloway"), EShadowlineDiscovery::Document);
	C->RecordDiscovery(TEXT("calloway"), EShadowlineDiscovery::Document);
	TestNearlyEqual(TEXT("calloway reads 2 of 6"),
		C->GetProvinceSurveyFraction(TEXT("calloway")), 2.f / 6.f, 1e-5f);

	// Over-firing every discovery must clamp rather than exceed 100%.
	for (int32 i = 0; i < 10; ++i)
	{
		C->RecordDiscovery(TEXT("calloway"), EShadowlineDiscovery::Document);
		C->RecordDiscovery(TEXT("calloway"), EShadowlineDiscovery::Landmark);
		C->RecordDiscovery(TEXT("calloway"), EShadowlineDiscovery::Cache);
	}
	TestNearlyEqual(TEXT("survey clamps at 1.0"),
		C->GetProvinceSurveyFraction(TEXT("calloway")), 1.f, 1e-5f);
	TestEqual(TEXT("the stored document count clamps to the definition"),
		C->GetProgress().Provinces[TEXT("calloway")].DocumentsFound, 3);

	TestEqual(TEXT("a province with nothing to find is fully surveyed"),
		C->GetProvinceSurveyFraction(TEXT("barren")), 1.f);
	TestEqual(TEXT("an unknown province is 0%"),
		C->GetProvinceSurveyFraction(TEXT("nonexistent")), 0.f);

	// Island survey must weight by content, not average percentages.
	// 18 discoverables in total; filling stacks (3) is 3/18, not a quarter of the island.
	UShadowlineCampaignSubsystem* C2 = BuildCampaign();
	C2->RecordDiscovery(TEXT("stacks"), EShadowlineDiscovery::Document);
	C2->RecordDiscovery(TEXT("stacks"), EShadowlineDiscovery::Document);
	C2->RecordDiscovery(TEXT("stacks"), EShadowlineDiscovery::Landmark);
	TestNearlyEqual(TEXT("stacks is complete"),
		C2->GetProvinceSurveyFraction(TEXT("stacks")), 1.f, 1e-5f);
	TestNearlyEqual(TEXT("island survey is weighted by content"),
		C2->GetIslandSurveyFraction(), 3.f / 18.f, 1e-5f);
	TestTrue(TEXT("one small province does not count as a quarter of the island"),
		C2->GetIslandSurveyFraction() < 0.25f);

	return true;
}

// ---------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShadowlineLadderTest,
	"Shadowline.Campaign.ConfrontationLadder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShadowlineLadderTest::RunTest(const FString& Parameters)
{
	UShadowlineCampaignSubsystem* C = BuildCampaign();

	TestFalse(TEXT("the East director is unavailable at the start"),
		C->IsRegionalDirectorAvailable(East));
	ClearCalloway(C);
	TestFalse(TEXT("the East director needs stacks cleared too"),
		C->IsRegionalDirectorAvailable(East));

	// A side operation must not be enough to clear a province.
	C->CompleteMission(TEXT("stk_recon"));
	TestEqual(TEXT("a side operation leaves the province Active"),
		C->GetProvinceState(TEXT("stacks")), EShadowlineProvinceState::Active);
	TestFalse(TEXT("the director stays locked while a province is Active"),
		C->IsRegionalDirectorAvailable(East));

	C->ResolveOverseer(TEXT("stacks"));
	TestEqual(TEXT("stacks clears once its overseer is resolved"),
		C->GetProvinceState(TEXT("stacks")), EShadowlineProvinceState::Cleared);
	TestTrue(TEXT("the East director opens once the region is cleared"),
		C->IsRegionalDirectorAvailable(East));

	TestFalse(TEXT("Meridian never offers a regional director"),
		C->IsRegionalDirectorAvailable(Central));

	// The finale needs all four directors resolved.
	TestFalse(TEXT("the finale is closed at the start"), C->IsFinaleAvailable());
	C->ResolveRegionalDirector(North);
	C->ResolveRegionalDirector(South);
	C->ResolveRegionalDirector(West);
	TestFalse(TEXT("three of four directors is not enough"), C->IsFinaleAvailable());
	C->ResolveRegionalDirector(East);
	TestTrue(TEXT("all four directors opens the finale"), C->IsFinaleAvailable());

	return true;
}

// ---------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShadowlineDetectionAndDifficultyTest,
	"Shadowline.Campaign.DetectionAndDifficulty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShadowlineDetectionAndDifficultyTest::RunTest(const FString& Parameters)
{
	// A clean run must be recorded as clean, and a detected one must not be.
	UShadowlineCampaignSubsystem* C = BuildCampaign();
	C->CompleteMission(TEXT("cal_recon"));
	TestTrue(TEXT("an undetected run is recorded as undetected"),
		C->GetProgress().Missions[TEXT("cal_recon")].bCompletedUndetected);

	UShadowlineCampaignSubsystem* C2 = BuildCampaign();
	C2->RecordDetection(TEXT("cal_recon"));
	C2->CompleteMission(TEXT("cal_recon"));
	TestFalse(TEXT("a detected run is not recorded as undetected"),
		C2->GetProgress().Missions[TEXT("cal_recon")].bCompletedUndetected);

	// Difficulty presets must round-trip, and Custom must seed from what was in force.
	UShadowlineCampaignSubsystem* C3 = BuildCampaign();
	TestEqual(TEXT("Standard is the default difficulty"),
		C3->GetDifficulty(), EShadowlineDifficulty::Standard);
	TestEqual(TEXT("Standard suspicion rate is the 1.0 baseline"),
		C3->GetDifficultyKnobs().SuspicionRate, 1.f);

	C3->SetDifficulty(EShadowlineDifficulty::Blackout);
	const FShadowlineDifficultyKnobs Blackout = C3->GetDifficultyKnobs();
	TestFalse(TEXT("Blackout removes the objective tracker"), Blackout.bObjectiveTracker);
	TestFalse(TEXT("Blackout removes the compass"), Blackout.bCompass);
	TestTrue(TEXT("Blackout raises suspicion rate above Standard"), Blackout.SuspicionRate > 1.f);

	C3->SetDifficulty(EShadowlineDifficulty::Custom);
	TestEqual(TEXT("Custom seeds from the difficulty that was in force"),
		C3->GetDifficultyKnobs().SuspicionRate, Blackout.SuspicionRate);

	// Veteran's defining change is losing the target marker, not tougher guards.
	UShadowlineCampaignSubsystem* C4 = BuildCampaign();
	C4->SetDifficulty(EShadowlineDifficulty::Veteran);
	TestFalse(TEXT("Veteran removes the target marker"),
		C4->GetDifficultyKnobs().bTargetMarkers);
	TestTrue(TEXT("Veteran keeps tagging"), C4->GetDifficultyKnobs().bTaggingEnabled);

	return true;
}

// ---------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShadowlineRobustnessTest,
	"Shadowline.Campaign.Robustness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShadowlineRobustnessTest::RunTest(const FString& Parameters)
{
	UShadowlineCampaignSubsystem* C = BuildCampaign();

	// Unknown ids must answer safely. These log warnings, which is expected.
	AddExpectedError(TEXT("unknown"), EAutomationExpectedErrorFlags::Contains, 0);
	TestEqual(TEXT("an unknown province reads Locked"),
		C->GetProvinceState(TEXT("nope")), EShadowlineProvinceState::Locked);
	TestEqual(TEXT("an unknown mission reads Locked"),
		C->GetMissionState(TEXT("nope")), EShadowlineMissionState::Locked);
	TestEqual(TEXT("an unknown mission has no satisfied objectives"),
		C->GetRequiredObjectivesSatisfied(TEXT("nope")), 0);

	C->RecordDiscovery(TEXT("nope"), EShadowlineDiscovery::Document);
	C->CompleteMission(TEXT("nope"));
	C->CompleteObjective(TEXT("nope"), TEXT("x"));
	C->ResolveOverseer(TEXT("nope"));
	TestEqual(TEXT("nothing was cleared by operations on unknown ids"),
		C->GetProvincesCleared(), 0);

	// A subsystem with no definitions at all must not assert: opening a level directly in
	// the editor skips RegisterDefinitions entirely.
	UShadowlineCampaignSubsystem* Bare =
		NewObject<UShadowlineCampaignSubsystem>(GetTransientPackage());
	TestEqual(TEXT("an unregistered campaign reports no cleared provinces"),
		Bare->GetProvincesCleared(), 0);
	TestEqual(TEXT("an unregistered campaign reports 0% surveyed"),
		Bare->GetIslandSurveyFraction(), 0.f);
	TestFalse(TEXT("an unregistered campaign does not open the finale"),
		Bare->IsFinaleAvailable());

	// Stale save data for a removed province must not inflate the overseer count.
	UShadowlineCampaignSubsystem* C2 = BuildCampaign();
	FShadowlineCampaignProgress Stale;
	FShadowlineProvinceProgress Removed;
	Removed.bOverseerResolved = true;
	Stale.Provinces.Add(TEXT("deleted_province"), Removed);
	C2->SetProgress(Stale);
	TestEqual(TEXT("progress for an unregistered province is ignored"),
		C2->GetOverseersResolved(), 0);

	C2->ResolveOverseer(TEXT("hale"));
	TestEqual(TEXT("only registered provinces count"), C2->GetOverseersResolved(), 1);

	// StartNewOperation must wipe everything.
	C2->StartNewOperation(EShadowlineDifficulty::Veteran);
	TestEqual(TEXT("a new operation clears overseers"), C2->GetOverseersResolved(), 0);
	TestEqual(TEXT("a new operation keeps the chosen difficulty"),
		C2->GetDifficulty(), EShadowlineDifficulty::Veteran);
	TestEqual(TEXT("a new operation starts at act 1"), C2->GetProgress().Act, 1);

	return true;
}

#endif	// WITH_DEV_AUTOMATION_TESTS
