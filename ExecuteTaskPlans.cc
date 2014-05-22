#include "PlanetWars.h"
#include "Logger.h"

void PlanetWars::ExecuteTaskPlans() {
	// LOG_STDOUT(LOGGER, "[ExecuteTaskPlans] mTaskPlans.size()=%u\n", mTaskPlans.size());

	std::list<unsigned int> invalidPlans;
	std::list<unsigned int>::iterator invalidPlansIt;

	// execute all old and newly-added plans
	// (perform heavy sanity-checking on the
	// per-plan data)
	for (std::map<unsigned int, TaskPlan>::iterator it = mTaskPlans.begin(); it != mTaskPlans.end(); ++it) {
		const unsigned int pDstID = it->first;

		TaskPlan& plan = it->second;
		std::list<TaskPlan::TaskMember>& members = plan.GetMembers();
		std::list<TaskPlan::TaskMember>::iterator membersIt;

		bool valid = !members.empty();

		// LOG_STDOUT(LOGGER, "\t[1] taskID=%u, valid=%d\n", it->first, valid);

		if (valid) {
			// check if this plan needs to be aborted
			for (membersIt = members.begin(); membersIt != members.end(); ++membersIt) {
				TaskPlan::TaskMember& taskMember = *membersIt;
				GamePlanet* taskPlanet = taskMember.GetPlanet();

				// LOG_STDOUT(LOGGER, "\t\ttaskMember=%u, owner=%u, numReservedShips=%u, spareShips=%u\n", taskPlanet->GetID(), taskPlanet->GetOwner(), taskMember.GetNumReservedShips(), mGameState.GetPlanetMaxSpareShips(taskPlanet->GetID()));

				if (taskPlanet->GetOwner() != OWNER_ALLIED) {
					valid = false; break;
				}

				// NOTE:
				//     if we have already issued orders in AddDefenseTasks, GetPlanetMaxSpareShips
				//     can return a lower value than it did at the start of DoTurn (bad because the
				//     spareShips array is filled based on the initial values, and the reservedShips
				//     map is based on *that* [in a bad way], see AddTaskPlan)
				if (taskMember.GetNumReservedShips() > mGameState.GetPlanetMaxSpareShips(taskPlanet->GetID())) {
					valid = false; break;
				}
			}
		}

		// LOG_STDOUT(LOGGER, "\t[2] taskID=%u, valid=%d\n", it->first, valid);

		if (!valid) {
			// abort the task and remove it
			invalidPlans.push_back(pDstID);
		} else {
			for (membersIt = members.begin(); membersIt != members.end(); ) {
				TaskPlan::TaskMember& member = *membersIt;
				GamePlanet* planet = member.GetPlanet();

				if ((plan.GetStartTurn() + member.GetNumWaitingTurns()) == mCurrentTurn) {
					planet->SetNumReservedShips(planet->GetNumReservedShips() - member.GetNumReservedShips());

					Order order(planet->GetID(), pDstID, member.GetNumReservedShips());
					order.Issue(mGameState);

					// NOTE: if all members are erased "too soon"
					// (before ships from all of them have actually
					// arrived at <pDst>), then the same planet can
					// be targeted multiple times
					membersIt = members.erase(membersIt);
				} else {
					++membersIt;
				}
			}
		}

		if (members.empty()) {
			invalidPlans.push_back(pDstID);
		}
	}


	for (invalidPlansIt = invalidPlans.begin(); invalidPlansIt != invalidPlans.end(); ++invalidPlansIt) {
		mTaskPlans[*invalidPlansIt].Reset();
		mTaskPlans.erase(*invalidPlansIt);
	}
}
