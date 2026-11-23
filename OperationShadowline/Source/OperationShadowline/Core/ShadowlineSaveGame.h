// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineSaveGame.generated.h"

class UGameInstance;

/**
 * The small, cheap part of a save: everything the main menu needs to describe a slot.
 *
 * Written to its own slot file so the menu never deserialises a full campaign just to draw
 * a card. Strings are FString rather than FText because FText does not survive a round trip
 * through a save file in any useful form.
 */
UCLASS()
class OPERATIONSHADOWLINE_API UShadowlineSaveHeader : public USaveGame
{
	GENERATED_BODY()

public:
	/** Title of the operation in progress, e.g. the current story mission's name. */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	FString OperationTitle;

	/** Display name of the province the team is in. */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	FString ProvinceName;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	int32 Act = 1;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	int32 Day = 1;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	float TimeOfDay = 9.5f;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	EShadowlineWeather Weather = EShadowlineWeather::Clear;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	EShadowlineDifficulty Difficulty = EShadowlineDifficulty::Standard;

	/** 0..1. The menu shows this as a percentage. */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	float SurveyFraction = 0.f;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	int32 ProvincesCleared = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	int32 ProvincesTotal = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	int32 OverseersResolved = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	float PlaytimeSeconds = 0.f;

	/** When this slot was last written, for sorting and for the card's timestamp. */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	FDateTime SavedAtUtc;

	/** Bumped whenever the save format changes, so old slots can be migrated or rejected. */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Header")
	int32 SaveVersion = 1;
};

/** The full campaign state. Loaded only when a slot is actually resumed. */
UCLASS()
class OPERATIONSHADOWLINE_API UShadowlineSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Save")
	FShadowlineCampaignProgress Progress;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Save")
	int32 SaveVersion = 1;
};

/**
 * Save and load for Operation Shadowline.
 *
 * Kept apart from UShadowlineCampaignSubsystem on purpose: the subsystem owns state and the
 * rules over it, this owns serialisation. Each slot writes two files — a header and a body —
 * and SaveSlot writes both, in that order, so a slot can never show a card for a body that
 * is not there.
 */
UCLASS()
class OPERATIONSHADOWLINE_API UShadowlineSaveLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Current save format version. Bump when FShadowlineCampaignProgress changes shape. */
	static constexpr int32 CurrentSaveVersion = 1;

	/** How many slots the UI offers. */
	static constexpr int32 SlotCount = 5;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Save")
	static FString GetBodySlotName(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Save")
	static FString GetHeaderSlotName(int32 SlotIndex);

	/**
	 * Writes both files for a slot, building the header from the campaign subsystem.
	 * OperationTitle is passed in because only the mission layer knows what to call the
	 * moment the player is in.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Save")
	static bool SaveSlot(UGameInstance* GameInstance, int32 SlotIndex, const FString& OperationTitle);

	/** Reads just the header. Cheap enough to call for every slot when the menu opens. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Save")
	static UShadowlineSaveHeader* ReadHeader(int32 SlotIndex);

	/** Loads a slot's body into the campaign subsystem. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Save")
	static bool LoadSlot(UGameInstance* GameInstance, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Shadowline|Save")
	static bool DeleteSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Save")
	static bool DoesSlotExist(int32 SlotIndex);
};
