#include <algorithm>
#include <limits>

#include "PlanetWars.h"
#include "Debugger.h"
#include "Logger.h"

void PlanetWars::AddDefenseTaskPlans(std::vector<GamePlanet*>& alliedPlanets, std::vector<unsigned int>& spareShips) {
	std::vector<GamePlanet*>& neutralPlanets = mGameState.GetOwnerPlanets(OWNER_NEUTRAL);
	std::vector<GamePlanet*>& frontierPlanets = mGameState.GetFrontierPlanets(OWNER_ALLIED);

	// NOTE: the intersection of alliedPlanets and frontierPlanets is not empty
	DefendAttackedPlanets(spareShips, alliedPlanets);
	DefendFutureFrontierPlanets(spareShips, frontierPlanets, alliedPlanets, neutralPlanets);
	DefendFrontierPlanets(spareShips, frontierPlanets, alliedPlanets);
}



void PlanetWars::DefendAttackedPlanets(
	std::vector<unsigned int>& spareShips,
	std::vector<GamePlanet*>& defendeePlanets
) {
	typedef SortedList<unsigned int, unsigned int>::KeyValPair KVP;
	typedef std::list<KVP>::const_iterator KVPIt;

	// defend any planets that _will_ be lost without reinforcements
	// these planets are (should be) sorted by "importance" / urgency
	// (ie. growth-rate, size of shortage, etc.)
	for (unsigned int n = 0; n < defendeePlanets.size(); n++) {
		GamePlanet* pDst = defendeePlanets[n];

		if (pDst->GetNumShortageShips() == 0) {
			// planet will not be lost to the enemy, no need
			// to defend it (if it is not a front-line planet)
			continue;
		}

		// planet will be lost unless we intervene
		const unsigned int minShips = pDst->GetNumShortageShips();
		const unsigned int maxTurns = pDst->GetTmpDistance();

		unsigned int sumShips = 0;
		unsigned int srcShips = 0;

		const SortedList<unsigned int, unsigned int>* pNgbs = GameMap::GetPlanetNeighbors(pDst->GetID());

		for (KVPIt it = pNgbs->begin(); it != pNgbs->end() && sumShips <= minShips; ++it) {
			const unsigned int pSrcID = (*it).second;
			const GamePlanet* pSrc = &mGameState.GetPlanet(pSrcID);

			if (pSrc->GetOwner() != OWNER_ALLIED) {
				continue;
			}
			if (static_cast<int>(spareShips[pSrcID]) <= 0) {
				continue;
			}
			if (GameMap::GetDistance(pDst, pSrc) > maxTurns) {
				// continue;
			}
			if (pSrc->GetIsFrontierPlanet(OWNER_ALLIED) && (pSrc->GetIncomingFleetIDs(OWNER_ALLIED)).empty()) {
				// continue;
			}

			srcShips  = spareShips[pSrc->GetID()];
			sumShips += srcShips;
			spareShips[pSrc->GetID()] -= srcShips;

			// anti-overkill
			if (sumShips > minShips) {
				spareShips[pSrc->GetID()] += srcShips;

				srcShips = srcShips - (sumShips - minShips);
				srcShips = std::min(spareShips[pSrc->GetID()], std::max(srcShips, 1U));

				spareShips[pSrc->GetID()] -= srcShips;
			}

			Order order(pSrc->GetID(), pDst->GetID(), srcShips);
			order.Issue(mGameState);
		}
	}
}



void PlanetWars::DefendFutureFrontierPlanets(
	std::vector<unsigned int>& spareShips,
	std::vector<GamePlanet*>& defendeePlanets,
	std::vector<GamePlanet*>& defenderPlanets,
	std::vector<GamePlanet*>& neutralPlanets
) {
	// copy our currently-owned and real-frontier planets
	std::vector<GamePlanet*> cpyDefendeePlanets(defendeePlanets.size());
	std::vector<GamePlanet*> cpyDefenderPlanets(defenderPlanets.size());
	std::copy(defenderPlanets.begin(), defenderPlanets.end(), cpyDefenderPlanets.begin());
	std::copy(defendeePlanets.begin(), defendeePlanets.end(), cpyDefendeePlanets.begin());
	std::vector<unsigned int> frontierStates(defendeePlanets.size(), 0);

	for (unsigned int n = 0; n < defendeePlanets.size(); n++) {
		frontierStates[n] = static_cast<unsigned int>(defendeePlanets[n]->GetIsFrontierPlanet(OWNER_ALLIED));
	}

	// compose the *FUTURE* front-line (note that this
	// should NOT be limited to OWNER_NEUTRAL planets!)
	//
	// we CANNOT use mGameStates[i].GetFrontierPlanets()
	// since a neutral might not necessarily become ours
	// (we only ASSUME it will here)
	for (unsigned int i = 0; i < neutralPlanets.size(); i++) {
		GamePlanet* pDst = neutralPlanets[i];

		if (pDst->GetIncomingFleetIDs(OWNER_ALLIED).empty()) {
			// not a claimed planet
			continue;
		}

		// temporarily change the owner
		pDst->SetOwner(OWNER_ALLIED);
		// temporarily add this planet to mOwnerPlanets[OWNER_ALLIED]
		defenderPlanets.push_back(pDst);
	}

	// temporarily erase our current front-line and repopulate
	// it (this is a reference to mFrontierPlanets[OWNER_ALLIED]
	defendeePlanets.clear();

	mGameState.FindFrontierPlanets(OWNER_ALLIED, defendeePlanets);
	DefendFrontierPlanets(spareShips, defendeePlanets, defenderPlanets);

	// undo changes
	for (unsigned int i = 0; i < neutralPlanets.size(); i++) {
		GamePlanet* pDst = neutralPlanets[i];

		if (pDst->GetIncomingFleetIDs(OWNER_ALLIED).empty()) {
			continue;
		}

		pDst->SetOwner(OWNER_NEUTRAL);
	}

	defenderPlanets.clear(); defenderPlanets.resize(cpyDefenderPlanets.size());
	defendeePlanets.clear(); defendeePlanets.resize(cpyDefendeePlanets.size());
	std::copy(cpyDefenderPlanets.begin(), cpyDefenderPlanets.end(), defenderPlanets.begin());
	std::copy(cpyDefendeePlanets.begin(), cpyDefendeePlanets.end(), defendeePlanets.begin());

	for (unsigned int n = 0; n < defendeePlanets.size(); n++) {
		defendeePlanets[n]->SetIsFrontierPlanet(OWNER_ALLIED, static_cast<bool>(frontierStates[n]));
	}
}



void PlanetWars::DefendFrontierPlanets(
	std::vector<unsigned int>& spareShips,
	std::vector<GamePlanet*>& defendeePlanets,
	std::vector<GamePlanet*>& defenderPlanets
) {
	if (defendeePlanets.empty()) {
		return;
	}

	// for each of our current non-frontier planets,
	// reinforce the front-line planet closest to it
	// (no routing)
	for (unsigned int i = 0; i < defenderPlanets.size(); i++) {
		const GamePlanet* pSrc = defenderPlanets[i];

		if (static_cast<int>(spareShips[pSrc->GetID()]) <= 0) {
			continue;
		}
		if (pSrc->GetIsFrontierPlanet(OWNER_ALLIED)) {
			// frontier planets should not reinforce other frontier planets
			continue;
		}

		const GamePlanet* pDst = GameMap::GetClosestFrontierPlanet(mGameState, pSrc, OWNER_ALLIED);
		const GamePlanet* pHub = NULL; // GameMap::GetHubPlanet(mGameState, pSrc, pDst, 1);
		const GamePlanet* pTgt = NULL;

		if (pDst != NULL) { pTgt = pDst; }
		if (pHub != NULL) { pTgt = pHub; }

		if (pTgt != NULL) {
			Order order(pSrc->GetID(), pTgt->GetID(), spareShips[pSrc->GetID()]);
			order.Issue(mGameState);

			spareShips[pSrc->GetID()] = 0;
		}
	}
}







		/*
		// for each of our planets, calculate how "safe" it is
		//
		//  will this actually work? planets that are FURTHER from
		//  an enemy will see *LARGER* future populations and thus
		//  bigger threats
		for (unsigned int n = 0; n < alliedPlanets.size(); n++) {
			GamePlanet* pDst = alliedPlanets[n];

			unsigned int planets = 0;
			double safety = pDst->GetNumShips() - pDst->GetNumReservedShips();

			const SortedList<unsigned int, unsigned int>* pNgbs = mPlanetNeighbors[pDst->GetID()];

			for (KVPIt it = pNgbs->begin(); it != pNgbs->end(); ++it) {
				const unsigned int pSrcID = (*it).second;
				const GamePlanet* pSrc = &mGameState.GetPlanet(pSrcID);
				const unsigned int d = std::max(1U, mMaxPlanetDistance - GetDistance(pSrc, pDst));

				switch (pSrc->GetOwner()) {
					case OWNER_ALLIED: {
						safety += (mGameState.GetPlanetFuturePopulation(pSrc->GetID(), d, NULL, NULL, NULL, NULL) * (pSrc->GetGrowthRate() + 1));
						planets += 1;
					} break;
					case OWNER_ENEMY: {
						safety -= (mGameState.GetPlanetFuturePopulation(pSrc->GetID(), d, NULL, NULL, NULL, NULL) * (pSrc->GetGrowthRate() + 1));
						planets += 1;
					} break;
					default: {
					} break;
				}

				if (planets > K) {
					break;
				}
			}

			pDst->SetTmpThreat(safety);
		}

		std::sort(alliedPlanets.begin(), alliedPlanets.end(), PlanetSortFunctor(SORTMODE_TMPTHREAT_INCR));


		for (unsigned int i = 0; i <= L; i++) {
			const GamePlanet* pDst = alliedPlanets[i];

			// make each planet NOT in the top-L of most-unsafe planets
			// send reinforcement streams to the L most-unsafe planets
			// this does not seem to work as well as "make each planet
			// send ships a neighbor closer to the enemy"
			for (unsigned int j = alliedPlanets.size() - 1; j > L; j--) {
				const GamePlanet* pSrc = alliedPlanets[j];

				if (static_cast<int>(spareShips[pSrc->GetID()]) <= 0) {
					continue;
				}

				const unsigned int numShips = std::max(1U, spareShips[pSrc->GetID()] / L);

				IssueRealOrder(pSrc->GetID(), pDst->GetID(), numShips);
				spareShips[pSrc->GetID()] -= numShips;
			}
		}
		*/
