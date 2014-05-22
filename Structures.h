#ifndef _STRUCTURES_HDR_
#define _STRUCTURES_HDR_

/*
// v10 structures
struct ClaimedPlanet {
	ClaimedPlanet() {
		claimant = -1U;
		distance = -1U;
	}
	ClaimedPlanet(unsigned int c, unsigned int d) {
		claimant = c;
		distance = d;
	}
	ClaimedPlanet(const ClaimedPlanet& ctp) {
		claimant = ctp.claimant;
		distance = ctp.distance;
	}
	unsigned int claimant; // planetID
	unsigned int distance; // distance from claimant to claimee
};

struct ThreatenedPlanet {
	ThreatenedPlanet() {
		numShips = -1U;
		lossTurn = -1U;
	}
	ThreatenedPlanet(unsigned int ships, unsigned int turn) {
		numShips = ships;
		lossTurn = turn;
	}
	unsigned int numShips;
	unsigned int lossTurn;
};

struct AttackedPlanet {
	AttackedPlanet() {
		numShips = -1U;
		futOwner = -1U;
	}
	AttackedPlanet(unsigned int ships, unsigned int owner) {
		numShips = ships;
		futOwner = owner;
	}
	unsigned int numShips;
	unsigned int futOwner;
};

typedef std::map<unsigned int, ClaimedPlanet> ClaimedPlanets;
typedef std::map<unsigned int, ClaimedPlanet>::iterator ClaimedPlanetsIt;
typedef std::map<unsigned int, ClaimedPlanet>::const_iterator ClaimedPlanetsCIt;
typedef std::map<unsigned int, ThreatenedPlanet> ThreatenedPlanets;
typedef std::map<unsigned int, ThreatenedPlanet>::iterator ThreatenedPlanetsIt;
typedef std::map<unsigned int, ThreatenedPlanet>::const_iterator ThreatenedPlanetsCIt;
typedef std::map<unsigned int, AttackedPlanet> AttackedPlanets;
typedef std::map<unsigned int, AttackedPlanet>::iterator AttackedPlanetsIt;
typedef std::map<unsigned int, AttackedPlanet>::const_iterator AttackedPlanetsCIt;
*/



class GamePlanet;
class GameState;

// v11 structures
// needed to support multi-pronged attacks
// on neutral and enemy planets, possibly
// also reinforcement)
struct TaskPlan {
	public:
		struct TaskMember {
			public:
				TaskMember(GamePlanet* planet, unsigned int numShips, unsigned int difTurns) {
					mPlanet   = planet;
					mNumShips = numShips;
					mDifTurns = difTurns;
				}

				GamePlanet* GetPlanet() { return mPlanet; }
				unsigned int GetNumReservedShips() const { return mNumShips; }
				unsigned int GetNumWaitingTurns() const { return mDifTurns; }

			private:
				// NOTE: a planet can be part of more than one task!
				GamePlanet* mPlanet;

				unsigned int mNumShips; // number of ships reserved by <mPlanet> for the plan we are member of
				unsigned int mDifTurns; // number of turns <mPlanet> must wait wrt. mStartTurn before IssueOrder
		};

		TaskPlan(): mStartTurn(-1U) {}
		TaskPlan(unsigned int startTurn): mStartTurn(startTurn) {}
		TaskPlan(const TaskPlan& tp) {
			mMembers = tp.mMembers;
			mStartTurn = tp.mStartTurn;
		}

		void Reset();
		unsigned int GetStartTurn() const { return mStartTurn; }
		void AddMember(GamePlanet*, unsigned int, unsigned int);
		std::list<TaskMember>& GetMembers() { return mMembers; }

	private:
		std::list<TaskMember> mMembers;

		// age of this plan is given by <mCurrentTurn - mStartTurn>
		unsigned int mStartTurn;
};

struct Order {
public:
	Order(unsigned int pSrcID, unsigned pDstID, unsigned int nShips, bool simulated = false): mIsSimulatedOrder(simulated) {
		mSrcPltID = pSrcID;
		mDstPltID = pDstID;
		mNumShips = nShips;
	}

	bool IsValid(const GameState&) const;
	void Issue(GameState&);

private:
	unsigned int mSrcPltID;
	unsigned int mDstPltID;
	unsigned int mNumShips;

	bool mIsSimulatedOrder;
};

#endif
