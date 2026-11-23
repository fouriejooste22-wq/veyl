// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Settings/ShadowlineGameUserSettings.h"

#include "OperationShadowline.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "RHI.h"
#include "Scalability.h"

UShadowlineGameUserSettings::UShadowlineGameUserSettings()
{
	QualityPreset = EShadowlineQualityPreset::High;
	bMotionBlurEnabled = true;
	MotionBlurAmount = 0.35f;
}

UShadowlineGameUserSettings* UShadowlineGameUserSettings::GetShadowlineSettings()
{
	return Cast<UShadowlineGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

int32 UShadowlineGameUserSettings::PresetToScalabilityLevel(EShadowlineQualityPreset Preset)
{
	switch (Preset)
	{
	case EShadowlineQualityPreset::Low:		return 0;
	case EShadowlineQualityPreset::Medium:	return 1;
	case EShadowlineQualityPreset::High:	return 2;
	case EShadowlineQualityPreset::Ultra:	return 3;
	default:								return -1;	// Custom has no single level
	}
}

void UShadowlineGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	QualityPreset = EShadowlineQualityPreset::High;
	bMotionBlurEnabled = true;
	MotionBlurAmount = 0.35f;
}

void UShadowlineGameUserSettings::ApplyQualityPreset(EShadowlineQualityPreset Preset)
{
	const int32 Level = PresetToScalabilityLevel(Preset);
	if (Level < 0)
	{
		// Custom is a description of the current state, not something to apply.
		return;
	}

	// SetFromSingleQualityLevel also rewrites ResolutionQuality. Screen percentage is a
	// separate slider in our settings menu, so it is preserved across a preset change.
	const float PreservedResolutionQuality = ScalabilityQuality.ResolutionQuality;

	Scalability::FQualityLevels Levels = ScalabilityQuality;
	Levels.SetFromSingleQualityLevel(Level);
	Levels.ResolutionQuality = PreservedResolutionQuality;

	ScalabilityQuality = Levels;
	QualityPreset = Preset;

	UE_LOG(LogShadowline, Log, TEXT("Applied quality preset %d (scalability level %d)."),
		static_cast<int32>(Preset), Level);
}

EShadowlineQualityPreset UShadowlineGameUserSettings::RefreshQualityPreset()
{
	// GetSingleQualityLevel returns -1 when the groups do not share one level.
	const int32 Single = ScalabilityQuality.GetSingleQualityLevel();

	switch (Single)
	{
	case 0:		QualityPreset = EShadowlineQualityPreset::Low; break;
	case 1:		QualityPreset = EShadowlineQualityPreset::Medium; break;
	case 2:		QualityPreset = EShadowlineQualityPreset::High; break;
	case 3:		QualityPreset = EShadowlineQualityPreset::Ultra; break;
	default:	QualityPreset = EShadowlineQualityPreset::Custom; break;
	}

	return QualityPreset;
}

void UShadowlineGameUserSettings::SetMotionBlurEnabled(bool bEnabled)
{
	bMotionBlurEnabled = bEnabled;
}

void UShadowlineGameUserSettings::SetMotionBlurAmount(float Amount)
{
	MotionBlurAmount = FMath::Clamp(Amount, 0.f, 1.f);
}

void UShadowlineGameUserSettings::ApplyMotionBlurCVars() const
{
	IConsoleManager& Console = IConsoleManager::Get();

	if (IConsoleVariable* QualityCVar = Console.FindConsoleVariable(TEXT("r.MotionBlurQuality")))
	{
		// 0 disables motion blur outright; 4 is the engine's highest quality.
		QualityCVar->Set(bMotionBlurEnabled ? 4 : 0, ECVF_SetByGameSetting);
	}

	if (IConsoleVariable* AmountCVar = Console.FindConsoleVariable(TEXT("r.MotionBlur.Amount")))
	{
		AmountCVar->Set(bMotionBlurEnabled ? MotionBlurAmount : 0.f, ECVF_SetByGameSetting);
	}
}

void UShadowlineGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	// The base class has just pushed the scalability groups; keep the preset label in step
	// with whatever is actually set, then apply the settings it does not know about.
	RefreshQualityPreset();
	ApplyMotionBlurCVars();
}

void UShadowlineGameUserSettings::RunBenchmarkAndApply()
{
	// Default work scales: enough samples to be meaningful without a long stall.
	RunHardwareBenchmark();
	ApplyHardwareBenchmarkResults();
	RefreshQualityPreset();

	UE_LOG(LogShadowline, Log,
		TEXT("Hardware benchmark applied. CPU score %.2f, GPU score %.2f, resulting preset %d."),
		LastCPUBenchmarkResult, LastGPUBenchmarkResult, static_cast<int32>(QualityPreset));
}

bool UShadowlineGameUserSettings::IsHardwareRayTracingCapable()
{
	// Hardware Lumen, Nanite's full path and Virtual Shadow Maps all require SM6.
	return GMaxRHIFeatureLevel >= ERHIFeatureLevel::SM6;
}
