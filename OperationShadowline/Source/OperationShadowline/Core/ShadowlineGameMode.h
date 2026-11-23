// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShadowlineGameMode.generated.h"

/**
 * Base game mode for playable Aurelia levels.
 *
 * Deliberately thin: it only wires the default pawn and controller classes. Mission flow,
 * objectives and squad state belong to their own subsystems in later stages rather than
 * to the game mode, so that a single level can host several mission types.
 */
UCLASS(Blueprintable)
class OPERATIONSHADOWLINE_API AShadowlineGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AShadowlineGameMode();
};
