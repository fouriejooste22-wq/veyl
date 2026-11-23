// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShadowlineTypes.generated.h"

/** Body posture. Drives capsule size, movement speed, noise output and camera framing. */
UENUM(BlueprintType)
enum class EShadowlineStance : uint8
{
	Standing	UMETA(DisplayName = "Standing"),
	Crouched	UMETA(DisplayName = "Crouched"),
	Prone		UMETA(DisplayName = "Prone")
};

/** Intended locomotion speed within the current stance. */
UENUM(BlueprintType)
enum class EShadowlineGait : uint8
{
	/** Deliberate, quietest movement. */
	Slow		UMETA(DisplayName = "Slow"),
	/** Default travel pace. */
	Normal		UMETA(DisplayName = "Normal"),
	/** Fast, loudest movement. Standing only. */
	Sprint		UMETA(DisplayName = "Sprint")
};

/** The five regions of Aurelia. Used by the atmosphere, audio and map systems. */
UENUM(BlueprintType)
enum class EShadowlineRegion : uint8
{
	None				UMETA(DisplayName = "Unassigned"),
	NorthernMountains	UMETA(DisplayName = "Kolda Range"),
	SouthernJungle		UMETA(DisplayName = "Vaelmire Basin"),
	WesternDesert		UMETA(DisplayName = "Ashen Flats"),
	EasternCoast		UMETA(DisplayName = "Siren Coast"),
	CentralIndustrial	UMETA(DisplayName = "Meridian Works")
};

/** Weather states the atmosphere subsystem can blend between. */
UENUM(BlueprintType)
enum class EShadowlineWeather : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Overcast	UMETA(DisplayName = "Overcast"),
	Fog			UMETA(DisplayName = "Fog"),
	Rain		UMETA(DisplayName = "Rain"),
	Storm		UMETA(DisplayName = "Storm"),
	Snow		UMETA(DisplayName = "Snow"),
	DustStorm	UMETA(DisplayName = "Dust Storm")
};

/** Rendering quality presets exposed in the settings menu. */
UENUM(BlueprintType)
enum class EShadowlineQualityPreset : uint8
{
	Low		UMETA(DisplayName = "Low"),
	Medium	UMETA(DisplayName = "Medium"),
	High	UMETA(DisplayName = "High"),
	Ultra	UMETA(DisplayName = "Ultra"),
	Custom	UMETA(DisplayName = "Custom")
};

/**
 * Result of a traversal probe in front of the character.
 * Produced by AShadowlineCharacter::ProbeTraversal and consumed by the vault/mantle action.
 */
USTRUCT(BlueprintType)
struct FShadowlineTraversalProbe
{
	GENERATED_BODY()

	/** True when a traversable obstacle was found within the configured height band. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	bool bValid = false;

	/** World location of the obstacle's near edge, at its top surface. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	FVector EdgeLocation = FVector::ZeroVector;

	/** World location the character should end up at, past the obstacle. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	FVector LandingLocation = FVector::ZeroVector;

	/** Height of the obstacle top above the character's feet, in centimetres. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float ObstacleHeight = 0.f;

	/** Horizontal depth of the obstacle. Thin obstacles are vaulted, thick ones are mantled onto. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float ObstacleDepth = 0.f;

	/** Outward-facing normal of the obstacle face the character approached. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	FVector FaceNormal = FVector::ZeroVector;
};
