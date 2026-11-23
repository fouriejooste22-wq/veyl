// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineDifficultyProfile.generated.h"

/**
 * A named difficulty, as an asset.
 *
 * The four built-in presets have their values in FShadowlineDifficultyKnobs::FromPreset so
 * that code has a dependency-free source of truth. This asset exists so designers can
 * author additional profiles, and so a mission can override the active difficulty for one
 * sequence without the player's chosen preset being changed behind their back.
 */
UCLASS(BlueprintType)
class OPERATIONSHADOWLINE_API UShadowlineDifficultyProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	EShadowlineDifficulty Preset = EShadowlineDifficulty::Standard;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** One or two sentences, written for the player choosing it, not for a designer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tuning")
	FShadowlineDifficultyKnobs Knobs;

	/** Overwrites Knobs with the built-in values for Preset. Editor convenience. */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Tuning")
	void ResetToPresetDefaults();
};
