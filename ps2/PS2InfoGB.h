/*
 *  Copyright (C) 2010  Krystian Karaœ <k4rasq@gmail.com>
 *
 *  I release it for fully free use by anyone, anywhere, for any purpose.
 *  But I still retain the copyright, so no one else can limit this release.
 */

#ifndef _PS2_INFOGB_H__
#define _PS2_INFOGB_H__

#include "PS2Browser.h"

//////////////////////////////////
// Globals declaration
//////////////////////////////////

// Sound
extern short int *g_SndBuff;
extern int g_SndSample;
extern int g_SndSampler;
extern int g_SamplePos;

// Graphic
extern int g_ShowFPS;   // 1 - show FPS on, 0 - ... off
extern int g_Stretch;   // 0 = original res (352x240 cut & 256 with black border)
extern int g_Filter;    // 1 = filtering on
extern int whichdrawbuf;
extern int endflag;

// Browser
extern Browser g_PS2Browser;
extern char    g_FilePath[MAX_FILENAME_CHARS + MAX_FILEPATH_CHARS];
extern int     CNF_edited;

#endif
