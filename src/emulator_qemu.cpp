#include <stdlib.h>
#include <iostream>

#include "emulator_qemu.h"

using namespace std;

const int Emulator_Qemu::mem_before = 10*1024; // 10 KiB, min 1k instuctions
const int Emulator_Qemu::mem_after = 80*1024; //80 KiB, min 8k instructions

unsigned long Emulator_Qemu::stack_size = 100 * 1024; // 100 KiB

Emulator_Qemu::Emulator_Qemu() {
	env = qemu_stepper_init();
	if (qemu_stepper_data_prepare(env, mem_before + mem_after, stack_size)) {
		cerr << "Error loading qemu" << endl;
		exit(0);
	}
}
Emulator_Qemu::~Emulator_Qemu() {
	qemu_stepper_free(env);
}
void Emulator_Qemu::begin(uint pos) {
	if (pos==0) {
		pos = reader->start();
	}
	uint start = max((int) reader->start(), (int) pos - mem_before), end = min(reader->size(), pos + mem_after);
	offset = pos - reader->map(pos) + qemu_stepper_offset(env) - start;
	qemu_stepper_stack_clear(env);
	qemu_stepper_data_set(env, reader->pointer() + start, end - start);
	qemu_stepper_entry_set(env, pos - start, stack_size / 4);
}
bool Emulator_Qemu::step() {
//	qemu_stepper_print_debug(env);
	switch (qemu_stepper_step(env)) {
		case 0x2c:
		case 0x80:
		case 0:
			return true;
		default:;
	}
	return false;
}
bool Emulator_Qemu::get_command(char *buff, uint size) {
	return qemu_stepper_read(env, buff, size) == 0;
}
bool Emulator_Qemu::get_memory(char *buff, int addr, uint size)
{
	return qemu_stepper_read_code(env, buff, size, addr) == 0;
}
unsigned int Emulator_Qemu::get_register(Register reg) {
	switch (reg) {
		case EAX:
			return qemu_stepper_register(env, 0);
		case EBX:
			return qemu_stepper_register(env, 3);
		case ECX:
			return qemu_stepper_register(env, 1);
		case EDX:
			return qemu_stepper_register(env, 2);
		case ESI:
			return qemu_stepper_register(env, 6);
		case EDI:
			return qemu_stepper_register(env, 7);
		case ESP:
			return qemu_stepper_register(env, 4);
		case EBP:
			return qemu_stepper_register(env, 5);
		case EIP:
			return qemu_stepper_eip(env) - offset;
		default:;
	}
	return 0;
}
unsigned int Emulator_Qemu::memory_offset() {
	return offset;
}
