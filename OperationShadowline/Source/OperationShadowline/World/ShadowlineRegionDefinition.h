// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineRegionDefinition.generated.h"

class USoundBase;
class UMaterialParameterCollection;
class UCurveLinearColor;
class UNiagaraSystem;

/** One weather state and how likely it is in a region. */
USTRUCT(BlueprintType)
struct FShadowlineWeatherChance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	EShadowlineWeather Weather = EShadowlineWeather::Clear;

	/** Relative weight. Weights are normalised across the region's list, so any scale works. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};

/**
 * Everything that makes one region of Aurelia feel different from the others:
 * its light, its air, its weather odds and its ambience.
 *
 * One asset per region lives in Content/Shadowline/World/Regions. The atmosphere actor
 * reads the definition for whichever region the player is standing in and blends toward it,
 * which is what keeps the five regions distinct without five separate lighting setups.
 */
UCLASS(BlueprintType)
class OPERATIONSHADOWLINE_API UShadowlineRegionDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	EShadowlineRegion Region = EShadowlineRegion::None;

	/** Shown on the world map and in the region-entered title card. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Flavour text for the map's region panel. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = "true"))
	FText Description;

	// --- Light and air ---

	/** Sun colour across the day, sampled by time of day normalised to 0..1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atmosphere")
	TObjectPtr<UCurveLinearColor> SunColourOverDay;

	/** Peak sun intensity at local noon, in lux. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0"))
	float NoonSunIntensity = 75000.f;

	/** Baseline height-fog density in clear weather. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0"))
	float ClearFogDensity = 0.015f;

	/** Height-fog density the region reaches in its heaviest weather. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0"))
	float HeavyFogDensity = 0.25f;

	/** Fog inscattering colour. Carries most of a region's colour identity. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atmosphere")
	FLinearColor FogInscatteringColour = FLinearColor(0.45f, 0.58f, 0.72f, 1.f);

	/** Cloud coverage in clear weather, 0..1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ClearCloudCoverage = 0.25f;

	/** Rayleigh/aerosol tint for the region's SkyAtmosphere. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atmosphere")
	FLinearColor AtmosphereTint = FLinearColor(0.18f, 0.33f, 0.62f, 1.f);

	// --- Weather ---

	/** Weather states this region can roll, with relative weights. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather")
	TArray<FShadowlineWeatherChance> WeatherTable;

	/** Average length of one weather state in this region, in in-game minutes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "1.0"))
	float MeanWeatherDurationMinutes = 45.f;

	/** Base wind speed, driving foliage motion and the wind audio bed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "0.0"))
	float BaseWindSpeed = 1.f;

	// --- Ambience ---

	/** Looping daytime ambience bed. A MetaSound source is expected here. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> DayAmbience;

	/** Looping night-time ambience bed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> NightAmbience;

	/** Precipitation / airborne-particulate effect used in this region's heavy weather. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<UNiagaraSystem> PrecipitationEffect;

	/** Returns a weather state drawn from WeatherTable, or Clear when the table is empty. */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	EShadowlineWeather RollWeather() const;
};
