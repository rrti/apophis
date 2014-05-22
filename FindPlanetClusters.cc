#include <cmath>
#include "PlanetWars.h"
#include "Logger.h"

// a planet is part of a cluster if its <N>
// nearest neighbors are less than <r> away
// (where <r> is a certain percentage of the
// map-extends)
// this identifies B as being in a cluster,
// but not A or C (in a config like "A B C")
//
// alternative: check if relative distance
// to next-closest neighbor is greater than
// K times relative distance to first N ngbs
void PlanetWars::GetPlanetClusters(unsigned int N, double r) {
	const double mapDiag = std::sqrt(
		(GameMap::GetMapDimension(0) * GameMap::GetMapDimension(0)) +
		(GameMap::GetMapDimension(1) * GameMap::GetMapDimension(1))
	);
	const double minDist = mapDiag * r;

	typedef SortedList<unsigned int, unsigned int>::KeyValPair KVP;
	typedef std::list<KVP>::const_iterator KVPIt;

	for (unsigned int n = 0; n < mGameState.GetNumPlanets(); n++) {
		const GamePlanet* p = &mGameState.GetPlanet(n);

		unsigned int numNgbs = 0;
		const SortedList<unsigned int, unsigned int>* pNgbs = GameMap::GetPlanetNeighbors(p->GetID());

		for (KVPIt it = pNgbs->begin(); it != pNgbs->end(); ++it) {
			const GamePlanet* pNgb = &mGameState.GetPlanet((*it).second);

			if (GameMap::GetDistance(pNgb, p) > minDist) {
				break;
			}

			numNgbs += 1;
		}

		if (numNgbs >= N) {
			// planet is in cluster; problem is how
			// to assign unique numbers to each one
		}
	}
}
