// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Player/ShadowlineCharacterMovement.h"
#include "GameFramework/Character.h"

UShadowlineCharacterMovement::UShadowlineCharacterMovement()
{
	Stance = EShadowlineStance::Standing;
	Gait = EShadowlineGait::Normal;

	// Slow / Normal / Sprint, in cm/s.
	StandingSpeeds[0] = 165.f;	// quiet walk
	StandingSpeeds[1] = 400.f;	// jog
	StandingSpeeds[2] = 620.f;	// sprint

	CrouchedSpeeds[0] = 95.f;
	CrouchedSpeeds[1] = 215.f;
	CrouchedSpeeds[2] = 215.f;	// no sprint while crouched

	ProneSpeeds[0] = 45.f;
	ProneSpeeds[1] = 80.f;
	ProneSpeeds[2] = 80.f;		// no sprint while prone

	StandingHalfHeight = 90.f;
	CrouchedHalfHeightOverride = 60.f;
	ProneHalfHeight = 34.f;

	SprintAcceleration = 1400.f;

	// Base character movement tuning for a grounded, weighty third-person character.
	MaxWalkSpeed = StandingSpeeds[1];
	MaxWalkSpeedCrouched = CrouchedSpeeds[1];
	MaxAcceleration = 2048.f;
	BrakingDecelerationWalking = 2000.f;
	GroundFriction = 8.f;
	BrakingFrictionFactor = 1.f;
	RotationRate = FRotator(0.f, 540.f, 0.f);
	bOrientRotationToMovement = false;	// controlled by the character; aim mode keeps the actor facing the camera
	bUseControllerDesiredRotation = false;

	// Slopes and steps: the island has a lot of rough terrain.
	SetWalkableFloorAngle(50.f);
	MaxStepHeight = 45.f;
	PerchRadiusThreshold = 15.f;
	PerchAdditionalHeight = 0.f;
	LedgeCheckThreshold = 4.f;

	// Crouch / prone path.
	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanSwim = true;
	NavAgentProps.bCanJump = true;
	SetCrouchedHalfHeight(CrouchedHalfHeightOverride);

	// Swimming. Requires water volumes (or the Water plugin's water bodies) in the level.
	MaxSwimSpeed = 300.f;
	Buoyancy = 1.f;
	OutofWaterZ = 420.f;

	// Falling.
	JumpZVelocity = 460.f;
	AirControl = 0.18f;
	GravityScale = 1.f;
	FallingLateralFriction = 0.2f;
}

float UShadowlineCharacterMovement::GetHalfHeightForStance(EShadowlineStance InStance) const
{
	switch (InStance)
	{
	case EShadowlineStance::Crouched:	return CrouchedHalfHeightOverride;
	case EShadowlineStance::Prone:		return ProneHalfHeight;
	case EShadowlineStance::Standing:	// fall through
	default:							return StandingHalfHeight;
	}
}

EShadowlineGait UShadowlineCharacterMovement::ResolveEffectiveGait() const
{
	// Sprinting only exists while standing.
	if (Gait == EShadowlineGait::Sprint && Stance != EShadowlineStance::Standing)
	{
		return EShadowlineGait::Normal;
	}
	return Gait;
}

void UShadowlineCharacterMovement::SetStance(EShadowlineStance NewStance)
{
	if (Stance == NewStance)
	{
		return;
	}

	Stance = NewStance;

	// Crouched and prone both ride the engine's crouched capsule, at different heights.
	const bool bWantsCrouchedCapsule = (NewStance != EShadowlineStance::Standing);
	SetCrouchedHalfHeight(GetHalfHeightForStance(NewStance));

	if (ACharacter* Owner = GetCharacterOwner())
	{
		if (bWantsCrouchedCapsule)
		{
			// Re-apply so the capsule resizes to the new crouched half height even when
			// transitioning crouched -> prone (already crouched, so Crouch() alone is a no-op).
			if (Owner->bIsCrouched)
			{
				Owner->UnCrouch(true);
			}
			Owner->Crouch(true);
		}
		else if (Owner->bIsCrouched)
		{
			Owner->UnCrouch(true);
		}
	}

	MaxWalkSpeedCrouched = CrouchedSpeeds[static_cast<int32>(ResolveEffectiveGait())];
}

void UShadowlineCharacterMovement::SetGait(EShadowlineGait NewGait)
{
	Gait = NewGait;
}

float UShadowlineCharacterMovement::GetMaxSpeed() const
{
	// Non-walking modes keep the engine's own limits (swimming, flying, custom).
	if (MovementMode != MOVE_Walking && MovementMode != MOVE_NavWalking)
	{
		return Super::GetMaxSpeed();
	}

	const int32 GaitIndex = static_cast<int32>(ResolveEffectiveGait());

	switch (Stance)
	{
	case EShadowlineStance::Crouched:	return CrouchedSpeeds[GaitIndex];
	case EShadowlineStance::Prone:		return ProneSpeeds[GaitIndex];
	case EShadowlineStance::Standing:	// fall through
	default:							return StandingSpeeds[GaitIndex];
	}
}

float UShadowlineCharacterMovement::GetMaxAcceleration() const
{
	if (MovementMode == MOVE_Walking && ResolveEffectiveGait() == EShadowlineGait::Sprint)
	{
		return SprintAcceleration;
	}
	return Super::GetMaxAcceleration();
}

float UShadowlineCharacterMovement::GetMovementNoiseLevel() const
{
	const float MaxForStance = GetMaxSpeed();
	if (MaxForStance <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	// How much of the stance's top speed is actually being used.
	const float Planar = Velocity.Size2D();
	const float SpeedRatio = FMath::Clamp(Planar / MaxForStance, 0.f, 1.f);

	// Stance ceiling: lying down is quiet even at full crawl speed.
	float StanceCeiling = 1.f;
	switch (Stance)
	{
	case EShadowlineStance::Crouched:	StanceCeiling = 0.45f; break;
	case EShadowlineStance::Prone:		StanceCeiling = 0.18f; break;
	default:							StanceCeiling = 1.f;   break;
	}

	// Airborne characters make no footfall noise; landing noise is a separate event.
	if (MovementMode == MOVE_Falling || MovementMode == MOVE_None)
	{
		return 0.f;
	}

	return SpeedRatio * StanceCeiling;
}
