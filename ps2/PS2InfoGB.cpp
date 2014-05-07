/*
 *  Copyright (C) 2010  Krystian Karaœ <k4rasq@gmail.com>
 *
 *  I release it for fully free use by anyone, anywhere, for any purpose.
 *  But I still retain the copyright, so no one else can limit this release.
 */

#define APP_NAME  "PS2 InfoGB 0.6c DS3/DS4"

// PS2 SDK headers
#include <tamtypes.h>
#include <kernel.h>
#include <fileio.h>
#include <sifrpc.h>
#include <stdio.h>
#include <libpad.h>
#include <audsrv.h>

#include "lib/libcdvd/ee/cdvd_rpc.h"

// InfoGB Emu headers
#include "../system.h"
#include "../joypad.h"
#include "../mem.h"
#include "../cpu.h"
#include "../rom.h"

// PS2 System headers
#include "PS2Init.h"
#include "PS2Input.h"
#include "PS2InfoGB.h"
#include "PS2GUI.h"
#include "PS2Time.h" // for FPS measurement

extern "C" {
   #include "PS2Print.h"

   #include "hw.h"
   #include "gs.h"
   #include "gfxpipe.h"
}

#include "common.h"
#include "../sound.h"

// for video
static const unsigned long infogb_window_width = 160;
static const unsigned long infogb_window_height = 144;

unsigned short g_RGBTab[256] __attribute__((aligned(16))) __attribute__((section(".bss")));
unsigned short g_BitmapData[256*256] __attribute__((aligned(16))) __attribute__((section(".bss")));

int g_Stretch = 1;
int g_Filter = 1;
int g_ShowFPS = 0;

// Sound
int g_SndSample = 0;
int g_SndSampler = 44100;
int g_SamplePos = 0;
short int *g_SndBuff;
struct audsrv_fmt_t format;
bool g_SndUpdate = false;

// Other
int whichdrawbuf = 0;
int endflag = 0;
int CNF_edited = false;

char g_FilePath[MAX_FILENAME_CHARS + MAX_FILEPATH_CHARS];

Browser g_PS2Browser;

/////////////////////////////////////////////////////////////
extern PS2Time timer;

int PS2_FPSMeasure() {
   static int LastTime = timer.getTime(0);
   static int LastFps = 0;
   static int CurrentFps = 0;

   int CurrentTime = timer.getTime(0);

   CurrentFps++;

   if ( CurrentTime - LastTime > 0 ) {
      LastTime = CurrentTime;
      LastFps = CurrentFps;
      CurrentFps = 0;
   }

   return LastFps;
}

/////////////////////////////////////////////////////////////
int infogb_load_rom(char *filepath) {
   char *name = strrchr(filepath, '/');

   if ( !name )
      return 0;

   name++;

   unsigned int length = ( g_PS2Browser.getCurrentSaveDevice() == DEV_USBMASS ) ?
      MAX_FILENAME_CHARS : 31;

   if ( strlen(name) > length )
      return -4;

   return load_rom(filepath);
}

/////////////////////////////////////////////////////////////
void infogb_open_audio();
void infogb_close_audio();

/////////////////////////////////////////////////////////////
int infogb_init(char *display) {
   initialize_memory();
   initialize_rom();

   return 1;
}

/////////////////////////////////////////////////////////////
int infogb_close() {
   free_rom();
   free_memory();

   return 1;
}

/////////////////////////////////////////////////////////////
inline void UpdateDrawing() {
   // Update drawing and display enviroments.
   gp_hardflush(&thegp);
   WaitForNextVRstart(1);
   GS_SetCrtFB(whichdrawbuf);
   whichdrawbuf ^= 1;
   GS_SetDrawFB(whichdrawbuf);
}

/////////////////////////////////////////////////////////////
//                         MAIN                            //
/////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	PS2_Init();

   audsrv_init();

	format.bits = 16;
	format.freq = 44100;
	format.channels = 2;
   audsrv_set_format(&format);

	guiFadeOut(4);
   DisplayIntroBox(APP_NAME);
	guiFadeIn(4);
	guiFadeOut(4);

	for ( int i = 0; i < 160*144; i++ )
      g_BitmapData[i] = 0;

   PS2_LoadButtonsConfig(ButtonsConfigPath);
   Load_CNF(ConfigPath);

   if ( SoundEnabled )
      audsrv_set_volume(MAX_VOLUME);
   else
      audsrv_set_volume(MIN_VOLUME);

   // Main Loop
   while ( true ) {
      g_SamplePos = 0;

      dwKeyPad1 = 0;
      dwKeyPad2 = 0;

      if ( !auto_ROM_flag ) {
         MenuBrowser();
      } else {
         // KarasQ: Here preselected ROM path is already
         // copied to g_FilePath (look function Load_CNF)
         auto_ROM_flag = 0;
      }

      int res = infogb_load_rom(g_FilePath);

      // KarasQ: display error and return to
      // browser if ROM is not loaded successfully
      if ( res < 1 ) {
         infogb_close();
      }

      switch ( res ) {
         case 0:
            DisplayMessageBox
            (
               "\nERROR!\n\n"
               "Could not load ROM\n"
               "Failed opening file\n"
               "File not found!",
               MSG_VOID
            );
            continue;
         case -1:
            DisplayMessageBox
            (
               "\nWARNING!\n\n"
               "Could not load ROM\n"
               "Unknown ROM type!",
               MSG_VOID
            );
            continue;
         case -2:
            DisplayMessageBox
            (
               "\nWARNING!\n\n"
               "Could not load ROM\n"
               "Unknown ROM size!",
               MSG_VOID
            );
            continue;
         case -3:
            DisplayMessageBox
            (
               "\nWARNING!\n\n"
               "Could not load ROM\n"
               "Unknown RAM size!",
               MSG_VOID
            );
            continue;
         case -4:
            DisplayMessageBox
            (
               "\nWARNING!\n\n"
               "File name of this ROM is longer\n"
               "than 32 characters! It's better\n"
               "to change it before playing!",
               MSG_VOID
            );
            continue;
         case -5:
            DisplayMessageBox
            (
               "\nERROR!\n\n"
               "Could not extract file archive\n",
               MSG_VOID
            );
            continue;
      }
      CDVD_Stop();

      infogb_init();

      gameboy_cpu_hardreset();
      gameboy_cpu_run();

      audsrv_stop_audio();

      infogb_close();
   }

	return 0;
}

/////////////////////////////////////////////////////////////
int infogb_poll_events()
{
   PS2_UpdateInput();
   current_joypad = dwKeyPad1;

   if ( endflag ) {
      gbz80.running = 0;
      endflag = 0;
   }

   return 1;
}

/////////////////////////////////////////////////////////////
void infogb_set_color(int x, unsigned short c) {
	g_RGBTab[x & 0xff] = c;
}

/////////////////////////////////////////////////////////////
void infogb_plot_line(int y, int *index)
{
	for ( unsigned int x = 0; x < infogb_window_width; x++ ) {
      g_BitmapData[y * infogb_window_width + x] = g_RGBTab[index[x] & 0x00FF];
	}
}

/////////////////////////////////////////////////////////////
void infogb_vram_blit()
{
   gp_uploadTexture(&thegp, NES_TEX, 256, 0, 0, 0x02, &g_BitmapData, 160, 144);
   gp_setTex(&thegp, NES_TEX, 256, 8, GS256, 0x02, 0, 0, 0, g_Filter);

   if ( g_Stretch == 2 ) {
      gp_texrect(&thegp, 0, 16, 0, 0, 320, (256-16), 160, 144, 2,
         GS_SET_RGBA(0x00, 0x00, 0x00, 0x80));
   } else if ( g_Stretch == 1 ) {
      gp_texrect(&thegp, 40, 20, 0, 0, (240+40), (216+20), 160, 144, 2,
         GS_SET_RGBA(0x00, 0x00, 0x00, 0x80));
   } else {
      gp_texrect(&thegp, 80, 56, 0, 0, (160+80), (144+56), 160, 144, 2,
         GS_SET_RGBA(0x00, 0x00, 0x00, 0x80));
   }

   if ( g_ShowFPS ) {
      char strInfo[32];
      sprintf(strInfo, "FPS Rate: %i", PS2_FPSMeasure());
      TextOut(15, 30, strInfo, 3);
   }

   UpdateDrawing();
}

/////////////////////////////////////////////////////////////
void infogb_write_sample(short int l, short int r)
{
   if ( g_SamplePos+2 >= g_SndSampler << 2 ) {
      //printf("Sound buffer overloaded! (%ib)\n", g_SamplePos);
      return;
      //g_SamplePos = 0;
   }

   g_SndBuff[g_SamplePos++] = l;
   g_SndBuff[g_SamplePos++] = r;
}
