#include "mediator.h"
#include "finder-cycle.h"
#include "finder-getpc.h"

Mediator::Mediator(int finderType, int emulatorType) {
	switch (finderType) {
		case 0:
			finder = new FinderCycle(emulatorType);
			break;
		case 1:
			finder = new FinderGetPC(emulatorType);
			break;
		default:
			cerr << "Unknown finder type!" << endl;
			finder = NULL;
	}
}
Mediator::~Mediator() {
	delete finder;
}
void Mediator::load(string name, bool guessType) {
	return finder->load(name, guessType);
}
void Mediator::link(const unsigned char *data, uint dataSize, bool guessType) {
	return finder->link(data, dataSize, guessType);
}
int Mediator::find() {
	return finder->find();
}
