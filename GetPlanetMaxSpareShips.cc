#include "PlanetWars.h"

// returns the absolute maximum number of ships this planet
// can spare for a reinforcement or capture operation given
// [potential] incoming threat(s)
//
// NOTE: use this to sort planets by smallest maxSpareShips and
// reinforce those in order (prioritizing those that will be lost)?
// won't work well: newly-captured inner planets would be at the
// top of the list, rather than those near the frontier
//
// NOTE: if a mass-attack is launched at the same time neighbors
// of <pID> send their ships away to capture neutrals, then these
// estimates are too optimistic and the ability to defend <pID> is
// misjudged
//
// NOTE: maybe restrict mass-attack scenario to enemy front-line
// planets?
unsigned int GameState::GetPlanetMaxSpareShips(unsigned int pID) const {
	const GamePlanet& p = mPlanets[pID];

	if (p.GetOwner() == OWNER_NEUTRAL) {
		return 0;
	}

	// number of ships arriving per turn
	std::map<unsigned int, int> arrivingShips;

	const unsigned int pOwn = p.GetOwner();
	const unsigned int pOpp = OWNER_OPPONENT(pOwn);
	const std::list<unsigned int>& fIDsOwn = p.GetIncomingFleetIDs(pOwn);
	const std::list<unsigned int>& fIDsOpp = p.GetIncomingFleetIDs(pOpp);

	// hypothetical part (mass-defense and mass-attack)
	for (unsigned int n = 0; n < mOwnerPlanets[pOwn].size(); n++) {
		const GamePlanet* gp = mOwnerPlanets[pOwn][n];
		const unsigned int dst = GameMap::GetDistance(&p, gp);
		const unsigned int ns = gp->GetNumShips();

		if (gp->GetID() != p.GetID()) {
			// any mass-defense reaction would lag 1 turn behind
			// NOTE: we add 2 to make the bot more conservative
			arrivingShips[dst + 2] += static_cast<int>(ns);
		}
	}
	for (unsigned int n = 0; n < mOwnerPlanets[pOpp].size(); n++) {
		const GamePlanet* gp = mOwnerPlanets[pOpp][n];
		const unsigned int dst = GameMap::GetDistance(&p, gp);
		const unsigned int ns = gp->GetNumShips();

		if (gp->GetIsFrontierPlanet(pOpp)) {
			arrivingShips[dst] -= static_cast<int>(ns);
		}
	}

	// actual part (defense and attack)
	for (std::list<unsigned int>::const_iterator it = fIDsOwn.begin(); it != fIDsOwn.end(); ++it) {
		const GameFleet& f = GetFleet(*it);

		arrivingShips[f.GetTurnsRemaining()] += static_cast<int>(f.GetNumShips());
	}
	for (std::list<unsigned int>::const_iterator it = fIDsOpp.begin(); it != fIDsOpp.end(); ++it) {
		const GameFleet& f = GetFleet(*it);

		arrivingShips[f.GetTurnsRemaining()] -= static_cast<int>(f.GetNumShips());
	}


	int numShips = static_cast<int>(p.GetNumShips());
	int difShips = numShips;

	unsigned int currTurn = 0;
	unsigned int prevTurn = 0;

	for (std::map<unsigned int, int>::const_iterator it = arrivingShips.begin(); it != arrivingShips.end(); ++it) {
		currTurn  = it->first;
		numShips += (p.GetGrowthRate() * (currTurn - prevTurn));
		numShips += it->second;
		prevTurn  = currTurn;

		if (numShips <= 0) {
			difShips = 0; break;
		} else {
			difShips = std::min(difShips, numShips);
		}
	}

	return (std::min(difShips, static_cast<int>(p.GetNumShips())));
}
