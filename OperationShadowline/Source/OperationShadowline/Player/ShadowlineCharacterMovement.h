// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineCharacterMovement.generated.h"

/**
 * Movement component for Shadowline characters.
 *
 * Adds a three-step stance system (standing / crouched / prone) on top of the engine's
 * crouch machinery, and resolves ground speed from stance + gait rather than from a
 * single MaxWalkSpeed value.
 *
 * Prone reuses the engine's crouched capsule path with a shorter half height, so capsule
 * resizing, floor adjustment and network correction all keep working without a custom
 * movement mode.
 */
UCLASS()
class OPERATIONSHADOWLINE_API UShadowlineCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UShadowlineCharacterMovement();

	//~ Begin UCharacterMovementComponent interface
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	//~ End UCharacterMovementComponent interface

	/** Current posture. Set through AShadowlineCharacter, not directly. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Movement")
	void SetStance(EShadowlineStance NewStance);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Movement")
	EShadowlineStance GetStance() const { return Stance; }

	/** Requested pace within the current stance. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Movement")
	void SetGait(EShadowlineGait NewGait);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Movement")
	EShadowlineGait GetGait() const { return Gait; }

	/** Capsule half height for a given stance, in centimetres. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Movement")
	float GetHalfHeightForStance(EShadowlineStance InStance) const;

	/**
	 * Relative loudness of the character's footfalls right now, 0..1.
	 * Derived from stance, gait and actual horizontal velocity. The stealth systems in
	 * later stages read this; nothing consumes it yet.
	 */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Stealth")
	float GetMovementNoiseLevel() const;

protected:
	/** Ground speeds in cm/s. Index order matches EShadowlineGait. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Speeds", meta = (ClampMin = "0.0"))
	float StandingSpeeds[3];

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Speeds", meta = (ClampMin = "0.0"))
	float CrouchedSpeeds[3];

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Speeds", meta = (ClampMin = "0.0"))
	float ProneSpeeds[3];

	/** Capsule half height while standing, in centimetres. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Stance", meta = (ClampMin = "20.0"))
	float StandingHalfHeight;

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Stance", meta = (ClampMin = "20.0"))
	float CrouchedHalfHeightOverride;

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Stance", meta = (ClampMin = "20.0"))
	float ProneHalfHeight;

	/** Acceleration while sprinting is lower, so direction changes carry momentum. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Speeds", meta = (ClampMin = "0.0"))
	float SprintAcceleration;

private:
	UPROPERTY(Transient)
	EShadowlineStance Stance;

	UPROPERTY(Transient)
	EShadowlineGait Gait;

	/** Clamps the requested gait to what the current stance allows. */
	EShadowlineGait ResolveEffectiveGait() const;
};
