// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Core/ShadowlinePlayerController.h"

AShadowlinePlayerController::AShadowlinePlayerController()
{
	// Gameplay runs without a visible cursor; menus turn it on while they are open.
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

void AShadowlinePlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}
