#include "GameState.h"
#include "Debugger.h"
#include "Logger.h"

GameStats::GameStats() {
	mPlanetCounts.resize(NUM_OWNERS, 0);
	mFleetCounts.resize(NUM_OWNERS, 0);
	mMaxGrowthRates.resize(NUM_OWNERS, 0);
	mSumGrowthRates.resize(NUM_OWNERS, 0);
	mAvgGrowthRates.resize(NUM_OWNERS, 0.0);
	mMaxNumShips.resize(NUM_OWNERS, 0);
	mSumNumShips.resize(NUM_OWNERS, 0);
	mAvgNumShips.resize(NUM_OWNERS, 0.0);
	mMaxFleetSizes.resize(NUM_OWNERS, 0);
	mSumFleetSizes.resize(NUM_OWNERS, 0);
	mAvgFleetSizes.resize(NUM_OWNERS, 0.0);
}

GameStats::~GameStats() {
	mPlanetCounts.clear();
	mFleetCounts.clear();
	mMaxGrowthRates.clear();
	mSumGrowthRates.clear();
	mAvgGrowthRates.clear();
	mMaxNumShips.clear();
	mSumNumShips.clear();
	mAvgNumShips.clear();
	mMaxFleetSizes.clear();
	mSumFleetSizes.clear();
	mAvgFleetSizes.clear();
}



void GameStats::ClearTurnData() {
	for (unsigned int i = 0; i < NUM_OWNERS; i++) {
		mPlanetCounts[i] = 0;
		mFleetCounts[i]  = 0;

		mMaxGrowthRates[i] = 0;
		mSumGrowthRates[i] = 0;
		mAvgGrowthRates[i] = 0.0;

		mMaxNumShips[i] = 0;
		mSumNumShips[i] = 0;
		mAvgNumShips[i] = 0.0;

		mMaxFleetSizes[i] = 0;
		mSumFleetSizes[i] = 0;
		mAvgFleetSizes[i] = 0.0;
	}
}



void GameStats::CalcTurnData(const GameState& gameState) {
	ClearTurnData();

	BOT_ASSERT(!mPlanetCounts.empty());
	BOT_ASSERT(mPlanetCounts[0] == 0);

	for (unsigned int n = 0; n < gameState.GetNumPlanets(); n++) {
		const GamePlanet& p = gameState.GetPlanet(n);

		mPlanetCounts[p.GetOwner()] += 1;
		mMaxGrowthRates[p.GetOwner()] = std::max(mMaxGrowthRates[p.GetOwner()], static_cast<unsigned int>(p.GetGrowthRate()));
		mSumGrowthRates[p.GetOwner()] += p.GetGrowthRate();

		mMaxNumShips[p.GetOwner()] = std::max(mMaxNumShips[p.GetOwner()], static_cast<unsigned int>(p.GetNumShips()));
		mSumNumShips[p.GetOwner()] += p.GetNumShips();
	}


	const std::map<unsigned int, GameFleet>& fleets = gameState.GetFleets();
	      std::map<unsigned int, GameFleet>::const_iterator fleetsIt;

	for (fleetsIt = fleets.begin(); fleetsIt != fleets.end(); ++fleetsIt) {
		const GameFleet& f = fleetsIt->second;

		mFleetCounts[f.GetOwner()] += 1;
		mMaxFleetSizes[f.GetOwner()] = std::max(mMaxFleetSizes[f.GetOwner()], f.GetNumShips());
		mSumFleetSizes[f.GetOwner()] += f.GetNumShips();
	}


	// the total number of planets should never change
	unsigned int sumNumPlanets = 0;

	// set averages
	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		const std::vector<GamePlanet*>& ownerPlanets = gameState.GetOwnerPlanets(owner);

		sumNumPlanets += ownerPlanets.size();

		if (mPlanetCounts[owner] > 0) { mAvgGrowthRates[owner] = static_cast<double>(mSumGrowthRates[owner]) / mPlanetCounts[owner]; }
		if (mPlanetCounts[owner] > 0) { mAvgNumShips[owner]    = static_cast<double>(mSumNumShips[owner])    / mPlanetCounts[owner]; }
		if (mFleetCounts[owner]  > 0) { mAvgFleetSizes[owner]  = static_cast<double>(mSumFleetSizes[owner])  / mFleetCounts[owner];  }
	}

	BOT_ASSERT(sumNumPlanets == gameState.GetNumPlanets());
}
