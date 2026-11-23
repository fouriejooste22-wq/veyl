// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Environment/ShadowlineWorldAtmosphere.h"

#include "OperationShadowline.h"
#include "World/ShadowlineRegionDefinition.h"

#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "Curves/CurveLinearColor.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Engine/World.h"

namespace ShadowlineAtmosphereParams
{
	static const FName CloudCoverage(TEXT("CloudCoverage"));
	static const FName RainIntensity(TEXT("RainIntensity"));
	static const FName SurfaceWetness(TEXT("Wetness"));
	static const FName SnowCoverage(TEXT("SnowCoverage"));
	static const FName DustIntensity(TEXT("DustIntensity"));
	static const FName WindSpeed(TEXT("WindSpeed"));
	static const FName NightAlpha(TEXT("NightAlpha"));
}

AShadowlineWorldAtmosphere::AShadowlineWorldAtmosphere()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(Root);

	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(Root);
	SunLight->SetAtmosphereSunLight(true);
	SunLight->SetDynamicShadowCascades(4);
	SunLight->SetDynamicShadowDistanceMovableLight(14000.f);
	SunLight->SetIntensity(75000.f);
	SunLight->SetUseTemperature(true);
	SunLight->SetTemperature(5900.f);
	SunLight->SetMobility(EComponentMobility::Movable);
	SunLight->bCastVolumetricShadow = true;
	SunLight->SetLightSourceAngle(0.53f);		// real angular diameter of the sun: sharp but not hard shadows

	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLight->SetupAttachment(Root);
	MoonLight->SetIntensity(0.45f);
	MoonLight->SetLightColor(FLinearColor(0.55f, 0.68f, 1.f));
	MoonLight->SetMobility(EComponentMobility::Movable);
	MoonLight->SetDynamicShadowCascades(2);
	MoonLight->SetLightSourceAngle(1.2f);		// softer than the sun, so night shadows read as moonlight

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->bRealTimeCapture = true;			// captures the SkyAtmosphere as it changes through the day
	SkyLight->SetIntensity(1.f);

	VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
	VolumetricClouds->SetupAttachment(Root);
	VolumetricClouds->LayerBottomAltitude = 3.f;
	VolumetricClouds->LayerHeight = 7.f;
	VolumetricClouds->TracingMaxDistance = 40.f;

	HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
	HeightFog->SetupAttachment(Root);
	HeightFog->SetFogDensity(0.015f);
	HeightFog->SetFogHeightFalloff(0.2f);
	HeightFog->SetVolumetricFog(true);
	HeightFog->SetVolumetricFogExtinctionScale(1.f);

	Precipitation = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Precipitation"));
	Precipitation->SetupAttachment(Root);
	Precipitation->bAutoActivate = false;

	Ambience = CreateDefaultSubobject<UAudioComponent>(TEXT("Ambience"));
	Ambience->SetupAttachment(Root);
	Ambience->bAutoActivate = false;
	Ambience->bAllowSpatialization = false;		// 2D ambience bed

	// Clock defaults: a 48-minute day, starting mid-morning.
	TimeOfDay = 9.5f;
	MinutesPerSecond = 0.5f;
	SunAzimuth = 15.f;
	SunriseHour = 6.f;
	SunsetHour = 20.f;

	StartingWeather = EShadowlineWeather::Clear;
	bAutoWeather = true;
	DryingSeconds = 180.f;
	WettingSeconds = 45.f;
	AmbienceCrossfadeSeconds = 6.f;

	ActiveRegion = EShadowlineRegion::None;
	CurrentWeather = EShadowlineWeather::Clear;
	TargetWeather = EShadowlineWeather::Clear;
	WeatherBlendAlpha = 1.f;
	WeatherBlendDuration = 0.f;
	BlendedSeverity = 0.f;
	Wetness = 0.f;
	DayAlpha = 1.f;
	MinutesUntilWeatherRoll = 0.f;
	bAmbienceIsNight = false;
	bAmbienceInitialised = false;
}

float AShadowlineWorldAtmosphere::GetSeverityForWeather(EShadowlineWeather InWeather)
{
	switch (InWeather)
	{
	case EShadowlineWeather::Clear:		return 0.f;
	case EShadowlineWeather::Overcast:	return 0.3f;
	case EShadowlineWeather::Fog:		return 0.7f;
	case EShadowlineWeather::Rain:		return 0.65f;
	case EShadowlineWeather::Storm:		return 1.f;
	case EShadowlineWeather::Snow:		return 0.6f;
	case EShadowlineWeather::DustStorm:	return 0.95f;
	default:							return 0.f;
	}
}

const UShadowlineRegionDefinition* AShadowlineWorldAtmosphere::GetActiveDefinition() const
{
	if (const TObjectPtr<UShadowlineRegionDefinition>* Found = RegionDefinitions.Find(ActiveRegion))
	{
		return Found->Get();
	}
	return nullptr;
}

bool AShadowlineWorldAtmosphere::IsNight() const
{
	return TimeOfDay < SunriseHour || TimeOfDay > SunsetHour;
}

void AShadowlineWorldAtmosphere::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Keeps the editor viewport honest: moving the TimeOfDay slider updates the sky
	// without needing to press Play.
	ApplyTimeOfDay();
	ApplyRegionAndWeather();
}

void AShadowlineWorldAtmosphere::BeginPlay()
{
	Super::BeginPlay();

	CurrentWeather = StartingWeather;
	TargetWeather = StartingWeather;
	WeatherBlendAlpha = 1.f;
	BlendedSeverity = GetSeverityForWeather(StartingWeather);

	// Start already wet if we open in rain, so the first frame is not a dry downpour.
	Wetness = (StartingWeather == EShadowlineWeather::Rain || StartingWeather == EShadowlineWeather::Storm) ? 1.f : 0.f;

	if (const UShadowlineRegionDefinition* Definition = GetActiveDefinition())
	{
		MinutesUntilWeatherRoll = Definition->MeanWeatherDurationMinutes;
	}
	else
	{
		UE_LOG(LogShadowline, Warning,
			TEXT("%s has no region definition for region %d. Sky will use its component defaults ")
			TEXT("until RegionDefinitions is populated and SetRegion is called."),
			*GetName(), static_cast<int32>(ActiveRegion));
	}

	ApplyTimeOfDay();
	ApplyRegionAndWeather();
	UpdateAmbience();
}

void AShadowlineWorldAtmosphere::SetTimeOfDay(float NewTimeOfDay)
{
	TimeOfDay = FMath::Fmod(FMath::Max(NewTimeOfDay, 0.f), 24.f);
	ApplyTimeOfDay();
}

void AShadowlineWorldAtmosphere::SetRegion(EShadowlineRegion NewRegion)
{
	if (ActiveRegion == NewRegion)
	{
		return;
	}

	ActiveRegion = NewRegion;

	if (const UShadowlineRegionDefinition* Definition = GetActiveDefinition())
	{
		MinutesUntilWeatherRoll = FMath::Min(MinutesUntilWeatherRoll, Definition->MeanWeatherDurationMinutes);
	}

	ApplyRegionAndWeather();
	UpdateAmbience();
}

void AShadowlineWorldAtmosphere::SetWeather(EShadowlineWeather NewWeather, float BlendSeconds)
{
	if (TargetWeather == NewWeather && WeatherBlendAlpha >= 1.f)
	{
		return;
	}

	// Blending from wherever the previous blend had got to avoids a visible snap when
	// weather changes while an earlier change is still in flight.
	CurrentWeather = (WeatherBlendAlpha >= 1.f) ? TargetWeather : CurrentWeather;
	TargetWeather = NewWeather;
	WeatherBlendDuration = FMath::Max(BlendSeconds, 0.f);
	WeatherBlendAlpha = (WeatherBlendDuration <= 0.f) ? 1.f : 0.f;

	if (WeatherBlendDuration <= 0.f)
	{
		CurrentWeather = NewWeather;
		BlendedSeverity = GetSeverityForWeather(NewWeather);
		ApplyRegionAndWeather();
	}
}

void AShadowlineWorldAtmosphere::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float ElapsedGameMinutes = DeltaSeconds * MinutesPerSecond;

	// --- Clock ---
	if (ElapsedGameMinutes > 0.f)
	{
		TimeOfDay = FMath::Fmod(TimeOfDay + ElapsedGameMinutes / 60.f, 24.f);
		ApplyTimeOfDay();
	}

	// --- Weather blend ---
	if (WeatherBlendAlpha < 1.f && WeatherBlendDuration > 0.f)
	{
		WeatherBlendAlpha = FMath::Min(WeatherBlendAlpha + DeltaSeconds / WeatherBlendDuration, 1.f);
		if (WeatherBlendAlpha >= 1.f)
		{
			CurrentWeather = TargetWeather;
		}
	}

	BlendedSeverity = FMath::Lerp(
		GetSeverityForWeather(CurrentWeather),
		GetSeverityForWeather(TargetWeather),
		FMath::SmoothStep(0.f, 1.f, WeatherBlendAlpha));

	// --- Automatic weather rolls ---
	if (bAutoWeather)
	{
		MinutesUntilWeatherRoll -= ElapsedGameMinutes;
		if (MinutesUntilWeatherRoll <= 0.f)
		{
			if (const UShadowlineRegionDefinition* Definition = GetActiveDefinition())
			{
				SetWeather(Definition->RollWeather(), 25.f);
				// Vary the next roll so weather does not change on a metronome.
				MinutesUntilWeatherRoll = Definition->MeanWeatherDurationMinutes * FMath::FRandRange(0.6f, 1.6f);
			}
			else
			{
				MinutesUntilWeatherRoll = 30.f;
			}
		}
	}

	// --- Wetness ---
	const bool bPrecipitating = (TargetWeather == EShadowlineWeather::Rain || TargetWeather == EShadowlineWeather::Storm);
	const float WetnessTarget = bPrecipitating ? 1.f : 0.f;
	const float WetnessRate = bPrecipitating ? (1.f / WettingSeconds) : (1.f / DryingSeconds);
	Wetness = FMath::FInterpConstantTo(Wetness, WetnessTarget, DeltaSeconds, WetnessRate);

	ApplyRegionAndWeather();
	UpdateAmbience();
}

void AShadowlineWorldAtmosphere::ApplyTimeOfDay()
{
	// Hour angle measured from sunrise: 0 at the eastern horizon, 180 at the western one.
	const float DayLength = FMath::Max(SunsetHour - SunriseHour, 1.f);
	const float HoursSinceSunrise = TimeOfDay - SunriseHour;

	// Daylight hours map onto 0..180 degrees; night maps onto the remaining 180..360.
	float SunPitchDegrees;
	if (HoursSinceSunrise >= 0.f && HoursSinceSunrise <= DayLength)
	{
		SunPitchDegrees = -(HoursSinceSunrise / DayLength) * 180.f;
	}
	else
	{
		const float NightLength = FMath::Max(24.f - DayLength, 1.f);
		const float HoursSinceSunset = (TimeOfDay > SunsetHour)
			? (TimeOfDay - SunsetHour)
			: (TimeOfDay + 24.f - SunsetHour);
		SunPitchDegrees = -180.f - (HoursSinceSunset / NightLength) * 180.f;
	}

	const FRotator SunRotation(SunPitchDegrees, SunAzimuth, 0.f);
	if (SunLight)
	{
		SunLight->SetWorldRotation(SunRotation);
	}
	if (MoonLight)
	{
		// Opposite the sun, offset so the moon is not a perfect mirror of it.
		MoonLight->SetWorldRotation(FRotator(SunPitchDegrees + 180.f, SunAzimuth + 35.f, 0.f));
	}

	// Sun elevation above the horizon, -1..1. Drives intensity, colour and the day/night mix.
	const float SunElevation = FMath::Sin(FMath::DegreesToRadians(-SunPitchDegrees));
	DayAlpha = FMath::Clamp(SunElevation * 4.f, 0.f, 1.f);	// quick, soft twilight ramp

	const UShadowlineRegionDefinition* Definition = GetActiveDefinition();
	const float NoonIntensity = Definition ? Definition->NoonSunIntensity : 75000.f;

	if (SunLight)
	{
		// Severity dims the sun: an overcast sky is darker, a storm much darker.
		const float WeatherDim = FMath::Lerp(1.f, 0.18f, BlendedSeverity);
		SunLight->SetIntensity(NoonIntensity * DayAlpha * WeatherDim);
		SunLight->SetVisibility(DayAlpha > 0.001f);

		if (Definition && Definition->SunColourOverDay)
		{
			const float CurveTime = FMath::Clamp(TimeOfDay / 24.f, 0.f, 1.f);
			SunLight->SetLightColor(Definition->SunColourOverDay->GetLinearColorValue(CurveTime));
			SunLight->SetUseTemperature(false);
		}
		else
		{
			// Without a curve, warm the light near the horizon: cheap but convincing dawn/dusk.
			SunLight->SetUseTemperature(true);
			SunLight->SetTemperature(FMath::Lerp(2300.f, 6200.f, FMath::Clamp(SunElevation * 3.f, 0.f, 1.f)));
		}
	}

	if (MoonLight)
	{
		const float NightAlpha = 1.f - DayAlpha;
		MoonLight->SetIntensity(0.55f * NightAlpha * FMath::Lerp(1.f, 0.1f, BlendedSeverity));
		MoonLight->SetVisibility(NightAlpha > 0.02f);
	}
}

void AShadowlineWorldAtmosphere::ApplyRegionAndWeather()
{
	const UShadowlineRegionDefinition* Definition = GetActiveDefinition();

	const float ClearFog = Definition ? Definition->ClearFogDensity : 0.015f;
	const float HeavyFog = Definition ? Definition->HeavyFogDensity : 0.2f;
	const FLinearColor FogColour = Definition ? Definition->FogInscatteringColour : FLinearColor(0.45f, 0.58f, 0.72f, 1.f);
	const FLinearColor Tint = Definition ? Definition->AtmosphereTint : FLinearColor(0.18f, 0.33f, 0.62f, 1.f);

	if (HeightFog)
	{
		HeightFog->SetFogDensity(FMath::Lerp(ClearFog, HeavyFog, BlendedSeverity));
		HeightFog->SetFogInscatteringColor(FogColour);
		// Heavy weather thickens the volumetric component noticeably more than the base fog.
		HeightFog->SetVolumetricFogExtinctionScale(FMath::Lerp(1.f, 3.2f, BlendedSeverity));
	}

	if (SkyAtmosphere)
	{
		SkyAtmosphere->SetRayleighScatteringScale(FMath::Lerp(0.0331f, 0.05f, BlendedSeverity));
		SkyAtmosphere->SetRayleighScattering(Tint);
	}

	if (Precipitation)
	{
		const bool bWantsPrecipitation =
			Definition && Definition->PrecipitationEffect && BlendedSeverity > 0.35f &&
			(TargetWeather == EShadowlineWeather::Rain
				|| TargetWeather == EShadowlineWeather::Storm
				|| TargetWeather == EShadowlineWeather::Snow
				|| TargetWeather == EShadowlineWeather::DustStorm);

		if (bWantsPrecipitation)
		{
			if (Precipitation->GetAsset() != Definition->PrecipitationEffect)
			{
				Precipitation->SetAsset(Definition->PrecipitationEffect);
			}
			if (!Precipitation->IsActive())
			{
				Precipitation->Activate();
			}
			Precipitation->SetFloatParameter(TEXT("Intensity"), BlendedSeverity);
		}
		else if (Precipitation->IsActive())
		{
			// Deactivate without killing existing particles, so rain tails off.
			Precipitation->Deactivate();
		}
	}

	PublishMaterialParameters();
}

void AShadowlineWorldAtmosphere::PublishMaterialParameters()
{
	if (!WeatherParameters)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const UShadowlineRegionDefinition* Definition = GetActiveDefinition();
	const float ClearCloud = Definition ? Definition->ClearCloudCoverage : 0.25f;
	const float BaseWind = Definition ? Definition->BaseWindSpeed : 1.f;

	const bool bRaining = (TargetWeather == EShadowlineWeather::Rain || TargetWeather == EShadowlineWeather::Storm);
	const bool bSnowing = (TargetWeather == EShadowlineWeather::Snow);
	const bool bDust = (TargetWeather == EShadowlineWeather::DustStorm);

	// DayAlpha is maintained by ApplyTimeOfDay, which runs before this every tick, so the
	// night factor here is the same one the lights use.
	const float NightFactor = 1.f - DayAlpha;

	namespace Params = ShadowlineAtmosphereParams;
	UKismetMaterialLibrary::SetScalarParameterValue(World, WeatherParameters, Params::CloudCoverage, FMath::Lerp(ClearCloud, 1.f, BlendedSeverity));
	UKismetMaterialLibrary::SetScalarParameterValue(World, WeatherParameters, Params::RainIntensity, bRaining ? BlendedSeverity : 0.f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, WeatherParameters, Params::SurfaceWetness, FMath::Clamp(Wetness, 0.f, 1.f));
	UKismetMaterialLibrary::SetScalarParameterValue(World, WeatherParameters, Params::SnowCoverage, bSnowing ? BlendedSeverity : 0.f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, WeatherParameters, Params::DustIntensity, bDust ? BlendedSeverity : 0.f);
	// Wind rises with severity: foliage material WPO and the wind audio bed both read this.
	UKismetMaterialLibrary::SetScalarParameterValue(World, WeatherParameters, Params::WindSpeed, BaseWind * FMath::Lerp(1.f, 3.5f, BlendedSeverity));
	UKismetMaterialLibrary::SetScalarParameterValue(World, WeatherParameters, Params::NightAlpha, NightFactor);
}

void AShadowlineWorldAtmosphere::UpdateAmbience()
{
	if (!Ambience)
	{
		return;
	}

	const UShadowlineRegionDefinition* Definition = GetActiveDefinition();
	if (!Definition)
	{
		return;
	}

	const bool bNightNow = IsNight();
	if (bAmbienceInitialised && bNightNow == bAmbienceIsNight && Ambience->IsPlaying())
	{
		return;
	}

	USoundBase* Desired = bNightNow ? Definition->NightAmbience : Definition->DayAmbience;
	if (!Desired)
	{
		return;
	}

	bAmbienceIsNight = bNightNow;
	bAmbienceInitialised = true;

	Ambience->SetSound(Desired);
	Ambience->FadeIn(AmbienceCrossfadeSeconds, 1.f);
}
