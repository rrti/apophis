#include <algorithm>

#include "PlanetWars.h"
#include "Debugger.h"
#include "Logger.h"

bool PlanetWars::CanCapturePlanetNN(
	const GamePlanet* pDst,
	unsigned int maxNeighbors,
	const std::vector<unsigned int>& spareShips,
	std::list<unsigned int>& taskPlanetIDs,
	unsigned int* minPlanTurns,
	unsigned int* maxPlanTurns,
	unsigned int* avgPlanTurns,
	unsigned int* minPlanShips
) const {
	taskPlanetIDs.clear();

	*minPlanTurns = GameMap::GetMaxPlanetDistance() + 1;
	*maxPlanTurns = 0;
	*avgPlanTurns = 0;
	*minPlanShips = 0;

	typedef SortedList<unsigned int, unsigned int>::KeyValPair KVP;
	typedef std::list<KVP>::const_iterator KVPIt;

	const SortedList<unsigned int, unsigned int>* pNgbs = GameMap::GetPlanetNeighbors(pDst->GetID());

	unsigned int numNeighbors = 0;
	unsigned int sumPlanShips = 0;

	for (KVPIt it = pNgbs->begin(); it != pNgbs->end() && numNeighbors < maxNeighbors; ++it) {
		const unsigned int pSrcID = (*it).second;
		const GamePlanet* pSrc = &mGameState.GetPlanet(pSrcID);

		if (pSrc->GetOwner() != OWNER_ALLIED) { continue; }
		if (!pSrc->GetIsFrontierPlanet(OWNER_ALLIED)) { continue; }
		if (static_cast<int>(spareShips[pSrc->GetID()]) <= 0) { continue; }
		if (!pSrc->GetIncomingFleetIDs(OWNER_ENEMY).empty()) { continue; }

		sumPlanShips += spareShips[pSrc->GetID()];
		*minPlanTurns = std::min(*minPlanTurns, GameMap::GetDistance(pDst, pSrc));
		*maxPlanTurns = std::max(*maxPlanTurns, GameMap::GetDistance(pDst, pSrc));
		*avgPlanTurns += GameMap::GetDistance(pDst, pSrc);

		taskPlanetIDs.push_back(pSrcID);

		numNeighbors++;
	}

	if (*maxPlanTurns ==                                   0) { return false; }
	if (*minPlanTurns == GameMap::GetMaxPlanetDistance() + 1) { return false; }

	*minPlanShips = mGameState.GetPlanetFuturePopulation(pDst->GetID(), *maxPlanTurns, NULL, NULL, NULL, NULL) + 1;
	*avgPlanTurns /= taskPlanetIDs.size();

	return (sumPlanShips >= *minPlanShips);
}



bool PlanetWars::CanCapturePlanet(
	const GamePlanet* pDst,
	const std::set<GamePlanet*>& attackerPlanets,
	const std::vector<unsigned int>& spareShips,
	std::list<unsigned int>& taskPlanetIDs,
	unsigned int* minPlanTurns,
	unsigned int* maxPlanTurns,
	unsigned int* avgPlanTurns,
	unsigned int* minPlanShips
) const {
	std::vector<GamePlanet*> attackerPlanetsVec(attackerPlanets.size());
	std::copy(attackerPlanets.begin(), attackerPlanets.end(), attackerPlanetsVec.begin());

	return (CanCapturePlanet(pDst, attackerPlanetsVec, spareShips, taskPlanetIDs, minPlanTurns, maxPlanTurns, avgPlanTurns, minPlanShips));
}

bool PlanetWars::CanCapturePlanet(
	const GamePlanet* pDst,
	const std::vector<GamePlanet*>& attackerPlanets,
	const std::vector<unsigned int>& spareShips,
	std::list<unsigned int>& taskPlanetIDs,
	unsigned int* minPlanTurns,
	unsigned int* maxPlanTurns,
	unsigned int* avgPlanTurns,
	unsigned int* minPlanShips
) const {
	taskPlanetIDs.clear();

	*minPlanTurns = GameMap::GetMaxPlanetDistance() + 1;
	*maxPlanTurns = 0;
	*avgPlanTurns = 0;
	*minPlanShips = 0;

	unsigned int sumPlanShips = 0;


	/*
	typedef SortedList<unsigned int, unsigned int>::KeyValPair KVP;
	typedef std::list<KVP>::const_iterator KVPIt;

	const SortedList<unsigned int, unsigned int>* pNgbs = GameMap::GetPlanetNeighbors(pDst->GetID());

	for (KVPIt it = pNgbs->begin(); it != pNgbs->end(); ++it) {
		const unsigned int pSrcID = (*it).second;
		const GamePlanet* pSrc = &mGameState.GetPlanet(pSrcID);

		if (pSrc->GetOwner() != OWNER_ALLIED) { continue; }
		if (!pSrc->GetIsFrontierPlanet(OWNER_ALLIED)) { continue; }
		if (!pSrc->GetIncomingFleetIDs(OWNER_ENEMY).empty()) { continue; }
		if (static_cast<int>(spareShips[pSrc->GetID()]) <= 0) { continue; }
		if (std::find(attackerPlanets.begin(), attackerPlanets.end(), pSrc) == attackerPlanets.end()) { continue; } // inefficient

		LOG_STDOUT(LOGGER, "\t\t\t[attacker] pSrc->GetID()=%u, DIST(pSrc, pDst)=%u, pSrc->GetNumShips()=%u, spareShips[pSrc->GetID()]=%u\n", pSrc->GetID(), GameMap::GetDistance(pSrc, pDst), pSrc->GetNumShips(), spareShips[pSrc->GetID()]);

		// calculate how many turns separate the closest
		// and furthest attacking planet <pSrc> from <pDst>
		sumPlanShips += spareShips[pSrc->GetID()];
		*minPlanTurns = std::min(*minPlanTurns, GameMap::GetDistance(pDst, pSrc));
		*maxPlanTurns = std::max(*maxPlanTurns, GameMap::GetDistance(pDst, pSrc));
		*avgPlanTurns += GameMap::GetDistance(pDst, pSrc);

		taskPlanetIDs.push_back(pSrc->GetID());
	}
	*/


	// evaluate all of our non-attacked frontier planets
	//
	// FIXME: <attackerPlanets> is sorted by remaining number of
	// ships, but should also be sorted by increasing distance to
	// <pDst> (implemented above, but does not work either because
	// of order in which each <pDst> is visited in CapturePlanets)
	//
	// more general but harder: a sorting of attackeePlanets based
	// on ROI time wrt. a group of <attackerPlanets>
	//
	//     for each potential attackee
	//         calculate sum-ROI-time of attackee wrt. all attackers (front-line planets)
	//     sort attackees by sum-ROI-time in increasing order
	for (unsigned int n = 0; n < attackerPlanets.size(); n++) {
		const GamePlanet* pSrc = attackerPlanets[n];

		if (static_cast<int>(spareShips[pSrc->GetID()]) <= 0) { continue; }
		if (!pSrc->GetIncomingFleetIDs(OWNER_ENEMY).empty()) { continue; }

		BOT_ASSERT(pSrc->GetIsFrontierPlanet(OWNER_ALLIED));

		// calculate how many turns separate the closest
		// and furthest attacking planet <pSrc> from <pDst>
		sumPlanShips += spareShips[pSrc->GetID()];
		*minPlanTurns = std::min(*minPlanTurns, GameMap::GetDistance(pDst, pSrc));
		*maxPlanTurns = std::max(*maxPlanTurns, GameMap::GetDistance(pDst, pSrc));
		*avgPlanTurns += GameMap::GetDistance(pDst, pSrc);

		taskPlanetIDs.push_back(pSrc->GetID());
	}

	if (*maxPlanTurns ==                                   0) { return false; }
	if (*minPlanTurns == GameMap::GetMaxPlanetDistance() + 1) { return false; }

	// NOTE: add multiplier here to make the bot more/less conservative?
	// FIXME: use the same population-minimum as in FindKnapSackPlanets?
	//
	// const GameMap& gameMap = mGameState.GetGameMap();
	//
	// const double pDstAvgDist = std::max(1.0, mGameMap.GetAvgPlanetDistance(*this, pDst->GetID(), OWNER_ALLIED));
	// const unsigned int pDstFutPop = mGameState.GetPlanetFuturePopulation(pDst->GetID(), pDstAvgDist, NULL, NULL, NULL, NULL) + 1);
	//
	// *minPlanShips = pDstFutPop;
	*minPlanShips = mGameState.GetPlanetFuturePopulation(pDst->GetID(), *maxPlanTurns, NULL, NULL, NULL, NULL) + 1;
	*avgPlanTurns /= taskPlanetIDs.size();

	LOG_STDOUT(LOGGER, "[CanCapturePlanet] pDst->GetID()=%u, pDst->GetNumShips()=%u, pDst->GetGrowthRate()=%u, attackerPlanets.size()=%u, minPlanShips=%u, sumPlanShips=%u\n", pDst->GetID(), pDst->GetNumShips(), pDst->GetGrowthRate(), attackerPlanets.size(), *minPlanShips, sumPlanShips);

	return (sumPlanShips >= *minPlanShips);
}
