// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Player/ShadowlineCharacter.h"

#include "OperationShadowline.h"
#include "Player/ShadowlineCharacterMovement.h"
#include "Player/ShadowlineInputConfig.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "MotionWarpingComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

AShadowlineCharacter::AShadowlineCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UShadowlineCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// The camera owns the view rotation; the capsule is turned by the movement component.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->InitCapsuleSize(38.f, 90.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = 12.f;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 18.f;
	CameraBoom->CameraRotationLagSpeed = 22.f;
	CameraBoom->CameraLagMaxDistance = 40.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));

	// Camera defaults.
	BoomLengthNormal = 300.f;
	BoomLengthFocused = 160.f;
	BoomLengthSprint = 345.f;
	BoomDistanceAdjustRange = 90.f;
	ShoulderOffsetNormal = 48.f;
	ShoulderOffsetFocused = 62.f;
	PivotHeightStanding = 62.f;
	PivotHeightCrouched = 44.f;
	PivotHeightProne = 16.f;
	FieldOfViewNormal = 75.f;
	FieldOfViewFocused = 55.f;
	FieldOfViewSprint = 82.f;
	CameraBlendSpeed = 8.f;

	// Traversal defaults. MinTraversalHeight sits just above the movement component's
	// MaxStepHeight so stepping and vaulting never compete for the same obstacle.
	MinTraversalHeight = 50.f;
	MaxTraversalHeight = 200.f;
	TraversalProbeDistance = 110.f;
	VaultMaxDepth = 60.f;
	TraversalWarpTargetName = TEXT("TraversalTarget");

	bSprintHeld = false;
	bSlowWalkHeld = false;
	bFocusHeld = false;
	ShoulderSign = 1.f;
	BoomDistanceAdjust = 0.f;
	bTraversalInProgress = false;

	// Start the rig at its standing, unfocused pose so frame one is already correct.
	CameraBoom->TargetArmLength = BoomLengthNormal;
	CameraBoom->SocketOffset = FVector(0.f, ShoulderOffsetNormal, PivotHeightStanding);
	FollowCamera->SetFieldOfView(FieldOfViewNormal);
}

UShadowlineCharacterMovement* AShadowlineCharacter::GetShadowlineMovement() const
{
	return Cast<UShadowlineCharacterMovement>(GetCharacterMovement());
}

EShadowlineStance AShadowlineCharacter::GetStance() const
{
	if (const UShadowlineCharacterMovement* Movement = GetShadowlineMovement())
	{
		return Movement->GetStance();
	}
	return EShadowlineStance::Standing;
}

void AShadowlineCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateRotationMode();
}

void AShadowlineCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Push the mapping context for local players.
	if (const APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (InputConfig && InputConfig->DefaultMappingContext)
				{
					Subsystem->AddMappingContext(InputConfig->DefaultMappingContext, InputConfig->MappingPriority);
				}
				else
				{
					UE_LOG(LogShadowline, Warning,
						TEXT("%s has no InputConfig/DefaultMappingContext assigned - player input will not respond. ")
						TEXT("Assign DA_ShadowlineInput on the character Blueprint."),
						*GetName());
				}
			}
		}
	}
}

void AShadowlineCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		UE_LOG(LogShadowline, Error,
			TEXT("Expected an UEnhancedInputComponent. Set DefaultPlayerInputClass / ")
			TEXT("DefaultInputComponentClass to the Enhanced Input classes in DefaultInput.ini."));
		return;
	}

	if (!InputConfig)
	{
		return;
	}

	if (InputConfig->MoveAction)
	{
		Input->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &AShadowlineCharacter::Input_Move);
	}
	if (InputConfig->LookAction)
	{
		Input->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &AShadowlineCharacter::Input_Look);
	}
	if (InputConfig->JumpAction)
	{
		Input->BindAction(InputConfig->JumpAction, ETriggerEvent::Started, this, &AShadowlineCharacter::Input_JumpOrTraverse);
		Input->BindAction(InputConfig->JumpAction, ETriggerEvent::Completed, this, &AShadowlineCharacter::Input_JumpReleased);
	}
	if (InputConfig->SprintAction)
	{
		Input->BindAction(InputConfig->SprintAction, ETriggerEvent::Started, this, &AShadowlineCharacter::Input_SprintStart);
		Input->BindAction(InputConfig->SprintAction, ETriggerEvent::Completed, this, &AShadowlineCharacter::Input_SprintStop);
	}
	if (InputConfig->SlowWalkAction)
	{
		Input->BindAction(InputConfig->SlowWalkAction, ETriggerEvent::Started, this, &AShadowlineCharacter::Input_SlowWalkStart);
		Input->BindAction(InputConfig->SlowWalkAction, ETriggerEvent::Completed, this, &AShadowlineCharacter::Input_SlowWalkStop);
	}
	if (InputConfig->CrouchAction)
	{
		Input->BindAction(InputConfig->CrouchAction, ETriggerEvent::Started, this, &AShadowlineCharacter::ToggleCrouchStance);
	}
	if (InputConfig->ProneAction)
	{
		Input->BindAction(InputConfig->ProneAction, ETriggerEvent::Started, this, &AShadowlineCharacter::ToggleProneStance);
	}
	if (InputConfig->SwapShoulderAction)
	{
		Input->BindAction(InputConfig->SwapShoulderAction, ETriggerEvent::Started, this, &AShadowlineCharacter::Input_SwapShoulder);
	}
	if (InputConfig->FocusAction)
	{
		Input->BindAction(InputConfig->FocusAction, ETriggerEvent::Started, this, &AShadowlineCharacter::Input_FocusStart);
		Input->BindAction(InputConfig->FocusAction, ETriggerEvent::Completed, this, &AShadowlineCharacter::Input_FocusStop);
	}
	if (InputConfig->CameraDistanceAction)
	{
		Input->BindAction(InputConfig->CameraDistanceAction, ETriggerEvent::Triggered, this, &AShadowlineCharacter::Input_CameraDistance);
	}
}

void AShadowlineCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CachedProbe = ProbeTraversal();
	UpdateCameraRig(DeltaSeconds);
}

// ---------------------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------------------

void AShadowlineCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero() || !Controller)
	{
		return;
	}

	// Move relative to the camera's yaw, not its pitch.
	const FRotator YawOnly(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void AShadowlineCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	// Sensitivity and inversion belong to the Enhanced Input modifiers on the look action,
	// so the settings menu can change them without touching gameplay code.
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AShadowlineCharacter::Input_JumpOrTraverse()
{
	// A traversable obstacle ahead takes priority over a plain jump.
	if (TryTraversal())
	{
		return;
	}

	// Lying down or crouched, the input stands the character up rather than jumping.
	if (GetStance() != EShadowlineStance::Standing)
	{
		if (UShadowlineCharacterMovement* Movement = GetShadowlineMovement())
		{
			Movement->SetStance(EShadowlineStance::Standing);
			RefreshGait();
		}
		return;
	}

	Jump();
}

void AShadowlineCharacter::Input_JumpReleased()
{
	StopJumping();
}

void AShadowlineCharacter::Input_SprintStart()
{
	bSprintHeld = true;
	RefreshGait();
}

void AShadowlineCharacter::Input_SprintStop()
{
	bSprintHeld = false;
	RefreshGait();
}

void AShadowlineCharacter::Input_SlowWalkStart()
{
	bSlowWalkHeld = true;
	RefreshGait();
}

void AShadowlineCharacter::Input_SlowWalkStop()
{
	bSlowWalkHeld = false;
	RefreshGait();
}

void AShadowlineCharacter::Input_SwapShoulder()
{
	ShoulderSign = -ShoulderSign;
}

void AShadowlineCharacter::Input_FocusStart()
{
	bFocusHeld = true;
	UpdateRotationMode();
}

void AShadowlineCharacter::Input_FocusStop()
{
	bFocusHeld = false;
	UpdateRotationMode();
}

void AShadowlineCharacter::Input_CameraDistance(const FInputActionValue& Value)
{
	const float Delta = Value.Get<float>();
	BoomDistanceAdjust = FMath::Clamp(
		BoomDistanceAdjust + Delta * 20.f,
		-BoomDistanceAdjustRange,
		BoomDistanceAdjustRange);
}

void AShadowlineCharacter::RefreshGait()
{
	UShadowlineCharacterMovement* Movement = GetShadowlineMovement();
	if (!Movement)
	{
		return;
	}

	// Slow walk wins over sprint when both are held: the quiet option is never overridden
	// by accident, which matters in a stealth game.
	EShadowlineGait NewGait = EShadowlineGait::Normal;
	if (bSlowWalkHeld)
	{
		NewGait = EShadowlineGait::Slow;
	}
	else if (bSprintHeld)
	{
		NewGait = EShadowlineGait::Sprint;
	}

	Movement->SetGait(NewGait);
}

// ---------------------------------------------------------------------------------------
// Stance
// ---------------------------------------------------------------------------------------

void AShadowlineCharacter::ToggleCrouchStance()
{
	UShadowlineCharacterMovement* Movement = GetShadowlineMovement();
	if (!Movement || Movement->IsFalling() || Movement->IsSwimming())
	{
		return;
	}

	const EShadowlineStance Current = Movement->GetStance();
	Movement->SetStance(Current == EShadowlineStance::Crouched
		? EShadowlineStance::Standing
		: EShadowlineStance::Crouched);

	RefreshGait();
}

void AShadowlineCharacter::ToggleProneStance()
{
	UShadowlineCharacterMovement* Movement = GetShadowlineMovement();
	if (!Movement || Movement->IsFalling() || Movement->IsSwimming())
	{
		return;
	}

	// Standing up out of prone goes through crouched, which reads better and keeps the
	// capsule from growing its full height inside low cover.
	const EShadowlineStance Current = Movement->GetStance();
	Movement->SetStance(Current == EShadowlineStance::Prone
		? EShadowlineStance::Crouched
		: EShadowlineStance::Prone);

	RefreshGait();
}

// ---------------------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------------------

void AShadowlineCharacter::UpdateRotationMode()
{
	UShadowlineCharacterMovement* Movement = GetShadowlineMovement();
	if (!Movement)
	{
		return;
	}

	// Focused: the body faces where the camera looks. Free: the body turns into its movement.
	Movement->bOrientRotationToMovement = !bFocusHeld;
	Movement->bUseControllerDesiredRotation = bFocusHeld;
	Movement->RotationRate = bFocusHeld ? FRotator(0.f, 720.f, 0.f) : FRotator(0.f, 460.f, 0.f);
}

void AShadowlineCharacter::UpdateCameraRig(float DeltaSeconds)
{
	if (!CameraBoom || !FollowCamera)
	{
		return;
	}

	const UShadowlineCharacterMovement* Movement = GetShadowlineMovement();
	const bool bSprinting = Movement
		&& Movement->GetGait() == EShadowlineGait::Sprint
		&& Movement->GetStance() == EShadowlineStance::Standing
		&& Movement->Velocity.Size2D() > 50.f;

	// Target boom length.
	float TargetLength = BoomLengthNormal;
	if (bFocusHeld)
	{
		TargetLength = BoomLengthFocused;
	}
	else if (bSprinting)
	{
		TargetLength = BoomLengthSprint;
	}
	TargetLength = FMath::Max(50.f, TargetLength + BoomDistanceAdjust);

	// Target pivot height, from stance.
	float TargetHeight = PivotHeightStanding;
	switch (GetStance())
	{
	case EShadowlineStance::Crouched:	TargetHeight = PivotHeightCrouched; break;
	case EShadowlineStance::Prone:		TargetHeight = PivotHeightProne; break;
	default:							break;
	}

	// Target shoulder offset. Prone pulls the camera to the centre line, where an
	// over-the-shoulder offset would otherwise bury it in the ground.
	const float ShoulderMagnitude = bFocusHeld ? ShoulderOffsetFocused : ShoulderOffsetNormal;
	const float TargetShoulder = (GetStance() == EShadowlineStance::Prone)
		? ShoulderMagnitude * 0.35f * ShoulderSign
		: ShoulderMagnitude * ShoulderSign;

	float TargetFOV = FieldOfViewNormal;
	if (bFocusHeld)
	{
		TargetFOV = FieldOfViewFocused;
	}
	else if (bSprinting)
	{
		TargetFOV = FieldOfViewSprint;
	}

	const FVector TargetOffset(0.f, TargetShoulder, TargetHeight);

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetLength, DeltaSeconds, CameraBlendSpeed);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetOffset, DeltaSeconds, CameraBlendSpeed);
	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaSeconds, CameraBlendSpeed));
}

// ---------------------------------------------------------------------------------------
// Traversal
// ---------------------------------------------------------------------------------------

FShadowlineTraversalProbe AShadowlineCharacter::ProbeTraversal() const
{
	FShadowlineTraversalProbe Result;

	const UWorld* World = GetWorld();
	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	const UShadowlineCharacterMovement* Movement = GetShadowlineMovement();
	if (!World || !Capsule || !Movement)
	{
		return Result;
	}

	// Traversal is a grounded, standing-or-crouched action.
	if (Movement->IsFalling() || Movement->IsSwimming() || GetStance() == EShadowlineStance::Prone)
	{
		return Result;
	}

	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float Radius = Capsule->GetScaledCapsuleRadius();
	const FVector Feet = GetActorLocation() - FVector(0.f, 0.f, HalfHeight);
	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	if (Forward.IsNearlyZero())
	{
		return Result;
	}

	FCollisionQueryParams Params(TEXT("ShadowlineTraversalProbe"), /*bTraceComplex=*/false, this);
	Params.AddIgnoredActor(this);

	// 1. Find the obstacle face, at a height just above the step-over range.
	const FVector FaceStart = Feet + FVector(0.f, 0.f, MinTraversalHeight * 0.5f);
	const FVector FaceEnd = FaceStart + Forward * TraversalProbeDistance;

	FHitResult FaceHit;
	if (!World->LineTraceSingleByChannel(FaceHit, FaceStart, FaceEnd, ECC_Visibility, Params))
	{
		return Result;
	}

	// A near-horizontal surface is a floor or a slope, not something to climb over.
	if (FMath::Abs(FaceHit.ImpactNormal.Z) > 0.5f)
	{
		return Result;
	}

	Result.FaceNormal = FaceHit.ImpactNormal;

	// 2. Find the obstacle's top surface, just past the face.
	const FVector TopProbeXY = FaceHit.ImpactPoint + Forward * 6.f;
	const FVector TopStart(TopProbeXY.X, TopProbeXY.Y, Feet.Z + MaxTraversalHeight + 40.f);
	const FVector TopEnd(TopProbeXY.X, TopProbeXY.Y, Feet.Z);

	FHitResult TopHit;
	if (!World->LineTraceSingleByChannel(TopHit, TopStart, TopEnd, ECC_Visibility, Params))
	{
		// Nothing above: the face belongs to something taller than MaxTraversalHeight.
		return Result;
	}

	// The top must be walkable, or there is nowhere to land.
	if (TopHit.ImpactNormal.Z < 0.7f)
	{
		return Result;
	}

	const float Height = TopHit.ImpactPoint.Z - Feet.Z;
	if (Height < MinTraversalHeight || Height > MaxTraversalHeight)
	{
		return Result;
	}

	Result.ObstacleHeight = Height;
	Result.EdgeLocation = FVector(FaceHit.ImpactPoint.X, FaceHit.ImpactPoint.Y, TopHit.ImpactPoint.Z);

	// 3. Measure the obstacle's depth by walking the top surface forward.
	//    The first sample with no top surface under it is past the far edge.
	const float DepthStep = 15.f;
	const float MaxDepthSample = VaultMaxDepth + 60.f;
	float MeasuredDepth = MaxDepthSample;

	for (float Depth = DepthStep; Depth <= MaxDepthSample; Depth += DepthStep)
	{
		const FVector SampleXY = Result.EdgeLocation + Forward * Depth;
		const FVector SampleStart(SampleXY.X, SampleXY.Y, Result.EdgeLocation.Z + 20.f);
		const FVector SampleEnd(SampleXY.X, SampleXY.Y, Result.EdgeLocation.Z - 20.f);

		FHitResult SampleHit;
		if (!World->LineTraceSingleByChannel(SampleHit, SampleStart, SampleEnd, ECC_Visibility, Params))
		{
			MeasuredDepth = Depth;
			break;
		}
	}
	Result.ObstacleDepth = MeasuredDepth;

	// 4. Work out where the character ends up, and check it fits.
	const bool bVault = MeasuredDepth <= VaultMaxDepth;

	FVector LandingFeet;
	if (bVault)
	{
		// Over and down the far side: drop onto whatever is beyond the obstacle.
		const FVector BeyondXY = Result.EdgeLocation + Forward * (MeasuredDepth + Radius + 10.f);
		const FVector GroundStart(BeyondXY.X, BeyondXY.Y, Result.EdgeLocation.Z + 20.f);
		const FVector GroundEnd(BeyondXY.X, BeyondXY.Y, Result.EdgeLocation.Z - MaxTraversalHeight - 200.f);

		FHitResult GroundHit;
		if (!World->LineTraceSingleByChannel(GroundHit, GroundStart, GroundEnd, ECC_Visibility, Params))
		{
			// No floor on the far side within a survivable drop.
			return Result;
		}
		LandingFeet = GroundHit.ImpactPoint;
	}
	else
	{
		// Up onto the top surface.
		LandingFeet = Result.EdgeLocation + Forward * (Radius + 12.f);
	}

	// The capsule must fit at the landing spot. Crouched height is enough: traversals
	// resolve into a crouched pose and the stance is raised afterwards if there is room.
	const float LandingHalfHeight = Movement->GetHalfHeightForStance(EShadowlineStance::Crouched);
	const FVector LandingCentre = LandingFeet + FVector(0.f, 0.f, LandingHalfHeight + 2.f);

	if (World->OverlapBlockingTestByChannel(
			LandingCentre,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeCapsule(Radius * 0.9f, LandingHalfHeight),
			Params))
	{
		return Result;
	}

	Result.LandingLocation = LandingCentre;
	Result.bValid = true;
	return Result;
}

bool AShadowlineCharacter::TryTraversal()
{
	if (bTraversalInProgress)
	{
		return true;	// already traversing; swallow the input
	}

	const FShadowlineTraversalProbe& Probe = CachedProbe;
	if (!Probe.bValid)
	{
		return false;
	}

	UShadowlineCharacterMovement* Movement = GetShadowlineMovement();
	if (!Movement)
	{
		return false;
	}

	const bool bVault = Probe.ObstacleDepth <= VaultMaxDepth;
	UAnimMontage* Montage = bVault ? VaultMontage : MantleMontage;

	if (Montage)
	{
		// Warp the montage's root motion onto the real obstacle so the hands and feet
		// land on the geometry instead of on where the animator imagined it.
		if (MotionWarping)
		{
			const FRotator FaceRotation = (-Probe.FaceNormal).GetSafeNormal2D().Rotation();
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
				TraversalWarpTargetName,
				Probe.LandingLocation,
				FaceRotation);
		}

		const float MontageLength = PlayAnimMontage(Montage);
		if (MontageLength > 0.f)
		{
			// Root motion drives the traversal. Flying keeps the movement component from
			// fighting it with gravity or floor sweeps; OnTraversalMontageEnded puts the
			// character back on the ground when the montage finishes OR is interrupted,
			// so an interrupted traversal can never leave the character flying.
			bTraversalInProgress = true;
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(MOVE_Flying);

			if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &AShadowlineCharacter::OnTraversalMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
			}
			else
			{
				// No anim instance to tell us when it ends: do not take over movement.
				bTraversalInProgress = false;
				Movement->SetMovementMode(MOVE_Walking);
			}

			return true;
		}
	}

	// No montage assigned yet: launch the character over the obstacle so the level is
	// still traversable for testing. This is deliberately unanimated placeholder motion
	// and is tracked in UNFINISHED_FEATURES.md.
	const FVector ToLanding = Probe.LandingLocation - GetActorLocation();
	const float HorizontalDistance = ToLanding.Size2D();
	const float VerticalRise = FMath::Max(ToLanding.Z, 0.f);

	// Enough upward speed to clear the edge, plus forward speed to cross the obstacle.
	const float UpSpeed = FMath::Sqrt(2.f * 980.f * FMath::Max(VerticalRise + 55.f, 60.f));
	const float TimeToApex = UpSpeed / 980.f;
	const float ForwardSpeed = (TimeToApex > KINDA_SMALL_NUMBER)
		? FMath::Min(HorizontalDistance / TimeToApex, 650.f)
		: 250.f;

	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	LaunchCharacter(Forward * ForwardSpeed + FVector(0.f, 0.f, UpSpeed), true, true);

	UE_LOG(LogShadowline, Verbose,
		TEXT("Traversal fallback used (no %s montage assigned): height %.0f, depth %.0f"),
		bVault ? TEXT("vault") : TEXT("mantle"), Probe.ObstacleHeight, Probe.ObstacleDepth);

	return true;
}

void AShadowlineCharacter::OnTraversalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bTraversalInProgress)
	{
		return;
	}

	bTraversalInProgress = false;

	if (UShadowlineCharacterMovement* Movement = GetShadowlineMovement())
	{
		// Falling rather than walking: the movement component finds the floor itself,
		// which is correct whether the traversal ended on top of the obstacle or in mid-air
		// on the far side of a vault.
		Movement->SetMovementMode(MOVE_Falling);
	}

	if (bInterrupted)
	{
		UE_LOG(LogShadowline, Verbose, TEXT("Traversal montage %s was interrupted."),
			Montage ? *Montage->GetName() : TEXT("<null>"));
	}
}
