// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineProvinceDefinition.generated.h"

class UShadowlineMissionDefinition;
class UTexture2D;

/**
 * One province of Aurelia: the unit the campaign is structured around.
 *
 * Aurelia has five regions, each divided into provinces. A region supplies the light, air
 * and weather (UShadowlineRegionDefinition); a province supplies the content — its missions,
 * its difficulty rating, its named Vantrell overseer, and what the player can find there.
 *
 * Progression reads from this asset rather than from Blueprint logic, so the campaign tree
 * can be rebalanced by editing data. One asset per province in
 * Content/Shadowline/World/Provinces.
 */
UCLASS(BlueprintType)
class OPERATIONSHADOWLINE_API UShadowlineProvinceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Stable identifier used by save games and prerequisite lists. Never rename it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName ProvinceId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Which region's atmosphere this province sits in. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	EShadowlineRegion Region = EShadowlineRegion::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = "true"))
	FText Description;

	/**
	 * Suggested difficulty, 1 (landing zone) to 5 (Meridian Works).
	 * Advisory only: the island is open after Act I and nothing stops the player walking
	 * into a tier 5 province on day one.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (ClampMin = "1", ClampMax = "5"))
	int32 DifficultyTier = 1;

	// --- Overseer: the province's named target ---

	/** Name of the Vantrell overseer who runs this province. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Overseer")
	FText OverseerName;

	/** Their job title, which is also the reason they matter to the investigation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Overseer")
	FText OverseerTitle;

	/** What the player has to achieve. Threshold has no authority to do them harm. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Overseer", meta = (MultiLine = "true"))
	FText OverseerObjective;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Overseer")
	TObjectPtr<UTexture2D> OverseerPortrait;

	// --- Content ---

	/** Every mission sited in this province, story and side alike. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Content")
	TArray<TSoftObjectPtr<UShadowlineMissionDefinition>> Missions;

	/** How many collectible documents exist here. Used for the completion readout. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Content", meta = (ClampMin = "0"))
	int32 DocumentCount = 0;

	/** Discoverable landmarks: wrecks, shrines, survivor camps, abandoned equipment. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Content", meta = (ClampMin = "0"))
	int32 LandmarkCount = 0;

	/** Recoverable supply caches. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Content", meta = (ClampMin = "0"))
	int32 CacheCount = 0;

	// --- Progression ---

	/**
	 * Provinces that must be Cleared before this one opens.
	 * Empty means the province is open from the end of Act I. Keep these shallow: a long
	 * prerequisite chain turns an open world back into a corridor.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression")
	TArray<FName> PrerequisiteProvinceIds;

	// --- Map placement ---

	/**
	 * Where the province's label and seed point sit in map space, in the same coordinates
	 * the world map widget uses. The province's area is derived from the seed points of all
	 * provinces, so moving one reshapes its neighbours.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map")
	FVector2D MapSeedPoint = FVector2D::ZeroVector;

	/** Tint used for this province on the world map. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map")
	FLinearColor MapTint = FLinearColor(0.32f, 0.44f, 0.55f, 1.f);

	/** Total discoverables, for the survey percentage denominator. */
	UFUNCTION(BlueprintPure, Category = "Content")
	int32 GetTotalDiscoverables() const;

	/** True when this province has no prerequisites and is open from the start. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsOpenFromStart() const { return PrerequisiteProvinceIds.Num() == 0; }
};
