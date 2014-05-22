#include <limits>
#include "PlanetWars.h"
#include "Logger.h"

// solves the 0-1 knapsack problem
//   <sack> is filled with the best combination of
//   planets (ie. the combination which maximizes
//   growth-rate)
unsigned int GameState::FindKnapSackPlanets(
	unsigned int owner,
	unsigned int maxWeight,
	const std::vector<GamePlanet*>& planets,
	std::vector<GamePlanet*>& sack
) {
	// LOG_STDOUT(LOGGER, "[FindKnapSackPlanets] owner=%u, maxWeight=%u\n", owner, maxWeight);

	if (owner == OWNER_NEUTRAL) { return 0; }
	if (planets.empty()) { return 0; }
	if (maxWeight == 0) { return 0; }

	std::vector<unsigned int> weights(planets.size());
	std::vector<unsigned int> values(planets.size());
	sack.reserve(planets.size());

	for (unsigned int n = 0; n < planets.size(); n++) {
		// weights and values are numShips and growthRate;
		// note that both *MUST* be unsigned integers here
		// NOTE: weights should still incorporate a notion
		// of distance or we risk over-extending ourselves
		const GamePlanet* p = planets[n];

		const double pAvgDist = std::max(1.0, mGameMap.GetAvgPlanetDistance(*this, p->GetID(), owner));
		const bool pIsSafe = ((p->GetGrowthRate() > 0) && !GameMap::IsPlanetCloserToOpponentThanOwner(*this, p, owner));
	//	const bool pIsSafe = (pAvgDist <= mGameMap.GetAvgPlanetDistance(*this, p->GetID(), OWNER_OPPONENT(owner)));

		if (pIsSafe) {
			weights[n] = GetPlanetFuturePopulation(p->GetID(), pAvgDist, NULL, NULL, NULL, NULL) + 1; // p->GetNumShips() + 1
			values[n] = p->GetGrowthRate();
		} else {
			// don't consider planets closer to the enemy
			weights[n] = std::numeric_limits<unsigned int>::max();
			values[n] = 0;
		}
	}

	std::vector< std::vector<unsigned int> > K(weights.size() + 1, std::vector<unsigned int>(maxWeight, 0));

	for (unsigned int k = 1; k <= weights.size(); k++) {
		for (unsigned int y = 1; y <= maxWeight; y++) {
			const unsigned int wgt = weights[k - 1];
			const unsigned int val = K[k - 1][y - 1];

			if (y < wgt) {
				K[k][y - 1] = val;
			} else if (y > wgt) {
				K[k][y - 1] = std::max(val,  K[k - 1][y - 1 - wgt] + values[k - 1]);
			} else {
				K[k][y - 1] = std::max(val, values[k - 1]);
			}
		}
	}

	// get the planets in the solution
	unsigned int idx = weights.size();
	unsigned int wgt = maxWeight - 1;
	unsigned int ret = 0;

	while (idx > 0) {
		const bool b0 = ((idx == 0) && (K[idx][wgt] > 0));
		const bool b1 = ((idx >  0) && (K[idx][wgt] != K[idx - 1][wgt]));

		if (b0 || b1) {
			sack.push_back(planets[idx - 1]);

			wgt -= weights[idx - 1];
			ret += planets[idx - 1]->GetGrowthRate();
		}

		if (static_cast<int>(wgt) <= 0) {
			break;
		}

		idx--;
	}

	return ret;
}
