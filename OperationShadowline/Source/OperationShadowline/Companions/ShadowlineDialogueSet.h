// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineDialogueSet.generated.h"

class USoundBase;

/** When a bark is allowed to fire, beyond its trigger. */
USTRUCT(BlueprintType)
struct FShadowlineBarkFilter
{
	GENERATED_BODY()

	/** Regions this line is allowed in. Empty means any region. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	TArray<EShadowlineRegion> Regions;

	/** Weather states this line is allowed in. Empty means any weather. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	TArray<EShadowlineWeather> Weather;

	/** 0 = any, 1 = day only, 2 = night only. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter", meta = (ClampMin = "0", ClampMax = "2"))
	int32 DayNight = 0;

	/** Lowest act this line may appear in. 0 means any. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter", meta = (ClampMin = "0", ClampMax = "5"))
	int32 MinAct = 0;

	/** Highest act this line may appear in. 0 means no upper bound. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter", meta = (ClampMin = "0", ClampMax = "5"))
	int32 MaxAct = 0;
};

/**
 * One thing a companion might say.
 *
 * Barks are the cheapest characterisation in the game and the easiest to get wrong: a line
 * that fires in the wrong weather, or for the third time in a minute, actively damages the
 * character it was written for. Hence the filters and the cooldown.
 */
USTRUCT(BlueprintType)
struct FShadowlineBark
{
	GENERATED_BODY()

	/** What prompts this line, e.g. "EnteredProvince", "SpottedPatrol", "FoundDocument". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark")
	FName Trigger;

	/** Variants. One is chosen at random, so a trigger does not always produce one line. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark", meta = (MultiLine = "true"))
	TArray<FText> Lines;

	/** Matching voice lines, index-aligned with Lines where they exist. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark")
	TArray<TObjectPtr<USoundBase>> VoiceLines;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark")
	FShadowlineBarkFilter Filter;

	/** Minimum seconds between repeats of this bark. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark", meta = (ClampMin = "0.0"))
	float CooldownSeconds = 45.f;

	/** Higher wins when several barks match the same trigger. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark", meta = (ClampMin = "0", ClampMax = "10"))
	int32 Priority = 0;

	/** True for lines that should only ever play once in a whole campaign. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark")
	bool bOnceEver = false;
};

/** The context a bark is selected against. */
USTRUCT(BlueprintType)
struct FShadowlineBarkContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	EShadowlineRegion Region = EShadowlineRegion::None;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	EShadowlineWeather Weather = EShadowlineWeather::Clear;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	bool bNight = false;

	UPROPERTY(BlueprintReadWrite, Category = "Context", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Act = 1;
};

/** One companion's lines. One asset per companion in Content/Shadowline/Characters/Dialogue. */
UCLASS(BlueprintType)
class OPERATIONSHADOWLINE_API UShadowlineDialogueSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
	TArray<FShadowlineBark> Barks;

	/** True when this bark's filters admit the given context. Pure; safe to call freely. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	static bool DoesBarkMatchContext(const FShadowlineBark& Bark, const FShadowlineBarkContext& Context);

	/**
	 * Highest-priority bark for a trigger whose filters admit the context, or INDEX_NONE.
	 * Ties are broken at random so a companion with two equally valid lines uses both.
	 * Cooldowns and once-ever tracking belong to the speaking component, not here, because
	 * this asset is shared and must stay stateless.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	int32 SelectBarkIndex(FName Trigger, const FShadowlineBarkContext& Context) const;
};
