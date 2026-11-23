// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineWorldAtmosphere.generated.h"

class USkyAtmosphereComponent;
class UDirectionalLightComponent;
class USkyLightComponent;
class UVolumetricCloudComponent;
class UExponentialHeightFogComponent;
class UNiagaraComponent;
class UAudioComponent;
class UMaterialParameterCollection;
class UShadowlineRegionDefinition;

/**
 * The single actor that owns Aurelia's sky.
 *
 * One instance is placed per persistent level. It advances the clock, rolls and blends
 * weather, drives the sun, moon, sky light, clouds and height fog, publishes weather values
 * to a material parameter collection (so landscape, foliage and prop materials can react to
 * wetness, snow and wind), and crossfades the regional ambience bed.
 *
 * Region switching is data-driven: a region volume calls SetRegion, and the actor blends
 * toward that region's UShadowlineRegionDefinition. There is no per-region lighting rig.
 *
 * Required setup in the level:
 *  - assign RegionDefinitions for every region the level contains
 *  - assign WeatherParameters to the project's material parameter collection
 *  - the actor's own sky components replace any Sky Atmosphere / fog actors in the level
 */
UCLASS(Blueprintable)
class OPERATIONSHADOWLINE_API AShadowlineWorldAtmosphere : public AActor
{
	GENERATED_BODY()

public:
	AShadowlineWorldAtmosphere();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Blends the sky toward another region's definition. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Atmosphere")
	void SetRegion(EShadowlineRegion NewRegion);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Atmosphere")
	EShadowlineRegion GetRegion() const { return ActiveRegion; }

	/** Starts a blend to a specific weather state. BlendSeconds of 0 snaps. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Atmosphere")
	void SetWeather(EShadowlineWeather NewWeather, float BlendSeconds = 20.f);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Atmosphere")
	EShadowlineWeather GetWeather() const { return CurrentWeather; }

	/** 0..24. Setting it applies the new sun position on the same frame. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Atmosphere")
	void SetTimeOfDay(float NewTimeOfDay);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Atmosphere")
	float GetTimeOfDay() const { return TimeOfDay; }

	UFUNCTION(BlueprintPure, Category = "Shadowline|Atmosphere")
	bool IsNight() const;

	/** 0..1 daylight factor: 1 in full day, 0 at night, ramping through twilight. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Atmosphere")
	float GetDayAlpha() const { return DayAlpha; }

	/** 0..1 how heavy the current weather is. Feeds fog, clouds and precipitation. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Atmosphere")
	float GetWeatherSeverity() const { return BlendedSeverity; }

	/** 0..1 surface wetness. Rises in rain, dries slowly afterwards. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Atmosphere")
	float GetWetness() const { return Wetness; }

	/** Severity constant for a weather state, 0 (clear) to 1 (storm). */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Atmosphere")
	static float GetSeverityForWeather(EShadowlineWeather InWeather);

protected:
	virtual void BeginPlay() override;

	void ApplyTimeOfDay();
	void ApplyRegionAndWeather();
	void PublishMaterialParameters();
	void UpdateAmbience();

	/** The definition for ActiveRegion, or nullptr when none is assigned. */
	const UShadowlineRegionDefinition* GetActiveDefinition() const;

protected:
	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> SunLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> MoonLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UVolumetricCloudComponent> VolumetricClouds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UExponentialHeightFogComponent> HeightFog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> Precipitation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> Ambience;

	// --- Data ---

	/** One definition per region present in this level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Atmosphere")
	TMap<EShadowlineRegion, TObjectPtr<UShadowlineRegionDefinition>> RegionDefinitions;

	/** Collection that landscape, foliage and prop materials read weather values from. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Atmosphere")
	TObjectPtr<UMaterialParameterCollection> WeatherParameters;

	// --- Clock ---

	/** Starting hour, 0..24. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Clock", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float TimeOfDay;

	/** In-game minutes that pass per real second. 1.0 gives a 24-minute day. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Clock", meta = (ClampMin = "0.0"))
	float MinutesPerSecond;

	/** Compass direction the sun travels along, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Clock", meta = (ClampMin = "-360.0", ClampMax = "360.0"))
	float SunAzimuth;

	/** Hour the sun crosses the horizon at dawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Clock", meta = (ClampMin = "0.0", ClampMax = "12.0"))
	float SunriseHour;

	/** Hour the sun crosses the horizon at dusk. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Clock", meta = (ClampMin = "12.0", ClampMax = "24.0"))
	float SunsetHour;

	// --- Weather ---

	/** Weather state the actor starts in. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Weather")
	EShadowlineWeather StartingWeather;

	/** When true, weather re-rolls from the region's table on its own. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Weather")
	bool bAutoWeather;

	/** Seconds a surface takes to go from soaked to dry once rain stops. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Weather", meta = (ClampMin = "1.0"))
	float DryingSeconds;

	/** Seconds a surface takes to soak through in full rain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Weather", meta = (ClampMin = "1.0"))
	float WettingSeconds;

	/** Hour the ambience bed crossfades from night to day, and back. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadowline|Audio", meta = (ClampMin = "0.1"))
	float AmbienceCrossfadeSeconds;

private:
	EShadowlineRegion ActiveRegion;

	EShadowlineWeather CurrentWeather;
	EShadowlineWeather TargetWeather;

	/** 0..1 progress of the blend from CurrentWeather to TargetWeather. */
	float WeatherBlendAlpha;
	float WeatherBlendDuration;

	/** Severity interpolated across the current blend. */
	float BlendedSeverity;

	float Wetness;

	/**
	 * Daylight factor, recomputed once per ApplyTimeOfDay from the sun's elevation.
	 * Cached so lighting, material parameters and ambience all agree on what time it is.
	 */
	float DayAlpha;

	/** In-game minutes until the next automatic weather roll. */
	float MinutesUntilWeatherRoll;

	/** Tracks which ambience bed is playing so the crossfade only fires on a change. */
	bool bAmbienceIsNight;
	bool bAmbienceInitialised;
};
