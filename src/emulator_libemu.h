#ifndef EMULATOR_LIBEMU_H
#define EMULATOR_LIBEMU_H

#include "emulator.h"

using namespace std;

extern "C" {
#include "../libemu/libemu.h"
}

/**
	@brief
	Emulation via libemu
*/

class Emulator_LibEmu : public Emulator {
public:
	Emulator_LibEmu();
	~Emulator_LibEmu();
	void bind(PEReader *r);
	void begin(int pos=0);
	void jump(int pos);
	bool step();
	bool get_command(char *buff, int size=10);
	unsigned int get_register(Register reg);
private:
	int offset; ///<Offset for emulated instructions (the memory/file adrress difference of the beginning of the block where they are situated).
	/**
	 Struct containing emulator.
	 @sa emu (libemu documentation)
	*/
	struct emu *e;
	/**
	 Struct containing emulator.
	 @sa emu (libemu documentation)
	*/
	struct emu_cpu *cpu;
	/**
	 Struct containing memory.
	 @sa emu (libemu documentation)
	*/
	struct emu_memory *mem;
};

#endif