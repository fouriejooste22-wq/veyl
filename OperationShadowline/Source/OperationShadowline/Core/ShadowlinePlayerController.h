// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShadowlinePlayerController.generated.h"

/**
 * Player controller for Operation Shadowline.
 *
 * Owns view-level concerns rather than character ones: input mode, and the camera-shake
 * and view-target handling that cutscenes and vehicles will need. Gameplay input bindings
 * live on AShadowlineCharacter, which is also where the Enhanced Input mapping context is
 * pushed, so that possessing a vehicle or a companion swaps the bindings with the pawn.
 */
UCLASS()
class OPERATIONSHADOWLINE_API AShadowlinePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShadowlinePlayerController();

protected:
	virtual void BeginPlay() override;
};
