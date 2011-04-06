#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "emulator_qemu.h"

using namespace std;

const int Emulator_Qemu::mem_before = 10*1024; // 10 KiB, min 1k instuctions
const int Emulator_Qemu::mem_after = 80*1024; //80 KiB, min 8k instructions

unsigned long Emulator_Qemu::stack_size = 8 * 1024 * 1024UL;

Emulator_Qemu::Emulator_Qemu() {
	qemu_init();

	qemu_mmap(mem_before + mem_after);
	
	unsigned long stack = qemu_mmap_stack(stack_size + qemu_page_size());
	esp = stack + stack_size;
	
	this->env = qemu_cpu_init();
	running = false;
	
	offset = qemu_offset();
}
Emulator_Qemu::~Emulator_Qemu() {
	end();
}
void Emulator_Qemu::begin(uint pos) {
	end();

	uint start = max((int) reader->start(), (int) pos - mem_before), end = min(reader->size(), pos + mem_after);

//	printf("Begin. Start: 0x%X, end: 0x%X, pos: 0x%X\n", start, end, pos);

	memcpy((void *) offset, reader->pointer() + start, end - start);

	qemu_reset(env);
	qemu_set_esp(env,this->esp);
	qemu_set_eip(env,pos - start);
	
	qemu_setup_segments(env);

	qemu_flog("Starting\n");

	if (qemu_exec_start(env)) {
//		return false;
		return;
	}

	this->pos = qemu_pos(env);
	running = true;

//	return true;
}
bool Emulator_Qemu::step() {
	qemu_disas(pos);

	while (pos == qemu_pos(env)) {
		while (qemu_set_jmp(env) != 0);
		if (qemu_exception_index(env) > 0) {
			fprintf(stderr, "qemu: 0x%08lx: interrupt recieved: 0x%x.\n", pos, qemu_exception_index(env));
			return false;
		}
		qemu_exec_middle(env);
	}
	
	pos = qemu_pos(env);
	return true;
}
bool Emulator_Qemu::get_command(char *buff, uint size) {
	memcpy(buff, (void *) (offset + pos), size);
	return true;
}
unsigned int Emulator_Qemu::get_register(Register reg) {
	switch (reg) {
/*		case EAX:
			return emu_cpu_reg32_get(cpu, eax);
		case EBX:
			return emu_cpu_reg32_get(cpu, ebx);
		case ECX:
			return emu_cpu_reg32_get(cpu, ecx);
		case EDX:
			return emu_cpu_reg32_get(cpu, edx);
		case ESI:
			return emu_cpu_reg32_get(cpu, esi);
		case EDI:
			return emu_cpu_reg32_get(cpu, edi);
		case ESP:
			return emu_cpu_reg32_get(cpu, esp);
		case EBP:
			return emu_cpu_reg32_get(cpu, ebp);*/
		case EIP:
			return pos;
		default:;
	}
	return 0;
}
void Emulator_Qemu::end() {
	if (!running) {
		return;
	}
	qemu_exec_end(env);
	running = false;
}
