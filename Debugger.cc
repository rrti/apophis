#include "./Debugger.h"

#include <iostream>

Debugger* Debugger::GetInstance() {
	static Debugger* d = NULL;

	if (d == NULL) {
		d = new Debugger();
	}

	return d;
}

void Debugger::FreeInstance(Debugger* d) {
	delete d;
}



#ifdef DEBUG
const char* NC = "\E[0m"; // No Color (reset to default)
const char* FATAL = "\E[1m\E[37m\E[41m"; // Red background, white bold text

Debugger::Debugger(): mEnabled(false) {
}

Debugger::~Debugger() {
}

bool Debugger::Begin(const char*, int) {
	mEnabled = true;
	mMessage.clear();

	return true;
}

void Debugger::Print(const char* msg) {
	mMessage += std::string(msg);

#if (!defined(WIN32) && !defined(__powerpc64__))
	fprintf(stderr, "%s%s%s%s", NC, FATAL, msg, NC);
#else
	fprintf(stderr, msg);
#endif
}

bool Debugger::End() {
	bool breakPoint = true;

	Print("\n\nType `continue` in gdb to ignore.");

	mEnabled = false;
	mMessage.clear();

	return breakPoint;
}

void Debugger::DumpStack(char** symbols, size_t size) {
	if (symbols != NULL) {
		Print("\n");

		for (size_t i = 0; i < size; i++) {
			Print(symbols[i]);
			Print("\n");
		}
	}
}
#endif // DEBUG
