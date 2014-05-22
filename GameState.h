#ifndef _GAMESTATE_HDR_
#define _GAMESTATE_HDR_

#include <list>
#include <map>
#include <vector>

#include "SortedList.h"
#include "Enums.h"

class GamePlanet;
class GameFleet;

class GamePlanet {
public:
	GamePlanet();
	GamePlanet(const GamePlanet&);
	~GamePlanet();

	void SetID(unsigned int pID) { mID = pID; }
	void SetOwner(unsigned int owner) { mOwner = owner; }
	void SetNumShips(unsigned int numShips) { mNumShips = numShips; }
	void SetGrowthRate(unsigned int growthRate) { mGrowthRate = growthRate; }
	void SetXPos(double xPos) { pos[0] = xPos; }
	void SetYPos(double yPos) { pos[1] = yPos; }

	void SetNumReservedShips(unsigned int n) { mNumReservedShips = n; }
	void SetNumSpareShips(unsigned int n) { mNumSpareShips = n; }
	void SetNumShortageShips(unsigned int n) { mNumShortageShips = n; }

	void SetIsFrontierPlanet(unsigned int owner, bool b) { mIsFrontierPlanet[owner] = b; }
	void SetSafetyLevel(unsigned int l) { mSafetyLevel = l; }
	void SetMapRegion(unsigned int r) { mMapRegion = r; }


	unsigned int GetID() const { return mID; }
	unsigned int GetOwner() const { return mOwner; }
	unsigned int GetNumShips() const { return mNumShips; }
	unsigned int GetGrowthRate() const { return mGrowthRate; }
	double GetXPos() const { return pos[0]; }
	double GetYPos() const { return pos[1]; }

	unsigned int GetNumReservedShips() const { return mNumReservedShips; }
	unsigned int GetNumSpareShips() const { return mNumSpareShips; }
	unsigned int GetNumShortageShips() const { return mNumShortageShips; }


	bool GetIsFrontierPlanet(unsigned int owner) const { return mIsFrontierPlanet[owner]; }
	unsigned int GetSafetyLevel() const { return mSafetyLevel; }
	unsigned int GetMapRegion() const { return mMapRegion; }


	const std::list<unsigned int>& GetIncomingFleetIDs(unsigned int owner) const { return mIncomingFleetIDs[owner]; }
	const std::list<unsigned int>& GetOutgoingFleetIDs(unsigned int owner) const { return mOutgoingFleetIDs[owner]; }

	void AddIncomingFleetID(unsigned int owner, unsigned int fleetID) { mIncomingFleetIDs[owner].push_back(fleetID); }
	void AddOutgoingFleetID(unsigned int owner, unsigned int fleetID) { mOutgoingFleetIDs[owner].push_back(fleetID); }

	// called during PlanetWars::ParsePlanetsAndFleets
	void ClearFleets();

	void ExecTurns(unsigned int, std::map<unsigned int, GameFleet>&, std::vector<GamePlanet>&) const;
	void ExecTurn(std::map<unsigned int, GameFleet>&);

	bool OwnerChanged() const { return mOwnerChanged; }


	double CalcIntrinsicValue(unsigned int, unsigned int) const;
	double CalcReturnInvestmentTime(double distance, unsigned int numShips) const {
		return (distance + ((numShips + 1) / static_cast<double>(mGrowthRate + 0.01)));
	}

	void SetTmpValue(double value) { mTmpValue = value; }
	void SetTmpDistance(double distance) { mTmpDistance = distance; }
	void SetTmpThreat(double threat) { mTmpThreat = threat; }
	void SetTmpROITime(double t) { mTmpROITime = t; }
	double GetTmpValue() const { return mTmpValue; }
	double GetTmpDistance() const { return mTmpDistance; }
	double GetTmpThreat() const { return mTmpThreat; }
	double GetTmpROITime() const { return mTmpROITime; }


	/*
	// v10 methods
	// void ClearTargets(const std::vector< std::vector<GamePlanet*> >&);
	// void AddTarget(unsigned int owner, GamePlanet* pTgt) { mTargets[owner].push_back(pTgt); }
	// void SortTargets();
	// const std::vector<GamePlanet*>& GetTargets(unsigned int owner) const { return mTargets[owner]; }

	// v11 rewrite (if ever)
	void AddForce(unsigned int owner, unsigned int numTurns, unsigned int numShips) { mForces[owner][numTurns] += numShips; }
	void Reset() {
		for (unsigned int o = 0; o < NUM_OWNERS; o++) {
			for (unsigned int n = 0; n < mForces[o].size(); n++) {
				mForces[o][n] = 0;
			}
		}
		for (unsigned int n = 0; n < mOwnerPerTurn.size(); n++) {
			mOwnerPerTurn[n] = 0;
			mShipsPerTurn[n] = 0;
		}
	}
	*/

private:
	void SetNewOwner(const std::vector<unsigned int>&);

	// planet ID (same as Planet::planet_id_)
	// note: growth-rate is assumed to never
	// be a negative number!
	unsigned int mID;
	unsigned int mOwner;
	unsigned int mNumShips;
	unsigned int mGrowthRate;
	double pos[2];


	// total number of ships reserved by running TaskPlans
	unsigned int mNumReservedShips;
	// maximum number of ships we can afford to send away this turn
	unsigned int mNumSpareShips;
	// number of ships we come up short to prevent enemy capture
	unsigned int mNumShortageShips;

	// used for sorting
	double mTmpValue;
	double mTmpDistance;
	double mTmpThreat;
	double mTmpROITime;

	// true if this planet changed hands during
	// the previous ExecTurn() simulation step
	bool mOwnerChanged;
	// true if this planet lies on the frontier
	// for owner <OWNER_*>
	bool mIsFrontierPlanet[NUM_OWNERS];

	// depends on distance to OWNER_{ALLIED, ENEMY}
	unsigned int mSafetyLevel;
	// does not depend on ownership
	unsigned int mMapRegion;

	std::list<unsigned int> mIncomingFleetIDs[NUM_OWNERS];
	std::list<unsigned int> mOutgoingFleetIDs[NUM_OWNERS];

	// v10 properties
	// std::vector<GamePlanet*> mTargets[NUM_OWNERS];
	//
	// v11 rewrite
	// std::vector< std::vector<unsigned int> > mForces;   // [NUM_OWNERS][gMaxPlanetDistance + 1]
};



struct PlanetSortFunctor {
	PlanetSortFunctor(): mSortMode(SORTMODE_NUMSHIPS_DECR) {}
	PlanetSortFunctor(unsigned int sortMode): mSortMode(sortMode) {}
	PlanetSortFunctor(const PlanetSortFunctor& psf): mSortMode(psf.mSortMode) {}

	bool operator () (const GamePlanet* p1, const GamePlanet* p2) const {
		switch (mSortMode) {
			case SORTMODE_GROWTHRATE_DECR: {
				if (p1->GetGrowthRate() == p2->GetGrowthRate()) {
					return (p1->GetNumShips() > p2->GetNumShips());
				} else {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				}
			} break;
			case SORTMODE_GROWTHRATE_INCR: {
				if (p1->GetGrowthRate() == p2->GetGrowthRate()) {
					return (p1->GetNumShips() > p2->GetNumShips());
				} else {
					return (p1->GetGrowthRate() < p2->GetGrowthRate());
				}
			} break;


			case SORTMODE_NUMSHIPS_DECR: {
				if (p1->GetNumShips() == p2->GetNumShips()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetNumShips() > p2->GetNumShips());
				}
			} break;
			case SORTMODE_NUMSHIPS_INCR: {
				if (p1->GetNumShips() == p2->GetNumShips()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetNumShips() < p2->GetNumShips());
				}
			} break;


			case SORTMODE_NUMSHIPSREM_DECR: {
				const unsigned int p1NumRemShips = (p1->GetNumShips() - p1->GetNumReservedShips());
				const unsigned int p2NumRemShips = (p2->GetNumShips() - p2->GetNumReservedShips());

				if (p1NumRemShips == p2NumRemShips) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1NumRemShips > p2NumRemShips);
				}
			} break;
			case SORTMODE_NUMSHIPSREM_INCR: {
				const unsigned int p1NumRemShips = (p1->GetNumShips() - p1->GetNumReservedShips());
				const unsigned int p2NumRemShips = (p2->GetNumShips() - p2->GetNumReservedShips());

				if (p1NumRemShips == p2NumRemShips) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1NumRemShips < p2NumRemShips);
				}
			} break;


			case SORTMODE_TMPVALUE_DECR: {
				if (p1->GetTmpValue() == p2->GetTmpValue()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetTmpValue() > p2->GetTmpValue());
				}
			} break;
			case SORTMODE_TMPVALUE_INCR: {
				if (p1->GetTmpValue() == p2->GetTmpValue()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetTmpValue() < p2->GetTmpValue());
				}
			} break;


			case SORTMODE_TMPDISTANCE_DECR: {
				if (p1->GetTmpDistance() == p2->GetTmpDistance()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetTmpDistance() > p2->GetTmpDistance());
				}
			} break;
			case SORTMODE_TMPDISTANCE_INCR: {
				if (p1->GetTmpDistance() == p2->GetTmpDistance()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetTmpDistance() < p2->GetTmpDistance());
				}
			} break;


			case SORTMODE_TMPTHREAT_DECR: {
				if (p1->GetTmpThreat() == p2->GetTmpThreat()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetTmpThreat() > p2->GetTmpThreat());
				}
			} break;
			case SORTMODE_TMPTHREAT_INCR: {
				if (p1->GetTmpThreat() == p2->GetTmpThreat()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetTmpThreat() < p2->GetTmpThreat());
				}
			} break;

			case SORTMODE_TMPROITIME_DECR: {
				if (p1->GetTmpROITime() == p2->GetTmpROITime()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetTmpROITime() > p2->GetTmpROITime());
				}
			} break;
			case SORTMODE_TMPROITIME_INCR: {
				if (p1->GetTmpROITime() == p2->GetTmpROITime()) {
					return (p1->GetGrowthRate() > p2->GetGrowthRate());
				} else {
					return (p1->GetTmpROITime() < p2->GetTmpROITime());
				}
			} break;
		}

		return false;
	}

	unsigned int mSortMode;
};



class GameFleet {
public:
	GameFleet() {
		SetID(-1U);
		SetOwner(-1U);
		SetNumShips(-1U);
		SetSrcPlanetID(-1U);
		SetDstPlanetID(-1U);
		SetTurnsRemaining(-1U);
	}
	GameFleet(const GameFleet& f) {
		SetID(f.GetID());
		SetOwner(f.GetOwner());
		SetNumShips(f.GetNumShips());
		SetSrcPlanetID(f.GetSrcPlanetID());
		SetDstPlanetID(f.GetDstPlanetID());
		SetTurnsRemaining(f.GetTurnsRemaining());
	}
	~GameFleet() {}

	bool operator < (const GameFleet& f) const { return (mID < f.mID); }

	void SetID(unsigned int fID) { mID = fID; }
	void SetOwner(unsigned int owner) { mOwner = owner; }
	void SetNumShips(unsigned int numShips) { mNumShips = numShips; }
	void SetSrcPlanetID(unsigned int srcPlanetID) { mSrcPlanetID = srcPlanetID; }
	void SetDstPlanetID(unsigned int dstPlanetID) { mDstPlanetID = dstPlanetID; }
	void SetTurnsRemaining(unsigned int turnsRemaining) { mTurnsRemaining = turnsRemaining; }

	unsigned int GetID() const { return mID; }
	unsigned int GetOwner() const { return mOwner; }
	unsigned int GetNumShips() const { return mNumShips; }
	unsigned int GetSrcPlanetID() const { return mSrcPlanetID; }
	unsigned int GetDstPlanetID() const { return mDstPlanetID; }
	unsigned int GetTurnsRemaining() const { return mTurnsRemaining; }

	void ExecTurn() {
		if (mTurnsRemaining > 0) {
			mTurnsRemaining -= 1;
		}
	}

private:
	// fleet ID (custom, not a member of Fleet)
	unsigned int mID;
	unsigned int mOwner;
	unsigned int mNumShips;

	unsigned int mSrcPlanetID;
	unsigned int mDstPlanetID;

	unsigned int mTurnsRemaining;
};






class GameState;
class GameMap {
public:
	GameMap() {}
	~GameMap();

	void ClearTurnData() { /* no-op */ }
	void CalcFirstTurnData(GameState&);
	void CalcTurnData(GameState&);

	// Returns the distance between two planets, rounded up to the next highest
	// integer. This is the number of discrete time steps it takes to get between
	// the two planets.
	static unsigned int GetDistance(const GamePlanet*, const GamePlanet*);
	static void GetDirection(const GamePlanet*, const GamePlanet*, double*);

	// get the OWNER_<owner> planet closest to <p>
	// NOTE: the caller *must* check if the planet
	// returned is not equal to <p> itself
	// const GamePlanet& GetClosestPlanet(const GamePlanet&, unsigned int);

	// get the average distance from <p> to all
	// planets currently owned by OWNER_<owner>
	double GetAvgPlanetDistance(const GameState&, unsigned int, unsigned int) const;

	static double GetMapMinBounds(unsigned int c) { return gMapMinBounds[c]; }
	static double GetMapMaxBounds(unsigned int c) { return gMapMaxBounds[c]; }
	static double GetMapDimension(unsigned int c) { return gMapDimension[c]; }
	static unsigned int GetMaxPlanetDistance() { return gMaxPlanetDistance; }

	static const SortedList<unsigned int, unsigned int>* GetPlanetNeighbors(unsigned int pID) { return gPlanetNeighbors[pID]; }

	static const GamePlanet* GetClosestNeighborPlanet(const GameState&, unsigned int, unsigned int);
	static const GamePlanet* GetClosestFrontierPlanet(const GameState&, const GamePlanet*, unsigned int);
	static const GamePlanet* GetHubPlanet(const GameState&, const GamePlanet*, const GamePlanet*, unsigned int);
	static bool IsFrontierPlanet(const GameState&, const GamePlanet*, unsigned int);
	static bool IsPlanetCloserToOpponentThanOwner(const GameState&, const GamePlanet*, unsigned int);

	static void GetSafeNeutralPlanets(const GameState&, std::vector<GamePlanet*>&, unsigned int);

private:
	// for each pair of planets <A, B>, this
	// holds the (fixed) trip-time (in turns)
	// from A to B
	static std::vector< std::vector<unsigned int> > gPlanetDistanceMatrix;
	       std::vector< std::vector<      double> > mPlanetAvgDistanceMatrix;

	// for each planet <p>, stores <p>'s <N> closest
	// neighbor planets sorted by increasing distance
	static std::vector<SortedList<unsigned int, unsigned int>* > gPlanetNeighbors;

	static double gMapMinBounds[2]; // minimum (x, y)-coordinates of any planet
	static double gMapMaxBounds[2]; // maximum (x, y)-coordinates of any planet
	static double gMapDimension[2]; // (max - min) (x, y)-extends

	double mPlanetAvgPositions[NUM_OWNERS][2];

	// maximum distance (in turns) between any two planets
	// calculated once, fixed for the duration of the game
	static unsigned int gMaxPlanetDistance;
};






struct GameStats {
public:
	GameStats();
	~GameStats();

	void ClearTurnData();
	void CalcFirstTurnData() {}
	void CalcTurnData(const GameState&);

	unsigned int GetPlanetCount(unsigned int owner) const { return mPlanetCounts[owner]; }
	unsigned int GetSumNumShips(unsigned int owner) const { return mSumNumShips[owner]; }
	unsigned int GetSumGrowthRate(unsigned int owner) const { return mSumGrowthRates[owner]; }

private:
	std::vector<unsigned int> mPlanetCounts;     // current number of N/A/E planets (same as mOwnerPlanets[OWNER_*].size())
	std::vector<unsigned int> mFleetCounts;      // current number of N/A/E fleets

	std::vector<unsigned int> mMaxGrowthRates;   // maximum growth-rate of any N/A/E planet
	std::vector<unsigned int> mSumGrowthRates;   // sum of all growth-rates per N/A/E planet
	std::vector<double> mAvgGrowthRates;         // average rate of growth per N/A/E planet

	std::vector<unsigned int> mMaxNumShips;      // current maximum number of ships over all N/A/E planets (EXCLUDING FLEETS)
	std::vector<unsigned int> mSumNumShips;      // sum of all ships per N/A/E planet (EXCLUDING FLEETS)
	std::vector<double> mAvgNumShips;            // average number of ships per N/A/E planet (EXCLUDING FLEETS)

	std::vector<unsigned int> mMaxFleetSizes;    // maximum size of any N/A/E fleet
	std::vector<unsigned int> mSumFleetSizes;    // sum of all ships per N/A/E fleet
	std::vector<double> mAvgFleetSizes;          // average number of ships per N/A/E fleet

};






class GameState {
public:
	GameState();
	GameState(const GameState&);
	~GameState();

	void AddPlanet(const GamePlanet&);
	void AddFleet(const GameFleet&, bool);

	const GamePlanet& GetPlanet(unsigned int pID) const;
	      GamePlanet& GetPlanet(unsigned int pID);
	const GameFleet& GetFleet(unsigned int fID) const;
	      GameFleet& GetFleet(unsigned int fID);

	const std::vector<GamePlanet>& GetPlanets() const { return mPlanets; }
	const std::map<unsigned int, GameFleet>& GetFleets() const { return mFleets; }

	void ClearTurnData();
	void CalcFirstTurnData();
	void CalcTurnData();

	// does NOT modify <this> (!)
	void ExecTurns(unsigned int, std::vector<GameState>&) const;

	// advance time for a single planet alone
	// like ExecTurns, does NOT modify <this>
	void GetPlanetStates(unsigned int, unsigned int, std::vector<GamePlanet>&) const;


	unsigned int GetNumPlanets() const { return mPlanets.size(); }
	unsigned int GetNumFleets() const { return mFleets.size(); }


	void FindFrontierPlanets(unsigned int, std::vector<GamePlanet*>&);
	unsigned int FindKnapSackPlanets(unsigned int, unsigned int, const std::vector<GamePlanet*>&, std::vector<GamePlanet*>&);

	std::vector<GamePlanet*>& GetOwnerPlanets(unsigned int owner) { return mOwnerPlanets[owner]; }
	std::vector<GamePlanet*>& GetFrontierPlanets(unsigned int owner) { return mFrontierPlanets[owner]; }
	std::vector<GamePlanet*>& GetKnapSackPlanets(unsigned int owner) { return mKnapSackPlanets[owner]; }
	const std::vector<GamePlanet*>& GetOwnerPlanets(unsigned int owner) const { return mOwnerPlanets[owner]; }
	const std::vector<GamePlanet*>& GetFrontierPlanets(unsigned int owner) const { return mFrontierPlanets[owner]; }
	const std::vector<GamePlanet*>& GetKnapSackPlanets(unsigned int owner) const { return mKnapSackPlanets[owner]; }
	unsigned int GetPlanetMaxSpareShips(unsigned int) const;

	bool IsStartingPlanet(unsigned int pID, unsigned int owner) const { return (pID == mStartPlanetIDs[owner]); }
	bool IsStaleMate() const;
	bool IsDeadLock() const;

	unsigned int GetPlanetFuturePopulation(
		unsigned int,
		unsigned int,
		unsigned int*,
		unsigned int*,
		std::vector<unsigned int>*,
		std::vector<unsigned int>*
	) const;

	const GameMap& GetGameMap() const { return mGameMap; }
	const GameStats& GetGameStats() const { return mGameStats; }

	unsigned int GetMaxSackWeight(unsigned int owner) const { return mMaxSackWeights[owner]; }
	unsigned int GetSumSackGrowth(unsigned int owner) const { return mSumSackGrowths[owner]; }

private:
	// may be called an arbitrary number of
	// times after InitTurn(), simulates one
	// full game-state update
	void ExecTurn();

	// planets are never removed from the game
	std::vector<GamePlanet> mPlanets;

	// fleets disappear when simulating future turns
	std::map<unsigned int, GameFleet> mFleets;

	// starting planet ID's of OWNER_*
	std::vector<unsigned int> mStartPlanetIDs;

	// references to all N/A/E planets sorted by owner
	std::vector<GamePlanet*> mOwnerPlanets[NUM_OWNERS];
	std::vector<GamePlanet*> mFrontierPlanets[NUM_OWNERS];
	std::vector<GamePlanet*> mKnapSackPlanets[NUM_OWNERS];

	unsigned int mMaxSackWeights[NUM_OWNERS];
	unsigned int mSumSackGrowths[NUM_OWNERS];

	GameMap mGameMap;
	GameStats mGameStats;
};

#endif
