#include "mediator.h"
#include "finder-cycle.h"

Mediator::Mediator(int type) {
	finder = new FinderCycle(type);
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
