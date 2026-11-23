// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "World/ShadowlineProvinceDefinition.h"

int32 UShadowlineProvinceDefinition::GetTotalDiscoverables() const
{
	return FMath::Max(0, DocumentCount)
		+ FMath::Max(0, LandmarkCount)
		+ FMath::Max(0, CacheCount);
}
