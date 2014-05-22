#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "PlanetWars.h"

void Run(FILE* input, bool debugMode) {
	PlanetWars pw(debugMode);

	std::string currLine;
	std::string gameData;

	while (true) {
		currLine += static_cast<char>(fgetc(input));

		if (currLine.at(currLine.size() - 1) == '\n') {
			if (currLine.length() >= 2 && currLine.substr(0, 2) == "go") {
				pw.ParseGameState(gameData);
				pw.DoTurn();
				pw.FinishTurn();

				gameData.clear();
			} else {
				gameData += currLine;
			}

			currLine.clear();
		}
	}
}

int main(int argc, char** argv) {
	if (argc >= 2) {
		// stand-alone execution mode; pass
		// a demofile.txt to serve as stdin
		// (useful to debug moves made for
		// a specific turn given the input)
		bool debugMode = false;
		char* demoName = NULL;
		FILE* demoFile = NULL;

		for (int n = 1; n < argc; n++) {
			const std::string arg = argv[n];

			if (arg.find("--dbg") != std::string::npos) {
				debugMode = true;
			} else if (arg.find(".txt") != std::string::npos) {
				demoName = argv[n];
			}
		}

		if (demoName == NULL) {
			return EXIT_FAILURE;
		}

		demoFile = fopen(demoName, "r");

		if (demoFile == NULL) {
			return EXIT_FAILURE;
		}

		Run(demoFile, debugMode);
		fclose(demoFile);
	} else {
		Run(stdin, false);
	}

	// NOTE: this is unreachable
	return EXIT_SUCCESS;
}
