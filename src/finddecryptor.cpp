#include "finddecryptor.h"
#include "finder-cycle.h"
#include "finder-getpc.h"
#include "finder-libemu.h"

FindDecryptor::FindDecryptor(int finderType, int emulatorType) {
	switch (finderType) {
		case 0:
			finder = new FinderCycle(emulatorType);
			break;
		case 1:
			finder = new FinderGetPC(emulatorType);
			break;
		case 2:
			finder = new FinderLibemu();
			break;
		default:
			cerr << "Unknown finder type!" << endl;
			finder = NULL;
	}
}
FindDecryptor::~FindDecryptor() {
	delete finder;
}
void FindDecryptor::load(string name, bool guessType) {
	return finder->load(name, guessType);
}
void FindDecryptor::link(const unsigned char *data, uint dataSize, bool guessType) {
	return finder->link(data, dataSize, guessType);
}
int FindDecryptor::find() {
	return finder->find();
}
