#include <iostream>
#include <list>

#include "Structures.h"
#include "GameState.h"
#include "Debugger.h"
#include "Logger.h"

bool Order::IsValid(const GameState& gs) const {
	if (mSrcPltID >= gs.GetNumPlanets()) { return false; }
	if (mDstPltID >= gs.GetNumPlanets()) { return false; }

	const GamePlanet& pSrc = gs.GetPlanet(mSrcPltID);
	const GamePlanet& pDst = gs.GetPlanet(mDstPltID);

	if (pSrc.GetID() == pDst.GetID()) { return false; }
	if (pSrc.GetOwner() != OWNER_ALLIED) { return false; }
	if (mNumShips > pSrc.GetNumShips()) { return false; }
	if (mNumShips == 0) { return false; }

	return true;
}

void Order::Issue(GameState& gs) {
	// LOG_STDOUT(LOGGER, "[Order::Issue] mSrcPltID=%u, mDstPltID=%u, mNumShips=%u, IsValid=%d\n", mSrcPltID, mDstPltID, mNumShips, IsValid(gs));
	BOT_ASSERT(IsValid(gs));

	GamePlanet& pSrc = gs.GetPlanet(mSrcPltID);
	GamePlanet& pDst = gs.GetPlanet(mDstPltID);
	pSrc.SetNumShips(pSrc.GetNumShips() - mNumShips);

	if (mIsSimulatedOrder) {
		GameFleet f;

		f.SetOwner(OWNER_ALLIED);
		f.SetNumShips(mNumShips);
		f.SetSrcPlanetID(mSrcPltID);
		f.SetDstPlanetID(mDstPltID);
		f.SetTurnsRemaining(GameMap::GetDistance(&pSrc, &pDst));

		gs.AddFleet(f, false);
	} else {
		std::cout << mSrcPltID << " ";
		std::cout << mDstPltID << " ";
		std::cout << mNumShips << std::endl;
		std::cout.flush();
	}
}



void TaskPlan::AddMember(GamePlanet* pSrc, unsigned int numShips, unsigned int difTurns) {
	mMembers.push_back(TaskMember(pSrc, numShips, difTurns));
}

void TaskPlan::Reset() {
	for (std::list<TaskMember>::iterator it = mMembers.begin(); it != mMembers.end(); ++it) {
		GamePlanet* p = (*it).GetPlanet();
		p->SetNumReservedShips(p->GetNumReservedShips() - (*it).GetNumReservedShips());
	}

	mMembers.clear();
}
