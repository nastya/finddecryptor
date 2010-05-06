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


#ifndef EMU_CPU_DATA_H_
#define EMU_CPU_DATA_H_

#include <stdint.h>
#include <stdbool.h>

#include <emu/emu.h>
#include <emu/emu_cpu_instruction.h>
#include <emu/emu_instruction.h>

enum emu_cpu_flag {
	f_cf = 0, f_pf = 2, f_af = 4, f_zf = 6, f_sf = 7, f_tf = 8, f_if = 9,
	f_df = 10, f_of = 11
};

struct emu_track_and_source;

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

extern int64_t max_inttype_borders[][2][2];

#endif /*EMU_CPU_DATA_H_*/
