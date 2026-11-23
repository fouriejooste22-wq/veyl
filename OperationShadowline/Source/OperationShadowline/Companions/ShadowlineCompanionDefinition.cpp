// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Companions/ShadowlineCompanionDefinition.h"

TArray<FName> UShadowlineCompanionDefinition::GetCapabilitiesAtTier(FName SpecialisationId, int32 Tier) const
{
	TArray<FName> Result;
	if (Tier <= 0)
	{
		return Result;
	}

	for (const FShadowlineSpecialisation& Spec : Specialisations)
	{
		if (Spec.SpecialisationId != SpecialisationId)
		{
			continue;
		}

		// Tiers accumulate: tier 3 grants what tiers 1 and 2 granted as well.
		const int32 Reached = FMath::Min(Tier, FMath::Min(Spec.MaxTier, Spec.TierCapabilities.Num()));
		for (int32 Index = 0; Index < Reached; ++Index)
		{
			if (!Spec.TierCapabilities[Index].IsNone())
			{
				Result.AddUnique(Spec.TierCapabilities[Index]);
			}
		}
		break;
	}

	return Result;
}
