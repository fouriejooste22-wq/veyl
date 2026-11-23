// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Settings/ShadowlineDifficultyProfile.h"

void UShadowlineDifficultyProfile::ResetToPresetDefaults()
{
	Knobs = FShadowlineDifficultyKnobs::FromPreset(Preset);
}
