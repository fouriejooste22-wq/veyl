// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Core/ShadowlineMissionDefinition.h"

int32 UShadowlineMissionDefinition::GetRequiredObjectiveCount() const
{
	int32 Count = 0;
	for (const FShadowlineObjective& Objective : Objectives)
	{
		if (Objective.bRequired)
		{
			++Count;
		}
	}
	return Count;
}

int32 UShadowlineMissionDefinition::GetOptionalObjectiveCount() const
{
	return Objectives.Num() - GetRequiredObjectiveCount();
}

bool UShadowlineMissionDefinition::IsConfrontation() const
{
	return MissionType == EShadowlineMissionType::Confrontation
		|| MissionType == EShadowlineMissionType::Director
		|| MissionType == EShadowlineMissionType::Finale;
}
