#include "PlanetWars.h"
#include "Debugger.h"

unsigned int GameState::GetPlanetFuturePopulation(
	unsigned int planetID,
	unsigned int numFutureTurns,
	unsigned int* ownerChangedTurn,
	unsigned int* firstFutureOwner,
	std::vector<unsigned int>* ownerPerTurn,
	std::vector<unsigned int>* shipsPerTurn
) const {
	BOT_ASSERT(numFutureTurns >=                              1U);
	BOT_ASSERT(numFutureTurns <= GameMap::GetMaxPlanetDistance());

	unsigned int r = 0;

	if (ownerPerTurn != NULL) { ownerPerTurn->resize(numFutureTurns); }
	if (shipsPerTurn != NULL) { shipsPerTurn->resize(numFutureTurns); }
	if (ownerChangedTurn != NULL) { (*ownerChangedTurn) = -1U; }
	if (firstFutureOwner != NULL) { (*firstFutureOwner) = -1U; }

	// mGameState contains a stale copy of <p> (if <p> has
	// had RemoveShips() called on it after CalcTurnData()),
	// so we have to use an external clone <gp>
	std::vector<GamePlanet> planetStates;
	std::map<unsigned int, GameFleet> fleetStates(mFleets);

	// note: planetStates[0] is the FIRST future turn, etc.
	// <numFutureTurns> may be at most <mMaxPlanetDistance>
	const GamePlanet& planet = mPlanets[planetID];

	planet.ExecTurns(numFutureTurns, fleetStates, planetStates);

	for (unsigned int n = 0; n < numFutureTurns; n++) {
		const GamePlanet& planetState = planetStates[n];

		if (ownerPerTurn != NULL) { (*ownerPerTurn)[n] = planetState.GetOwner();    }
		if (shipsPerTurn != NULL) { (*shipsPerTurn)[n] = planetState.GetNumShips(); }

		if (planetState.GetOwner() != planet.GetOwner()) {
			if (ownerChangedTurn != NULL && (*ownerChangedTurn == -1U)) { (*ownerChangedTurn) =                  n + 1; }
			if (firstFutureOwner != NULL && (*firstFutureOwner == -1U)) { (*firstFutureOwner) = planetState.GetOwner(); }
		}

		r = planetState.GetNumShips();
	}

	return r;
}
