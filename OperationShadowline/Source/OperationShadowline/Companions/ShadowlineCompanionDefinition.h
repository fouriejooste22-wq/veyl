// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineCompanionDefinition.generated.h"

class UShadowlineDialogueSet;
class USkeletalMesh;
class UTexture2D;

/**
 * One of a companion's progression tracks.
 *
 * Tiers gate capability, never combat strength. Tier 3 Survey lets Ferreiro read a current
 * the player cannot; Tier 4 Signals lets Okonkwo intercept traffic that is otherwise noise.
 * Progression opens routes, not damage numbers.
 */
USTRUCT(BlueprintType)
struct FShadowlineSpecialisation
{
	GENERATED_BODY()

	/** Stable id, e.g. "Survey", "Signals", "Medical", "Climbing". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specialisation")
	FName SpecialisationId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specialisation")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specialisation", meta = (ClampMin = "1", ClampMax = "5"))
	int32 MaxTier = 4;

	/** What each tier unlocks, written for the player. Index 0 describes tier 1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specialisation")
	TArray<FText> TierDescriptions;

	/** Capability ids granted at each tier, index-aligned with TierDescriptions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specialisation")
	TArray<FName> TierCapabilities;
};

/** One squad companion. Three exist: Ferreiro, Okonkwo and Varga. */
UCLASS(BlueprintType)
class OPERATIONSHADOWLINE_API UShadowlineCompanionDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Stable identifier used by save games. Never rename it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName CompanionId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Surname only, for the HUD's companion pips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText ShortName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText Role;

	/** Two or three sentences. This is the character brief voice work is written against. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = "true"))
	FText Personality;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
	TObjectPtr<UTexture2D> Portrait;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
	TSoftObjectPtr<USkeletalMesh> BodyMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression")
	TArray<FShadowlineSpecialisation> Specialisations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
	TSoftObjectPtr<UShadowlineDialogueSet> DialogueSet;

	/** Capabilities a given tier in a given specialisation grants. Empty if unknown. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	TArray<FName> GetCapabilitiesAtTier(FName SpecialisationId, int32 Tier) const;
};
