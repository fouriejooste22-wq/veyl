// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "World/ShadowlineRegionDefinition.h"

EShadowlineWeather UShadowlineRegionDefinition::RollWeather() const
{
	float TotalWeight = 0.f;
	for (const FShadowlineWeatherChance& Entry : WeatherTable)
	{
		TotalWeight += FMath::Max(Entry.Weight, 0.f);
	}

	if (TotalWeight <= KINDA_SMALL_NUMBER)
	{
		return EShadowlineWeather::Clear;
	}

	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (const FShadowlineWeatherChance& Entry : WeatherTable)
	{
		Roll -= FMath::Max(Entry.Weight, 0.f);
		if (Roll <= 0.f)
		{
			return Entry.Weather;
		}
	}

	return WeatherTable.Last().Weather;
}
