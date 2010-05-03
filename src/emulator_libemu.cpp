#include "emulator_libemu.h"

using namespace std;

Emulator_LibEmu::Emulator_LibEmu() {
	e = emu_new();
	cpu = emu_cpu_get(e);
	mem = emu_memory_get(e);
}
Emulator_LibEmu::~Emulator_LibEmu() {
	emu_free(e);
}
void Emulator_LibEmu::start(PEReader *r) {
	reader = r;
}
void Emulator_LibEmu::begin(int pos) {
	if (pos==0) pos = reader->start();
	offset = reader->map(pos) - pos;

	for (int i=0; i<8; i++)
		cpu->reg[i] = 0;
	cpu->reg[esp] = 0x1000000;	

	emu_memory_clear(mem);
	for (int i = 0; i < reader->size(); i++)
		emu_memory_write_byte(mem, offset+i, reader->pointer()[i]);
	emu_memory_write_byte(mem, offset+reader->size(), '\xcc');

	jump(pos);
}
void Emulator_LibEmu::jump(int pos) {
	emu_cpu_eip_set(cpu, offset + pos);
}
bool Emulator_LibEmu::step() {
	if (emu_cpu_parse(cpu) != 0)
		return false;
	if (emu_cpu_step(cpu) != 0)
		return false;
	return true;
}
bool Emulator_LibEmu::get_command(char *buff, int size) {
	emu_memory_read_block(mem, cpu->eip, buff, size);
	return true;
}
unsigned int Emulator_LibEmu::get_register(Register reg) {
	switch (reg)
	{
		case EAX:
			return cpu->reg[eax];
		case EBX:
			return cpu->reg[ebx];
		case ECX:
			return cpu->reg[ecx];
		case EDX:
			return cpu->reg[edx];
		case ESI:
			return cpu->reg[esi];
		case EDI:
			return cpu->reg[edi];
		case ESP:
			return cpu->reg[esp];
		case EBP:
			return cpu->reg[ebp];
		case EIP:
			return cpu->eip;
		default:;
	}
	return 0;
}
