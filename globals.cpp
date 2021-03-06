//#include "globals.h"

// CPU Globals
int g_CycleTargetSGB = (int)(4295454 / 61.17);
int g_CycleTargetGB = (int)(4194304 / 59.73);
int g_CycleTarget = g_CycleTargetGB;

unsigned char g_gbCyclesTable[] = { // OpCode = NX
/*
//X 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f     N
	 8,12,12,16,12,16, 8,16, 8,16,12, 0,12,24, 8,16, // c
	 8,12,12, 0,12,16, 8,16, 8,16,12, 0,12, 0, 8,16, // d
	12,12, 8, 0, 0,16, 8,16,16, 4,16, 0, 0, 0, 8,16, // e
	12,12, 8, 4, 0,16, 8,16,12, 8,16, 4, 0, 0, 8,16  // f
*/

//X 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f     N
    4,12, 8, 8, 4, 4, 8, 4,20, 8, 8, 8, 4, 4, 8, 4, // 0
    4,12, 8, 8, 4, 4, 8, 4,12, 8, 8, 8, 4, 4, 8, 4, // 1
    8,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4, // 2
    8,12, 8, 8,12,12,12, 4, 8, 8, 8, 8, 4, 4, 8, 4, // 3
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 4
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 5
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 6
    8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4, // 7
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 8
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 9
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // a
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // b
    8,12,12,16,12,16, 8,16, 8,16,12, 8,12,24, 8,16, // c
    8,12,12, 4,12,16, 8,16, 8,16,12, 4,12, 4, 8,16, // d
   12,12, 8, 4, 4,16, 8,16,16, 4,16, 4, 4, 4, 8,16, // e
   12,12, 8, 4, 4,16, 8,16,12, 8,16, 4, 0, 4, 8,16
};

unsigned char g_gbCyclesCBTable[] = { // ExtendedOpCode = NX
//X 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f     N
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // 0
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // 1
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // 2
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // 3
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8, // 4
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8, // 5
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8, // 6
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8, // 7
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // 8
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // 9
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // a
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // b
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // c
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // d
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8, // e
    8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
};
