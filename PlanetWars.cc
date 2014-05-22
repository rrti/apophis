#include <cstring>
#include <cstdlib>
#include <iostream>

#include "PlanetWars.h"
#include "Globals.h"
#include "Debugger.h"
#include "Logger.h"
#include "Timer.h"
#include "StringUtil.h"



PlanetWars::PlanetWars(bool): mCurrentTurn(0) {
	InitGlobals();
}

PlanetWars::~PlanetWars() {
	mGameStates.clear();
	mTurnTimes.clear();

	KillGlobals();
}

void PlanetWars::FinishTurn() {
	mCurrentTurn += 1;

	for (std::list<Order>::iterator it = mOrders.begin(); it != mOrders.end(); ++it) {
		(*it).Issue(mGameState);
	}

	mOrders.clear();

	std::cout << "go" << std::endl;
	std::cout.flush();
}






bool PlanetWars::ParsePlanet(const std::vector<std::string>& tokens, std::list<GamePlanet>& parsedPlanets) {
	GamePlanet pp;

	pp.SetID(parsedPlanets.size());
	pp.SetOwner(atoi(tokens[3].c_str()));
	pp.SetNumShips(atoi(tokens[4].c_str()));
	pp.SetGrowthRate(atoi(tokens[5].c_str()));
	pp.SetXPos(atof(tokens[1].c_str()));
	pp.SetYPos(atof(tokens[2].c_str()));

	parsedPlanets.push_back(pp);
	return true;
}

bool PlanetWars::ParseFleet(const std::vector<std::string>& tokens, std::list<GameFleet>& parsedFleets) {
	GameFleet f;

	f.SetOwner(atoi(tokens[1].c_str()));
	f.SetNumShips(atoi(tokens[2].c_str()));
	f.SetSrcPlanetID(atoi(tokens[3].c_str()));
	f.SetDstPlanetID(atoi(tokens[4].c_str()));
	f.SetTurnsRemaining(atoi(tokens[6].c_str()));

	// PlayGame allows zero-sized fleets, prevent exploits
	if (f.GetNumShips() > 0) {
		parsedFleets.push_back(f);
		return true;
	}

	return false;
}



bool PlanetWars::ParsePlanetsAndFleets(const std::string& s) {
	const std::vector<std::string>& lines = StringUtil::Tokenize(s, "\n");

	std::list<GamePlanet> parsedPlanets;
	std::list<GameFleet> parsedFleets;

	for (unsigned int i = 0; i < lines.size(); ++i) {
		const std::string& line = lines[i];
		const size_t comment_begin = line.find_first_of('#');
		const std::vector<std::string>& tokens = StringUtil::Tokenize(line.substr(0, comment_begin));

		if (tokens.empty()) {
			continue;
		}

		if (tokens[0] == "P" && tokens.size() == 6) {
			ParsePlanet(tokens, parsedPlanets);
		} else if (tokens[0] == "F" && tokens.size() == 7) {
			ParseFleet(tokens, parsedFleets);
		} else {
			return false;
		}
	}


	// add all parsed planets to <mGameState>
	// (or just update their state if this is
	// not the first turn)
	for (std::list<GamePlanet>::const_iterator it = parsedPlanets.begin(); it != parsedPlanets.end(); ++it) {
		const GamePlanet& pp = *it;

		if (mCurrentTurn == 0) {
			mGameState.AddPlanet(pp);
		} else {
			GamePlanet& gp = mGameState.GetPlanet(pp.GetID());

			// update this planet's state and clear
			// its fleet-lists (they are immediately
			// repopulated below)
			gp.SetOwner(pp.GetOwner());
			gp.SetNumShips(pp.GetNumShips());
			gp.ClearFleets();
		}
	}

	// add all parsed fleets to <mGameState>
	for (std::list<GameFleet>::const_iterator it = parsedFleets.begin(); it != parsedFleets.end(); ++it) {
		const GameFleet& f = *it;

		mGameState.AddFleet(f, false);
	}

	return true;
}



void PlanetWars::ClearTurnData() {
	// new turn, clear <mGameState>'s fleets (not planets)
	// between here and the end of ParsePlanetsAndFleets(),
	// no planet's fleet-lists are valid!
	mGameStates.clear();
	mGameState.ClearTurnData();
}

void PlanetWars::CalcTurnData() {
	if (mCurrentTurn == 0) {
		mGameState.CalcFirstTurnData();
	}

	mGameState.CalcTurnData(); // current turn
	mGameState.ExecTurns(GameMap::GetMaxPlanetDistance(), mGameStates); // future turns
}



void PlanetWars::ParseGameState(const std::string& stateString) {
	LOG_STDINP(LOGGER, "%s", stateString.c_str());

	LOG_STDOUT(LOGGER, "\n\n\n");
	/* LOG_STDOUT(LOGGER, "[ParseGameState][A]\n"); */ ClearTurnData();
	/* LOG_STDOUT(LOGGER, "[ParseGameState][B]\n"); */ ParsePlanetsAndFleets(stateString);
	/* LOG_STDOUT(LOGGER, "[ParseGameState][C]\n"); */ CalcTurnData();
}
