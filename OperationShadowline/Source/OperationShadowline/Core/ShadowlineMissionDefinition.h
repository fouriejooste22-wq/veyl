// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineMissionDefinition.generated.h"

class UWorld;

/**
 * One mission.
 *
 * Holds what the operations screen, the map marker and the objective tracker all need, and
 * nothing about how the mission is actually scripted — that lives in the mission's own level
 * or StateTree. Keeping the two apart means the campaign can be reordered and rebalanced
 * without touching a single piece of mission logic.
 *
 * One asset per mission in Content/Shadowline/Missions, grouped by act.
 */
UCLASS(BlueprintType)
class OPERATIONSHADOWLINE_API UShadowlineMissionDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Stable identifier used by save games and prerequisite lists. Never rename it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName MissionId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText Title;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	EShadowlineMissionType MissionType = EShadowlineMissionType::Reconnaissance;

	/** The province this mission is sited in. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName ProvinceId;

	/** 1-5 for a story mission's act; 0 for a side operation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (ClampMin = "0", ClampMax = "5"))
	int32 Act = 0;

	/** The briefing text, written as Threshold would write it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = "true"))
	FText Briefing;

	/** Ordered objective list. Optional entries never block completion. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objectives")
	TArray<FShadowlineObjective> Objectives;

	/** Missions that must be complete before this one appears. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression")
	TArray<FName> PrerequisiteMissionIds;

	/**
	 * True when the mission must be finished without ever being detected.
	 * On these, a detection does not fail the mission outright: it changes the ending the
	 * mission can reach. There is no death screen in this game.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	bool bUndetectedForBestOutcome = false;

	/** Where the marker sits in map space, matching the world map widget's coordinates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map")
	FVector2D MapLocation = FVector2D::ZeroVector;

	/** Count of required objectives, for the operations screen's progress readout. */
	UFUNCTION(BlueprintPure, Category = "Objectives")
	int32 GetRequiredObjectiveCount() const;

	UFUNCTION(BlueprintPure, Category = "Objectives")
	int32 GetOptionalObjectiveCount() const;

	/** True for the mission types that resolve a named antagonist. */
	UFUNCTION(BlueprintPure, Category = "Identity")
	bool IsConfrontation() const;
};
