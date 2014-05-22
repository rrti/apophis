#include "PlanetWars.h"
#include "Debugger.h"

void GameState::FindFrontierPlanets(unsigned int owner, std::vector<GamePlanet*>& planets) {
	BOT_ASSERT(owner != OWNER_NEUTRAL);

	if (mOwnerPlanets[OWNER_OPPONENT(owner)].empty()) {
		return;
	}

	// if P is closer to its closest enemy neighbor than
	// any other allied planet, P is part of the frontier
	for (unsigned int i = 0; i < mOwnerPlanets[owner].size(); i++) {
		GamePlanet* pSrc = mOwnerPlanets[owner][i];
		pSrc->SetIsFrontierPlanet(owner, false);

		if (GameMap::IsFrontierPlanet(*this, pSrc, owner)) {
			pSrc->SetIsFrontierPlanet(owner, true);
			planets.push_back(pSrc);
		}
	}
}
