#ifndef _INFOGB_COMMON_H__
#define _INFOGB_COMMON_H__

#define SCREEN_W 320
#define SCREEN_H 256

#define NWIDTH  SCREEN_W
#define NHEIGHT SCREEN_H

#define SCROLL_BOX_MAX_ENTRY  10

#define Z_TEXTURE    2
#define Z_BOX1       3
#define Z_BOX2       4
#define Z_LIST       5
#define Z_SELECT     6
#define Z_SCROLLBG	7
#define Z_SCROLL     8
#define Z_SCROLL_M	9

#define TXTMIN  0
#define TXTMAX  320

#define NES_TEX	0x0A0000 + 0x0A0000
#define NES_CLUT	0x128000 + 0x0A0000
#define VRAM_MAX	0x3E8000

#define GS256   8
#define GS320   9
#define GS640   9
#define GS1024  10

#endif
