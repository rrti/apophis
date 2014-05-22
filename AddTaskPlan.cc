#include "PlanetWars.h"
#include "Debugger.h"
#include "Logger.h"

void PlanetWars::AddTaskPlan(
	const GamePlanet* pDst,
	unsigned int maxPlanTurns,
	unsigned int minPlanShips,
	std::map<unsigned int, unsigned int>& reservedShips,
	std::vector<unsigned int>& spareShips,
	const std::list<unsigned int>& taskPlanetIDs)
{
	unsigned int difPlanTurns = 0;
	unsigned int sumPlanShips = 0;
	unsigned int srcNumShips  = 0;

	std::list<GamePlanet*> taskPlanets;

	// if this planet's closest OWNER_ENEMY neighbor is as close
	// or closer than its closest OWNER_ALLIED neighbor, we want
	// to invest more than the minimum number of ships
	// (only checking if the planet is in MAP_REGION_INNER is not
	// good enough, it must lie "in the direction of OWNER_ENEMY")
	//
	// NOTE: the knapsack is filled based on FUTURE populations
	// and AVERAGE distances (<pDstAvgDist>, which means results
	// from CanCapturePlanet are not always reliable!!)
	//
	// NOTE: separate question whether to allow and how much
	// (too much overkill hurts rate of expansion at neutrals)
	//
	const GamePlanet* pDstNgbA = GameMap::GetClosestNeighborPlanet(mGameState, pDst->GetID(), OWNER_ALLIED);
	const GamePlanet* pDstNgbE = GameMap::GetClosestNeighborPlanet(mGameState, pDst->GetID(), OWNER_ENEMY);
	const bool allowOverkill = (GameMap::GetDistance(pDst, pDstNgbE) <= GameMap::GetDistance(pDst, pDstNgbA) || pDst->GetOwner() == OWNER_ENEMY);

	// generate new task-plans and reserve ships for
	// each planet <pSrc>, but do NOT issue orders yet
	for (std::list<unsigned int>::const_iterator it = taskPlanetIDs.begin(); it != taskPlanetIDs.end() && sumPlanShips <= minPlanShips; ++it) {
		GamePlanet* pSrc = &mGameState.GetPlanet(*it);

		BOT_ASSERT(spareShips[pSrc->GetID()] > 0);

		taskPlanets.push_back(pSrc);

		srcNumShips = spareShips[pSrc->GetID()];
		sumPlanShips += srcNumShips;
		spareShips[pSrc->GetID()] -= srcNumShips;
		reservedShips[pSrc->GetID()] = srcNumShips;

		if (sumPlanShips > minPlanShips) {
			if (!allowOverkill) {
				spareShips[pSrc->GetID()] += srcNumShips;

				srcNumShips = srcNumShips - (sumPlanShips - minPlanShips);
				srcNumShips = std::min(spareShips[pSrc->GetID()], std::max(srcNumShips, 1U));

				spareShips[pSrc->GetID()] -= srcNumShips;
				reservedShips[pSrc->GetID()] = srcNumShips;
			} else {
				spareShips[pSrc->GetID()] += srcNumShips;

				srcNumShips = srcNumShips - (sumPlanShips - minPlanShips);
				srcNumShips = srcNumShips + ((sumPlanShips - minPlanShips) >> 2) + 1;
				srcNumShips = std::min(spareShips[pSrc->GetID()], std::max(srcNumShips, 1U));

				spareShips[pSrc->GetID()] -= srcNumShips;
				reservedShips[pSrc->GetID()] = srcNumShips;
			}
		}

		BOT_ASSERT(srcNumShips > 0);
	}


	// we want all ships to arrive on the same turn
	// therefore, attackers closer than the farthest
	// must wait and hold ships in reserve for some
	// turns
	//
	// NOTE: during this time, the planets that must
	// wait will also gain ships again, or could be
	// attacked so predictions are thrown off, etc.
	// (or even captured by enemy!) ==> huge number
	// of consistency-checks needed
	TaskPlan taskPlan(mCurrentTurn);

	for (std::list<GamePlanet*>::iterator it = taskPlanets.begin(); it != taskPlanets.end(); ++it) {
		GamePlanet* pSrc = *it;

		// calculate the number of turns this attacker needs to wait
		// (relative to the current turn) before sending out ships
		difPlanTurns = maxPlanTurns - GameMap::GetDistance(pDst, pSrc);

		taskPlan.AddMember(pSrc, reservedShips[pSrc->GetID()], difPlanTurns);
		pSrc->SetNumReservedShips(pSrc->GetNumReservedShips() + reservedShips[pSrc->GetID()]);
	}

	mTaskPlans[pDst->GetID()] = taskPlan;
}
