// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "Core/ShadowlineTypes.h"
#include "ShadowlineGameUserSettings.generated.h"

/**
 * Graphics and display settings for Operation Shadowline.
 *
 * UGameUserSettings already stores and applies resolution, window mode, VSync, frame rate
 * cap and the ten scalability groups (view distance, shadows, global illumination,
 * reflections, anti-aliasing, textures, effects, post processing, foliage, shading). This
 * subclass adds what the settings menu needs on top of that:
 *
 *  - named Low / Medium / High / Ultra presets, plus a Custom state the UI falls into
 *    automatically as soon as any individual group is changed away from its preset value
 *  - motion blur, which is a console variable rather than a scalability group
 *  - a one-call hardware benchmark for first launch
 *
 * Registered through [/Script/Engine.Engine] GameUserSettingsClassName in DefaultEngine.ini.
 */
UCLASS(Config = GameUserSettings, ConfigDoNotCheckDefaults)
class OPERATIONSHADOWLINE_API UShadowlineGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UShadowlineGameUserSettings();

	/** Typed accessor, so callers do not have to cast. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Settings", meta = (DisplayName = "Get Shadowline Settings"))
	static UShadowlineGameUserSettings* GetShadowlineSettings();

	//~ Begin UGameUserSettings interface
	virtual void SetToDefaults() override;
	virtual void ApplyNonResolutionSettings() override;
	//~ End UGameUserSettings interface

	/**
	 * Sets every scalability group to one preset.
	 * Passing Custom does nothing: Custom is a state the settings fall into, not one to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Settings")
	void ApplyQualityPreset(EShadowlineQualityPreset Preset);

	/** The stored preset, or Custom when the groups no longer all match it. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Settings")
	EShadowlineQualityPreset GetQualityPreset() const { return QualityPreset; }

	/**
	 * Recomputes the preset from the current scalability levels.
	 * Call after changing any individual group so the UI can show Custom.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Settings")
	EShadowlineQualityPreset RefreshQualityPreset();

	UFUNCTION(BlueprintCallable, Category = "Shadowline|Settings")
	void SetMotionBlurEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Settings")
	bool IsMotionBlurEnabled() const { return bMotionBlurEnabled; }

	/** 0..1 strength multiplier applied when motion blur is enabled. */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Settings")
	void SetMotionBlurAmount(float Amount);

	UFUNCTION(BlueprintPure, Category = "Shadowline|Settings")
	float GetMotionBlurAmount() const { return MotionBlurAmount; }

	/**
	 * Measures CPU and GPU performance and applies the scalability levels it suggests.
	 * Takes a noticeable moment: run it from a first-launch screen, not mid-game.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shadowline|Settings")
	void RunBenchmarkAndApply();

	/** True when this machine reports a Shader Model 6 RHI, which Lumen and Nanite need. */
	UFUNCTION(BlueprintPure, Category = "Shadowline|Settings")
	static bool IsHardwareRayTracingCapable();

protected:
	/** Maps a preset onto the 0..3 scalability level the engine's Scalability.ini uses. */
	static int32 PresetToScalabilityLevel(EShadowlineQualityPreset Preset);

	/** Pushes MotionBlur values to their console variables. */
	void ApplyMotionBlurCVars() const;

protected:
	UPROPERTY(Config)
	EShadowlineQualityPreset QualityPreset;

	UPROPERTY(Config)
	bool bMotionBlurEnabled;

	UPROPERTY(Config)
	float MotionBlurAmount;
};
