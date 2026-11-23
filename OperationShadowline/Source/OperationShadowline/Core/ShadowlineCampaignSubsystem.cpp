// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Core/ShadowlineCampaignSubsystem.h"

#include "OperationShadowline.h"
#include "Core/ShadowlineMissionDefinition.h"
#include "World/ShadowlineProvinceDefinition.h"

void UShadowlineCampaignSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Progress = FShadowlineCampaignProgress();
}

void UShadowlineCampaignSubsystem::Deinitialize()
{
	ProvinceDefs.Empty();
	MissionDefs.Empty();
	Super::Deinitialize();
}

// ---------------------------------------------------------------------------------------
// Registry
// ---------------------------------------------------------------------------------------

void UShadowlineCampaignSubsystem::RegisterDefinitions(
	const TArray<UShadowlineProvinceDefinition*>& InProvinces,
	const TArray<UShadowlineMissionDefinition*>& InMissions)
{
	ProvinceDefs.Empty(InProvinces.Num());
	MissionDefs.Empty(InMissions.Num());

	for (UShadowlineProvinceDefinition* Province : InProvinces)
	{
		if (!Province)
		{
			continue;
		}
		if (Province->ProvinceId.IsNone())
		{
			UE_LOG(LogShadowline, Warning,
				TEXT("Province definition %s has no ProvinceId and was skipped."), *Province->GetName());
			continue;
		}
		if (ProvinceDefs.Contains(Province->ProvinceId))
		{
			UE_LOG(LogShadowline, Error,
				TEXT("Duplicate ProvinceId '%s' (%s). Ids must be unique; the later asset was skipped."),
				*Province->ProvinceId.ToString(), *Province->GetName());
			continue;
		}
		ProvinceDefs.Add(Province->ProvinceId, Province);
	}

	for (UShadowlineMissionDefinition* Mission : InMissions)
	{
		if (!Mission)
		{
			continue;
		}
		if (Mission->MissionId.IsNone())
		{
			UE_LOG(LogShadowline, Warning,
				TEXT("Mission definition %s has no MissionId and was skipped."), *Mission->GetName());
			continue;
		}
		if (MissionDefs.Contains(Mission->MissionId))
		{
			UE_LOG(LogShadowline, Error,
				TEXT("Duplicate MissionId '%s' (%s). Ids must be unique; the later asset was skipped."),
				*Mission->MissionId.ToString(), *Mission->GetName());
			continue;
		}
		// A mission sited in a province that does not exist would be unreachable, so it is
		// worth saying so loudly at startup rather than leaving it silently absent.
		if (!Mission->ProvinceId.IsNone() && !ProvinceDefs.Contains(Mission->ProvinceId))
		{
			UE_LOG(LogShadowline, Warning,
				TEXT("Mission '%s' names province '%s', which is not registered."),
				*Mission->MissionId.ToString(), *Mission->ProvinceId.ToString());
		}
		MissionDefs.Add(Mission->MissionId, Mission);
	}

	UE_LOG(LogShadowline, Log, TEXT("Campaign registered: %d provinces, %d missions."),
		ProvinceDefs.Num(), MissionDefs.Num());

	OnCampaignReset.Broadcast();
}

const UShadowlineProvinceDefinition* UShadowlineCampaignSubsystem::FindProvince(FName ProvinceId) const
{
	const TObjectPtr<const UShadowlineProvinceDefinition>* Found = ProvinceDefs.Find(ProvinceId);
	return Found ? Found->Get() : nullptr;
}

const UShadowlineMissionDefinition* UShadowlineCampaignSubsystem::FindMission(FName MissionId) const
{
	const TObjectPtr<const UShadowlineMissionDefinition>* Found = MissionDefs.Find(MissionId);
	return Found ? Found->Get() : nullptr;
}

TArray<FName> UShadowlineCampaignSubsystem::GetProvinceIdsInRegion(EShadowlineRegion Region) const
{
	TArray<FName> Result;
	for (const TPair<FName, TObjectPtr<const UShadowlineProvinceDefinition>>& Pair : ProvinceDefs)
	{
		if (Pair.Value && Pair.Value->Region == Region)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

TArray<FName> UShadowlineCampaignSubsystem::GetMissionIdsInProvince(FName ProvinceId) const
{
	TArray<FName> Result;
	for (const TPair<FName, TObjectPtr<const UShadowlineMissionDefinition>>& Pair : MissionDefs)
	{
		if (Pair.Value && Pair.Value->ProvinceId == ProvinceId)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

// ---------------------------------------------------------------------------------------
// Stored progress access
// ---------------------------------------------------------------------------------------

const FShadowlineProvinceProgress& UShadowlineCampaignSubsystem::GetProvinceProgress(FName ProvinceId) const
{
	if (const FShadowlineProvinceProgress* Found = Progress.Provinces.Find(ProvinceId))
	{
		return *Found;
	}
	return EmptyProvinceProgress;
}

const FShadowlineMissionProgress& UShadowlineCampaignSubsystem::GetMissionProgress(FName MissionId) const
{
	if (const FShadowlineMissionProgress* Found = Progress.Missions.Find(MissionId))
	{
		return *Found;
	}
	return EmptyMissionProgress;
}

// ---------------------------------------------------------------------------------------
// Derived state
// ---------------------------------------------------------------------------------------

EShadowlineProvinceState UShadowlineCampaignSubsystem::GetProvinceStateIgnoringPrerequisites(FName ProvinceId) const
{
	const UShadowlineProvinceDefinition* Def = FindProvince(ProvinceId);
	if (!Def)
	{
		return EShadowlineProvinceState::Locked;
	}

	const FShadowlineProvinceProgress& P = GetProvinceProgress(ProvinceId);

	// Cleared needs the overseer resolved AND every required mission in the province done.
	// "Required" means a story mission or a confrontation; side operations never gate it,
	// so a player is not forced to hoover up every cache to move the campaign on.
	bool bAllRequiredMissionsDone = true;
	for (const TPair<FName, TObjectPtr<const UShadowlineMissionDefinition>>& Pair : MissionDefs)
	{
		const UShadowlineMissionDefinition* Mission = Pair.Value.Get();
		if (!Mission || Mission->ProvinceId != ProvinceId)
		{
			continue;
		}
		const bool bGating = (Mission->Act > 0) || Mission->IsConfrontation();
		if (bGating && !GetMissionProgress(Pair.Key).bComplete)
		{
			bAllRequiredMissionsDone = false;
			break;
		}
	}

	if (P.bOverseerResolved && bAllRequiredMissionsDone)
	{
		return EShadowlineProvinceState::Cleared;
	}

	// Any trace of the player's presence makes it Active.
	const bool bTouched =
		P.DocumentsFound > 0 || P.LandmarksFound > 0 || P.CachesFound > 0 || P.bOverseerResolved;
	if (bTouched)
	{
		return EShadowlineProvinceState::Active;
	}
	for (const FName& MissionId : GetMissionIdsInProvince(ProvinceId))
	{
		const FShadowlineMissionProgress& M = GetMissionProgress(MissionId);
		if (M.bComplete || M.CompletedObjectives.Num() > 0)
		{
			return EShadowlineProvinceState::Active;
		}
	}

	return EShadowlineProvinceState::Open;
}

bool UShadowlineCampaignSubsystem::ArePrerequisitesMet(const UShadowlineProvinceDefinition& Province) const
{
	for (const FName& RequiredId : Province.PrerequisiteProvinceIds)
	{
		// Checked without the prerequisite test, so a cycle in the data cannot hang the game.
		// Shallow chains are a design rule, not something enforced here.
		if (GetProvinceStateIgnoringPrerequisites(RequiredId) != EShadowlineProvinceState::Cleared)
		{
			return false;
		}
	}
	return true;
}

EShadowlineProvinceState UShadowlineCampaignSubsystem::GetProvinceState(FName ProvinceId) const
{
	const UShadowlineProvinceDefinition* Def = FindProvince(ProvinceId);
	if (!Def)
	{
		return EShadowlineProvinceState::Locked;
	}

	const EShadowlineProvinceState Intrinsic = GetProvinceStateIgnoringPrerequisites(ProvinceId);

	// Prerequisites gate opening, not progress already made. A province the player has
	// somehow worked in is never retroactively locked.
	if (Intrinsic == EShadowlineProvinceState::Open && !ArePrerequisitesMet(*Def))
	{
		return EShadowlineProvinceState::Locked;
	}

	return Intrinsic;
}

EShadowlineMissionState UShadowlineCampaignSubsystem::GetMissionState(FName MissionId) const
{
	const UShadowlineMissionDefinition* Def = FindMission(MissionId);
	if (!Def)
	{
		return EShadowlineMissionState::Locked;
	}

	const FShadowlineMissionProgress& M = GetMissionProgress(MissionId);
	if (M.bComplete)
	{
		return EShadowlineMissionState::Complete;
	}

	// Work already done outranks a lock, for the same reason as provinces.
	if (M.CompletedObjectives.Num() > 0)
	{
		return EShadowlineMissionState::Active;
	}

	if (!Def->ProvinceId.IsNone() && GetProvinceState(Def->ProvinceId) == EShadowlineProvinceState::Locked)
	{
		return EShadowlineMissionState::Locked;
	}
	for (const FName& RequiredId : Def->PrerequisiteMissionIds)
	{
		if (!GetMissionProgress(RequiredId).bComplete)
		{
			return EShadowlineMissionState::Locked;
		}
	}

	return EShadowlineMissionState::Available;
}

int32 UShadowlineCampaignSubsystem::GetRequiredObjectivesSatisfied(FName MissionId) const
{
	const UShadowlineMissionDefinition* Def = FindMission(MissionId);
	if (!Def)
	{
		return 0;
	}

	const FShadowlineMissionProgress& M = GetMissionProgress(MissionId);
	int32 Count = 0;
	for (const FShadowlineObjective& Objective : Def->Objectives)
	{
		if (Objective.bRequired && M.CompletedObjectives.Contains(Objective.CompletionEvent))
		{
			++Count;
		}
	}
	return Count;
}

float UShadowlineCampaignSubsystem::GetProvinceSurveyFraction(FName ProvinceId) const
{
	const UShadowlineProvinceDefinition* Def = FindProvince(ProvinceId);
	if (!Def)
	{
		return 0.f;
	}

	const int32 Total = Def->GetTotalDiscoverables();
	if (Total <= 0)
	{
		// A province with nothing to find is fully surveyed by definition, which keeps it
		// from dragging the island average down forever.
		return 1.f;
	}

	const FShadowlineProvinceProgress& P = GetProvinceProgress(ProvinceId);
	const int32 Found =
		FMath::Min(P.DocumentsFound, Def->DocumentCount)
		+ FMath::Min(P.LandmarksFound, Def->LandmarkCount)
		+ FMath::Min(P.CachesFound, Def->CacheCount);

	return FMath::Clamp(static_cast<float>(Found) / static_cast<float>(Total), 0.f, 1.f);
}

float UShadowlineCampaignSubsystem::GetIslandSurveyFraction() const
{
	// Weighted by content, not an average of percentages: a province holding forty
	// discoverables should count for more than one holding four.
	int32 TotalFound = 0;
	int32 TotalAvailable = 0;

	for (const TPair<FName, TObjectPtr<const UShadowlineProvinceDefinition>>& Pair : ProvinceDefs)
	{
		const UShadowlineProvinceDefinition* Def = Pair.Value.Get();
		if (!Def)
		{
			continue;
		}
		const int32 Total = Def->GetTotalDiscoverables();
		if (Total <= 0)
		{
			continue;
		}

		const FShadowlineProvinceProgress& P = GetProvinceProgress(Pair.Key);
		TotalAvailable += Total;
		TotalFound +=
			FMath::Min(P.DocumentsFound, Def->DocumentCount)
			+ FMath::Min(P.LandmarksFound, Def->LandmarkCount)
			+ FMath::Min(P.CachesFound, Def->CacheCount);
	}

	if (TotalAvailable <= 0)
	{
		return 0.f;
	}
	return FMath::Clamp(static_cast<float>(TotalFound) / static_cast<float>(TotalAvailable), 0.f, 1.f);
}

int32 UShadowlineCampaignSubsystem::GetProvincesCleared() const
{
	int32 Count = 0;
	for (const TPair<FName, TObjectPtr<const UShadowlineProvinceDefinition>>& Pair : ProvinceDefs)
	{
		if (GetProvinceState(Pair.Key) == EShadowlineProvinceState::Cleared)
		{
			++Count;
		}
	}
	return Count;
}

int32 UShadowlineCampaignSubsystem::GetOverseersResolved() const
{
	int32 Count = 0;
	for (const TPair<FName, FShadowlineProvinceProgress>& Pair : Progress.Provinces)
	{
		// Only count provinces that actually exist, so stale save data cannot inflate it.
		if (Pair.Value.bOverseerResolved && ProvinceDefs.Contains(Pair.Key))
		{
			++Count;
		}
	}
	return Count;
}

int32 UShadowlineCampaignSubsystem::GetMissionsCompleted() const
{
	int32 Count = 0;
	for (const TPair<FName, FShadowlineMissionProgress>& Pair : Progress.Missions)
	{
		if (Pair.Value.bComplete && MissionDefs.Contains(Pair.Key))
		{
			++Count;
		}
	}
	return Count;
}

bool UShadowlineCampaignSubsystem::IsRegionalDirectorAvailable(EShadowlineRegion Region) const
{
	// Meridian Works has no regional director; its antagonist is the finale.
	if (Region == EShadowlineRegion::CentralIndustrial || Region == EShadowlineRegion::None)
	{
		return false;
	}

	const TArray<FName> Ids = GetProvinceIdsInRegion(Region);
	if (Ids.Num() == 0)
	{
		return false;
	}

	for (const FName& Id : Ids)
	{
		if (GetProvinceState(Id) != EShadowlineProvinceState::Cleared)
		{
			return false;
		}
	}
	return true;
}

bool UShadowlineCampaignSubsystem::IsRegionalDirectorResolved(EShadowlineRegion Region) const
{
	return Progress.ResolvedDirectors.Contains(Region);
}

bool UShadowlineCampaignSubsystem::IsFinaleAvailable() const
{
	static const EShadowlineRegion DirectorRegions[] = {
		EShadowlineRegion::NorthernMountains,
		EShadowlineRegion::SouthernJungle,
		EShadowlineRegion::WesternDesert,
		EShadowlineRegion::EasternCoast
	};

	for (const EShadowlineRegion Region : DirectorRegions)
	{
		if (!IsRegionalDirectorResolved(Region))
		{
			return false;
		}
	}
	return true;
}

// ---------------------------------------------------------------------------------------
// Difficulty
// ---------------------------------------------------------------------------------------

FShadowlineDifficultyKnobs UShadowlineCampaignSubsystem::GetDifficultyKnobs() const
{
	if (Progress.Difficulty == EShadowlineDifficulty::Custom)
	{
		return Progress.CustomKnobs;
	}
	return FShadowlineDifficultyKnobs::FromPreset(Progress.Difficulty);
}

void UShadowlineCampaignSubsystem::SetDifficulty(EShadowlineDifficulty NewDifficulty)
{
	if (Progress.Difficulty == NewDifficulty)
	{
		return;
	}

	// Moving to Custom without knobs seeds them from whatever was in force, so the player
	// starts customising from where they were rather than from Standard.
	if (NewDifficulty == EShadowlineDifficulty::Custom)
	{
		Progress.CustomKnobs = GetDifficultyKnobs();
	}

	Progress.Difficulty = NewDifficulty;
	OnDifficultyChanged.Broadcast();
}

void UShadowlineCampaignSubsystem::SetCustomDifficulty(const FShadowlineDifficultyKnobs& NewKnobs)
{
	Progress.CustomKnobs = NewKnobs;
	Progress.Difficulty = EShadowlineDifficulty::Custom;
	OnDifficultyChanged.Broadcast();
}

// ---------------------------------------------------------------------------------------
// Mutations
// ---------------------------------------------------------------------------------------

void UShadowlineCampaignSubsystem::BroadcastProvinceAndDependents(FName ProvinceId)
{
	OnProvinceChanged.Broadcast(ProvinceId);

	// Clearing a province can unlock others, so anything naming it must be refreshed too.
	for (const TPair<FName, TObjectPtr<const UShadowlineProvinceDefinition>>& Pair : ProvinceDefs)
	{
		const UShadowlineProvinceDefinition* Def = Pair.Value.Get();
		if (Def && Pair.Key != ProvinceId && Def->PrerequisiteProvinceIds.Contains(ProvinceId))
		{
			OnProvinceChanged.Broadcast(Pair.Key);
		}
	}
}

void UShadowlineCampaignSubsystem::CompleteObjective(FName MissionId, FName ObjectiveEvent)
{
	if (ObjectiveEvent.IsNone())
	{
		UE_LOG(LogShadowline, Warning,
			TEXT("CompleteObjective called on mission '%s' with no event name."), *MissionId.ToString());
		return;
	}
	if (!MissionDefs.Contains(MissionId))
	{
		UE_LOG(LogShadowline, Warning,
			TEXT("CompleteObjective for unknown mission '%s'."), *MissionId.ToString());
		return;
	}

	FShadowlineMissionProgress& M = Progress.Missions.FindOrAdd(MissionId);
	if (M.CompletedObjectives.Contains(ObjectiveEvent))
	{
		return;	// idempotent: a mission script may raise the same event twice
	}

	M.CompletedObjectives.Add(ObjectiveEvent);
	OnMissionChanged.Broadcast(MissionId);

	if (const UShadowlineMissionDefinition* Def = FindMission(MissionId))
	{
		if (!Def->ProvinceId.IsNone())
		{
			BroadcastProvinceAndDependents(Def->ProvinceId);
		}
	}
}

void UShadowlineCampaignSubsystem::RecordDetection(FName MissionId)
{
	if (!MissionDefs.Contains(MissionId))
	{
		return;
	}

	FShadowlineMissionProgress& M = Progress.Missions.FindOrAdd(MissionId);
	if (M.bDetectedThisAttempt)
	{
		return;
	}

	M.bDetectedThisAttempt = true;
	OnMissionChanged.Broadcast(MissionId);
}

void UShadowlineCampaignSubsystem::CompleteMission(FName MissionId)
{
	const UShadowlineMissionDefinition* Def = FindMission(MissionId);
	if (!Def)
	{
		UE_LOG(LogShadowline, Warning,
			TEXT("CompleteMission for unknown mission '%s'."), *MissionId.ToString());
		return;
	}

	FShadowlineMissionProgress& M = Progress.Missions.FindOrAdd(MissionId);
	if (M.bComplete)
	{
		return;
	}

	M.bComplete = true;
	// Read from the stored flag rather than from an argument, so no caller can assert a
	// clean run the stealth systems did not actually observe.
	M.bCompletedUndetected = !M.bDetectedThisAttempt;

	// Confrontations resolve their province's overseer as part of completing.
	if (Def->MissionType == EShadowlineMissionType::Confrontation && !Def->ProvinceId.IsNone())
	{
		Progress.Provinces.FindOrAdd(Def->ProvinceId).bOverseerResolved = true;
	}

	// A story mission moves the act forward, never backward.
	if (Def->Act > 0)
	{
		Progress.Act = FMath::Max(Progress.Act, Def->Act);
	}

	OnMissionChanged.Broadcast(MissionId);
	if (!Def->ProvinceId.IsNone())
	{
		BroadcastProvinceAndDependents(Def->ProvinceId);
	}
}

void UShadowlineCampaignSubsystem::RecordDiscovery(FName ProvinceId, EShadowlineDiscovery Kind)
{
	const UShadowlineProvinceDefinition* Def = FindProvince(ProvinceId);
	if (!Def)
	{
		UE_LOG(LogShadowline, Warning,
			TEXT("RecordDiscovery for unknown province '%s'."), *ProvinceId.ToString());
		return;
	}

	FShadowlineProvinceProgress& P = Progress.Provinces.FindOrAdd(ProvinceId);

	// Clamped to what the province actually contains, so a double-fire cannot push the
	// survey percentage past 100.
	switch (Kind)
	{
	case EShadowlineDiscovery::Document:
		if (P.DocumentsFound >= Def->DocumentCount) { return; }
		++P.DocumentsFound;
		break;
	case EShadowlineDiscovery::Landmark:
		if (P.LandmarksFound >= Def->LandmarkCount) { return; }
		++P.LandmarksFound;
		break;
	case EShadowlineDiscovery::Cache:
		if (P.CachesFound >= Def->CacheCount) { return; }
		++P.CachesFound;
		break;
	default:
		return;
	}

	BroadcastProvinceAndDependents(ProvinceId);
	OnSurveyChanged.Broadcast();
}

void UShadowlineCampaignSubsystem::ResolveOverseer(FName ProvinceId)
{
	if (!ProvinceDefs.Contains(ProvinceId))
	{
		UE_LOG(LogShadowline, Warning,
			TEXT("ResolveOverseer for unknown province '%s'."), *ProvinceId.ToString());
		return;
	}

	FShadowlineProvinceProgress& P = Progress.Provinces.FindOrAdd(ProvinceId);
	if (P.bOverseerResolved)
	{
		return;
	}

	P.bOverseerResolved = true;
	BroadcastProvinceAndDependents(ProvinceId);
}

void UShadowlineCampaignSubsystem::ResolveRegionalDirector(EShadowlineRegion Region)
{
	if (Region == EShadowlineRegion::None || Region == EShadowlineRegion::CentralIndustrial)
	{
		UE_LOG(LogShadowline, Warning,
			TEXT("ResolveRegionalDirector called for a region with no director (%d)."),
			static_cast<int32>(Region));
		return;
	}
	if (Progress.ResolvedDirectors.Contains(Region))
	{
		return;
	}

	// Deliberately not gated on IsRegionalDirectorAvailable: the mission that resolves a
	// director is itself gated, and refusing here would make the call order load-bearing.
	Progress.ResolvedDirectors.Add(Region);
	OnCampaignReset.Broadcast();
}

void UShadowlineCampaignSubsystem::SetCurrentProvince(FName ProvinceId)
{
	if (Progress.CurrentProvinceId == ProvinceId)
	{
		return;
	}
	Progress.CurrentProvinceId = ProvinceId;
	OnProvinceChanged.Broadcast(ProvinceId);
}

void UShadowlineCampaignSubsystem::SyncWorldClock(int32 InDay, float InTimeOfDay, EShadowlineWeather InWeather)
{
	Progress.Day = FMath::Max(0, InDay);
	Progress.TimeOfDay = FMath::Fmod(FMath::Max(InTimeOfDay, 0.f), 24.f);
	Progress.Weather = InWeather;
}

void UShadowlineCampaignSubsystem::AddPlaytime(float DeltaSeconds)
{
	if (DeltaSeconds > 0.f)
	{
		Progress.PlaytimeSeconds += DeltaSeconds;
	}
}

// ---------------------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------------------

void UShadowlineCampaignSubsystem::SetProgress(const FShadowlineCampaignProgress& NewProgress)
{
	Progress = NewProgress;
	OnCampaignReset.Broadcast();
	OnDifficultyChanged.Broadcast();
	OnSurveyChanged.Broadcast();
}

void UShadowlineCampaignSubsystem::StartNewOperation(EShadowlineDifficulty InDifficulty)
{
	Progress = FShadowlineCampaignProgress();
	Progress.Difficulty = InDifficulty;
	if (InDifficulty == EShadowlineDifficulty::Custom)
	{
		Progress.CustomKnobs = FShadowlineDifficultyKnobs::FromPreset(EShadowlineDifficulty::Standard);
	}

	OnCampaignReset.Broadcast();
	OnDifficultyChanged.Broadcast();
	OnSurveyChanged.Broadcast();
}
