#include <algorithm>
#include <limits>

#include "PlanetWars.h"
#include "Logger.h"

unsigned int PlanetWars::GetMaxSackGrowthTurn(unsigned int maxTurns, unsigned int owner) {
	unsigned int maxGrowthTurn = 0;
	double maxGrowthRatio = 1.0;

	// examine achievable growth-rates on future turns
	// note: should this use a moving or fixed boundary?
	//
	#if (0 == 1)
		unsigned int sumSackGrowth = mGameState.GetSumSackGrowth(owner);

		for (unsigned int turn = 0; turn < maxTurns; turn++) {
			if (mGameStates[turn].GetSumSackGrowth(owner) > (sumSackGrowth + turn + 1)) {
				sumSackGrowth = mGameStates[turn].GetSumSackGrowth(owner);
				maxGrowthTurn = turn + 1;
			}
		}
	#else
		for (unsigned int turn = 0; turn < maxTurns; turn++) {
			const unsigned int growthDiff = (mGameStates[turn].GetSumSackGrowth(owner) - mGameState.GetSumSackGrowth(owner));
			const double growthRatio = growthDiff / (turn + 1);

			if (static_cast<int>(growthDiff) <= 0) {
				continue;
			}

			if (growthRatio > maxGrowthRatio) {
				maxGrowthRatio = growthRatio;
				maxGrowthTurn = turn + 1;
			}
		}
	#endif

	return maxGrowthTurn;
}



// set how much each planet in <planets> is worth (heuristically) to owner <owner>
// without any reference to a particular source planet; needs a clustering component
//
// TODO: center planets need to receive higher value (or have more ships sent to them)
void PlanetWars::SetPlanetValues(unsigned int owner, std::vector<GamePlanet*>& planets) const {
	const GameMap& gameMap = mGameState.GetGameMap();

	for (unsigned int n = 0; n < planets.size(); n++) {
		GamePlanet* p = planets[n];

		switch (owner) {
			case OWNER_NEUTRAL: {
				const double d = gameMap.GetAvgPlanetDistance(mGameState, p->GetID(), owner);
				const unsigned int ad = std::max(1.0, d);
				const unsigned int fp = mGameState.GetPlanetFuturePopulation(p->GetID(), ad, NULL, NULL, NULL, NULL);
				const double v = p->CalcIntrinsicValue(owner, fp);

				p->SetTmpValue(v);

				if (v != -std::numeric_limits<double>::max()) {
					// growth-rate greater than zero, divide the
					// planet's value by its average distance to
					// <owner>
					p->SetTmpValue(v / d);
				}
			} break;

			case OWNER_ALLIED: {
				// time-to-live will be >= 0 only if planet is going to change owner
				// if so, the planet's future population right after being captured
				// equals the number of ships we came up short
				unsigned int pTTL = -1U;
				unsigned int pFNS = mGameState.GetPlanetFuturePopulation(p->GetID(), GameMap::GetMaxPlanetDistance(), &pTTL, NULL, NULL, NULL);

				if (pTTL != -1U) {
					p->SetNumSpareShips(0);
					p->SetNumShortageShips(pFNS + 1); // needed by DefendAttackedPlanets
					p->SetTmpDistance(pTTL); // needed by DefendAttackedPlanets
					p->SetTmpValue(((GameMap::GetMaxPlanetDistance() - pTTL) * (p->GetGrowthRate() * p->GetGrowthRate())) / gameMap.GetAvgPlanetDistance(mGameState, p->GetID(), OWNER_ALLIED));
				} else {
					p->SetNumSpareShips(mGameState.GetPlanetMaxSpareShips(p->GetID()));
					p->SetNumShortageShips(0);
					p->SetTmpValue(-std::numeric_limits<double>::max());
				}
			} break;

			default: {
			} break;
		}
	}

	std::sort(planets.begin(), planets.end(), PlanetSortFunctor(SORTMODE_TMPVALUE_DECR));
}



void PlanetWars::UpdatePlanetSpareShipCounts(std::vector<GamePlanet*>& planets, std::vector<unsigned int>& spareShips) const {
	for (unsigned int n = 0; n < planets.size(); n++) {
		GamePlanet* p = planets[n];

		const unsigned int numSpareShips = p->GetNumSpareShips();
		const unsigned int numReservedShips = p->GetNumReservedShips();

		// compensate for ships that are already reserved
		spareShips[p->GetID()] = numSpareShips - numReservedShips;
	}
}



bool PlanetWars::DetectStaleMate(unsigned int maxStaleMateTurns) {
	const GameStats& gameStats = mGameState.GetGameStats();
	const bool isAhead =
		(gameStats.GetSumNumShips(OWNER_ALLIED) > gameStats.GetSumNumShips(OWNER_ENEMY)) &&
		(gameStats.GetSumGrowthRate(OWNER_ALLIED) > gameStats.GetSumGrowthRate(OWNER_ENEMY));

	unsigned int k = 0;
	unsigned int n = 0;

	for (std::list<bool>::const_iterator it = mStaleMateTurns.begin(); it != mStaleMateTurns.end() && n < maxStaleMateTurns; ++it, n++) {
		k += static_cast<unsigned int>(*it);
	}

	return (k == maxStaleMateTurns && isAhead);
}
