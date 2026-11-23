// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineCampaignSubsystem.generated.h"

class UShadowlineProvinceDefinition;
class UShadowlineMissionDefinition;
class UShadowlineSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShadowlineProvinceEvent, FName, ProvinceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShadowlineMissionEvent, FName, MissionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShadowlineCampaignEvent);

/**
 * The single owner of campaign state, and the thing every UI screen binds to.
 *
 * Holds one FShadowlineCampaignProgress — only facts that cannot be derived — and computes
 * everything else from it plus the registered definition assets: province state, mission
 * state, survey percentages, how many overseers are resolved, whether a regional director
 * or the finale is available.
 *
 * Nothing derived is ever stored. Storing "province 7 is cleared" alongside the facts that
 * make it cleared is how a save game and the game's own rules drift apart, and it is the
 * single most common source of unreproducible progression bugs.
 *
 * Lives on the game instance, so it survives level transitions — which matters because
 * Aurelia's provinces are separate streamed areas.
 */
UCLASS()
class OPERATIONSHADOWLINE_API UShadowlineCampaignSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem interface

	// --- Definition registry ---

	/**
	 * Registers the province and mission definitions the campaign runs on.
	 * Call once at startup. Without this every query returns a safe empty answer rather
	 * than asserting, so a level opened directly in the editor still works.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void RegisterDefinitions(
		const TArray<UShadowlineProvinceDefinition*>& InProvinces,
		const TArray<UShadowlineMissionDefinition*>& InMissions);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	const UShadowlineProvinceDefinition* FindProvince(FName ProvinceId) const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	const UShadowlineMissionDefinition* FindMission(FName MissionId) const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	TArray<FName> GetProvinceIdsInRegion(EShadowlineRegion Region) const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	TArray<FName> GetMissionIdsInProvince(FName ProvinceId) const;

	// --- Derived state ---

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	EShadowlineProvinceState GetProvinceState(FName ProvinceId) const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	EShadowlineMissionState GetMissionState(FName MissionId) const;

	/** 0..1 of this province's discoverables found. Returns 1 for a province with none. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	float GetProvinceSurveyFraction(FName ProvinceId) const;

	/** 0..1 across the whole island, weighted by how much each province contains. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	float GetIslandSurveyFraction() const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	int32 GetProvincesCleared() const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	int32 GetOverseersResolved() const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	int32 GetMissionsCompleted() const;

	/** Required objectives satisfied on a mission, for the operations screen. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	int32 GetRequiredObjectivesSatisfied(FName MissionId) const;

	/** True once every province in the region is Cleared. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	bool IsRegionalDirectorAvailable(EShadowlineRegion Region) const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	bool IsRegionalDirectorResolved(EShadowlineRegion Region) const;

	/** The finale needs all four regional directors resolved. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	bool IsFinaleAvailable() const;

	// --- Difficulty ---

	UFUNCTION(BlueprintCallable, Category = "Shadowline|Difficulty")
	void SetDifficulty(EShadowlineDifficulty NewDifficulty);

	/** Switches to Custom and stores these knobs. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Difficulty")
	void SetCustomDifficulty(const FShadowlineDifficultyKnobs& NewKnobs);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Difficulty")
	EShadowlineDifficulty GetDifficulty() const { return Progress.Difficulty; }

	/** The knobs in force: the preset's values, or the custom set. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Difficulty")
	FShadowlineDifficultyKnobs GetDifficultyKnobs() const;

	// --- Mutations ---

	/** Raises an objective's completion event. No-op if already raised. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void CompleteObjective(FName MissionId, FName ObjectiveEvent);

	/** Records a detection during a mission, which is what gates its best outcome. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void RecordDetection(FName MissionId);

	/**
	 * Marks a mission complete. Whether it counts as undetected is read from the stored
	 * detection flag rather than passed in, so a caller cannot claim a clean run.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void CompleteMission(FName MissionId);

	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void RecordDiscovery(FName ProvinceId, EShadowlineDiscovery Kind);

	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void ResolveOverseer(FName ProvinceId);

	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void ResolveRegionalDirector(EShadowlineRegion Region);

	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void SetCurrentProvince(FName ProvinceId);

	/** Keeps the save header's clock in step with the world atmosphere actor. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void SyncWorldClock(int32 InDay, float InTimeOfDay, EShadowlineWeather InWeather);

	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void AddPlaytime(float DeltaSeconds);

	// --- Persistence ---

	UFUNCTION(BlueprintPure, Category = "Shadowline|Campaign")
	const FShadowlineCampaignProgress& GetProgress() const { return Progress; }

	/** Replaces all progress, e.g. after loading. Fires the change delegates once. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void SetProgress(const FShadowlineCampaignProgress& NewProgress);

	/** Resets to a fresh operation at the given difficulty. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Campaign")
	void StartNewOperation(EShadowlineDifficulty InDifficulty);

	// --- Events for UI binding ---

	UPROPERTY(BlueprintAssignable, Category = "Shadowline|Campaign")
	FShadowlineProvinceEvent OnProvinceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Shadowline|Campaign")
	FShadowlineMissionEvent OnMissionChanged;

	/** Fired when the island survey percentage moves. */
	UPROPERTY(BlueprintAssignable, Category = "Shadowline|Campaign")
	FShadowlineCampaignEvent OnSurveyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Shadowline|Campaign")
	FShadowlineCampaignEvent OnDifficultyChanged;

	/** Fired after SetProgress or StartNewOperation, when everything may have moved. */
	UPROPERTY(BlueprintAssignable, Category = "Shadowline|Campaign")
	FShadowlineCampaignEvent OnCampaignReset;

private:
	/** Looks up stored progress, or a default-constructed value if there is none yet. */
	const FShadowlineProvinceProgress& GetProvinceProgress(FName ProvinceId) const;
	const FShadowlineMissionProgress& GetMissionProgress(FName MissionId) const;

	/** True when every province the given one depends on is Cleared. */
	bool ArePrerequisitesMet(const UShadowlineProvinceDefinition& Province) const;

	/** Province state without the prerequisite check, to break recursion. */
	EShadowlineProvinceState GetProvinceStateIgnoringPrerequisites(FName ProvinceId) const;

	/** Notifies a province's own change plus any province that depends on it. */
	void BroadcastProvinceAndDependents(FName ProvinceId);

private:
	UPROPERTY()
	FShadowlineCampaignProgress Progress;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<const UShadowlineProvinceDefinition>> ProvinceDefs;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<const UShadowlineMissionDefinition>> MissionDefs;

	/** Default instances returned for unknown ids, so lookups never return a null reference. */
	FShadowlineProvinceProgress EmptyProvinceProgress;
	FShadowlineMissionProgress EmptyMissionProgress;
};
