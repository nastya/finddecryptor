#ifndef MEDIATOR_H
#define MEDIATOR_H

#include <string>

using namespace std;

class Finder;

class Mediator {
public:
	Mediator(int finderType = 0, int emulatorType = 1);
	~Mediator();
	void load(string name, bool guessType=false);
	void link(const unsigned char *data, unsigned int dataSize, bool guessType=false);
	int find();
	
private:
	Finder *finder;
};
#endif