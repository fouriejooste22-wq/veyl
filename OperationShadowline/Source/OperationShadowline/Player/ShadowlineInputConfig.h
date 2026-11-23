// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShadowlineInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Designer-facing container for the Enhanced Input assets the player character binds.
 *
 * Create one instance at Content/Shadowline/Input/DA_ShadowlineInput and assign it on the
 * player character Blueprint. Keeping the references in a data asset means rebinding or
 * adding an action never needs a C++ change.
 */
UCLASS(BlueprintType)
class OPERATIONSHADOWLINE_API UShadowlineInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Mapping context pushed when the player character is possessed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context")
	int32 MappingPriority = 0;

	/** 2D axis: X = strafe, Y = forward. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> MoveAction;

	/** 2D axis: X = yaw, Y = pitch. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> LookAction;

	/** Context-sensitive: vault/mantle when an obstacle is in front, otherwise jump. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> JumpAction;

	/** Held. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> SprintAction;

	/** Held, for the quietest pace in the current stance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> SlowWalkAction;

	/** Tap to toggle crouch. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> CrouchAction;

	/** Tap to toggle prone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> ProneAction;

	/** Swaps the camera to the other shoulder. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> SwapShoulderAction;

	/** Held: tightens the camera and locks actor facing to the camera. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> FocusAction;

	/** Mouse wheel / stick: pulls the camera in and out within its clamped range. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UInputAction> CameraDistanceAction;
};
