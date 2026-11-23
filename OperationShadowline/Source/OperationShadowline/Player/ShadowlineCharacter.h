// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UMotionWarpingComponent;
class UShadowlineCharacterMovement;
class UShadowlineInputConfig;
class UAnimMontage;

/**
 * Player character for Operation Shadowline.
 *
 * Owns the third-person camera rig, the stance/gait state machine (delegating the actual
 * movement maths to UShadowlineCharacterMovement) and the traversal probe that decides
 * whether the jump input becomes a jump, a vault or a mantle.
 *
 * Intended use: derive a Blueprint (BP_ShadowlineCharacter) from this class, assign the
 * skeletal mesh, animation Blueprint, input config and traversal montages there.
 */
UCLASS(Blueprintable)
class OPERATIONSHADOWLINE_API AShadowlineCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShadowlineCharacter(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor / APawn interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	//~ End AActor / APawn interface

	UFUNCTION(BlueprintPure, Category = "Shadowline|Camera")
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure, Category = "Shadowline|Camera")
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UFUNCTION(BlueprintPure, Category = "Shadowline|Movement")
	UShadowlineCharacterMovement* GetShadowlineMovement() const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Movement")
	EShadowlineStance GetStance() const;

	UFUNCTION(BlueprintPure, Category = "Shadowline|Camera")
	bool IsFocusing() const { return bFocusHeld; }

	/** Cycles standing -> crouched -> standing. Blocked while airborne or swimming. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Movement")
	void ToggleCrouchStance();

	/** Cycles standing/crouched -> prone -> crouched. Blocked while airborne or swimming. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Movement")
	void ToggleProneStance();

	/**
	 * Casts for a traversable obstacle directly ahead.
	 * Pure query: it moves nothing and is safe to call every frame for UI prompts.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Traversal")
	FShadowlineTraversalProbe ProbeTraversal() const;

	/** The probe result cached by the most recent Tick. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Traversal")
	const FShadowlineTraversalProbe& GetCachedTraversalProbe() const { return CachedProbe; }

protected:
	virtual void BeginPlay() override;

	// --- Input handlers ---
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_JumpOrTraverse();
	void Input_JumpReleased();
	void Input_SprintStart();
	void Input_SprintStop();
	void Input_SlowWalkStart();
	void Input_SlowWalkStop();
	void Input_SwapShoulder();
	void Input_FocusStart();
	void Input_FocusStop();
	void Input_CameraDistance(const FInputActionValue& Value);

	/**
	 * Attempts a vault or mantle using the cached probe.
	 * Returns true only when a traversal was actually started.
	 */
	bool TryTraversal();

	/** Drives the camera rig toward the targets implied by stance, focus and gait. */
	void UpdateCameraRig(float DeltaSeconds);

	/** Applies the rotation mode for the current focus state. */
	void UpdateRotationMode();

	/** Recomputes gait from the currently held sprint/slow modifiers. */
	void RefreshGait();

	/** Restores normal movement when a traversal montage finishes or is interrupted. */
	UFUNCTION()
	void OnTraversalMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shadowline|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shadowline|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** Supplies warp targets so traversal montages land on the real obstacle edge. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shadowline|Traversal")
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	/** Enhanced Input assets. Assign on the Blueprint; input is inert until it is set. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shadowline|Input")
	TObjectPtr<UShadowlineInputConfig> InputConfig;

	// --- Camera tuning ---

	/** Boom length while moving normally. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera", meta = (ClampMin = "50.0"))
	float BoomLengthNormal;

	/** Boom length while the focus input is held. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera", meta = (ClampMin = "50.0"))
	float BoomLengthFocused;

	/** Boom length while sprinting. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera", meta = (ClampMin = "50.0"))
	float BoomLengthSprint;

	/** Player-adjustable offset added to the boom length, clamped to +/- BoomDistanceAdjustRange. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera")
	float BoomDistanceAdjustRange;

	/** Sideways camera offset. Sign flips on shoulder swap. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera")
	float ShoulderOffsetNormal;

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera")
	float ShoulderOffsetFocused;

	/** Camera pivot height above the capsule centre, per stance. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera")
	float PivotHeightStanding;

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera")
	float PivotHeightCrouched;

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera")
	float PivotHeightProne;

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera", meta = (ClampMin = "30.0", ClampMax = "130.0"))
	float FieldOfViewNormal;

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera", meta = (ClampMin = "30.0", ClampMax = "130.0"))
	float FieldOfViewFocused;

	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera", meta = (ClampMin = "30.0", ClampMax = "130.0"))
	float FieldOfViewSprint;

	/** Blend speed for every camera rig value, in units per second of interpolation alpha. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Camera", meta = (ClampMin = "0.1"))
	float CameraBlendSpeed;

	// --- Traversal tuning ---

	/** Obstacles below this are stepped over by the movement component instead. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Traversal", meta = (ClampMin = "0.0"))
	float MinTraversalHeight;

	/** Obstacles above this cannot be traversed from the ground. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Traversal", meta = (ClampMin = "0.0"))
	float MaxTraversalHeight;

	/** How far ahead the probe looks for an obstacle face. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Traversal", meta = (ClampMin = "0.0"))
	float TraversalProbeDistance;

	/** Obstacles thinner than this are vaulted over; thicker ones are mantled onto. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Traversal", meta = (ClampMin = "0.0"))
	float VaultMaxDepth;

	/**
	 * Root-motion montage for vaulting a thin obstacle.
	 * Until a montage is assigned, TryTraversal falls back to a physical launch so the
	 * geometry is still traversable; the fallback is deliberately unanimated.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Traversal")
	TObjectPtr<UAnimMontage> VaultMontage;

	/** Root-motion montage for mantling onto a thick obstacle. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Traversal")
	TObjectPtr<UAnimMontage> MantleMontage;

	/** Name of the warp target the traversal montages' Motion Warping notifies reference. */
	UPROPERTY(EditDefaultsOnly, Category = "Shadowline|Traversal")
	FName TraversalWarpTargetName;

private:
	/** Set while the corresponding input is held. */
	bool bSprintHeld;
	bool bSlowWalkHeld;
	bool bFocusHeld;

	/** -1 = left shoulder, +1 = right shoulder. */
	float ShoulderSign;

	/** Player camera-distance adjustment, clamped to +/- BoomDistanceAdjustRange. */
	float BoomDistanceAdjust;

	/** Refreshed every Tick so UI and the jump handler share one query per frame. */
	FShadowlineTraversalProbe CachedProbe;

	/** True between starting a traversal montage and its end delegate firing. */
	bool bTraversalInProgress;
};
