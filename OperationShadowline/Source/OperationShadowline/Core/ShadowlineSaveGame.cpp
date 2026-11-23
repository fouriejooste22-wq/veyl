// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Core/ShadowlineSaveGame.h"

#include "OperationShadowline.h"
#include "Core/ShadowlineCampaignSubsystem.h"
#include "World/ShadowlineProvinceDefinition.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

FString UShadowlineSaveLibrary::GetBodySlotName(int32 SlotIndex)
{
	return FString::Printf(TEXT("Shadowline_Slot%d"), FMath::Clamp(SlotIndex, 0, SlotCount - 1));
}

FString UShadowlineSaveLibrary::GetHeaderSlotName(int32 SlotIndex)
{
	return FString::Printf(TEXT("Shadowline_Slot%d_Header"), FMath::Clamp(SlotIndex, 0, SlotCount - 1));
}

bool UShadowlineSaveLibrary::SaveSlot(UGameInstance* GameInstance, int32 SlotIndex, const FString& OperationTitle)
{
	if (!GameInstance)
	{
		UE_LOG(LogShadowline, Error, TEXT("SaveSlot called with no game instance."));
		return false;
	}

	UShadowlineCampaignSubsystem* Campaign = GameInstance->GetSubsystem<UShadowlineCampaignSubsystem>();
	if (!Campaign)
	{
		UE_LOG(LogShadowline, Error, TEXT("SaveSlot could not reach the campaign subsystem."));
		return false;
	}

	const FShadowlineCampaignProgress& Progress = Campaign->GetProgress();

	// --- body ---
	UShadowlineSaveGame* Body = Cast<UShadowlineSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UShadowlineSaveGame::StaticClass()));
	if (!Body)
	{
		return false;
	}
	Body->Progress = Progress;
	Body->SaveVersion = CurrentSaveVersion;

	if (!UGameplayStatics::SaveGameToSlot(Body, GetBodySlotName(SlotIndex), 0))
	{
		UE_LOG(LogShadowline, Error, TEXT("Failed to write save body for slot %d."), SlotIndex);
		return false;
	}

	// --- header ---
	UShadowlineSaveHeader* Header = Cast<UShadowlineSaveHeader>(
		UGameplayStatics::CreateSaveGameObject(UShadowlineSaveHeader::StaticClass()));
	if (!Header)
	{
		return false;
	}

	Header->OperationTitle = OperationTitle;
	Header->Act = Progress.Act;
	Header->Day = Progress.Day;
	Header->TimeOfDay = Progress.TimeOfDay;
	Header->Weather = Progress.Weather;
	Header->Difficulty = Progress.Difficulty;
	Header->SurveyFraction = Campaign->GetIslandSurveyFraction();
	Header->ProvincesCleared = Campaign->GetProvincesCleared();
	Header->OverseersResolved = Campaign->GetOverseersResolved();
	Header->PlaytimeSeconds = Progress.PlaytimeSeconds;
	Header->SavedAtUtc = FDateTime::UtcNow();
	Header->SaveVersion = CurrentSaveVersion;

	if (const UShadowlineProvinceDefinition* Province = Campaign->FindProvince(Progress.CurrentProvinceId))
	{
		Header->ProvinceName = Province->DisplayName.ToString();
	}
	else
	{
		Header->ProvinceName = TEXT("Aurelia");
	}

	// Counted from the registry, not from stored progress, so the card reads "4 of 15"
	// against the content that actually exists in this build.
	int32 ProvincesTotal = 0;
	for (const EShadowlineRegion Region : {
			EShadowlineRegion::NorthernMountains, EShadowlineRegion::SouthernJungle,
			EShadowlineRegion::WesternDesert, EShadowlineRegion::EasternCoast,
			EShadowlineRegion::CentralIndustrial })
	{
		ProvincesTotal += Campaign->GetProvinceIdsInRegion(Region).Num();
	}
	Header->ProvincesTotal = ProvincesTotal;

	if (!UGameplayStatics::SaveGameToSlot(Header, GetHeaderSlotName(SlotIndex), 0))
	{
		// The body is already on disk. Report it rather than leaving the caller to assume
		// the slot is fine: the menu would show a stale card for a newer body.
		UE_LOG(LogShadowline, Error,
			TEXT("Save body for slot %d was written but its header was not. The slot will ")
			TEXT("describe itself incorrectly until the next successful save."), SlotIndex);
		return false;
	}

	UE_LOG(LogShadowline, Log, TEXT("Saved slot %d (%s, act %d, %.0f%% surveyed)."),
		SlotIndex, *Header->ProvinceName, Header->Act, Header->SurveyFraction * 100.f);
	return true;
}

UShadowlineSaveHeader* UShadowlineSaveLibrary::ReadHeader(int32 SlotIndex)
{
	const FString SlotName = GetHeaderSlotName(SlotIndex);
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return nullptr;
	}

	UShadowlineSaveHeader* Header = Cast<UShadowlineSaveHeader>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Header)
	{
		UE_LOG(LogShadowline, Warning, TEXT("Save header for slot %d exists but would not load."), SlotIndex);
		return nullptr;
	}

	if (Header->SaveVersion > CurrentSaveVersion)
	{
		// From a newer build. Showing it would misdescribe it and loading it could corrupt.
		UE_LOG(LogShadowline, Warning,
			TEXT("Slot %d was written by save version %d; this build understands %d. Hiding it."),
			SlotIndex, Header->SaveVersion, CurrentSaveVersion);
		return nullptr;
	}

	return Header;
}

bool UShadowlineSaveLibrary::LoadSlot(UGameInstance* GameInstance, int32 SlotIndex)
{
	if (!GameInstance)
	{
		return false;
	}

	UShadowlineCampaignSubsystem* Campaign = GameInstance->GetSubsystem<UShadowlineCampaignSubsystem>();
	if (!Campaign)
	{
		return false;
	}

	const FString SlotName = GetBodySlotName(SlotIndex);
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return false;
	}

	UShadowlineSaveGame* Body = Cast<UShadowlineSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Body)
	{
		UE_LOG(LogShadowline, Error, TEXT("Save body for slot %d would not load."), SlotIndex);
		return false;
	}

	if (Body->SaveVersion > CurrentSaveVersion)
	{
		UE_LOG(LogShadowline, Error,
			TEXT("Refusing to load slot %d: save version %d is newer than this build's %d."),
			SlotIndex, Body->SaveVersion, CurrentSaveVersion);
		return false;
	}

	Campaign->SetProgress(Body->Progress);
	UE_LOG(LogShadowline, Log, TEXT("Loaded slot %d."), SlotIndex);
	return true;
}

bool UShadowlineSaveLibrary::DeleteSlot(int32 SlotIndex)
{
	// Both files, so a deleted slot cannot leave a header behind advertising a missing body.
	const bool bBody = UGameplayStatics::DeleteGameInSlot(GetBodySlotName(SlotIndex), 0);
	const bool bHeader = UGameplayStatics::DeleteGameInSlot(GetHeaderSlotName(SlotIndex), 0);
	return bBody || bHeader;
}

bool UShadowlineSaveLibrary::DoesSlotExist(int32 SlotIndex)
{
	return UGameplayStatics::DoesSaveGameExist(GetBodySlotName(SlotIndex), 0);
}
