#include <algorithm>
#include <limits>

#include "PlanetWars.h"
#include "FindPowerSet.h"
#include "Debugger.h"
#include "Logger.h"

void PlanetWars::AssignAttackersToAttackees(
	std::map<unsigned int, std::set<GamePlanet*> >& attackersPerAttackee,
	const std::vector< std::vector< std::set<GamePlanet*> > >& attackersPowerSet,
	const std::vector<GamePlanet*>& attackees,
	const std::vector<unsigned int>& spareShips
) {
	// if (attackers.empty()) { return; }
	if (attackees.empty()) { return; }

	for (unsigned int m = 0; m < attackees.size(); m++) {
		attackees[m]->SetTmpROITime(std::numeric_limits<double>::max());
	}

	for (unsigned int n = 0; n < attackersPowerSet.size(); n++) {
		// get all sub-sets of size <n + 1>
		const std::vector< std::set<GamePlanet*> >& attackersSubSets = attackersPowerSet[n];

		for (unsigned int k = 0; k < attackersSubSets.size(); k++) {
			// for each sub-set <k> of size <n + 1>
			const std::set<GamePlanet*>& attackersSubSet = attackersSubSets[k];

			if (attackersSubSet.empty()) {
				continue;
			}

			// find the ROI-time of each potential attackee wrt. this <attackersSubSet>
			for (unsigned int m = 0; m < attackees.size(); m++) {
				GamePlanet* attackee = attackees[m];

				unsigned int minPlanTurns = GameMap::GetMaxPlanetDistance() + 1;
				unsigned int maxPlanTurns = 0;
				unsigned int avgPlanTurns = 0;
				unsigned int minPlanShips = 0;

				std::list<unsigned int> tmp;

				LOG_STDOUT(LOGGER, "[AssignAttackersToAttackees][n=%u, k=%u, m=%u]\n", n, k, m);
				LOG_STDOUT(LOGGER, "\tattackeeID=%u (popul.=%u, gRate.=%u, owner=%u)\n", attackee->GetID(), attackee->GetNumShips(), attackee->GetGrowthRate(), attackee->GetOwner());

				if (!CanCapturePlanet(attackee, attackersSubSet, spareShips, tmp, &minPlanTurns, &maxPlanTurns, &avgPlanTurns, &minPlanShips)) {
					continue;
				}

				// NOTE:
				//     do we want <maxPlanTurns> for the ROI-time or (eg.) the average?
				//     if multiple attackersSubSet's have the same ROI, averaging could
				//     help break ties, but the ROI predictions become less accurate
				const unsigned int attackeePop = mGameState.GetPlanetFuturePopulation(attackee->GetID(), avgPlanTurns, NULL, NULL, NULL, NULL);
				const double attackeeROITime = attackee->CalcReturnInvestmentTime(avgPlanTurns, attackeePop);

				LOG_STDOUT(LOGGER, "\tCanCapture(%u)=true: futurePop.=%u, ROI-time=%g, minPlanTurns=%u, maxPlanTurns=%u, avgPlanTurns=%u\n", attackee->GetID(), attackeePop, attackeeROITime, minPlanTurns, maxPlanTurns, avgPlanTurns);

				/*
				if (attackeeROITime > 50.0 && sumGrowth(OWNER_ALLIED) >= sumGrowth(OWNER_ENEMY)) {
					continue;
				}
				*/
				if (attackeeROITime < attackee->GetTmpROITime()) {
					attackee->SetTmpROITime(attackeeROITime);
					attackersPerAttackee[attackee->GetID()] = attackersSubSet;
				}
			}
		}
	}
}






#if (1 == 1)
void PlanetWars::DoTurn() {
	LOG_STDOUT(LOGGER, "[DoTurn][mCurrentTurn=%u]\n", mCurrentTurn);

	std::vector<GamePlanet*>& alliedPlanets = mGameState.GetOwnerPlanets(OWNER_ALLIED);
	std::vector<GamePlanet*>& enemyPlanets = mGameState.GetOwnerPlanets(OWNER_ENEMY);
	std::vector<GamePlanet*>& attackerPlanets = mGameState.GetFrontierPlanets(OWNER_ALLIED); // frontier planets of OWNER_ALLIED
	std::vector<GamePlanet*>  attackeePlanetsN;
	std::vector<GamePlanet*>& attackeePlanetsE = mGameState.GetFrontierPlanets(OWNER_ENEMY);  // frontier planets of OWNER_ENEMY
	std::vector<unsigned int> spareShips(mGameState.GetNumPlanets());

	mStaleMateTurns.push_front(mGameState.IsStaleMate());



	/*
	#define DEFEND() {                                          \
		SetPlanetValues(OWNER_ALLIED, alliedPlanets);           \
		UpdatePlanetSpareShipCounts(alliedPlanets, spareShips); \
		AddDefenseTaskPlans(alliedPlanets, spareShips);         \
	}

	#define ATTACK() {                                                      \
		if (true) {                                                         \
			SetPlanetValues(OWNER_ALLIED, attackeePlanetsE);                \
			UpdatePlanetSpareShipCounts(attackerPlanets, spareShips);       \
			CapturePlanets(attackerPlanets, attackeePlanetsE, spareShips);  \
		} else {                                                            \
			SetPlanetValues(OWNER_ALLIED, enemyPlanets);                    \
			UpdatePlanetSpareShipCounts(attackerPlanets, spareShips);       \
			CapturePlanets(attackerPlanets, enemyPlanets, spareShips);      \
		}                                                                   \
	}

	#define EXPAND() {                                                      \
		if (maxSackGrowthTurn == 0) {                                       \
			SetPlanetValues(OWNER_ALLIED, attackeePlanetsN);                \
			UpdatePlanetSpareShipCounts(attackerPlanets, spareShips);       \
			CapturePlanets(attackerPlanets, attackeePlanetsN, spareShips);  \
		}                                                                   \
	}

	#undef EXPAND
	#undef ATTACK
	#undef DEFEND
	*/



	{
		// attackee planetID ==> attacker planets
		std::map<unsigned int, std::set<GamePlanet*> > attackersPerAttackee;
		// set of all front-line planets capable of attacking
		std::set<GamePlanet*> attackersSet;
		std::vector< std::vector< std::set<GamePlanet*> > > attackersPowerSet;

		// copy <attackerPlanets> into a std::set
		for (unsigned int n = 0; n < attackerPlanets.size(); n++) {
			attackersSet.insert(attackerPlanets[n]);
		}

		// get the set of all subsets of <attackersSet>
		FindPowerSet(attackersSet, attackersPowerSet, attackersSet.size());
		// initialize GamePlanet::numSpareShips for all attackers (stupid)
		// SetPlanetValues(OWNER_ALLIED, attackerPlanets);

		{
			LOG_STDOUT(LOGGER, "[DoTurn][Defend]\n");

			// defend (NOTE: alliedPlanets contains attackerPlanets)
			SetPlanetValues(OWNER_ALLIED, alliedPlanets);
			UpdatePlanetSpareShipCounts(alliedPlanets, spareShips);
			AddDefenseTaskPlans(alliedPlanets, spareShips);
		}

		{
			LOG_STDOUT(LOGGER, "[DoTurn][Expand]\n");

			// expand
			UpdatePlanetSpareShipCounts(attackerPlanets, spareShips);
			GameMap::GetSafeNeutralPlanets(mGameState, attackeePlanetsN, OWNER_ALLIED);
			AssignAttackersToAttackees(attackersPerAttackee, attackersPowerSet, attackeePlanetsN, spareShips);

			if (!attackersPerAttackee.empty()) {
				std::sort(attackeePlanetsN.begin(), attackeePlanetsN.end(), PlanetSortFunctor(SORTMODE_TMPROITIME_INCR));

				// don't iterate directly over <attackersPerAttackee>;
				// planets are not sorted in order of increasing ROI
				//
				// note: after one set of attackers has issued orders,
				// the remaining assignments may no longer be "optimal"
				// ==> do we recalculate all ROI's or ignore this?
				for (unsigned int n = 0; n < attackeePlanetsN.size(); n++) {
					std::map<unsigned int, std::set<GamePlanet*> >::iterator it = attackersPerAttackee.find(attackeePlanetsN[n]->GetID());

					if (it != attackersPerAttackee.end()) {
						CapturePlanet(*attackeePlanetsN[n], it->second, spareShips);
					}
				}

				attackersPerAttackee.clear();
			}
		}

		{
			LOG_STDOUT(LOGGER, "[DoTurn][Attack] attackeePlanetsE.size()=%u\n", attackeePlanetsE.size());

			// attack
			UpdatePlanetSpareShipCounts(attackerPlanets, spareShips);
			AssignAttackersToAttackees(attackersPerAttackee, attackersPowerSet, attackeePlanetsE, spareShips);

			if (!attackersPerAttackee.empty()) {
				std::sort(attackeePlanetsE.begin(), attackeePlanetsE.end(), PlanetSortFunctor(SORTMODE_TMPROITIME_INCR));

				for (unsigned int n = 0; n < attackeePlanetsE.size(); n++) {
					std::map<unsigned int, std::set<GamePlanet*> >::iterator it = attackersPerAttackee.find(attackeePlanetsE[n]->GetID());

					if (it != attackersPerAttackee.end()) {
						CapturePlanet(*attackeePlanetsE[n], it->second, spareShips);
					}
				}

				attackersPerAttackee.clear();
			} else {
				if (DetectStaleMate(20)) {
					AssignAttackersToAttackees(attackersPerAttackee, attackersPowerSet, enemyPlanets, spareShips);
					std::sort(enemyPlanets.begin(), enemyPlanets.end(), PlanetSortFunctor(SORTMODE_TMPROITIME_INCR));

					for (unsigned int n = 0; n < enemyPlanets.size(); n++) {
						std::map<unsigned int, std::set<GamePlanet*> >::iterator it = attackersPerAttackee.find(enemyPlanets[n]->GetID());

						if (it != attackersPerAttackee.end()) {
							CapturePlanet(*enemyPlanets[n], it->second, spareShips);
						}
					}
				}
			}
		}
	}

	ExecuteTaskPlans();
}






bool PlanetWars::CapturePlanet(const GamePlanet& attackee, std::set<GamePlanet*>& attackers, std::vector<unsigned int>& spareShips) {
	unsigned int attackeeOwner = attackee.GetOwner();
	unsigned int attackeeShips = attackee.GetNumShips();

	// const GameMap& gameMap = mGameState.GetGameMap();
	// const double pDstAvgDist = std::max(1.0, gameMap.GetAvgPlanetDistance(mGameState, attackee.GetID(), OWNER_ALLIED));
	//
	// attackeeShips = mGameState.GetPlanetFuturePopulation(attackee.GetID(), pDstAvgDist, NULL, &attackeeOwner, NULL, NULL);
	attackeeShips = mGameState.GetPlanetFuturePopulation(attackee.GetID(), GameMap::GetMaxPlanetDistance(), NULL, &attackeeOwner, NULL, NULL);

	LOG_STDOUT(LOGGER, "[CapturePlanet] attackee.GetID()=%u, attackee.GetNumShips()=%u, attackee.GetGrowthRate()=%u\n", attackee.GetID(), attackee.GetNumShips(), attackee.GetGrowthRate());

	if (!attackee.GetIncomingFleetIDs(OWNER_ALLIED).empty()) {
		if (attackeeOwner == OWNER_ALLIED) {
			// planet is already going to be ours, or is not in
			// MAP_REGION_INNER, so no need to send more ships
			return false;
		}
	}

	if (mTaskPlans.find(attackee.GetID()) != mTaskPlans.end()) {
		// there is a still-running plan to capture <pDst>
		return false;
	}

	std::map<unsigned int, unsigned int> reservedShips;
	std::list<unsigned int> taskPlanetIDs;

	unsigned int minPlanTurns = GameMap::GetMaxPlanetDistance() + 1;
	unsigned int maxPlanTurns = 0;
	unsigned int avgPlanTurns = 0;
	unsigned int minPlanShips = 0;

	if (!CanCapturePlanet(&attackee, attackers, spareShips, taskPlanetIDs, &minPlanTurns, &maxPlanTurns, &avgPlanTurns, &minPlanShips)) {
		return false;
	}

	// at this point, it is "guaranteed" that the planet(s)
	// in <taskPlanetIDs> can capture <pDst> but it is not
	// necessarily the best choice for these attackers
	AddTaskPlan(&attackee, maxPlanTurns, minPlanShips, reservedShips, spareShips, taskPlanetIDs);
	return true;
}



void PlanetWars::CapturePlanets(
	std::vector<GamePlanet*>& attackerPlanets,
	std::vector<GamePlanet*>& attackeePlanets,
	std::vector<unsigned int>& spareShips
) {
	if (attackerPlanets.empty()) { return; }
	if (attackeePlanets.empty()) { return; }

	std::set<GamePlanet*> attackerPlanetsSet;

	for (unsigned int n = 0; n < attackerPlanets.size(); n++) {
		attackerPlanetsSet.insert(attackerPlanets[n]);
	}

	for (unsigned int n = 0; n < attackeePlanets.size(); n++) {
		CapturePlanet(*attackeePlanets[n], attackerPlanetsSet, spareShips);
	}
}



#else



/*
void PlanetWars::DoTurn() {
	const GameMap& gameMap = mGameState.GetGameMap();
	const GameStats& gameStats = mGameState.GetGameStats();

	std::vector<GamePlanet*>& alliedPlanets = mGameState.GetOwnerPlanets(OWNER_ALLIED);
	std::vector<GamePlanet*>& neutralPlanets = mGameState.GetOwnerPlanets(OWNER_NEUTRAL);
	std::vector<GamePlanet*>& enemyPlanets = mGameState.GetOwnerPlanets(OWNER_ENEMY);

	if (alliedPlanets.empty()) {
		return;
	}

	const bool attackUnsafePlanets =
		(gameStats.GetPlanetCount(OWNER_ALLIED) >= gameStats.GetPlanetCount(OWNER_ENEMY) * 2) &&
		(gameStats.GetSumNumShips(OWNER_ALLIED) >= gameStats.GetSumNumShips(OWNER_ENEMY) * 2) &&
		(gameStats.GetSumGrowthRate(OWNER_ALLIED) >= gameStats.GetSumGrowthRate(OWNER_ENEMY) * 2);

	// LOG_STDOUT(LOGGER, "\n");
	// LOG_STDOUT(LOGGER, "[DoTurn][mCurrentTurn=%u][attackUnsafePlanets=%d][GetMaxSackGrowthTurn(2, OWNER_ALLIED)=%u\n", mCurrentTurn, attackUnsafePlanets, GetMaxSackGrowthTurn(2, OWNER_ALLIED));


	// calculate how many ships each of our planets can spare this turn
	std::vector<unsigned int> spareShips(mGameState.GetNumPlanets());

	for (unsigned int n = 0; n < alliedPlanets.size(); n++) {
		GamePlanet* p = alliedPlanets[n];

		{
			// will be >= 0 if planet is going to change owner
			unsigned int pTTL = -1U;

			mGameState.GetPlanetFuturePopulation(p->GetID(), GameMap::GetMaxPlanetDistance(), &pTTL, NULL, NULL, NULL);

			if (pTTL != -1U) {
				BOT_ASSERT(pTTL <= GameMap::GetMaxPlanetDistance());

				const GameState& gs = mGameStates[pTTL - 1];
				const GamePlanet& gp = gs.GetPlanet(p->GetID());

				// the planet's enemy population right after
				// being captured equals the number of ships
				// we came up short
				// multiply this by the growth-rate so we are
				// more inclined to assist big planets first
				p->SetNumSpareShips(0);
				p->SetTmpValue(gp.GetNumShips() * p->GetGrowthRate() * p->GetGrowthRate());
				p->SetTmpDistance(pTTL);
				p->SetTmpThreat(std::numeric_limits<double>::max());
			} else {
				p->SetNumSpareShips(mGameState.GetPlanetMaxSpareShips(p->GetID()));
				p->SetTmpValue(-std::numeric_limits<double>::max());
				p->SetTmpDistance(-1U);
				p->SetTmpThreat(0.0);
			}
		}

		spareShips[p->GetID()] = p->GetNumSpareShips() - p->GetNumReservedShips();

		// LOG_STDOUT(LOGGER, "\tp->GetID()=%u (numShips=%u, numSpareShips=%u)\n", p->GetID(), p->GetNumShips(), spareShips[p->GetID()]);
	}

	for (unsigned int n = 0; n < neutralPlanets.size(); n++) {
		GamePlanet* p = neutralPlanets[n];

		unsigned int nt = GameMap::GetMaxPlanetDistance();
		const unsigned int ad = std::max(1.0, gameMap.GetAvgPlanetDistance(mGameState, p->GetID(), OWNER_ALLIED));
		const unsigned int fp = mGameState.GetPlanetFuturePopulation(p->GetID(), ad, &nt, NULL, NULL, NULL);
		const double v = p->CalcIntrinsicValue(OWNER_ALLIED, fp);
		const double d = gameMap.GetAvgPlanetDistance(mGameState, p->GetID(), OWNER_ALLIED);

		p->SetNumSpareShips(0);
		p->SetNumReservedShips(0);
		p->SetTmpValue(v);
		p->SetTmpThreat(0.0);

		if (v != -std::numeric_limits<double>::max()) {
			p->SetTmpValue(v / d);
		}

		// alternative suggested value-function
		// p->SetTmpValue(std::numeric_limits<double>::max() / ((p->GetNumShips() / (p->GetGrowthRate() + 1)) + std::log(std::pow(2, d))));
	}

	for (unsigned int n = 0; n < enemyPlanets.size(); n++) {
		GamePlanet* p = enemyPlanets[n];

		unsigned int nt = GameMap::GetMaxPlanetDistance();
		const unsigned int ad = std::max(1.0, gameMap.GetAvgPlanetDistance(mGameState, p->GetID(), OWNER_ALLIED));
		const unsigned int fp = mGameState.GetPlanetFuturePopulation(p->GetID(), ad, &nt, NULL, NULL, NULL);
		const double v = p->CalcIntrinsicValue(OWNER_ALLIED, fp);
		const double d = gameMap.GetAvgPlanetDistance(mGameState, p->GetID(), OWNER_ALLIED);

		p->SetNumSpareShips(0);
		p->SetNumReservedShips(0);
		p->SetTmpValue(v);
		p->SetTmpThreat(0.0);

		if (v != -std::numeric_limits<double>::max()) {
			p->SetTmpValue(v / d);
		}
	}


	std::sort(alliedPlanets.begin(), alliedPlanets.end(), PlanetSortFunctor(SORTMODE_TMPVALUE_DECR));
	std::sort(neutralPlanets.begin(), neutralPlanets.end(), PlanetSortFunctor(SORTMODE_TMPVALUE_DECR));
	std::sort(enemyPlanets.begin(), enemyPlanets.end(), PlanetSortFunctor(SORTMODE_TMPVALUE_DECR));

	AddDefenseTaskPlans(spareShips);

	std::sort(alliedPlanets.begin(), alliedPlanets.end(), PlanetSortFunctor(SORTMODE_NUMSHIPS_DECR));

	AddOffenseTaskPlansOLD(OWNER_ENEMY, spareShips);
	AddOffenseTaskPlansOLD(OWNER_NEUTRAL, spareShips);

	if (attackUnsafePlanets) {
		AddOffenseTaskPlansOLD(OWNER_ENEMY, spareShips, PLANET_INNER_ENEMY_BIT);
	}

	ExecuteTaskPlans();
}
*/

#endif

