## NOTE:
##     contest server uses its own makefile (which
##     does not link the binary to anything except
##     libm)
CC=g++
CXXFLAGS=-Wall -Wextra -O3 -fomit-frame-pointer
LXXFLAGS=-lrt
TARGET=MyBot
OBJECTS= \
	MyBot.o \
	PlanetWars.o \
	Globals.o \
	GamePlanet.o \
	GameState.o \
	GameMap.o \
	GameStats.o \
	DoTurn.o \
	AddDefenseTaskPlans.o \
	AddOffenseTaskPlans.o \
	AddTaskPlan.o \
	ExecuteTaskPlans.o \
	FindFrontierPlanets.o \
	FindKnapSackPlanets.o \
	FindPlanetClusters.o \
	FindPowerSet.o \
	CanCapturePlanet.o \
	GetPlanetFuturePopulation.o \
	GetPlanetMaxSpareShips.o \
	Debugger.o \
	Misc.o \
	Logger.o \
	Timer.o \
	Structures.o \
	StringUtil.o
PPFLAGS=-DDEBUG -DDEBUG_LOG

default:
	make objects
	make target

MyBot.o: MyBot.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
PlanetWars.o: PlanetWars.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
Globals.o: Globals.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<

GamePlanet.o: GamePlanet.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
GameState.o: GameState.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
GameMap.o: GameMap.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
GameStats.o: GameStats.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
DoTurn.o: DoTurn.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<

AddDefenseTaskPlans.o: AddDefenseTaskPlans.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
AddOffenseTaskPlans.o: AddOffenseTaskPlans.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
AddTaskPlan.o: AddTaskPlan.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
ExecuteTaskPlans.o: ExecuteTaskPlans.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<

FindFrontierPlanets.o: FindFrontierPlanets.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
FindKnapSackPlanets.o: FindKnapSackPlanets.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
FindPlanetClusters.o: FindPlanetClusters.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
FindPowerSet.o: FindPowerSet.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
CanCapturePlanet.o: CanCapturePlanet.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
GetPlanetFuturePopulation.o: GetPlanetFuturePopulation.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
GetPlanetMaxSpareShips.o: GetPlanetMaxSpareShips.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<

Debugger.o: Debugger.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
Misc.o: Misc.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
Logger.o: Logger.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
Timer.o: Timer.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
Structures.o: Structures.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<
StringUtil.o: StringUtil.cc
	$(CC) $(CXXFLAGS) $(PPFLAGS) -c   -o $@    $<

objects: $(OBJECTS)
target:
	$(CC) $(LXXFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -rf *.o $(TARGET)

zip:
	## zip $(TARGET)-`git describe --tags`.zip *.cc *.h
	zip $(TARGET).zip *.cc *.h
