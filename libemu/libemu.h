/********************************************************************************
 *                               libemu
 *
 *                    - x86 shellcode emulation -
 *
 *
 * Copyright (C) 2007  Paul Baecher & Markus Koetter
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * 
 *             contact nepenthesdev@users.sourceforge.net  
 *
 *******************************************************************************/

#ifndef LIBEMU_H
#define LIBEMU_H

#include <stdint.h>

struct emu;
struct emu_cpu;
struct emu_fpu;
struct emu_memory;
struct emu_logging;
struct emu_track_and_source;

enum emu_reg32 {
	eax = 0, ecx, edx, ebx, esp, ebp, esi, edi
};

struct emu *emu_new();
struct emu_memory *emu_memory_get(struct emu *e);
struct emu_cpu *emu_cpu_get(struct emu *e);
void emu_free(struct emu *e);
void emu_memory_clear(struct emu_memory *em);

int32_t emu_memory_read_block(struct emu_memory *m, uint32_t addr, void *dest, size_t len);
int32_t emu_memory_write_block(struct emu_memory *m, uint32_t addr, void *src, size_t len);

void emu_cpu_eip_set(struct emu_cpu *c, uint32_t eip);
uint32_t emu_cpu_eip_get(struct emu_cpu *c);
int32_t emu_cpu_parse(struct emu_cpu *c);
int32_t emu_cpu_step(struct emu_cpu *c);


struct emu_tracking_info
{
	uint32_t eip;

	uint32_t eflags;
	uint32_t reg[8];

	uint8_t fpu:1; // used to store the last_instruction information required for fnstenv
};

struct emu_cpu_instruction
{
	uint8_t opc;
	uint8_t opc_2nd;
	uint16_t prefixes;
	uint8_t s_bit : 1;
	uint8_t w_bit : 1;
	uint8_t operand_size : 2;

	struct /* mod r/m data */
	{
		union
		{
			uint8_t mod : 2;
			uint8_t x : 2;
		};

		union
		{
			uint8_t reg1 : 3;
			uint8_t opc : 3;
			uint8_t sreg3 : 3;
			uint8_t y : 3;
		};

		union
		{
			uint8_t reg : 3;
			uint8_t reg2 : 3;
			uint8_t rm : 3;
			uint8_t z : 3;
		};

		struct
		{
			uint8_t scale : 2;
			uint8_t index : 3;
			uint8_t base : 3;
		} sib;

		union
		{
			uint8_t s8;
			uint16_t s16;
			uint32_t s32;
		} disp;
		
		uint32_t ea;
	} modrm;

	uint32_t imm;
	uint16_t *imm16;
	uint8_t *imm8;

	int32_t disp;
};


struct emu_fpu_instruction
{
	uint16_t prefixes;
	
	uint8_t fpu_data[2]; /* TODO: split into correct fields */
	uint32_t ea;
	
	uint32_t last_instr;

};

struct emu_instruction
{
	uint16_t prefixes;
	uint8_t opc;
	uint8_t is_fpu : 1;
	
	union
	{
		struct emu_cpu_instruction cpu;
		struct emu_fpu_instruction fpu;
	};

	struct 
	{
		struct emu_tracking_info init;
		struct emu_tracking_info need;		
	} track;

	struct 
	{
		uint8_t has_cond_pos : 1;
		uint32_t norm_pos;
		uint32_t cond_pos;
	} source;
};

struct emu_cpu
{
	struct emu *emu;
	struct emu_memory *mem;
	
	uint32_t eip;
	uint32_t eflags;
	uint32_t reg[8];
	uint16_t *reg16[8];
	uint8_t *reg8[8];

	struct emu_instruction 			instr;
	struct emu_cpu_instruction_info 	*cpu_instr_info;
	
	uint32_t last_fpu_instr[2];

	char *instr_string;

	bool repeat_current_instr;

	struct emu_track_and_source *tracking;
};

#endif // HAVE_EMU_H
