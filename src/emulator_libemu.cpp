#include "emulator_libemu.h"

#include <fstream>
#include <algorithm>

using namespace std;

const int Emulator_LibEmu::mem_before = 10*1024; // 10 KiB, min 1k instuctions
const int Emulator_LibEmu::mem_after = 80*1024; //80 KiB, min 8k instructions

Emulator_LibEmu::Emulator_LibEmu() {
	//ofstream log("../log/libemu.txt");
	//log.close();
	e = emu_new();
	cpu = emu_cpu_get(e);
	mem = emu_memory_get(e);
}
Emulator_LibEmu::~Emulator_LibEmu() {
	emu_free(e);
}
void Emulator_LibEmu::begin(uint pos) {
	if (pos==0) pos = reader->start();
	offset = reader->map(pos) - pos;
	uint start = max(reader->start(), pos - mem_before), end = min(reader->size(), pos + mem_after);

	for (int i=0; i<8; i++)
		cpu->reg[i] = 0;
	cpu->reg[esp] = 0x1000000;	

	emu_memory_clear(mem);
	//emu_memory_write_block(mem, offset + reader->start(), reader->pointer(true), reader->size(true));
	emu_memory_write_block(mem, offset + start, reader->pointer() + start, end - start);

	jump(pos);
}
void Emulator_LibEmu::jump(uint pos) {
	emu_cpu_eip_set(cpu, offset + pos);
}
/*bool Emulator_LibEmu::step() {
	ofstream log("../log/libemu.txt",ios_base::out|ios_base::app);
	bool ok = true;
	if (emu_cpu_parse(cpu) != 0)
		ok = false;
	log << "Command: " << cpu->instr_string << endl;
	if (ok && (emu_cpu_step(cpu) != 0))
		ok = false;
	if (!ok)
		log << "ERROR: " << emu_strerror(cpu->emu) << endl;
	log.close();
	return ok;
}*/
bool Emulator_LibEmu::step() {
	if (emu_cpu_parse(cpu) != 0)
		return false;
	if (emu_cpu_step(cpu) != 0)
		return false;
	return true;
}
bool Emulator_LibEmu::get_command(char *buff, uint size) {
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
