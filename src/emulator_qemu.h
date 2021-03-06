#ifndef EMULATOR_QEMU_H
#define EMULATOR_QEMU_H

#include "emulator.h"

namespace find_decryptor
{

using namespace std;

extern "C" {
	struct CPUState;
	#include "../qemu/qemu-stepper.h"
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
	bool get_memory(char *buff, int addr, uint size=1);
	unsigned int get_register(Register reg);
	unsigned int memory_offset();
private:
	CPUState *env;
	bool running;
	unsigned long esp, pos, offset;
	static unsigned long stack_size;
	static const int mem_before; ///<We do not want to copy more bytes than this before start instruction.
	static const int mem_after; ///<We do not want to copy more bytes than this after start instruction.
};

} //namespace find_decryptor

#endif