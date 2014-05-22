#include "Globals.h"
#include "Debugger.h"
#include "Logger.h"
#include "Timer.h"

void InitGlobals() {
	BOT_ASSERT(LOGGER == NULL);
	BOT_ASSERT(TIMER == NULL);

	LOGGER = new Logger("log-");
	TIMER = new Timer();
}

void KillGlobals() {
	BOT_ASSERT(LOGGER != NULL);
	BOT_ASSERT(TIMER != NULL);

	delete TIMER;
	delete LOGGER;

	LOGGER = NULL;
	TIMER = NULL;
}
