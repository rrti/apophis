#include <limits>
#include "GameState.h"
#include "Debugger.h"
#include "Logger.h"

GamePlanet::GamePlanet() {
	// PlanetWars properties
	SetID(-1U);
	SetOwner(-1U);
	SetNumShips(-1U);
	SetGrowthRate(-1U);
	SetXPos(-1.0);
	SetYPos(-1.0);

	// custom properties (these do not [yet] need
	// recalculation for simulating this planet's
	// future state)
	SetNumReservedShips(0);
	SetNumSpareShips(0);
	SetNumShortageShips(0);
	SetSafetyLevel(-1U);
	SetMapRegion(-1U);
	SetIsFrontierPlanet(OWNER_ALLIED, false);
	SetIsFrontierPlanet(OWNER_NEUTRAL, false);
	SetIsFrontierPlanet(OWNER_ENEMY, false);
	SetTmpValue(-std::numeric_limits<double>::max());
	SetTmpDistance(-std::numeric_limits<double>::max());
	SetTmpThreat(-std::numeric_limits<double>::max());
	SetTmpROITime(std::numeric_limits<double>::max());

	mOwnerChanged = false;
}

GamePlanet::GamePlanet(const GamePlanet& p) {
	SetID(p.GetID());
	SetOwner(p.GetOwner());
	SetNumShips(p.GetNumShips());
	SetGrowthRate(p.GetGrowthRate());
	SetXPos(p.GetXPos());
	SetYPos(p.GetYPos());

	SetNumReservedShips(p.GetNumReservedShips());
	SetNumSpareShips(p.GetNumSpareShips());
	SetNumShortageShips(p.GetNumShortageShips());
	SetSafetyLevel(p.GetSafetyLevel());
	SetMapRegion(p.GetMapRegion());
	SetIsFrontierPlanet(OWNER_ALLIED, p.GetIsFrontierPlanet(OWNER_ALLIED));
	SetIsFrontierPlanet(OWNER_NEUTRAL, p.GetIsFrontierPlanet(OWNER_NEUTRAL));
	SetIsFrontierPlanet(OWNER_ENEMY, p.GetIsFrontierPlanet(OWNER_ENEMY));
	SetTmpValue(p.GetTmpValue());
	SetTmpDistance(p.GetTmpDistance());
	SetTmpThreat(p.GetTmpThreat());
	SetTmpROITime(p.GetTmpROITime());

	// deep-copy <p>'s incoming fleets, since these are relevant for ExecTurn()
	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		const std::list<unsigned int>& incFleetIDs = p.GetIncomingFleetIDs(owner);
		const std::list<unsigned int>& outFleetIDs = p.GetOutgoingFleetIDs(owner);

		for (std::list<unsigned int>::const_iterator it = incFleetIDs.begin(); it != incFleetIDs.end(); ++it) {
			AddIncomingFleetID(owner, *it);
		}
		for (std::list<unsigned int>::const_iterator it = outFleetIDs.begin(); it != outFleetIDs.end(); ++it) {
			AddOutgoingFleetID(owner, *it);
		}
	}

	mOwnerChanged = false;
}

GamePlanet::~GamePlanet() {
	ClearFleets();
}



void GamePlanet::ExecTurns(unsigned int numTurns, std::map<unsigned int, GameFleet>& allFleets, std::vector<GamePlanet>& planetStates) const {
	GamePlanet planet = *this; // deep-copy
	planetStates.reserve(numTurns);

	for (unsigned int n = 0; n < numTurns; n++) {
		planet.ExecTurn(allFleets);
		// deep-copy each intermediate state
		planetStates.push_back(planet);
	}
}

void GamePlanet::ExecTurn(std::map<unsigned int, GameFleet>& allFleets) {
	mOwnerChanged = false;

	if (mOwner != OWNER_NEUTRAL) {
		mNumShips += mGrowthRate;
	}

	std::vector<unsigned int> forceSizes(NUM_OWNERS, 0);
	std::list< std::list<unsigned int>::iterator > arrivingFleets;
	std::list< std::list<unsigned int>::iterator >::iterator arrivingFleetsIt;

	// update all incoming fleets (NOT the
	// outgoing fleets, since those will be
	// updated by their destination planet)
	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		for (std::list<unsigned int>::iterator it = mIncomingFleetIDs[owner].begin(); it != mIncomingFleetIDs[owner].end(); ++it) {
			GameFleet& f = allFleets[*it];

			f.ExecTurn();

			// collect the fleets arriving *this* turn
			if (f.GetTurnsRemaining() == 0) {
				arrivingFleets.push_back(it);
			}
		}
	}

	if (arrivingFleets.empty()) {
		// none of the in-bound fleets have arrived yet
		return;
	}

	// set our initial force-size
	forceSizes[mOwner] = mNumShips;

	for (arrivingFleetsIt = arrivingFleets.begin(); arrivingFleetsIt != arrivingFleets.end(); ++arrivingFleetsIt) {
		const std::list<unsigned int>::iterator& incFleetIt = *arrivingFleetsIt;

		const GameFleet& f = allFleets[*incFleetIt];

		// compose the total force-sizes
		forceSizes[f.GetOwner()] += f.GetNumShips();

		// eliminate the arriving fleet from our incoming-list
		mIncomingFleetIDs[f.GetOwner()].erase(incFleetIt);

		// eliminate the arriving fleet from the map-list
		allFleets.erase(f.GetID());
	}

	// figure out who ends up owning us
	SetNewOwner(forceSizes);

	// cleanup
	arrivingFleets.clear();
	forceSizes.clear();
}

void GamePlanet::SetNewOwner(const std::vector<unsigned int>& forceSizes) {
	const unsigned int numForces =
		static_cast<unsigned int>(forceSizes[OWNER_NEUTRAL] != 0) +
		static_cast<unsigned int>(forceSizes[OWNER_ALLIED ] != 0) +
		static_cast<unsigned int>(forceSizes[OWNER_ENEMY  ] != 0);

	unsigned int winningForce = mOwner;

	switch (numForces) {
		case 1: {
			// one force; owner does not change
			mNumShips = forceSizes[mOwner];
		} break;

		case 2: {
			// new owner is owner of the largest force
			// (which may or may not be the old owner)
			//
			// if force-sizes are equal, owner remains
			// unchanged but planet population becomes
			// 0
			int forceSizeDif = 0;

			if (forceSizes[OWNER_NEUTRAL] != 0 && forceSizes[OWNER_ALLIED] != 0) {
				forceSizeDif = static_cast<int>(forceSizes[OWNER_NEUTRAL] - forceSizes[OWNER_ALLIED]);

				if (forceSizeDif > 0) { winningForce = OWNER_NEUTRAL; }
				if (forceSizeDif < 0) { winningForce = OWNER_ALLIED; }
			}

			if (forceSizes[OWNER_NEUTRAL] != 0 && forceSizes[OWNER_ENEMY] != 0) {
				forceSizeDif = static_cast<int>(forceSizes[OWNER_NEUTRAL] - forceSizes[OWNER_ENEMY]);

				if (forceSizeDif > 0) { winningForce = OWNER_NEUTRAL; }
				if (forceSizeDif < 0) { winningForce = OWNER_ENEMY; }
			}

			if (forceSizes[OWNER_ALLIED] != 0 && forceSizes[OWNER_ENEMY] != 0) {
				forceSizeDif = static_cast<int>(forceSizes[OWNER_ALLIED] - forceSizes[OWNER_ENEMY]);

				if (forceSizeDif > 0) { winningForce = OWNER_ALLIED; }
				if (forceSizeDif < 0) { winningForce = OWNER_ENEMY; }
			}

			if (forceSizeDif != 0) {
				// winning force was uniquely defined
				// (therefore the planet changes hands)
				mOwnerChanged = (winningForce != mOwner);
				mOwner = winningForce;
			}

			mNumShips = std::max(forceSizeDif, -forceSizeDif);
		} break;

		case 3: {
			// there can only be three forces when the planet is
			// neutral; allied or enemy planets can only be fought
			// over by two sides
			BOT_ASSERT(mOwner == OWNER_NEUTRAL);

			unsigned int maxForceSize = 0;
			unsigned int midForceSize = 0;

			// find the size of the largest force
			if (forceSizes[OWNER_NEUTRAL] > maxForceSize) { maxForceSize = forceSizes[OWNER_NEUTRAL]; winningForce = OWNER_NEUTRAL; }
			if (forceSizes[OWNER_ALLIED ] > maxForceSize) { maxForceSize = forceSizes[OWNER_ALLIED ]; winningForce = OWNER_ALLIED;  }
			if (forceSizes[OWNER_ENEMY  ] > maxForceSize) { maxForceSize = forceSizes[OWNER_ENEMY  ]; winningForce = OWNER_ENEMY;   }
			// find the size of the second-largest force
			if (forceSizes[OWNER_NEUTRAL] > midForceSize && winningForce != OWNER_NEUTRAL) { midForceSize = forceSizes[OWNER_NEUTRAL]; }
			if (forceSizes[OWNER_ALLIED ] > midForceSize && winningForce != OWNER_ALLIED ) { midForceSize = forceSizes[OWNER_ALLIED ]; }
			if (forceSizes[OWNER_ENEMY  ] > midForceSize && winningForce != OWNER_ENEMY  ) { midForceSize = forceSizes[OWNER_ENEMY  ]; }

			if ((maxForceSize - midForceSize) != 0) {
				// winning force was uniquely defined
				mOwnerChanged = (winningForce != mOwner);
				mOwner = winningForce;
			}

			mNumShips = maxForceSize - midForceSize;
		} break;
	}
}

void GamePlanet::ClearFleets() {
	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		mIncomingFleetIDs[owner].clear();
		mOutgoingFleetIDs[owner].clear();
	}
}









GameState::GameState() {
	mStartPlanetIDs.resize(NUM_OWNERS);

	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		mMaxSackWeights[owner] = 0;
		mSumSackGrowths[owner] = 0;
	}
}

GameState::GameState(const GameState& g) {
	const std::vector<GamePlanet>& planets = g.GetPlanets();
	const std::map<unsigned int, GameFleet>& fleets = g.GetFleets();

	BOT_ASSERT(mPlanets.empty());

	mPlanets.clear();
	mPlanets.reserve(planets.size());

	std::vector<GamePlanet>::const_iterator pit;
	std::map<unsigned int, GameFleet>::const_iterator fit;

	for (pit = planets.begin(); pit != planets.end(); ++pit) { AddPlanet(*pit); }
	for (fit = fleets.begin(); fit != fleets.end(); ++fit) { AddFleet(fit->second, true); }

	mStartPlanetIDs = g.mStartPlanetIDs;

	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		mMaxSackWeights[owner] = g.mMaxSackWeights[owner];
		mSumSackGrowths[owner] = g.mSumSackGrowths[owner];

		BOT_ASSERT(mFrontierPlanets[owner].empty());
		BOT_ASSERT(mKnapSackPlanets[owner].empty());

		mFrontierPlanets[owner].resize(g.mFrontierPlanets[owner].size());
		mKnapSackPlanets[owner].resize(g.mKnapSackPlanets[owner].size());

		for (unsigned int n = 0; n < g.mFrontierPlanets[owner].size(); n++) {
			mFrontierPlanets[owner][n] = &mPlanets[ g.mFrontierPlanets[owner][n]->GetID() ];
		}
		for (unsigned int n = 0; n < g.mKnapSackPlanets[owner].size(); n++) {
			mKnapSackPlanets[owner][n] = &mPlanets[ g.mKnapSackPlanets[owner][n]->GetID() ];
		}
	}
}

GameState::~GameState() {
	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		mOwnerPlanets[owner].clear();
		mFrontierPlanets[owner].clear();
		mKnapSackPlanets[owner].clear();
	}

	mPlanets.clear();
	mFleets.clear();
	mStartPlanetIDs.clear();
}






const GamePlanet& GameState::GetPlanet(unsigned int planetID) const {
	const static GamePlanet dummy;

	if (planetID >= mPlanets.size()) {
		BOT_ASSERT(false);
		return dummy;
	}

	return mPlanets[planetID];
}

GamePlanet& GameState::GetPlanet(unsigned int planetID) {
	static GamePlanet dummy;

	if (planetID >= mPlanets.size()) {
		BOT_ASSERT(false);
		return dummy;
	}

	return mPlanets[planetID];
}

const GameFleet& GameState::GetFleet(unsigned int fleetID) const {
	const static GameFleet dummy;

	std::map<unsigned int, GameFleet>::const_iterator it = mFleets.find(fleetID);

	if (it != mFleets.end()) {
		return it->second;
	}

	BOT_ASSERT(false);
	return dummy;
}

GameFleet& GameState::GetFleet(unsigned int fleetID) {
	static GameFleet dummy;

	std::map<unsigned int, GameFleet>::iterator it = mFleets.find(fleetID);

	if (it != mFleets.end()) {
		return it->second;
	}

	BOT_ASSERT(false);
	return dummy;
}



void GameState::GetPlanetStates(unsigned int planetID, unsigned int numTurns, std::vector<GamePlanet>& planetStates) const {
	GameState gameState = *this; // deep-copy
	GamePlanet& gamePlanet = gameState.mPlanets[planetID];

	gamePlanet.ExecTurns(numTurns, gameState.mFleets, planetStates);
}






void GameState::AddPlanet(const GamePlanet& p) { mPlanets.push_back(p); }
void GameState::AddFleet(const GameFleet& f, bool gameStateCopy) {
	const unsigned int fleetID = mFleets.size();

	mFleets[fleetID] = f;
	mFleets[fleetID].SetID(fleetID);

	// if we are copying a game-state for ExecTurns(),
	// each planet already knows all its incoming and
	// outgoing fleets
	// if we are copying a new PlanetWars state for
	// InitTurn(), then each planet is a blank slate
	if (!gameStateCopy) {
		GamePlanet& srcPlanet = mPlanets[f.GetSrcPlanetID()];
		GamePlanet& dstPlanet = mPlanets[f.GetDstPlanetID()];

		srcPlanet.AddOutgoingFleetID(f.GetOwner(), fleetID);
		dstPlanet.AddIncomingFleetID(f.GetOwner(), fleetID);
	}
}



void GameState::ClearTurnData() {
	// called from PlanetWars::ClearTurnData (between two ACTUAL
	// non-simulated turns)
	//
	// doing this would invalidate PlanetWars::mPlanets[OWNER_*]
	// (it is unnecessary in any case since planets are fixed in
	// number and *most* default properties)
	// mPlanets.clear();
	mFleets.clear();
}

void GameState::CalcFirstTurnData() {
	mGameMap.CalcFirstTurnData(*this);
	mGameStats.CalcFirstTurnData();

	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		if (!mOwnerPlanets[owner].empty()) {
			// OWNER_ALLIED and OWNER_ENEMY always start with
			// the same number of ships and planets equal in
			// growth-rate
			mStartPlanetIDs[owner] = mOwnerPlanets[owner][0]->GetID();
		}
	}
}



void GameState::ExecTurns(unsigned int numTurns, std::vector<GameState>& gameStates) const {
	GameState gameState = *this; // deep-copy
	gameStates.reserve(numTurns);

	for (unsigned int n = 0; n < numTurns; n++) {
		gameState.ExecTurn();
		gameState.CalcTurnData();

		// deep-copy each intermediate state
		gameStates.push_back(gameState);
	}
}

void GameState::ExecTurn() {
	// NOTE:
	//     each planet updates its own incoming fleets,
	//     and should never try to access the state of
	//     any but its own (since those might not have
	//     been updated yet, depending on the iteration
	//     order)
	//     this is necessary to support advancing time
	//     for just a single planet instead of the full
	//     map (see GetPlanetStates)
	for (unsigned int i = 0; i < mPlanets.size(); i++) {
		mPlanets[i].ExecTurn(mFleets);
	}
}



void GameState::CalcTurnData() {
	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		// between two SIMULATED turns, planets can have changed
		// owners so we need to update the by-owner information
		// (also between two ACTUAL turns)
		mOwnerPlanets[owner].clear();
		mOwnerPlanets[owner].reserve(mOwnerPlanets[owner].size());
	}

	for (unsigned int n = 0; n < mPlanets.size(); n++) {
		mOwnerPlanets[mPlanets[n].GetOwner()].push_back(&mPlanets[n]);
	}


	// mOwnerPlanets must be filled for these
	mGameMap.CalcTurnData(*this);
	mGameStats.CalcTurnData(*this);


	mFrontierPlanets[OWNER_ALLIED].clear();
	mFrontierPlanets[OWNER_ENEMY].clear();
	FindFrontierPlanets(OWNER_ALLIED, mFrontierPlanets[OWNER_ALLIED]);
	FindFrontierPlanets(OWNER_ENEMY, mFrontierPlanets[OWNER_ENEMY]);


	// NOTE:
	//     only front-line planets will have ships to spare (since
	//     all others just route to them and/or defend under-attack
	//     planets), so the weights are composed only of front-line
	//     planets as well
	for (unsigned int owner = 0; owner < NUM_OWNERS; owner++) {
		mMaxSackWeights[owner] = 0;
		mSumSackGrowths[owner] = 0;
	}

	for (unsigned int n = 0; n < mFrontierPlanets[OWNER_ALLIED].size(); n++) {
		const GamePlanet* p = mFrontierPlanets[OWNER_ALLIED][n];

		mMaxSackWeights[OWNER_ALLIED] += GetPlanetMaxSpareShips(p->GetID());
		// mMaxSackWeights[OWNER_ALLIED] += p->GetNumShips();
	}

	for (unsigned int n = 0; n < mFrontierPlanets[OWNER_ENEMY].size(); n++) {
		const GamePlanet* p = mFrontierPlanets[OWNER_ENEMY][n];

		mMaxSackWeights[OWNER_ENEMY] += GetPlanetMaxSpareShips(p->GetID());
		// mMaxSackWeights[OWNER_ENEMY] += p->GetNumShips();
	}

	mKnapSackPlanets[OWNER_ALLIED].clear();
	mKnapSackPlanets[OWNER_ENEMY].clear();
	mSumSackGrowths[OWNER_ALLIED] = FindKnapSackPlanets(OWNER_ALLIED, mMaxSackWeights[OWNER_ALLIED], mOwnerPlanets[OWNER_NEUTRAL], mKnapSackPlanets[OWNER_ALLIED]);
	mSumSackGrowths[OWNER_ENEMY] = FindKnapSackPlanets(OWNER_ENEMY, mMaxSackWeights[OWNER_ENEMY], mOwnerPlanets[OWNER_NEUTRAL], mKnapSackPlanets[OWNER_ENEMY]);
}



bool GameState::IsStaleMate() const {
	bool r = true;

	for (unsigned int n = 0; n < mOwnerPlanets[OWNER_ALLIED].size(); n++) {
		const GamePlanet* p = mOwnerPlanets[OWNER_ALLIED][n];

		if (!p->GetIncomingFleetIDs(OWNER_ENEMY).empty()) {
			r = false; break; 
		}
	}

	for (unsigned int n = 0; n < mOwnerPlanets[OWNER_NEUTRAL].size(); n++) {
		const GamePlanet* p = mOwnerPlanets[OWNER_NEUTRAL][n];

		if (!p->GetIncomingFleetIDs(OWNER_ALLIED).empty()) { r = false; break; }
		if (!p->GetIncomingFleetIDs(OWNER_ENEMY).empty()) { r = false; break; }
	}

	for (unsigned int n = 0; n < mOwnerPlanets[OWNER_ENEMY].size(); n++) {
		const GamePlanet* p = mOwnerPlanets[OWNER_ENEMY][n];

		if (!p->GetIncomingFleetIDs(OWNER_ALLIED).empty()) {
			r = false; break;
		}
	}

	return r;
}

bool GameState::IsDeadLock() const {
	// TODO:
	//   define a dead-locked state to be one in which a single
	//   planet <P> is mass-attacked *and* mass-defended at the
	//   same time, and no attacks on neutrals are taking place
	//   by either side
	return false;
}
