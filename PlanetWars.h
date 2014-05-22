// This file contains helper code that does all the boring stuff for you.
// The code in this file takes care of storing lists of planets and fleets, as
// well as communicating with the game engine. You can get along just fine
// without ever looking at this file. However, you are welcome to modify it
// if you want to.
#ifndef _PLANETWARS_HDR_
#define _PLANETWARS_HDR_

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>

#include "GameState.h"
#include "Structures.h"



class PlanetWars {
public:
	PlanetWars(bool);
	~PlanetWars();

	void ParseGameState(const std::string&);

	void FinishTurn();
	void DoTurn();

private:
	void AssignAttackersToAttackees(
		std::map<unsigned int, std::set<GamePlanet*> >&,
		const std::vector< std::vector< std::set<GamePlanet*> > >&,
		const std::vector<GamePlanet*>&,
		const std::vector<unsigned int>&
	);

	void GetPlanetClusters(unsigned int, double);
	unsigned int GetMaxSackGrowthTurn(unsigned int, unsigned int);

	void AddDefenseTaskPlans(std::vector<GamePlanet*>&, std::vector<unsigned int>&);
	void AddOffenseTaskPlans(unsigned int, std::vector<unsigned int>&);

	void AddTaskPlan(
		const GamePlanet*,
		unsigned int,
		unsigned int,
		std::map<unsigned int, unsigned int>&,
		std::vector<unsigned int>&,
		const std::list<unsigned int>&
	);
	void ExecuteTaskPlans();


	bool DetectStaleMate(unsigned int);

	bool CanCapturePlanetNN(
		const GamePlanet*,
		unsigned int,
		const std::vector<unsigned int>&, std::list<unsigned int>&,
		unsigned int*, unsigned int*, unsigned int*, unsigned int*
	) const;
	bool CanCapturePlanet(
		const GamePlanet*,
		const std::set<GamePlanet*>&, const std::vector<unsigned int>&, std::list<unsigned int>&,
		unsigned int*, unsigned int*, unsigned int*, unsigned int*
	) const;
	bool CanCapturePlanet(
		const GamePlanet*,
		const std::vector<GamePlanet*>&, const std::vector<unsigned int>&, std::list<unsigned int>&,
		unsigned int*, unsigned int*, unsigned int*, unsigned int*
	) const;


	bool CapturePlanet(const GamePlanet&, std::set<GamePlanet*>&, std::vector<unsigned int>&);
	void CapturePlanets(std::vector<GamePlanet*>&, std::vector<GamePlanet*>&, std::vector<unsigned int>&);
	void SetPlanetValues(unsigned int, std::vector<GamePlanet*>&) const;
	void UpdatePlanetSpareShipCounts(std::vector<GamePlanet*>&, std::vector<unsigned int>&) const;

	void DefendAttackedPlanets(std::vector<unsigned int>&, std::vector<GamePlanet*>&);
	void DefendFutureFrontierPlanets(std::vector<unsigned int>&, std::vector<GamePlanet*>&, std::vector<GamePlanet*>&, std::vector<GamePlanet*>&);
	void DefendFrontierPlanets(std::vector<unsigned int>&, std::vector<GamePlanet*>&, std::vector<GamePlanet*>&);




	bool ParsePlanetsAndFleets(const std::string&);
	bool ParsePlanet(const std::vector<std::string>&, std::list<GamePlanet>&);
	bool ParseFleet(const std::vector<std::string>&, std::list<GameFleet>&);

	void ClearTurnData();
	void CalcTurnData();



	// number of current turn being processed
	unsigned int mCurrentTurn;

	// weights for the scoring function
	// std::vector<double> mPlanetValueWeights;
	std::map<unsigned int, TaskPlan> mTaskPlans;

	// calculation time per turn (in seconds)
	std::list<double> mTurnTimes;

	std::list<Order> mOrders;
	std::list<bool> mStaleMateTurns;

	// current state; for simulating wanted orders
	// call AddFleet(fleet, false) for every order
	// (followed by GetPlanetStates(pID))
	GameState mGameState;
	// future states derived from <mGameState>
	// (at the start of CalcTurnData, so these
	// do not take our wanted orders for THIS
	// turn into account)
	std::vector<GameState> mGameStates;
};

#endif
