#ifndef EMULATOR_QEMU_H
#define EMULATOR_QEMU_H

#include "emulator.h"

using namespace std;

extern "C" {
	#include "../qemu/libqemu.h"
}

/**
	@brief
	Emulation via QEMU
*/

class Emulator_Qemu : public Emulator {
public:
	Emulator_Qemu();
	~Emulator_Qemu();
	void begin(uint pos=0);
	bool step();
	bool get_command(char *buff, uint size=10);
	unsigned int get_register(Register reg);
private:
	void end();
	
	CPUState *env;
	bool running;
	unsigned long esp, pos, offset;
	static unsigned long stack_size;
	static const int mem_before; ///<We do not want to copy more bytes than this before start instruction.
	static const int mem_after; ///<We do not want to copy more bytes than this after start instruction.
};

#endif