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

/** Mission categories. Drives the map icon, the operations filter and the reward shape. */
UENUM(BlueprintType)
enum class EShadowlineMissionType : uint8
{
	Reconnaissance	UMETA(DisplayName = "Reconnaissance"),
	Rescue			UMETA(DisplayName = "Rescue"),
	Investigation	UMETA(DisplayName = "Investigation"),
	SupplyRecovery	UMETA(DisplayName = "Supply Recovery"),
	Search			UMETA(DisplayName = "Search and Discovery"),
	Puzzle			UMETA(DisplayName = "Environmental Puzzle"),
	/** A named Vantrell overseer: expose, record or extract them. Never lethal. */
	Confrontation	UMETA(DisplayName = "Confrontation"),
	/** The regional director, unlocked once a region's provinces are cleared. */
	Director		UMETA(DisplayName = "Regional Director"),
	/** The final antagonist at Meridian Works. */
	Finale			UMETA(DisplayName = "Finale")
};

/** Named difficulty presets. Custom exposes every knob in FShadowlineDifficultyKnobs. */
UENUM(BlueprintType)
enum class EShadowlineDifficulty : uint8
{
	/** Story-forward. Generous detection, every assist on. */
	Field		UMETA(DisplayName = "Field"),
	Standard	UMETA(DisplayName = "Standard"),
	/** Faster detection, fewer assists, no marker on the target. */
	Veteran		UMETA(DisplayName = "Veteran"),
	/** No HUD, no map markers, a detection cascade with no second chance. */
	Blackout	UMETA(DisplayName = "Blackout"),
	Custom		UMETA(DisplayName = "Custom")
};

/** How far a province has progressed. Drives the map and the campaign tree. */
UENUM(BlueprintType)
enum class EShadowlineProvinceState : uint8
{
	/** Not yet reachable: its prerequisites are unmet. */
	Locked		UMETA(DisplayName = "Locked"),
	/** Reachable, nothing done. */
	Open		UMETA(DisplayName = "Open"),
	/** Some missions complete. */
	Active		UMETA(DisplayName = "Active"),
	/** Every required mission complete, overseer resolved. */
	Cleared		UMETA(DisplayName = "Cleared")
};

/**
 * One objective line on a mission.
 * Optional objectives are how the investigation missions reward curiosity, so the
 * distinction is data rather than presentation.
 */
USTRUCT(BlueprintType)
struct FShadowlineObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FText Description;

	/** False objectives are shown differently and never block completion. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	bool bRequired = true;

	/** Hidden until a prior objective completes, for mid-mission reveals. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	bool bHiddenUntilReached = false;

	/** Tag the mission logic raises when this objective is satisfied. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FName CompletionEvent;
};

/**
 * Every knob a difficulty preset sets.
 *
 * Kept as one struct so a preset, a custom configuration and a per-mission override are
 * all the same type, and so that adding a knob cannot leave a preset half-defined.
 * Multipliers are relative to Standard, which is 1.0 throughout.
 */
USTRUCT(BlueprintType)
struct FShadowlineDifficultyKnobs
{
	GENERATED_BODY()

	// --- Detection ---

	/** How fast an NPC's suspicion rises while the player is visible. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection", meta = (ClampMin = "0.1", ClampMax = "4.0"))
	float SuspicionRate = 1.f;

	/** How fast suspicion decays once the player breaks line of sight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection", meta = (ClampMin = "0.1", ClampMax = "4.0"))
	float SuspicionDecayRate = 1.f;

	/** Multiplier on NPC sight range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection", meta = (ClampMin = "0.25", ClampMax = "2.5"))
	float SightRange = 1.f;

	/** Multiplier on how far NPCs hear the player's movement noise. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection", meta = (ClampMin = "0.25", ClampMax = "2.5"))
	float HearingRange = 1.f;

	/** How long a search persists after contact is lost, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection", meta = (ClampMin = "5.0", ClampMax = "600.0"))
	float SearchPersistenceSeconds = 60.f;

	/** True when a detection anywhere alerts the whole site rather than the nearest patrol. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
	bool bSiteWideAlerts = false;

	// --- Opposition ---

	/** Multiplier on the number of NPCs placed at a site. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Opposition", meta = (ClampMin = "0.4", ClampMax = "2.0"))
	float PatrolDensity = 1.f;

	/** Multiplier on how far NPCs relay a sighting to each other. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Opposition", meta = (ClampMin = "0.25", ClampMax = "3.0"))
	float CommunicationRange = 1.f;

	/** True when night patrols carry lights and thermal optics. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Opposition")
	bool bNightOpticsEnabled = false;

	// --- Assists ---

	/** Marks the mission target on the map and compass. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assists")
	bool bTargetMarkers = true;

	/** Shows discovered NPCs as map and world markers after tagging. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assists")
	bool bTaggingEnabled = true;

	/** Shows the on-screen objective tracker. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assists")
	bool bObjectiveTracker = true;

	/** Shows the concealment readout. With it off, the player reads cover from the world. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assists")
	bool bConcealmentReadout = true;

	/** Shows the compass bearing strip. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assists")
	bool bCompass = true;

	/** Multiplier on the hand drone's range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assists", meta = (ClampMin = "0.25", ClampMax = "2.0"))
	float DroneRange = 1.f;

	// --- Consequences ---

	/** Companions recover on their own after this long. Zero means they need reviving. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequences", meta = (ClampMin = "0.0", ClampMax = "300.0"))
	float CompanionSelfRecoverySeconds = 45.f;

	/**
	 * True when a downed companion is unavailable for the rest of the act rather than
	 * revivable. Threshold has no deaths: an unavailable companion has been extracted.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequences")
	bool bCompanionLossPersists = false;

	/** Multiplier on weather severity, which affects both concealment and visibility. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequences", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float WeatherSeverity = 1.f;

	/** Returns the knob set for a named preset. Custom returns Standard as its start point. */
	static FShadowlineDifficultyKnobs FromPreset(EShadowlineDifficulty Preset);
};

/** Runtime state of one mission. Derived from progress, never stored directly. */
UENUM(BlueprintType)
enum class EShadowlineMissionState : uint8
{
	/** Its prerequisites, or its province's, are unmet. */
	Locked		UMETA(DisplayName = "Locked"),
	/** Startable, nothing done. */
	Available	UMETA(DisplayName = "Available"),
	/** At least one objective satisfied, not all required ones. */
	Active		UMETA(DisplayName = "In Progress"),
	Complete	UMETA(DisplayName = "Complete")
};

/** What kind of thing the player just found. Feeds the survey percentage. */
UENUM(BlueprintType)
enum class EShadowlineDiscovery : uint8
{
	Document	UMETA(DisplayName = "Document"),
	Landmark	UMETA(DisplayName = "Landmark"),
	Cache		UMETA(DisplayName = "Supply Cache")
};

/** Stored progress for one mission. */
USTRUCT(BlueprintType)
struct FShadowlineMissionProgress
{
	GENERATED_BODY()

	/** Completion events already raised, matching FShadowlineObjective::CompletionEvent. */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	TArray<FName> CompletedObjectives;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	bool bComplete = false;

	/** True when the mission was finished without ever being detected. */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	bool bCompletedUndetected = false;

	/** Set the first time the player is detected during this mission. */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	bool bDetectedThisAttempt = false;
};

/** Stored progress for one province. */
USTRUCT(BlueprintType)
struct FShadowlineProvinceProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress", meta = (ClampMin = "0"))
	int32 DocumentsFound = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress", meta = (ClampMin = "0"))
	int32 LandmarksFound = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress", meta = (ClampMin = "0"))
	int32 CachesFound = 0;

	/** True once the province's named overseer has been dealt with. */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	bool bOverseerResolved = false;
};

/**
 * The whole campaign's stored state.
 *
 * Deliberately only what cannot be derived: everything else (province state, mission state,
 * survey percentages, whether the finale is open) is computed by
 * UShadowlineCampaignSubsystem from this plus the definition assets. Storing derived state
 * is how save games and game logic drift apart.
 */
USTRUCT(BlueprintType)
struct FShadowlineCampaignProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	TMap<FName, FShadowlineProvinceProgress> Provinces;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	TMap<FName, FShadowlineMissionProgress> Missions;

	/** Regions whose director has been resolved. The finale needs all four. */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	TSet<EShadowlineRegion> ResolvedDirectors;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	EShadowlineDifficulty Difficulty = EShadowlineDifficulty::Standard;

	/** Only meaningful when Difficulty is Custom. */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	FShadowlineDifficultyKnobs CustomKnobs;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress", meta = (ClampMin = "1"))
	int32 Act = 1;

	/** In-game days elapsed since landfall. */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress", meta = (ClampMin = "0"))
	int32 Day = 1;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	float TimeOfDay = 9.5f;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	EShadowlineWeather Weather = EShadowlineWeather::Clear;

	/** Province the team is currently in. */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress")
	FName CurrentProvinceId;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Progress", meta = (ClampMin = "0.0"))
	float PlaytimeSeconds = 0.f;
};
