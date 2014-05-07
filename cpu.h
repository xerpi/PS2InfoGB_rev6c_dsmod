/*
 *  InfoGB - A portable GameBoy emulator
 *  Copyright (C) 2003  Jay's Factory <jays_factory@excite.co.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  based on gbe - gameboy emulator
 *  Copyright (C) 1999  Chuck Mason, Steven Fuller, Jeff Miller
 */

#ifndef __CPU_H__
#define __CPU_H__

#include "data.h"

// CPU Interrupts flags
#define INT_VBL	0x01
#define INT_LCD	0x02
#define INT_TIM	0x04
#define INT_SIO	0x08
#define INT_JOY	0x10

// CPU Flags
#define FLAGS_ZERO      0x80
#define FLAGS_NEGATIVE  0x40
#define FLAGS_HALFCARRY 0x20
#define FLAGS_CARRY     0x10

// Macros
#define FLAGS_ISZERO(proc)       (gbz80.AF.b.l & FLAGS_ZERO)
#define FLAGS_ISNEGATIVE(proc)   (gbz80.AF.b.l & FLAGS_NEGATIVE)
#define FLAGS_ISCARRY(proc)      (gbz80.AF.b.l & FLAGS_CARRY)
#define FLAGS_ISHALFCARRY(proc)  (gbz80.AF.b.l & FLAGS_HALFCARRY)

#define FLAGS_SET(proc, x)       (gbz80.AF.b.l |= x)
#define FLAGS_ISSET(proc, x)     (gbz80.AF.b.l & x)
#define FLAGS_CLEAR(proc, x)     (gbz80.AF.b.l &= ~(x))

//
typedef struct _gameboy_proc_t {
	wordun   AF;
	wordun 	BC;
	wordun   DE;
	wordun   HL;
	wordun   SP;
	wordun   PC;
	int IFF;
	int running;
	int machine_cycles;
} gameboy_proc_t;

extern gameboy_proc_t gbz80;
extern int CPUSpeed;

#ifdef __cplusplus
extern "C" {
#endif
   void gameboy_cpu_hardreset();
   void gameboy_cpu_run();
#ifdef __cplusplus
}
#endif
void gameboy_cpu_execute_opcode(unsigned char OpCode);
void gameboy_update();

bool gameboy_read_state();
bool gameboy_save_state();

#define MORE_CYCLES(X) {}
#endif
