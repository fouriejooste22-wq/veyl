// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Core/ShadowlineGameMode.h"
#include "Player/ShadowlineCharacter.h"
#include "Core/ShadowlinePlayerController.h"

AShadowlineGameMode::AShadowlineGameMode()
{
	DefaultPawnClass = AShadowlineCharacter::StaticClass();
	PlayerControllerClass = AShadowlinePlayerController::StaticClass();

	// Blueprint subclasses (BP_ShadowlineGameMode) override these with the Blueprint
	// character that carries the skeletal mesh, anim Blueprint and input config.
}
