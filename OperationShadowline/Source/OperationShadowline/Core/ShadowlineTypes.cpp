// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Core/ShadowlineTypes.h"

FShadowlineDifficultyKnobs FShadowlineDifficultyKnobs::FromPreset(EShadowlineDifficulty Preset)
{
	// Standard is the 1.0 baseline every other preset is expressed against.
	FShadowlineDifficultyKnobs K;

	switch (Preset)
	{
	case EShadowlineDifficulty::Field:
		// Story-forward. The island should still feel watched, so detection is softened
		// rather than removed: patrols notice you, they are just slow and forgetful.
		K.SuspicionRate = 0.55f;
		K.SuspicionDecayRate = 2.0f;
		K.SightRange = 0.75f;
		K.HearingRange = 0.7f;
		K.SearchPersistenceSeconds = 25.f;
		K.bSiteWideAlerts = false;
		K.PatrolDensity = 0.7f;
		K.CommunicationRange = 0.6f;
		K.bNightOpticsEnabled = false;
		K.DroneRange = 1.4f;
		K.CompanionSelfRecoverySeconds = 20.f;
		K.WeatherSeverity = 0.8f;
		break;

	case EShadowlineDifficulty::Standard:
		break;	// the baseline, unchanged

	case EShadowlineDifficulty::Veteran:
		K.SuspicionRate = 1.6f;
		K.SuspicionDecayRate = 0.65f;
		K.SightRange = 1.25f;
		K.HearingRange = 1.3f;
		K.SearchPersistenceSeconds = 150.f;
		K.bSiteWideAlerts = false;
		K.PatrolDensity = 1.3f;
		K.CommunicationRange = 1.6f;
		K.bNightOpticsEnabled = true;
		// The target is no longer pinned on the map: finding them is the mission.
		K.bTargetMarkers = false;
		K.bTaggingEnabled = true;
		K.bConcealmentReadout = false;
		K.DroneRange = 0.8f;
		K.CompanionSelfRecoverySeconds = 0.f;
		K.WeatherSeverity = 1.2f;
		break;

	case EShadowlineDifficulty::Blackout:
		// No interface help at all. The player navigates by terrain and reads cover by eye.
		K.SuspicionRate = 2.1f;
		K.SuspicionDecayRate = 0.4f;
		K.SightRange = 1.45f;
		K.HearingRange = 1.6f;
		K.SearchPersistenceSeconds = 300.f;
		K.bSiteWideAlerts = true;
		K.PatrolDensity = 1.55f;
		K.CommunicationRange = 2.2f;
		K.bNightOpticsEnabled = true;
		K.bTargetMarkers = false;
		K.bTaggingEnabled = false;
		K.bObjectiveTracker = false;
		K.bConcealmentReadout = false;
		K.bCompass = false;
		K.DroneRange = 0.6f;
		K.CompanionSelfRecoverySeconds = 0.f;
		K.bCompanionLossPersists = true;
		K.WeatherSeverity = 1.5f;
		break;

	case EShadowlineDifficulty::Custom:
		// Custom starts from Standard; the settings screen mutates it from there.
		break;

	default:
		break;
	}

	return K;
}
