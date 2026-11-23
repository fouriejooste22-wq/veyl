// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

#include "Companions/ShadowlineDialogueSet.h"

bool UShadowlineDialogueSet::DoesBarkMatchContext(const FShadowlineBark& Bark, const FShadowlineBarkContext& Context)
{
	const FShadowlineBarkFilter& F = Bark.Filter;

	// An empty list means "no restriction", not "nothing matches".
	if (F.Regions.Num() > 0 && !F.Regions.Contains(Context.Region))
	{
		return false;
	}
	if (F.Weather.Num() > 0 && !F.Weather.Contains(Context.Weather))
	{
		return false;
	}
	if (F.DayNight == 1 && Context.bNight)
	{
		return false;
	}
	if (F.DayNight == 2 && !Context.bNight)
	{
		return false;
	}
	if (F.MinAct > 0 && Context.Act < F.MinAct)
	{
		return false;
	}
	if (F.MaxAct > 0 && Context.Act > F.MaxAct)
	{
		return false;
	}

	// A bark with no lines would select successfully and then say nothing.
	return Bark.Lines.Num() > 0;
}

int32 UShadowlineDialogueSet::SelectBarkIndex(FName Trigger, const FShadowlineBarkContext& Context) const
{
	int32 BestPriority = -1;
	TArray<int32, TInlineAllocator<8>> Candidates;

	for (int32 Index = 0; Index < Barks.Num(); ++Index)
	{
		const FShadowlineBark& Bark = Barks[Index];
		if (Bark.Trigger != Trigger || !DoesBarkMatchContext(Bark, Context))
		{
			continue;
		}

		if (Bark.Priority > BestPriority)
		{
			BestPriority = Bark.Priority;
			Candidates.Reset();
			Candidates.Add(Index);
		}
		else if (Bark.Priority == BestPriority)
		{
			Candidates.Add(Index);
		}
	}

	if (Candidates.Num() == 0)
	{
		return INDEX_NONE;
	}
	return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}
