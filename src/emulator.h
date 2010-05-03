#ifndef EMULATOR_H
#define EMULATOR_H

#include "PEReader.h"
#include "data.h"

using namespace std;

class Emulator: protected Data {
public:	
	virtual void start(PEReader *r) = 0;
	virtual void begin(int pos=0) = 0;
	virtual void jump(int pos) = 0;
	virtual void step() = 0;
	virtual bool get_command(char *buff, int size=10) = 0;
	virtual unsigned int get_register(Register reg) = 0;
};

#endif 
