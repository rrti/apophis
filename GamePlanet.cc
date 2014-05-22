#include <algorithm>
#include <limits>

#include "GameState.h"

/*
void GamePlanet::ClearTargets(const std::vector< std::vector<GamePlanet*> >& newTargetSizes) {
	mTargets[OWNER_NEUTRAL].clear(); mTargets[OWNER_NEUTRAL].reserve(newTargetSizes[OWNER_NEUTRAL].size());
	mTargets[OWNER_ALLIED ].clear(); mTargets[OWNER_ALLIED ].reserve(newTargetSizes[OWNER_ALLIED ].size());
	mTargets[OWNER_ENEMY  ].clear(); mTargets[OWNER_ENEMY  ].reserve(newTargetSizes[OWNER_ENEMY  ].size());
}

void GamePlanet::SortTargets() {
	// sort targets in decreasing order by heuristic value
	std::sort(mTargets[OWNER_ALLIED ].begin(), mTargets[OWNER_ALLIED ].end(), PlanetSortFunctor(SORTMODE_TMPVALUE_DECR));
	std::sort(mTargets[OWNER_NEUTRAL].begin(), mTargets[OWNER_NEUTRAL].end(), PlanetSortFunctor(SORTMODE_TMPVALUE_DECR));
	std::sort(mTargets[OWNER_ENEMY  ].begin(), mTargets[OWNER_ENEMY  ].end(), PlanetSortFunctor(SORTMODE_TMPVALUE_DECR));
}
*/

double GamePlanet::CalcIntrinsicValue(unsigned owner, unsigned int fNumShips) const {
	double value = -std::numeric_limits<double>::max();

	if (static_cast<int>(mGrowthRate) <= 0) {
		return value;
	}

	value = mGrowthRate * mGrowthRate;

	// the value of this planet for OWNER_ALLIED is
	// different than it would be for OWNER_ENEMY
	if (mOwner == owner) {
		value *= (mNumShips + 1);
	} else {
		value /= (fNumShips + 1);
	}

	switch (mMapRegion) {
		case MAP_REGION_INNER:  { value *= 1.333; } break;
		case MAP_REGION_MIDDLE: { value *= 1.000; } break;
		case MAP_REGION_OUTER:  { value *= 0.666; } break;
	}

	return value;
}
