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
	void begin(uint pos=0);
	void jump(uint pos);
	bool step();
	bool get_command(char *buff, uint size=10);
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
	static const int mem_before; ///<We do not want to copy more bytes than this before start instruction.
	static const int mem_after; ///<We do not want to copy more bytes than this after start instruction.
};

#endif