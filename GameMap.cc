#include <cmath>
#include <limits>
#include "GameState.h"
#include "Debugger.h"
#include "Logger.h"

std::vector< std::vector<unsigned int> > GameMap::gPlanetDistanceMatrix;
std::vector<SortedList<unsigned int, unsigned int>* > GameMap::gPlanetNeighbors;

double GameMap::gMapMinBounds[2] = {0.0};
double GameMap::gMapMaxBounds[2] = {0.0};
double GameMap::gMapDimension[2] = {0.0};

unsigned int GameMap::gMaxPlanetDistance = 0;



GameMap::~GameMap() {
	for (unsigned int n = 0; n < mPlanetAvgDistanceMatrix.size(); n++) {
		mPlanetAvgDistanceMatrix[n].clear();
	}

	mPlanetAvgDistanceMatrix.clear();
}

void GameMap::CalcFirstTurnData(GameState& gameState) {
	gMapMinBounds[0] = gMapMinBounds[1] =  std::numeric_limits<double>::max();
	gMapMaxBounds[0] = gMapMaxBounds[1] = -std::numeric_limits<double>::max();

	// initialize the statically-sized matrices
	// (the values of mPlanetAvgDistanceMatrix
	// change per turn however)
	gPlanetDistanceMatrix.resize(gameState.GetNumPlanets());
	gPlanetNeighbors.resize(gameState.GetNumPlanets());

	gMaxPlanetDistance = 0;

	for (unsigned int i = 0; i < gameState.GetNumPlanets(); i++) {
		gPlanetDistanceMatrix[i].resize(gameState.GetNumPlanets(), -1U);

		for (unsigned int j = 0; j < gameState.GetNumPlanets(); j++) {
			gPlanetDistanceMatrix[i][j] = GetDistance(&gameState.GetPlanet(i), &gameState.GetPlanet(j));
			gMaxPlanetDistance = std::max(gMaxPlanetDistance, gPlanetDistanceMatrix[i][j]);
		}

		{
			const GamePlanet& p = gameState.GetPlanet(i);

			// set the map-bounds
			gMapMinBounds[0] = std::min(gMapMinBounds[0], p.GetXPos());
			gMapMinBounds[1] = std::min(gMapMinBounds[1], p.GetYPos());
			gMapMaxBounds[0] = std::max(gMapMaxBounds[0], p.GetXPos());
			gMapMaxBounds[1] = std::max(gMapMaxBounds[1], p.GetYPos());
		}
	}

	gMapDimension[0] = gMapMaxBounds[0] - gMapMinBounds[0];
	gMapDimension[1] = gMapMaxBounds[1] - gMapMinBounds[1];

	// calculate each planet's closest neighbors
	// (this needs to be done after the distance
	// matrix is completely filled)
	for (unsigned int i = 0; i < gameState.GetNumPlanets(); i++) {
		gPlanetNeighbors[i] = new SortedList<unsigned int, unsigned int>(gameState.GetNumPlanets() - 1, gMaxPlanetDistance);

		for (unsigned int j = 0; j < gameState.GetNumPlanets(); j++) {
			if (j == i) {
				continue;
			}

			const GamePlanet* pi = &gameState.GetPlanet(i);
			const GamePlanet* pj = &gameState.GetPlanet(j);

			gPlanetNeighbors[i]->push_back(GetDistance(pi, pj), pj->GetID());
		}

		// planet coordinates are in [0, N] where N is
		// the full coord-range (max - min) of the map
		// first map the range to [-0.5, 0.5], then to
		// [-1.0, 1.0], then take the absolute values
		const double xrel = std::abs((((gameState.GetPlanet(i)).GetXPos() / gMapDimension[0]) - 0.5) * 2.0);
		const double yrel = std::abs((((gameState.GetPlanet(i)).GetYPos() / gMapDimension[1]) - 0.5) * 2.0);

		if (xrel <= 0.333 && yrel <= 0.333) {
			(gameState.GetPlanet(i)).SetMapRegion(MAP_REGION_INNER);
		} else if (xrel <= 0.666 && yrel <= 0.666) {
			(gameState.GetPlanet(i)).SetMapRegion(MAP_REGION_MIDDLE);
		} else {
			(gameState.GetPlanet(i)).SetMapRegion(MAP_REGION_OUTER);
		}
	}

	// GetPlanetClusters(2, 0.20);
}



void GameMap::CalcTurnData(GameState& gameState) {
	if (mPlanetAvgDistanceMatrix.empty()) {
		// this cannot be done in either ctor since they do not know the game-state
		mPlanetAvgDistanceMatrix.resize(gameState.GetNumPlanets());

		for (unsigned int i = 0; i < gameState.GetNumPlanets(); i++) {
			mPlanetAvgDistanceMatrix[i].resize(NUM_OWNERS, -1.0);
		}
	}

	for (unsigned int planetNum = 0; planetNum < gameState.GetNumPlanets(); planetNum++) {
		for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
			// when simulating future turns, the average distances
			// change and we need to recalculate them sans caching
			mPlanetAvgDistanceMatrix[planetNum][owner] = -1.0;
			mPlanetAvgDistanceMatrix[planetNum][owner] = GetAvgPlanetDistance(gameState, planetNum, owner);
		}
	}

	// set planet "safety levels"
	for (unsigned int i = 0; i < gameState.GetNumPlanets(); i++) {
		const double da = mPlanetAvgDistanceMatrix[i][OWNER_ALLIED];
		const double de = mPlanetAvgDistanceMatrix[i][OWNER_ENEMY];

		if (da <= (de * 0.333)) {
			(gameState.GetPlanet(i)).SetSafetyLevel(PLANET_SAFETY_LEVEL_MAX);
		} else if (da <= (de * 0.666)) {
			(gameState.GetPlanet(i)).SetSafetyLevel(PLANET_SAFETY_LEVEL_MID);
		} else {
			(gameState.GetPlanet(i)).SetSafetyLevel(PLANET_SAFETY_LEVEL_MIN);
		}
	}

	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		const std::vector<GamePlanet*>& ownerPlanets = gameState.GetOwnerPlanets(owner);

		// reset the average planet-position for each owner
		mPlanetAvgPositions[owner][0] = 0.0;
		mPlanetAvgPositions[owner][1] = 0.0;

		for (unsigned int n = 0; n < ownerPlanets.size(); n++) {
			mPlanetAvgPositions[owner][0] += ownerPlanets[n]->GetXPos();
			mPlanetAvgPositions[owner][1] += ownerPlanets[n]->GetYPos();
		}
		if (!ownerPlanets.empty()) {
			mPlanetAvgPositions[owner][0] /= ownerPlanets.size();
			mPlanetAvgPositions[owner][1] /= ownerPlanets.size();
		}
	}
}



unsigned int GameMap::GetDistance(const GamePlanet* pSrc, const GamePlanet* pDst) {
	if (pSrc->GetID() == pDst->GetID()) {
		return 0;
	}

	unsigned int d = gPlanetDistanceMatrix[pSrc->GetID()][pDst->GetID()];

	if (d == -1U) {
		const double dx = pSrc->GetXPos() - pDst->GetXPos();
		const double dy = pSrc->GetYPos() - pDst->GetYPos();

		d = std::ceil(std::sqrt(dx * dx + dy * dy));
	}

	return d;
}

void GameMap::GetDirection(const GamePlanet* pSrc, const GamePlanet* pDst, double* dir) {
	const double dx = pDst->GetXPos() - pSrc->GetXPos();
	const double dy = pDst->GetYPos() - pSrc->GetYPos();
	const double d  = std::sqrt(dx * dx + dy * dy);
	dir[0] = dx / d;
	dir[1] = dy / d;
}



double GameMap::GetAvgPlanetDistance(const GameState& gameState, unsigned int pID, unsigned int owner) const {
	if (mPlanetAvgDistanceMatrix[pID][owner] >= 0.0) {
		return mPlanetAvgDistanceMatrix[pID][owner];
	}

	const GamePlanet* pSrc = &gameState.GetPlanet(pID);
	const std::vector<GamePlanet*>& planets = gameState.GetOwnerPlanets(owner);

	double avgDistance = std::numeric_limits<double>::max();

	if (!planets.empty()) {
		avgDistance = 0.0;

		for (std::vector<GamePlanet*>::const_iterator it = planets.begin(); it != planets.end(); ++it) {
			avgDistance += GetDistance(pSrc, *it);
		}

		avgDistance /= planets.size();
	}

	return avgDistance;
}






// get the closest neighbor of <planetID> whose owner is <owner>
const GamePlanet* GameMap::GetClosestNeighborPlanet(const GameState& gs, unsigned int planetID, unsigned int owner) {
	const SortedList<unsigned int, unsigned int>* pNgbs = gPlanetNeighbors[planetID];
	const GamePlanet* p = NULL;

	typedef SortedList<unsigned int, unsigned int>::KeyValPair KVP;
	typedef std::list<KVP>::const_iterator KVPIt;

	for (KVPIt it = pNgbs->begin(); it != pNgbs->end(); ++it) {
		p = &gs.GetPlanet((*it).second);

		if (p->GetOwner() == owner) {
			break;
		}
	}

	return p;
}

// get the planet closest to <pSrc> that lies on the frontier
const GamePlanet* GameMap::GetClosestFrontierPlanet(const GameState& gs, const GamePlanet* pSrc, unsigned int owner) {
	const GamePlanet* closestFrontierPlanet = NULL;
	const std::vector<GamePlanet*>& frontierPlanets = gs.GetFrontierPlanets(owner);

	unsigned int closestFrontierDist2D = GameMap::GetMaxPlanetDistance() + 1;

	// find closest frontier planet of owner <owner> that is not equal to <pSrc>
	for (unsigned int j = 0; j < frontierPlanets.size(); j++) {
		const GamePlanet* pDst = frontierPlanets[j];

		if (pSrc->GetID() == pDst->GetID()) {
			continue;
		}

		if (GameMap::GetDistance(pSrc, pDst) < closestFrontierDist2D) {
			closestFrontierDist2D = GameMap::GetDistance(pSrc, pDst);
			closestFrontierPlanet = pDst;
		}
	}

	return closestFrontierPlanet;
}



// get any non-frontier planet <p> "in between" <pSrc> and <pDst> such that
// dist(pSrc, p> + dist(p, pDst> is less or equal to dist(pSrc, pDst) + delta
const GamePlanet* GameMap::GetHubPlanet(const GameState& gs, const GamePlanet* pSrc, const GamePlanet* pDst, unsigned int maxTurnDelta) {
	if (pSrc == NULL || pDst == NULL) {
		return NULL;
	}

	BOT_ASSERT(pSrc->GetOwner() == pDst->GetOwner());
	BOT_ASSERT(!pSrc->GetIsFrontierPlanet(pSrc->GetOwner()));
	BOT_ASSERT( pDst->GetIsFrontierPlanet(pDst->GetOwner()));

	const std::vector<GamePlanet*>& planets = gs.GetOwnerPlanets(pSrc->GetOwner());
	const GamePlanet* planet = NULL;

	for (unsigned int n = 0; n < planets.size(); n++) {
		const GamePlanet* p = planets[n];

		if (p->GetID() == pSrc->GetID()) { continue; }
		if (p->GetID() == pDst->GetID()) { continue; }
		if (p->GetIsFrontierPlanet(p->GetOwner())) { continue; }

		const unsigned int pSrcDist = GetDistance(pSrc, p);
		const unsigned int pDstDist = GetDistance(pDst, p);

		if ((pSrcDist + pDstDist) <= (GetDistance(pSrc, pDst) + maxTurnDelta)) {
			planet = p; break;
		}
	}

	return planet;
}



bool GameMap::IsFrontierPlanet(const GameState& gs, const GamePlanet* pSrc, unsigned int owner) {
	typedef SortedList<unsigned int, unsigned int>::KeyValPair KVP;
	typedef std::list<KVP>::const_iterator KVPIt;

	const GamePlanet* pNgb = NULL;
	const SortedList<unsigned int, unsigned int>* pNgbs = GameMap::GetPlanetNeighbors(pSrc->GetID());

	// get this planet's closest enemy neighbor <pNgb>
	// (there is guaranteed to be at least one enemy
	// *if* we are called from GetFrontierPlanets())
	for (KVPIt it = pNgbs->begin(); it != pNgbs->end(); ++it) {
		const GamePlanet* pDst = &gs.GetPlanet((*it).second);

		if (pDst->GetOwner() == OWNER_OPPONENT(owner)) {
			pNgb = pDst; break;
		}
	}

	if (pNgb == NULL) {
		return false;
	}

	bool isFrontierPlanet = true;
	const unsigned int frontierDist = GameMap::GetDistance(pSrc, pNgb);
	const std::vector<GamePlanet*>& ownerPlanets = gs.GetOwnerPlanets(owner);

	for (unsigned int j = 0; j < ownerPlanets.size(); j++) {
		if (ownerPlanets[j] == pSrc) { continue; }

		const GamePlanet* pDst = ownerPlanets[j];
		const unsigned int dist = GameMap::GetDistance(pNgb, pDst);

		if (dist < frontierDist) {
			// another allied planet is closer
			// to the enemy <pNgb>, so <pSrc>
			// is not a frontier planet
			isFrontierPlanet = false; break;
		}
	}

	return isFrontierPlanet;
}



// returns true *iif* <p>'s closest friendly neighbor
// is further away than <p>'s closest hostile neighbor
bool GameMap::IsPlanetCloserToOpponentThanOwner(const GameState& gs, const GamePlanet* p, unsigned int owner) {
	if (owner == OWNER_NEUTRAL) {
		return false;
	}

	const unsigned int oppon = OWNER_OPPONENT(owner);

	const GamePlanet* pNgbOwner = NULL;
	const GamePlanet* pNgbOppon = NULL;

	const SortedList<unsigned int, unsigned int>* pNgbs = GetPlanetNeighbors(p->GetID());

	typedef SortedList<unsigned int, unsigned int>::KeyValPair KVP;
	typedef std::list<KVP>::const_iterator KVPIt;

	// get this planet's closest allied and enemy neighbors
	for (KVPIt it = pNgbs->begin(); it != pNgbs->end(); ++it) {
		const GamePlanet* pNgb = &gs.GetPlanet((*it).second);

		if (pNgb->GetOwner() == owner && pNgbOwner == NULL) { pNgbOwner = pNgb; }
		if (pNgb->GetOwner() == oppon && pNgbOppon == NULL) { pNgbOppon = pNgb; }
	}

	if (pNgbOppon == NULL) { return false; }
	if (pNgbOwner == NULL) { return false; }

	// if distance to closest enemy neighbor of this neutral planet
	// is strictly LESS than distance to closest allied neighbor, we
	// consider it "unsafe"
	return (GetDistance(p, pNgbOppon) < GetDistance(p, pNgbOwner));
}



void GameMap::GetSafeNeutralPlanets(const GameState& gs, std::vector<GamePlanet*>& planets, unsigned int owner) {
	// get all neutral planets
	const std::vector<GamePlanet*>& neutrals = gs.GetOwnerPlanets(OWNER_NEUTRAL);

	for (unsigned int n = 0; n < neutrals.size(); n++) {
		GamePlanet* p = neutrals[n];

		if (((p->GetGrowthRate() > 0) && !IsPlanetCloserToOpponentThanOwner(gs, p, owner))) {
			planets.push_back(p);
		}
	}
}
