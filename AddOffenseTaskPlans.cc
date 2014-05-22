#include <limits>
#include <algorithm>
#include "PlanetWars.h"
#include "Logger.h"

void PlanetWars::AddOffenseTaskPlans(unsigned int owner, std::vector<unsigned int>& spareShips) {
	LOG_STDOUT(LOGGER, "[AddOffenseTaskPlans] owner=%u, planetMask=%u\n", mCurrentTurn, owner);

	std::vector<GamePlanet*>& alliedPlanets = mGameState.GetOwnerPlanets(OWNER_ALLIED);
	std::vector<GamePlanet*>& ownerPlanets = mGameState.GetOwnerPlanets(owner);

	for (unsigned int n = 0; n < ownerPlanets.size(); n++) {
		const GamePlanet* pDst = ownerPlanets[n];
		unsigned int pDstOwner = pDst->GetOwner();


		{
			/*
			bool maskMatched = false;

			switch (owner) {
				case OWNER_NEUTRAL: {
					if ((planetMask & PLANET_CENTER_NEUTRAL_BIT) != 0) {
						maskMatched = maskMatched || (pDst->GetMapRegion() == MAP_REGION_INNER && pDst->GetSafetyLevel() <= PLANET_SAFETY_LEVEL_MID);
					}
					if ((planetMask & PLANET_INNER_NEUTRAL_BIT) != 0) {
						maskMatched = maskMatched || (pDst->GetSafetyLevel() >= PLANET_SAFETY_LEVEL_MID);
					}
				} break;
				case OWNER_ENEMY: {
					if ((planetMask & PLANET_CENTER_ENEMY_BIT) != 0) {
						maskMatched = maskMatched || (pDst->GetMapRegion() == MAP_REGION_INNER);
					}
					if ((planetMask & PLANET_INNER_ENEMY_BIT) != 0) {
						maskMatched = maskMatched || (pDst->GetSafetyLevel() == PLANET_SAFETY_LEVEL_MIN);
					}
				} break;
			}

			if (!maskMatched) {
				continue;
			}
			*/
		}

		if (pDst->GetTmpValue() == -std::numeric_limits<double>::max()) {
			// planet has zero growth-rate or is otherwise useless
			// since ownerPlanets is sorted by value, all remaining
			// planets will be useless too
			break;
		}


		if (owner == OWNER_NEUTRAL && GameMap::IsPlanetCloserToOpponentThanOwner(mGameState, pDst, OWNER_ALLIED)) {
			continue;
		}


		mGameState.GetPlanetFuturePopulation(pDst->GetID(), GameMap::GetMaxPlanetDistance(), NULL, &pDstOwner, NULL, NULL);

		if (!pDst->GetIncomingFleetIDs(OWNER_ALLIED).empty()) {
			// an old plan might have been executed and finished (such
			// that all participants have sent their ships to <pDst>);
			// check who this planet's future owner will be
			if (pDstOwner == OWNER_ALLIED || pDst->GetMapRegion() != MAP_REGION_INNER) {
				continue;
			}
		}

		if (mTaskPlans.find(pDst->GetID()) != mTaskPlans.end()) {
			// there is already a plan for capturing
			// this target planet (neutral or enemy)
			// still running, ie. !members.empty()
			continue;
		}


		// re-sort by *remaining* population (mNumShips - mNumReservedShips)
		std::sort(alliedPlanets.begin(), alliedPlanets.end(), PlanetSortFunctor(SORTMODE_NUMSHIPSREM_DECR));

		// number of ships reserved by each allied planet
		// for a task targeted at <pDst>; allied planets
		// that will participate in a task targeted at
		//<pDst>
		std::map<unsigned int, unsigned int> reservedShips;
		std::list<unsigned int> taskPlanetIDs;

		unsigned int minPlanTurns = GameMap::GetMaxPlanetDistance() + 1;
		unsigned int maxPlanTurns = 0;
		unsigned int avgPlanTurns = 0;
		unsigned int minPlanShips = 0;

		// first check the N-nearest OWNER_ALLIED frontier neighbors of <pDst>
		if (!CanCapturePlanetNN(pDst, 3, spareShips, taskPlanetIDs, &minPlanTurns, &maxPlanTurns, &avgPlanTurns, &minPlanShips)) {
			if (!CanCapturePlanet(pDst, alliedPlanets, spareShips, taskPlanetIDs, &minPlanTurns, &maxPlanTurns, &avgPlanTurns, &minPlanShips)) {
				continue;
			}
		}

		AddTaskPlan(pDst, maxPlanTurns, minPlanShips, reservedShips, spareShips, taskPlanetIDs);
	}
}
