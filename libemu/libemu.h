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
struct emu_memory;

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

uint32_t emu_cpu_reg32_get(struct emu_cpu *cpu_p, enum emu_reg32 reg);
void  emu_cpu_reg32_set(struct emu_cpu *cpu_p, enum emu_reg32 reg, uint32_t val);

void emu_cpu_eip_set(struct emu_cpu *c, uint32_t eip);
uint32_t emu_cpu_eip_get(struct emu_cpu *c);

int32_t emu_cpu_parse(struct emu_cpu *c);
int32_t emu_cpu_step(struct emu_cpu *c);

const char *emu_strerror(struct emu *e);

#endif // HAVE_EMU_H
