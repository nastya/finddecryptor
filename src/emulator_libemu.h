#ifndef EMULATOR_LIBEMU_H
#define EMULATOR_LIBEMU_H

#include "emulator.h"

using namespace std;

extern "C" {
#include "emu/emu.h"
#include "emu/emu_memory.h"
#include "emu/emu_cpu.h"
#include "emu/emu_cpu_data.h"
}

class Emulator_LibEmu : public Emulator {
public:
	Emulator_LibEmu();
	~Emulator_LibEmu();
	void start(PEReader *r);
	void begin(int pos=0);
	void jump(int pos);
	bool step();
	bool get_command(char *buff, int size=10);
	unsigned int get_register(Register reg);
private:
	PEReader *reader;
	int offset;
	struct emu *e;
	struct emu_cpu *cpu;
	struct emu_memory *mem;
};

#endif