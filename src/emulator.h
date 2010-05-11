#ifndef EMULATOR_H
#define EMULATOR_H

#include "reader.h"
#include "data.h"

using namespace std;

/**
	@brief
	Interface for emulators
*/

class Emulator : protected Data {
public:	
	/**
	  Binds reader to emulator.
	  @param r Pointer to an examplar of class PEReader which is used for taking special information out of PE-header.
	*/
	void bind(Reader *r);
	/**
	  Runs emulation from the instruction situated on specified position in input file.
	  @param pos Position to run emulation from.
	*/
	virtual void begin(int pos=0) = 0;
	/**
	  Сontinues emulation from the spesified position.
	  @param pos Spesified position.
	*/
	virtual void jump(int pos) = 0;
	/**
	  Passes emulation to the next instruction.
	*/
	virtual bool step() = 0;
	/**
	  Copies @ref size bytes of current emulated instruction into buffer @ref buff.
	*/
	virtual bool get_command(char *buff, int size=10) = 0;
	/**
	  Returns current state of register @ref reg.
	*/
	virtual unsigned int get_register(Register reg) = 0;
	
protected:
	Reader *reader; ///<Pointer to an examplar of class PEReader which is used for taking special information out of PE-header.
};

#endif 
