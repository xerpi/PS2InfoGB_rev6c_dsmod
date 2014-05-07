/*
 *  Copyright (C) 2010  Krystian Karaœ <k4rasq@gmail.com>
 *
 *  I release it for fully free use by anyone, anywhere, for any purpose.
 *  But I still retain the copyright, so no one else can limit this release.
 */

#include <tamtypes.h>
#include <libpad.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <audsrv.h>

#include "../cpu.h"
#include "../sound.h"
#include "../system.h"

#include "PS2GUI.h"
#include "PS2Init.h"
#include "PS2Input.h"
#include "PS2InfoGB.h"

extern "C" {
   #include "PS2Print.h"

   #include "hw.h"
   #include "gs.h"
   #include "gfxpipe.h"

   void RunLoaderElf(char *filename, char *party);
}

#include "common.h"

////////////////////////////////////////////////////////////////
static char boot_ELF[1024]    = "mc0:/BOOT/BOOT.ELF"; // default ELF path
static char auto_ROM[1024]    = ""; // ROM pathname for auto booting
static int SaveDevice = 0;          // Number of current device where ROM saves will be
                                    // stored. Values are mapped to PS2Device enum

const char *ConfigPath        = "mc0:/PS2GB/INFOGB.CNF"; // default CNF Path
const char *ButtonsConfigPath = "mc0:/PS2GB/BUTTONS.DAT";

int auto_ROM_flag = 0;              // 0 disables auto boot of a preselect ROM

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
inline void RunELF(char *ElfPath) {
   RunLoaderElf(ElfPath, "");
}

/////////////////////////////////////////////////////////////
// y2 is insignificant if Centred = false
void PrintMessage(int x, int y1, int y2, bool Centered, int Z, const char *StrMsg) {
   int new_line = 0;
   char temp[256];
   char *p = 0, *n = 0;

   strncpy(temp, StrMsg, 256);
   p = (char *)temp;

   // looking for new lines
   if ( Centered ) {
      do {
         n = strchr(p, '\n');
         if ( n ) {
            *n = 0;
         }
         TextOutC2(x, y1, y2 + (15*new_line++), p, Z);
         p = n+1;
      } while ( n );
   } else {
      do {
         n = strchr(p, '\n');
         if ( n ) {
            *n = 0;
         }
         TextOut(x, y1 + (15*new_line++), p, Z);
         p = n+1;
      } while ( n );
   }
}

/////////////////////////////////////////////////////////////
bool DisplayMessageBox(const char *StrMsg, int Type)
{
   gp_frect(&thegp, 0, 0, SCREEN_W, SCREEN_H, 1, GS_SET_RGBA(0, 0, 0, 0x80));
   gp_gouradrect(&thegp, 50, 75, GS_SET_RGBA(0x00, 0x00, 0x40, 0x80),
      (SCREEN_W-50), 185, GS_SET_RGBA(0x40,0x40, 0x80, 0x80), 3);

   PrintMessage(0, 320, 75, true, 5, StrMsg);

   if ( Type == MSG_VOID ) {
      TextOutC2(0, 320, 200, " - Push Start - ", 5);
   }

   UpdateDrawing();

   while ( true ) {
      if ( Type != MSG_FATAL ) {
         int Action = PS2_UpdateInputMenu();

         if ( Action == PAD_START ) { return true; }
         if ( Type == MSG_ACCEPT_ABORT ) {
            if ( Action == PAD_CROSS ) { return false; }
         }
      }
   }
}

/////////////////////////////////////////////////////////////
void DisplayIntroBox(const char *StrMsg)
{
   static struct padButtonStatus lpad1;
   static int lpad1_data = 0;
   int cmpt = 0;

   while ( true ) {
      if ( cmpt++ > 80 ) {
         cmpt = 0;
      }

      gp_frect(&thegp, 0, 0, 320, 256, 1, GS_SET_RGBA(0, 0, 0, 0x80));
      gp_gouradrect(&thegp, (31), 70,GS_SET_RGBA(0x00, 0x00, 0x40, 0x80),
         (320-31), (142+64),  GS_SET_RGBA(0x40,0x40, 0x80, 0x80), 3);

      textCpixel(0, 320, 80, GS_SET_RGBA(255, 255, 255, 255), 0, 0, 4, StrMsg);
      textCpixel(0, 320, 80+18+8, GS_SET_RGBA(255, 255, 255, 255), 0, 0, 4,
         "InfoGB by 'dlanor' & 'KarasQ' rev.6c");
      textCpixel(0, 320, 80+36+18, GS_SET_RGBA(255, 255, 255, 255), 0, 0, 4,
         "based on InfoGB by Jay'Factory");
      textCpixel(0, 320, 80+72, GS_SET_RGBA(255, 255, 255, 255), 0, 0, 4,
         " USB MASS SUPPORT Version");

      if ( cmpt < 31 ) {
         textCpixel(0, (320), 224, GS_SET_RGBA(255, 255, 255, 255),
            0, 0, 4," - Push Start - ");
      } else if ( cmpt < 64 ) {
         textCpixel(0,(320),224,GS_SET_RGBA(255, 255, 255,63 - cmpt),
            0, 0, 4," - Push Start - ");
      }

      textCpixel(0, 320, 190, GS_SET_RGBA(255, 255,255, 255), 0, 0, 4, " 7not6");

		UpdateDrawing();

      if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
         padRead(0, 0, &lpad1);
         lpad1_data = 0xffff ^ lpad1.btns;
      }

      if ( lpad1_data & PAD_START ) {
         break;
      }
   }
}
/////////////////////////////////////////////////////////////
void ConfigButtons() {
   int Action = 0, i;
   int Mask = 0;

   char *msg[] = {
      "turbo button A",
      "turbo button B",
      "turbo button Select",
      "turbo button Start",
      "gameboy button A",
      "gameboy button B",
      "gameboy button Select",
      "gameboy button Start",
      "in-game menu",
      0
   };

   if ( g_BntCnf.turbo_on ) {
      i = 0;
   } else {
      i = 4;
   }

   while ( msg[i] ) {
      gp_frect(&thegp, 0,0, SCREEN_W, SCREEN_H, 0, GS_SET_RGBA(0, 0, 0, 0x80));

      TextOutC2(0, 320, 115, "Choose button for", Z_BOX2);
      TextOutC2(0, 320, 130, msg[i], Z_BOX2);

      gp_frect(&thegp, 80, 80, SCREEN_W-80, SCREEN_H-80, Z_BOX1,
         GS_SET_RGBA(0x40,0x40, 0x80, 0x60));

      Action = PS2_UpdateInputMenu();

      UpdateDrawing();

      // Block DPAD buttons
      if ( Action & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT) )
         continue;

      // Assignt buttons
      if ( (Action & ~Mask) ) {
         switch ( i++ ) {
            case 0: g_BntCnf.gb_Ta = Action; break;
            case 1: g_BntCnf.gb_Tb = Action; break;
            case 2: g_BntCnf.gb_Tselect = Action; break;
            case 3: g_BntCnf.gb_Tstart = Action; break;
            case 4: g_BntCnf.gb_a = Action; break;
            case 5: g_BntCnf.gb_b = Action; break;
            case 6: g_BntCnf.gb_select = Action; break;
            case 7: g_BntCnf.gb_start = Action; break;
            case 8: g_BntCnf.browser_start = Action; break;
         }
         Mask |= Action;
      }
   }
}

/////////////////////////////////////////////////////////////
void ScreenAdjustment(bool InGame) {
   int Action = 0;
   do
   {
      if ( InGame ) {
         if ( g_Stretch == 2 )  {
            gp_texrect(&thegp, 0, 16, 0, 0, (320), (256-16), 160, 144, Z_TEXTURE,
               GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0xC8));
         } else if ( g_Stretch == 1 ) {
            gp_texrect(&thegp, 40, 20, 0, 0, (240+40), (216+20), 160, 144, Z_TEXTURE,
               GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0xC8));
         } else {
            gp_texrect(&thegp, 80, 56, 0, 0, (160+80), (144+56), 160, 144, Z_TEXTURE,
               GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0xC8));
         }
      }

      gp_frect(&thegp, 0, 0, SCREEN_W, SCREEN_H, Z_TEXTURE, GS_SET_RGBA(0, 0, 0, 0x40));

      textCpixel(0, 320, 118, GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0x80), 0, 0,
         Z_BOX2, "X: %i, Y: %i", g_DispX, g_DispY);
      textCpixel(0, 320, 128, GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0x80), 0, 0,
         Z_BOX2, "Press X if done!");

      gp_frect(&thegp, 100, 100, SCREEN_W-100, SCREEN_H-100, Z_BOX1,
         GS_SET_RGBA(0x40,0x40, 0x80, 0x60));
      gp_frect(&thegp, 0, 20, 20, SCREEN_H-20, Z_BOX1,
         GS_SET_RGBA(0x40,0x40, 0x80, 0x60));
      gp_frect(&thegp, SCREEN_W-20, 20, SCREEN_W, SCREEN_H-20, Z_BOX1,
         GS_SET_RGBA(0x40,0x40, 0x80, 0x60));
      gp_frect(&thegp, 20, SCREEN_H-20, SCREEN_W-20, SCREEN_H, Z_BOX1,
         GS_SET_RGBA(0x40,0x40, 0x80, 0x60));
      gp_frect(&thegp, 20, 0, SCREEN_W-20, 20, Z_BOX1,
         GS_SET_RGBA(0x40,0x40, 0x80, 0x60));

      Action = PS2_UpdateInputMenu();

      switch ( Action ) {
         case PAD_UP:
            if ( g_DispY > 0 ) {
               g_DispY--;
            }
            break;
         case PAD_LEFT:
            if ( g_DispX > 0 ) {
               g_DispX--;
            }
            break;
         case PAD_DOWN: g_DispY++; break;
         case PAD_RIGHT: g_DispX++; break;
      }

      if ( Action ) {
         GS_SetDispMode(g_DispX, g_DispY, SCREEN_W, SCREEN_H);
      }

      UpdateDrawing();
   } while ( Action != PAD_CROSS );
}

/////////////////////////////////////////////////////////////
#define MENU_OPRIONS_ENTRIES  7

void MenuOptions() {
   int Action = 0, Entry = 0;
   char *DeviceName[] = {
      "MC0",
      "MC1",
      "MASS"
   };

   SaveDevice = (int)g_PS2Browser.getCurrentSaveDevice();

   do
   {
      // Clear screen
      gp_frect(&thegp, 0,0, SCREEN_W, SCREEN_H, 0, GS_SET_RGBA(0, 0, 0, 0x80));

      // Draw Menu Box
		gp_gouradrect(&thegp, 32, 70, GS_SET_RGBA(0x00, 0x00, 0x40, 0x80),
         (SCREEN_W-32), (SCREEN_H-18),  GS_SET_RGBA(0x40,0x40, 0x80, 0x80), Z_BOX1);

      // Draw Separate line and Print Path
      gp_frect(&thegp, 36, 220, (SCREEN_W-36), 221, Z_SCROLL,
         GS_SET_RGBA(96, 142, 159, 0x80));

      // Draw Highlighted Bar
      gp_frect(&thegp, 160, 70+(Entry*15), (320-32), 70+(Entry*15)+15, Z_SELECT,
         GS_SET_RGBA(0x20, 0x51, 0x7C, 0x80));

      // Print Text
      textpixel(32+5, 70+4, GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0x80),
         0, 0, Z_SCROLL, "Current Save Device:  %s (%sPS2GB/)",
         DeviceName[SaveDevice], DeviceID[SaveDevice]);

      if ( strlen(auto_ROM) > 0 ) {
         TextOut(32+5, 70+15, "Preselected ROM:      Turn off", Z_SCROLL);
      } else {
         TextOut(32+5, 70+15, "Preselected ROM:      Not defined", Z_SCROLL);
      }

      TextOut(32+5, 70+30, "Screen Adjustment:    Open", Z_SCROLL);
      TextOut(32+5, 70+45, "Button Configuration: Open", Z_SCROLL);

      if ( g_BntCnf.turbo_on ) {
         TextOut(32+5, 70+60, "Turbo Buttons:        On", Z_SCROLL);
      } else {
         TextOut(32+5, 70+60, "Turbo Buttons:        Off", Z_SCROLL);
      }

      if ( g_ShowFPS ) {
         TextOut(32+5, 70+75, "Show FPS rate:        On", Z_SCROLL);
      } else {
         TextOut(32+5, 70+75, "Show FPS rate:        Off", Z_SCROLL);
      }

      TextOut(32+5, 70+90, "Boot ELF:             Run", Z_SCROLL);

      textpixel(32+5, 221+4, GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0x80),
         0, 0, Z_SCROLL, "ELF Path: %s", boot_ELF);

      // PS2 Input
      Action = PS2_UpdateInputMenu();

      switch ( Action ) {
         case PAD_UP:
            if ( --Entry < 0 ) {
               Entry = MENU_OPRIONS_ENTRIES-1;
            }
            break;
         case PAD_DOWN:
            if ( ++Entry > MENU_OPRIONS_ENTRIES-1 ) {
               Entry = 0;
            }
            break;
         case PAD_CROSS:
            switch ( Entry ) {
               case 0:
                  if ( ++SaveDevice >= (int) DEV_CDVD ) {
                     SaveDevice = 0;
                  }
                  break;
               case 1:
                  strcpy(auto_ROM, "");
                  break;
               case 2:
                  ScreenAdjustment(false);
                  break;
               case 4:
                  g_BntCnf.turbo_on ^= 1;

                  if ( !g_BntCnf.turbo_on ) { // button config will run if ON
                     PS2_SaveButtonsConfig(ButtonsConfigPath);
                     break;
                  }
               case 3:
                  ConfigButtons();
                  PS2_SaveButtonsConfig(ButtonsConfigPath);
                  break;
               case 5:
                  g_ShowFPS ^= 1;
                  break;
               case 6:
                  Save_CNF(ConfigPath);
                  RunELF(boot_ELF);
                  break;
            }
            break;
      }

      UpdateDrawing();
   } while ( Action != PAD_START && Action != PAD_TRIANGLE );

   if ( !g_PS2Browser.setCurrentSaveDevice((PS2Device) SaveDevice) ) {
      char msg[128];

      sprintf(msg,
         "\nWarning!\n\n"
         "Could not create \"PS2GB\"\n"
         "folder on device: %s\n"
         "Set to previous device",
         DeviceName[SaveDevice]
      );
      DisplayMessageBox(msg, MSG_VOID);

      SaveDevice = (int)g_PS2Browser.getCurrentSaveDevice();
   }
   Save_CNF(ConfigPath);
}

/////////////////////////////////////////////////////////////
/* MENU_ENTRIES - number of current menu entries,
 * if you are going to add new option or remove an option,
 * remember to set number of MENU_ENTRIES!
 * Don't bother about current X, Y position, POSY(element)
 * will take care about it, just follow the existing code.
 *
 * If current resolution is set to 320x256, MENU_ENTRIES
 * should be less than 18, otherwise it will draw outside
 * the screen!
 */

#define MENU_ENTRIES 10

// Calculate Y pos
#define CALC_OFFSET_Y1(NumEntries, ScrnH) ((ScrnH-(NumEntries*15))/2)
#define CALC_OFFSET_Y2(NumEntries, ScrnH) (ScrnH-OFFSET_Y1(NumEntries, ScrnH))

#define OFFSET_Y1 (CALC_OFFSET_Y1(MENU_ENTRIES, SCREEN_H))
#define OFFSET_Y2 (SCREEN_H - OFFSET_Y1)

#define POSY(element) (((element-1)*15)+OFFSET_Y1)

void MenuIngame()
{
   int Action = 0, Entry = 1, EndMenu = 0;
   char temp[128];

   if ( SoundEnabled ) {
      audsrv_stop_audio();
   }

   do {
      // Draw Texture
      if ( g_Stretch == 2 )  {
         gp_texrect(&thegp, 0, 16, 0, 0, (320), (256-16), 160, 144, Z_TEXTURE,
            GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0xC8));
      } else if ( g_Stretch == 1 ) {
         gp_texrect(&thegp, 40, 20, 0, 0, (240+40), (216+20), 160, 144, Z_TEXTURE,
            GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0xC8));
		} else {
         gp_texrect(&thegp, 80, 56, 0, 0, (160+80), (144+56), 160, 144, Z_TEXTURE,
            GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0xC8));
      }

      // Shade emu display
      gp_frect(&thegp, 0, 0, SCREEN_W, SCREEN_H, Z_TEXTURE, GS_SET_RGBA(0, 0, 0, 0x40));

      gp_gouradrect(&thegp, 80, OFFSET_Y1, GS_SET_RGBA(0x00, 0x00, 0x40, 0x80),
			(SCREEN_W-80), OFFSET_Y2, GS_SET_RGBA(0x40,0x40, 0x80, 0x40), Z_BOX2);

      gp_frect(&thegp, 80, OFFSET_Y1+((Entry-1)*15), SCREEN_W-80, OFFSET_Y1+((Entry-1)*15)+15,
            Z_SELECT, GS_SET_RGBA(123, 255, 255, 40));

      gp_linerect(&thegp, 80-1, OFFSET_Y1-1, (SCREEN_W-80), OFFSET_Y2, Z_SELECT,
            GS_SET_RGBA(0xFF, 0xFF, 0xFF, 0x80));


      // Draw entries
      TextOutC2(0, 320, POSY(1), "Continue Game", Z_SCROLL);

      sprintf(temp, "Save State to %s", DeviceID[SaveDevice]);
      TextOutC2(0, 320, POSY(2), temp, Z_SCROLL);

      sprintf(temp, "Load State of %s", DeviceID[SaveDevice]);
      TextOutC2(0, 320, POSY(3), temp, Z_SCROLL);

      TextOutC2(0, 320, POSY(4), "Reset Gameboy", Z_SCROLL);
      //TextOutC2(0, 320, POSY(5), "Save CNF", Z_SCROLL);
      //TextOutC2(0, 320, POSY(6), "Load CNF", Z_SCROLL);

      // Sound
      if ( SoundEnabled ) {
         TextOutC2(0, 320, POSY(5), "Sound: On", Z_SCROLL);
      } else {
         TextOutC2(0, 320, POSY(5), "Sound: Off", Z_SCROLL);
      }

      // Stretch
      if ( g_Stretch == 1 ) {
         TextOutC2(0, 320, POSY(6), "Stretch: X 1.5", Z_SCROLL);
      } else if ( g_Stretch == 2 ) {
         TextOutC2(0, 320, POSY(6), "Stretch: X 2", Z_SCROLL);
      } else {
         TextOutC2(0, 320, POSY(6), "Stretch: Off", Z_SCROLL);
      }

      // Filter On/Off
      if ( g_Filter ) {
			TextOutC2(0, 320, POSY(7), "Filter: On", Z_SCROLL);
      } else {
         TextOutC2(0, 320, POSY(7), "Filter: Off", Z_SCROLL);
      }

      if ( strlen(boot_ELF) == 0 ) {
         sprintf(temp, "No system ELF set by CNF");
      } else if ( strlen(boot_ELF) < 22 ) {
         sprintf(temp, "Run %s", boot_ELF);
      } else {
         strcpy(temp, "Run system ELF set by CNF");
      }

      if ( strcmp(g_FilePath, auto_ROM) ) {
         TextOutC2(0, 320, POSY(8), "Preselect this ROM", Z_SCROLL);
      } else {
         TextOutC2(0, 320, POSY(8), "This ROM is preselected", Z_SCROLL);
      }

      TextOutC2(0, 320, POSY(9), temp, Z_SCROLL);
      TextOutC2(0, 320, POSY(10), "Back to the main menu", Z_SCROLL);

      UpdateDrawing();

      // Get pressed button
		Action = PS2_UpdateInputMenu();

		// Buttons controller
      switch ( Action ) {
         case PAD_UP:
            if ( --Entry < 1 ) {
               Entry = MENU_ENTRIES;
            }
            break;
         case PAD_DOWN:
            if ( ++Entry > MENU_ENTRIES ) {
               Entry = 1;
            }
            break;
         case PAD_CROSS:
            switch ( Entry ) {
               case 1: EndMenu = 1; break;
               case 2: gameboy_save_state(); break;
               case 3: gameboy_read_state(); EndMenu = 1; break;
               case 4:
                  gameboy_cpu_hardreset();
                  gameboy_cpu_run();
                  EndMenu = 1;
                  break;
               case 5:
                  SoundEnabled ^= 1;
                  CNF_edited |= 1;

                  if ( SoundEnabled ) {
                     audsrv_set_volume(MAX_VOLUME);
                     audsrv_stop_audio();
                  } else
                     audsrv_set_volume(MIN_VOLUME);

                  break;
               case 6:
                  if ( ++g_Stretch > 2) {
                     g_Stretch = 0;
                  }
                  CNF_edited |= 1;
                  break;
               case 7: g_Filter ^= 1; CNF_edited |= 1; break;
               case 8:
                  if ( strcmp(g_FilePath, auto_ROM) ) {
                     strcpy(auto_ROM, g_FilePath);
                     CNF_edited |= 1;
                  }
                  break;
               case 9:
                  if ( strlen(boot_ELF) ) {
                     infogb_close();
                     Save_CNF(ConfigPath);
                     RunELF(boot_ELF);
                  }
                  break;
               case 10: endflag = 1; return;
            }

            break;
         case PAD_START:
            EndMenu = 1;
            break;
         case PAD_SELECT:
            ScreenAdjustment(true);
            break;
      }
   } while ( EndMenu != 1 );

   if ( CNF_edited ) {
      Save_CNF(ConfigPath);
      CNF_edited = false;
   }
}

/////////////////////////////////////////////////////////////
void MenuBrowser() {
   int Action = 0;

   int numEntryA, numEntryB, numSum;
   static int Entry = 0, OffsetY = 0;

   int DrawY = 0, DrawX = (int)g_PS2Browser.getCurrentDevice();

   PS2Device Current = DEV_MC0;

   g_PS2Browser.getCurrentDeviceContent();

   while ( true ) {
      numEntryA = g_PS2Browser.getCurrentDirsNum();
      numEntryB = g_PS2Browser.getCurrentFilesNum();

      numSum = numEntryA + numEntryB;

	   // Get action
      Action = PS2_UpdateInputMenu();

	   // Handle Pad State
      switch ( Action ) {
         // Change Device
         case PAD_LEFT:
            Current = g_PS2Browser.getCurrentDevice();

            if ( Current == DEV_USBMASS ) {
               g_PS2Browser.setCurrentDevice(DEV_MC1);
            } else if ( Current == DEV_CDVD) {
               g_PS2Browser.setCurrentDevice(DEV_USBMASS);
            } else if ( Current == DEV_MC0 ) {
               g_PS2Browser.setCurrentDevice(DEV_CDVD);
            } else if ( Current == DEV_MC1 ) {
               g_PS2Browser.setCurrentDevice(DEV_MC0);
            }

            DrawX = (int) g_PS2Browser.getCurrentDevice();
            OffsetY = Entry = 0;
         break;

         case PAD_RIGHT:
            Current = g_PS2Browser.getCurrentDevice();

            if ( Current == DEV_USBMASS ) {
               g_PS2Browser.setCurrentDevice(DEV_CDVD);
            } else if ( Current == DEV_MC0 ) {
               g_PS2Browser.setCurrentDevice(DEV_MC1);
            } else if ( Current == DEV_MC1 ) {
               g_PS2Browser.setCurrentDevice(DEV_USBMASS);
            } else if ( Current == DEV_CDVD) {
               g_PS2Browser.setCurrentDevice(DEV_MC0);
            }

            DrawX = (int) g_PS2Browser.getCurrentDevice();
            OffsetY = Entry = 0;
         break;

         // Scroll list
         case PAD_UP:
            if ( Entry > 0 )
               Entry--;
            if ( Entry < OffsetY )
               OffsetY--;
         break;

         case PAD_DOWN:
            if ( Entry + 1 < numSum )
               Entry++;
            if ( Entry >= OffsetY + SCROLL_BOX_MAX_ENTRY )
               OffsetY++;
         break;

         // Selecr dir/file
         case PAD_CROSS:
            if ( Entry < numEntryA ) {
               // open another dir
               g_PS2Browser.setCurrentFilesPath(Entry);
               OffsetY = Entry = 0;
            } else if ( numEntryB > 0 ) {
               // copy path to file and quit browser
               strcpy(g_FilePath, g_PS2Browser.getCurrentFilesPath());
               strncat(g_FilePath, g_PS2Browser.getEntryFileName(Entry-numEntryA),
                  MAX_FILENAME_CHARS);

               return;
            }
         break;

         // Return to upper dir
         case PAD_TRIANGLE:
            g_PS2Browser.setToUpperDir();
            OffsetY = Entry = 0;
         break;

         // Open menu options
         case PAD_START:
            MenuOptions();
         break;

         default: break;
      }

		DrawY = 70;

      // Clear screen
      gp_frect(&thegp, 0,0, SCREEN_W, SCREEN_H, 0,
         GS_SET_RGBA(0, 0, 0, 0x80));

		// Draw Menu Bar
      gp_frect(&thegp, 32, 55, (32+160), 70, Z_BOX1,
         GS_SET_RGBA(0, 0, 0x40, 0x80));

      // Draw Highlighted Menu Bar
      gp_frect(&thegp, 32+(DrawX*40), 55, 32+((DrawX+1)*40), 70, Z_SELECT,
        GS_SET_RGBA(0, 0x2D, 0x55, 0x80));

      // Print Menu Bar text
      TextOutC2(32, 32+105, 55, "MC0", Z_SCROLL);
      TextOutC2(32, 32+185, 55, "MC1", Z_SCROLL);
      TextOutC2(32, 32+265, 55, "MASS", Z_SCROLL);
      TextOutC2(32, 32+345, 55, "CDFS", Z_SCROLL);

      // Draw Separate line and Print Path
      gp_frect(&thegp, 36, 220, (SCREEN_W-36), 221, Z_SCROLL,
         GS_SET_RGBA(96, 142, 159, 0x80));

      TextOut(32+5, 221, g_PS2Browser.getCurrentFilesPath(), Z_SCROLL);

		// Draw Scroll Box
		gp_gouradrect(&thegp, 32, 70, GS_SET_RGBA(0x00, 0x00, 0x40, 0x80),
         (SCREEN_W-32), (SCREEN_H-18),  GS_SET_RGBA(0x40,0x40, 0x80, 0x80), Z_BOX1);

      // Draw text in scroll box
      for ( int i = OffsetY; i < OffsetY + SCROLL_BOX_MAX_ENTRY && i < numSum; i++, DrawY += 15) {
         // Draw highlighted bar
         if ( i == Entry ) {
            gp_frect(&thegp, 32, DrawY, (320-32), DrawY+15, Z_SELECT,
               GS_SET_RGBA(0x20, 0x51, 0x7C, 0x80));
         }

         // Print entries
         if ( i < numEntryA )
            TextOutC2(0, 320, DrawY, g_PS2Browser.getEntryDirName(i), Z_SCROLL); // List dirs
         else
            TextOutC2(0, 320, DrawY, g_PS2Browser.getEntryFileName(i-numEntryA), Z_SCROLL); // List files
      }

      UpdateDrawing();
   }
}
/////////////////////////////////////////////////////////////
void guiFadeOut(int pa)
{
   int color = 0;

   while ( color < 0xFF )
   {
      gp_frect(&thegp, 0, 0, SCREEN_W, SCREEN_H, 5,
         GS_SET_RGBA(color, color, color, 0x80));
      UpdateDrawing();

      color +=pa;
	}
}

/////////////////////////////////////////////////////////////
void guiFadeIn(int pa)
{
   int color = 0xFF;

   while ( color > 0 )
   {
      gp_frect(&thegp, 0, 0, SCREEN_W, SCREEN_H, 5,
         GS_SET_RGBA(color, color, color, 0x80));
      UpdateDrawing();

      color -=pa;
   }
}

//---------------------------------------------------------------------------
// The following code is copyrighted by me, (c)2006 Ronald Andersson.
// I release it for fully free use by anyone, anywhere, for any purpose.
// But I still retain the copyright, so no one else can limit this release.
//---------------------------------------------------------------------------
// get_CNF_string analyzes a config file held in a single string, parsing it
// to find one config variable definition per call. The return value is a
// true/false flag, set true if a variable was found, but set false if not.
// This means it also returns false for an empty file.
//
// The config file is passed as the address of a string pointer, which will
// be set to point beyond the variable definition retrieved, but on failure
// it will remain at the point where failure was detected. Thus it will not
// pass beyond the terminating NUL of the string. The original string data
// will be partly slaughtered by the analysis, as new string terminators are
// inserted at end of each variable name, and also at the end of each value
// string. Those new strings are then passed back to the caller through the
// other arguments, these being pointers to string pointers of the calling
// procedure.
//
// So with a variable definition like this: "SomeVar = some bloody string",
// the results are the string pair: "SomeVar" and "some bloody string".
//
// A name must begin with a letter (here ascii > 0x40), so lines that begin
// with other non-whitespace characters will be considered comment lines by
// this function. (Simply ignored.) Whitespace is permitted in the variable
// definitions, and will be ignored if occurring before the value string,
// but once a value begins any whitespace used is considered a part of the
// value string. It will remain intact in the returned results.
//
// Note that the name part can only contain non-whitespace characters, but
// the value part can contain non-leading whitespace different from CR/LF.
// So a value starts with the first non-whitespace character after the '='
// and ends at the end of the line.
//
// Intended usage is to repeatedly call get_CNF_string to retrieve each of
// the variables in the config file, until the function returns false, which
// signals either the end of the file, or a syntax error. Analysis of the
// variables found, and usage of their values, is not dealt with at all.
//
// Such matters are left entirely up to the calling procedures, Which also
// means that caller may decide to allow comments terminating lines with a
// variable definition. That's just one of the many value analysis choices.

/////////////////////////////////////////////////////////////
int get_CNF_string(unsigned char **CNF_p_p, unsigned char **name_p_p,
                   unsigned char **value_p_p)
{
   unsigned char *np, *vp, *tp = *CNF_p_p;

start_line:

   while ( (*tp<=' ') && (*tp>'\0') ) tp += 1; //Skip leading whitespace, if any
   if ( *tp == '\0' ) return false;            //but exit at EOF
   np = tp;                                    //Current pos is potential name
   if ( *tp < 'A' ) {                          //but may be a comment line                                      //We must skip a comment line
      while ( (*tp!='\r') && (*tp!='\n') && (tp!='\0') ) tp+=1;  //Seek line end
      goto start_line;                     //Go back to try next line
   }

   while((*tp>='A')||((*tp>='0')&&(*tp<='9'))) tp+=1;  //Seek name end
   if(*tp=='\0')	return false;            //but exit at EOF
   *tp++ = '\0';                          //terminate name string (passing)
   while((*tp<=' ') && (*tp>'\0')) tp+=1; //Skip post-name whitespace, if any
   if(*tp!='=') return false;             //exit (syntax error) if '=' missing
   tp += 1;                               //skip '='
   while((*tp<=' ') && (*tp>'\0')) tp+=1; //Skip pre-value whitespace, if any
   if(*tp=='\0')	return false;            //but exit at EOF
   vp = tp;                               //Current pos is potential value

   while((*tp!='\r')&&(*tp!='\n')&&(tp!='\0')) tp+=1;  //Seek line end
   if(*tp!='\0') *tp++ = '\0';            //terminate value (passing if not EOF)
   while((*tp<=' ') && (*tp>'\0')) tp+=1;  //Skip following whitespace, if any

   *CNF_p_p = tp;                         //return new CNF file position
   *name_p_p = np;                        //return found variable name
   *value_p_p = vp;                       //return found variable value
   return true;                           //return control to caller*/
}

/////////////////////////////////////////////////////////////
void Load_CNF(const char *CNF_path_p)
{
   int fd, var_cnt, disp_f = 0;
   unsigned char  *RAM_p, *CNF_p, *name, *value;
   size_t TST_size, CNF_size;

   fd = fioOpen(CNF_path_p,O_RDONLY);

   if ( fd < 0 ) {
      printf("Load_CNF: %s Open failed %d.\r\n", CNF_path_p, fd);
      fioMkdir("mc0:PS2GB");
      return;
   }

   CNF_size = fioLseek(fd, 0, SEEK_END);
   CNF_p = (RAM_p = (unsigned char *)malloc(CNF_size+1));

   fioLseek(fd, 0, SEEK_SET);

   if	( CNF_p == NULL ) {
      printf("Load_CNF failed malloc(%d).\r\n", CNF_size);
      return;
   }

   TST_size = fioRead(fd, CNF_p, CNF_size);
   fioClose(fd);
   CNF_p[CNF_size] = '\0';
   printf("Load_CNF read %d bytes.\r\n",TST_size);

   for ( var_cnt = 0; get_CNF_string(&CNF_p, &name, &value); var_cnt++ ) {
      // A variable was found, now we dispose of its value.
		printf("Found variable \"%s\" with value \"%s\"\r\n", name, value);

      if ( !strcmp((char *)name,"dispx") ) {	g_DispX = atoi((char *)value); disp_f = true; }
      else if(!strcmp((char *)name,"dispy")) {	g_DispY = atoi((char *)value); disp_f = true;	}
      else if(!strcmp((char *)name,"stretch")) g_Stretch = atoi((char *)value);
      else if(!strcmp((char *)name,"filter")) g_Filter = atoi((char *)value);
      else if(!strcmp((char *)name,"showfps")) g_ShowFPS = atoi((char *)value);
      else if(!strcmp((char *)name,"sound")) SoundEnabled = atoi((char *)value);
      else if(!strcmp((char *)name,"save_device")) { SaveDevice = atoi((char *)value); }
      else if(!strcmp((char *)name,"boot_ELF")) strcpy(boot_ELF, (char *)value);
      else if(!strcmp((char *)name,"auto_ROM")) {
         if ( value[0] != '#' ) {
            strcpy(auto_ROM, (char *)value);
         }
      }
   }
	auto_ROM_flag = 0;

	if ( auto_ROM[0] ) {
      char msg[128];
      char *path = auto_ROM, *fname = strrchr(auto_ROM, '/');

      if ( fname ) {
         *fname = 0;

         sprintf(msg,
            "\n"
            "Found preselected ROM\n"
            "Push X button to abort preselected\n"
            "ROM or Start button to continue\n\n"
            "File: %s\n"
            "Path: %s",
            fname+1,
            path
         );

         *fname = '/';

         if ( DisplayMessageBox(msg, MSG_ACCEPT_ABORT) ) {
            strcpy(g_FilePath, auto_ROM);
            auto_ROM_flag = 1;
         }
      }
	}

   if ( !g_PS2Browser.setCurrentSaveDevice((PS2Device) SaveDevice) ) {
      char msg[128];
      char *DeviceName[] = {
         "MC0",
         "MC1",
         "MASS"
      };

      sprintf(msg,
         "\nWarning!\n\n"
         "Could not open save device: %s\n"
         "Set to default device!\n",
         DeviceName[SaveDevice]
      );

      DisplayMessageBox(msg, MSG_VOID);

       // If SaveDevice is saved incorrect in config, set to default value
      SaveDevice = (int) g_PS2Browser.getCurrentSaveDevice();
   }

   if ( strlen((char *)CNF_p) )  // Was there any unprocessed CNF remainder ?
      CNF_edited = false;        // false == current settings match CNF file
   else
      printf("Syntax error in CNF file at position %d.\r\n", (CNF_p-RAM_p));

   free(RAM_p);

   if ( disp_f )
      GS_SetDispMode(g_DispX, g_DispY, NWIDTH, NHEIGHT);
}

/////////////////////////////////////////////////////////////
void Save_CNF(const char *CNF_path_p)
{
   int fd, CNF_error;
   int CNF_size = 4096; //safe preliminary value
   char  *CNF_p;

   CNF_error = true;
   CNF_p = (char *)malloc(CNF_size);
   if	( CNF_p == NULL ) return;

   sprintf(CNF_p,
      "# INFOGB.CNF == Configuration file for the emulator InfoGB\r\n"
      "# --------------------------------------------------------\r\n"
      "dispx    = %d\r\n"
      "dispy    = %d\r\n"
      "stretch  = %d\r\n"
      "filter   = %d\r\n"
      "showfps  = %d\r\n"
      "sound    = %d\r\n"
      "save_device = %d\r\n"
      "boot_ELF = %s\r\n"
      "auto_ROM = %s\r\n"
      "# --------------------------------------------------------\r\n"
      "# End-Of-File for InfoGB.CNF\r\n"
      "%n", // NB: The %n specifier causes NO output, but only a measurement
      g_DispX,
      g_DispY,
      g_Stretch,
      g_Filter,
      g_ShowFPS,
      SoundEnabled,
      SaveDevice,
      boot_ELF,
      auto_ROM,
      &CNF_size
   );

   // Note that the final argument above measures accumulated string size,
   // used for fioWrite below, so it's not one of the config variables.

   fd = fioOpen(CNF_path_p, O_CREAT | O_WRONLY | O_TRUNC);

   if ( fd >= 0 ) {
      if ( (signed) CNF_size == fioWrite(fd, CNF_p, CNF_size))
         CNF_edited = false;
      fioClose(fd);
   }

   free(CNF_p);
}
/////////////////////////////////////////////////////////////
