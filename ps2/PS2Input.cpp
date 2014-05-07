#include <tamtypes.h>
#include <kernel.h>
#include <stdio.h>
#include <libpad.h>

#include "PS2Input.h"
#include "PS2GUI.h"

#include "hw.h"
#include "common.h"

#include <libds3ps2.h>
#include <libds4ps2.h>
#include <string.h>


////////////////////////////////////////////////////////////////
// Gameboy buttons
////////////////////////////////////////////////////////////////
#define INPUT_RIGHT  0x01
#define INPUT_LEFT   0x02
#define INPUT_UP	   0x04
#define INPUT_DOWN	0x08
#define INPUT_B1     0x10
#define INPUT_B2     0x20
#define INPUT_SELECT 0x40
#define INPUT_RUN    0x80

#define TURBO_MASK (1 << 1) // RA NB: 2 scans per transition

////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////
unsigned long dwKeyPad1;
unsigned long dwKeyPad2;
struct SS_GAMEPAD ds3Input;
struct ds4_input ds4Input;

// Buttons config with default buttons
ButtonsConfig g_BntCnf = {
   // Turbo buttons
   PAD_R1,              // gb_Ta
   PAD_L1,              // gb_Tb
   PAD_L2,              // gb_Tselect
   PAD_R2,              // gb_Tstart
   // Normal buttons
   PAD_CIRCLE,          // gb_a
   PAD_CROSS,           // gb_b
   PAD_SQUARE,          // gb_select
   PAD_TRIANGLE,        // gb_start
   PAD_START,           // browser_start
   // Tutbo buttons - 1 = ON, 0 = OFF
   0
};

////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////
static int  actuators;
static char actAlign[6];

static char padBuf1[256] __attribute__((aligned(64))) __attribute__ ((section (".bss")));
static char padBuf2[256] __attribute__((aligned(64))) __attribute__ ((section (".bss")));

////////////////////////////////////////////////////////////////
bool PS2_SaveButtonsConfig(const char *path) {
   int fd = fioOpen(path, O_WRONLY | O_CREAT | O_BINARY);

   if ( fd < 0 ) {
      return false;
   }

   fioWrite(fd, &g_BntCnf, sizeof(ButtonsConfig));
   fioClose(fd);

   return true;
}

////////////////////////////////////////////////////////////////
bool PS2_LoadButtonsConfig(const char *path) {
   int fd = fioOpen(path, O_RDONLY | O_BINARY);

   if ( fd < 0 ) {
      return false;
   }

   fioRead(fd, &g_BntCnf, sizeof(ButtonsConfig));
   fioClose(fd);

   return true;
}

////////////////////////////////////////////////////////////////
int PS2_WaitPadReady(int port, int slot)
{
   int state, lastState;
   char stateString[16];

   state = padGetState(port, slot);
   lastState = -1;

   padStateInt2String(state, stateString);
   printf("0)Please wait, pad(%d,%d) is in state %s\n", port, slot, stateString);

   WaitForNextVRstart(1);

   while ( (state != PAD_STATE_DISCONN) && (state != PAD_STATE_STABLE) &&
           (state != PAD_STATE_FINDCTP1) )
   {
      if ( state != lastState ) {
         padStateInt2String(state, stateString);
         printf("Please wait, pad(%d,%d) is in state %s\n", port, slot, stateString);
      }

      lastState = state;
      state = padGetState(port, slot);
   }

   // Were the pad ever 'out of sync'?
   if ( lastState != -1 ) {
      printf("1)Pad OK!\n");
   }
   return 0;
}

////////////////////////////////////////////////////////////////
int PS2_InitializePad(int port, int slot)
{
   int ret, modes, i = 0;

   PS2_WaitPadReady(port, slot);

   // How many different modes can this device operate in?
   // i.e. get # entrys in the modetable
   modes = padInfoMode(port, slot, PAD_MODETABLE, -1);
   printf("The device has %d modes\n", modes);

   if ( modes > 0 ) {
      printf("( ");
      for (i = 0; i < modes; i++) {
         printf("%d ", padInfoMode(port, slot, PAD_MODETABLE, i));
      }
      printf(")");
   }

   printf("It is currently using mode %d\n", padInfoMode(port, slot, PAD_MODECURID, 0));

   // If modes == 0, this is not a Dual shock controller
   // (it has no actuator engines)
   if ( modes == 0 ) {
      printf("This is a digital controller?\n");
      return 1;
   }

   // Verify that the controller has a DUAL SHOCK mode
   do {
      if ( padInfoMode(port, slot, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK )
         break;

      i++;
   } while ( i < modes );

   if ( i >= modes ) {
      printf("This is no Dual Shock controller\n");
      return 1;
   }

   // If ExId != 0x0 => This controller has actuator engines
   // This check should always pass if the Dual Shock test above passed
   ret = padInfoMode(port, slot, PAD_MODECUREXID, 0);

   if ( ret == 0 ) {
      printf("This is no Dual Shock controller??\n");
      return 1;
   }

   printf("Enabling dual shock functions\n");

   // When using MMODE_LOCK, user cant change mode with Select button
   padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);

   PS2_WaitPadReady(port, slot);
   printf("infoPressMode: %d\n", padInfoPressMode(port, slot));

   PS2_WaitPadReady(port, slot);
   printf("enterPressMode: %d\n", padEnterPressMode(port, slot));

   PS2_WaitPadReady(port, slot);
   actuators = padInfoAct(port, slot, -1, 0);
   printf("# of actuators: %d\n",actuators);

   if ( actuators != 0 ) {
      actAlign[0] = 0;   // Enable small engine
      actAlign[1] = 1;   // Enable big engine
      actAlign[2] = 0xff;
      actAlign[3] = 0xff;
      actAlign[4] = 0xff;
      actAlign[5] = 0xff;

      PS2_WaitPadReady(port, slot);
      printf("padSetActAlign: %d\n", padSetActAlign(port, slot, actAlign));
   } else {
      printf("Did not find any actuators.\n");
   }

   PS2_WaitPadReady(port, slot);
   return 1;
}

////////////////////////////////////////////////////////////////
int PS2_SetupPad()
{
   ds3ps2_init();
   ds4ps2_init();
    
   padInit(0);

   // hard way to get psxpad working !

   printf("PortMax: %d\n", padGetPortMax());
   printf("SlotMax: %d\n", padGetSlotMax(0));

   if ( padPortOpen(0, 0, padBuf1) == 0 ) {
      printf("padOpenPort 0 failed: \n");
      SleepThread();
   }

   WaitForNextVRstart(1);

   if ( !PS2_InitializePad(0, 0) ) {
      printf("pad0 initalization failed!\n");
      SleepThread();
   }

   WaitForNextVRstart(1);

   if ( padPortOpen(1, 0, padBuf2) == 0 ) {
      printf("padOpenPort 1 failed: \n");
      SleepThread();
   }

   WaitForNextVRstart(1);

   if ( !PS2_InitializePad(1, 0) ) {
      printf("pad1 initalization failed!\n");
      SleepThread();
   }

   WaitForNextVRstart(1);

   return 0;
}

void read_dualshock()
{
    memset(&ds3Input, 0x0, sizeof(struct SS_GAMEPAD));
    ds3ps2_get_input((u8 *)&ds3Input);
    
    memset(&ds4Input, 0x0, sizeof(struct ds4_input));
    ds4ps2_get_input((u8 *)&ds4Input);
}


void update_dualshock()
{
    read_dualshock();
 
    if ( ds3Input.buttons.left  )  dwKeyPad1 |= INPUT_LEFT;
    if ( ds3Input.buttons.right )  dwKeyPad1 |= INPUT_RIGHT;
    if ( ds3Input.buttons.up    )  dwKeyPad1 |= INPUT_UP;
    if ( ds3Input.buttons.down  )  dwKeyPad1 |= INPUT_DOWN;
    
    if ( ds3Input.buttons.circle   )  dwKeyPad1 |= INPUT_B1;
    if ( ds3Input.buttons.cross    )  dwKeyPad1 |= INPUT_B2;
    if ( ds3Input.buttons.triangle )  dwKeyPad1 |= INPUT_RUN;
    if ( ds3Input.buttons.square   )  dwKeyPad1 |= INPUT_SELECT;
    
    switch (ds4Input.dpad) {
    case 0:
        dwKeyPad1 |= INPUT_UP; break;
    case 1:
        dwKeyPad1 |= (INPUT_RIGHT | INPUT_UP); break;
    case 2:
        dwKeyPad1 |= INPUT_RIGHT; break;
    case 3:
        dwKeyPad1 |= (INPUT_RIGHT | INPUT_DOWN); break;
    case 4:
        dwKeyPad1 |= INPUT_DOWN; break;
    case 5:
        dwKeyPad1 |= (INPUT_LEFT | INPUT_DOWN); break;
    case 6:
        dwKeyPad1 |= INPUT_LEFT; break;
    case 7:
        dwKeyPad1 |= (INPUT_LEFT | INPUT_UP); break;
    }

    if ( ds4Input.circle   )  dwKeyPad1 |= INPUT_B1;
    if ( ds4Input.cross    )  dwKeyPad1 |= INPUT_B2;
    if ( ds4Input.triangle )  dwKeyPad1 |= INPUT_RUN;
    if ( ds4Input.square   )  dwKeyPad1 |= INPUT_SELECT;
}

////////////////////////////////////////////////////////////////
void PS2_UpdateInput()
{
   static int filter_method = 0;
   static int pad1_connected = 0, pad2_connected = 0;
   static struct padButtonStatus pad1, pad2; // just in case

   // turbo button stuff
   static int p1_tB=0, p1_tA=0, p1_tSel=0, p1_tSta=0;
   static int p2_tB=0, p2_tA=0, p2_tSel=0, p2_tSta=0;

   int pad1_data = 0, pad2_data = 0;
	int ret1, ret2;

   if ( filter_method != 1 && filter_method != 0 ) {
      filter_method = 0;
   }

	ret1 = padGetState(0, 0);

   while ( (ret1 != PAD_STATE_DISCONN) && (ret1 != PAD_STATE_STABLE) &&
           (ret1 != PAD_STATE_FINDCTP1) )
   {
      if ( ret1 == PAD_STATE_DISCONN ) {
         printf("Pad(%d, %d) is disconnected\n", 0, 0);
         break;
      }

      ret1 = padGetState(0, 0);
   }

   if ( ret1 != PAD_STATE_DISCONN )
      pad1_connected = padRead(0, 0, &pad1); // now working with psx pad!

   ret2 = padGetState(1, 0);

   while ( (ret2 != PAD_STATE_DISCONN) && (ret2 != PAD_STATE_STABLE) &&
           (ret2 != PAD_STATE_FINDCTP1) )
   {
      if ( ret2 == PAD_STATE_DISCONN ) {
         printf("Pad(%d, %d) is disconnected\n", 1, 0);
         break;
      }

      ret2 = padGetState(1, 0);
   }

   if ( ret2 != PAD_STATE_DISCONN )
      pad2_connected = padRead(1, 0, &pad2); //now working with psx pad!

	dwKeyPad1 = 0;
	dwKeyPad2 = 0;

   if ( pad1_connected ) {
      pad1_data = 0xffff ^ pad1.btns;

      // Turbo buttons
      if ( g_BntCnf.turbo_on ) {
         if ( pad1_data & g_BntCnf.gb_Ta )
            p1_tA += 1; else p1_tA = 0;
         if ( pad1_data & g_BntCnf.gb_Tb )
            p1_tB += 1; else p1_tB = 0;
         if ( pad1_data & g_BntCnf.gb_Tselect )
            p1_tSel += 1; else p1_tSel = 0;
         if ( pad1_data & g_BntCnf.gb_Tstart )
            p1_tSta += 1; else p1_tSta = 0;
      }

      // Arrows
      if ( pad1_data & PAD_LEFT )   dwKeyPad1 |= INPUT_LEFT;
      if ( pad1_data & PAD_RIGHT )  dwKeyPad1 |= INPUT_RIGHT;
      if ( pad1_data & PAD_UP )     dwKeyPad1 |= INPUT_UP;
      if ( pad1_data & PAD_DOWN )   dwKeyPad1 |= INPUT_DOWN;

      // Cross, Circle, Tiangle, Square buttons
      if ( (pad1_data & g_BntCnf.gb_a) || (TURBO_MASK & p1_tA) )
         dwKeyPad1 |= INPUT_B1;
      if ( (pad1_data & g_BntCnf.gb_b) || (TURBO_MASK & p1_tB) )
         dwKeyPad1 |= INPUT_B2;
      if ( (pad1_data & g_BntCnf.gb_select) || (TURBO_MASK & p1_tSel) )
         dwKeyPad1 |= INPUT_SELECT;
      if ( (pad1_data & g_BntCnf.gb_start) || (TURBO_MASK & p1_tSta) )
         dwKeyPad1 |= INPUT_RUN;

      if ( (pad1.mode >> 4) == 0x07 ) {
         if ( pad1.ljoy_h < 64 )
            dwKeyPad1 |= INPUT_LEFT;
			else if ( pad1.ljoy_h > 192 )
            dwKeyPad1 |= INPUT_RIGHT;

         if ( pad1.ljoy_v < 64 )
            dwKeyPad1 |= INPUT_UP;
         else if ( pad1.ljoy_v > 192 )
            dwKeyPad1 |= INPUT_DOWN;
      }
   }

   if ( pad2_connected ) {
      pad2_data = 0xffff ^ pad2.btns;

      // Turbo buttons
      if ( g_BntCnf.turbo_on ) {
         if ( pad2_data & g_BntCnf.gb_Ta )
            p2_tA += 1; else p2_tA = 0;
         if ( pad2_data & g_BntCnf.gb_Tb )
            p2_tB += 1; else p2_tB = 0;
         if ( pad2_data & g_BntCnf.gb_Tselect )
            p2_tSel += 1; else p2_tSel = 0;
         if ( pad2_data & g_BntCnf.gb_Tstart )
            p2_tSta += 1; else p2_tSta = 0;
      }

      // Arrows
      if ( pad2_data & PAD_LEFT )    dwKeyPad2 |= INPUT_LEFT;
      if ( pad2_data & PAD_RIGHT )   dwKeyPad2 |= INPUT_RIGHT;
      if ( pad2_data & PAD_UP )      dwKeyPad2 |= INPUT_UP;
      if ( pad2_data & PAD_DOWN )    dwKeyPad2 |= INPUT_DOWN;

      // Cross, Circle, Tiangle, Square buttons
      if ( (pad2_data & g_BntCnf.gb_a) || (TURBO_MASK & p2_tA) )
         dwKeyPad2 |= INPUT_B1;
      if ( (pad2_data & g_BntCnf.gb_b) || (TURBO_MASK & p2_tB) )
         dwKeyPad2 |= INPUT_B2;
      if ( (pad2_data & g_BntCnf.gb_select) || (TURBO_MASK & p2_tSel) )
         dwKeyPad2 |= INPUT_SELECT;
      if ( (pad2_data & g_BntCnf.gb_start) || (TURBO_MASK & p2_tSta) )
         dwKeyPad2 |= INPUT_RUN;

      if ( (pad2.mode >> 4) == 0x07) {
         if ( pad2.ljoy_h < 64 )
            dwKeyPad2 |= INPUT_LEFT;
         else if ( pad2.ljoy_h > 192 )
            dwKeyPad2 |= INPUT_RIGHT;

			if ( pad2.ljoy_v < 64 )
            dwKeyPad2 |= INPUT_UP;
			else if ( pad2.ljoy_v > 192 )
            dwKeyPad2 |= INPUT_DOWN;
		}
	}
    
    update_dualshock();

	// Start button
   if ( pad1_data & g_BntCnf.browser_start ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &pad1); // port, slot, buttons
            pad1_data = 0xffff ^ pad1.btns;
         }

         if ( !(pad1_data & PAD_CROSS) && !(pad1_data & PAD_START) )
            break; // break when those 2 buttons are released
		}

      // filter method change
      MenuIngame();
   }
    if (ds3Input.buttons.start) {
        while ( true ) {
            read_dualshock();
            if ( !(ds3Input.buttons.cross) && !(ds3Input.buttons.start) )
                break; // break when those 2 buttons are released
        }
        // filter method change
        MenuIngame();
    }
    
    if (ds4Input.options) {
        while ( true ) {
            read_dualshock();
            if ( !(ds4Input.cross) && !(ds4Input.options) )
                break; // break when those 2 buttons are released
        }
        // filter method change
        MenuIngame();
    }
}

////////////////////////////////////////////////////////////////
int PS2_UpdateInputMenu()
{
   static struct padButtonStatus lpad1; // just in case
   static int pad1_connected = 0;
   static int padcountdown = 0;
   static int pad_held_down = 0;
   int ret1, lpad1_data = 0;
   
    read_dualshock();
   
    if ( ds3Input.buttons.cross ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds3Input.buttons.cross) ) break;
        }
        return PAD_CROSS;
    }
    
    if ( ds3Input.buttons.square ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds3Input.buttons.square) ) break;
        }
        return PAD_SQUARE;
    }
    
    if ( ds3Input.buttons.triangle ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds3Input.buttons.triangle) ) break;
        }
        return PAD_TRIANGLE;
    }
    if ( ds3Input.buttons.circle ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds3Input.buttons.circle) ) break;
        }
        return PAD_CIRCLE;
    }
    
    if ( ds3Input.buttons.up ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds3Input.buttons.up) ) break;
        }
        return PAD_UP;
    }
    
    if ( ds3Input.buttons.down ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds3Input.buttons.down) ) break;
        }
        return PAD_DOWN;
    }
    
    if ( ds3Input.buttons.right ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds3Input.buttons.right) ) break;
        }
        return PAD_RIGHT;
    }
    
    if ( ds3Input.buttons.left ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds3Input.buttons.left) ) break;
        }
        return PAD_LEFT;
    }
    
    
    //DS4
    
    
     if ( ds4Input.cross ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds4Input.cross) ) break;
        }
        return PAD_CROSS;
    }
    
    if ( ds4Input.square ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds4Input.square) ) break;
        }
        return PAD_SQUARE;
    }
    
    if ( ds4Input.triangle ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds4Input.triangle) ) break;
        }
        return PAD_TRIANGLE;
    }
    if ( ds4Input.circle ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds4Input.circle) ) break;
        }
        return PAD_CIRCLE;
    }
    
    //THIS IS DIRTY!!!!!
    /*if ( ds4Input.dpad == 0 ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds4Input.dpad == 0) ) break;
        }
        return PAD_UP;
    }*/
    
    if ( ds4Input.dpad == 4 ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds4Input.dpad == 4) ) break;
        }
        return PAD_DOWN;
    }
    
    if ( ds4Input.dpad == 2 ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds4Input.dpad == 2) ) break;
        }
        return PAD_RIGHT;
    }
    
    if ( ds4Input.dpad == 6 ) {
        while ( true ) {
            read_dualshock();
             if ( !(ds4Input.dpad == 6) ) break;
        }
        return PAD_LEFT;
    }
   
   ret1 = padGetState(0, 0);
   while ( (ret1 != PAD_STATE_STABLE) && (ret1 != PAD_STATE_FINDCTP1) ) {
      if ( ret1 == PAD_STATE_DISCONN ) {
         printf("Pad(%d, %d) is disconnected\n", 0, 0);
      }
      ret1 = padGetState(0, 0);
   }

   pad1_connected = padRead(0, 0, &lpad1); // now working with psx pad!

   if ( pad1_connected ) {
      lpad1_data = 0xffff ^ lpad1.btns;

      if ( (lpad1.mode >> 4) == 0x07 ) {
         if ( lpad1.ljoy_v < 64 )
            lpad1_data |= PAD_UP;
         else if ( lpad1.ljoy_v > 192 )
            lpad1_data |= PAD_DOWN;

         if ( lpad1.ljoy_h < 64 )
            lpad1_data |= PAD_LEFT;
         else if ( lpad1.ljoy_h > 192 )
            lpad1_data |= PAD_RIGHT;
      }
   }

   if ( lpad1_data & PAD_CROSS ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1); // port, slot, buttons
            lpad1_data = 0xffff ^ lpad1.btns;
         }

			if( !(lpad1_data & PAD_CROSS) ) break;
      }
      return PAD_CROSS;
   }

   if ( lpad1_data & PAD_CIRCLE ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1);
            lpad1_data = 0xffff ^ lpad1.btns;
         }

         if ( !(lpad1_data & PAD_CIRCLE) ) break;
      }
		return PAD_CIRCLE;
   }

   if ( lpad1_data & PAD_TRIANGLE ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1);
            lpad1_data = 0xffff ^ lpad1.btns;
         }

         if ( !(lpad1_data & PAD_TRIANGLE) ) break;
      }
		return PAD_TRIANGLE;
	}

	if ( lpad1_data & PAD_SQUARE ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1);
            lpad1_data = 0xffff ^ lpad1.btns;
         }

         if ( !(lpad1_data & PAD_SQUARE ) ) break;
      }
		return PAD_SQUARE;
	}

	if ( lpad1_data & PAD_R1 ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1);
            lpad1_data = 0xffff ^ lpad1.btns;
         }

         if ( !(lpad1_data & PAD_R1 ) ) break;
      }
		return PAD_R1;
	}

	if ( lpad1_data & PAD_R2 ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1);
            lpad1_data = 0xffff ^ lpad1.btns;
         }

         if ( !(lpad1_data & PAD_R2 ) ) break;
      }
		return PAD_R2;
	}

	if ( lpad1_data & PAD_L1 ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1);
            lpad1_data = 0xffff ^ lpad1.btns;
         }

         if ( !(lpad1_data & PAD_L1 ) ) break;
      }
		return PAD_L1;
	}

	if ( lpad1_data & PAD_L2 ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1);
            lpad1_data = 0xffff ^ lpad1.btns;
         }

         if ( !(lpad1_data & PAD_L2 ) ) break;
      }
		return PAD_L2;
	}

	if ( lpad1_data & PAD_START ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1);
            lpad1_data = 0xffff ^ lpad1.btns;
         }

         if ( !(lpad1_data & PAD_START ) ) break;
      }
		return PAD_START;
	}

	if ( lpad1_data & PAD_SELECT ) {
      while ( true ) {
         if ( padGetState(0, 0) == PAD_STATE_STABLE ) {
            padRead(0, 0, &lpad1);
            lpad1_data = 0xffff ^ lpad1.btns;
         }

         if ( !(lpad1_data & PAD_SELECT ) ) break;
      }
		return PAD_SELECT;
	}

	if ( padcountdown )
      padcountdown--;

   if ( (lpad1_data & PAD_DOWN) && (padcountdown == 0) ) {
      if ( pad_held_down++ < 4 )
         padcountdown = 10;
      else
         padcountdown = 2;

      return PAD_DOWN;
   }

   if ( (lpad1_data & PAD_UP) && (padcountdown == 0) ) {
      if ( pad_held_down++ < 4 )
         padcountdown = 10;
      else
         padcountdown = 2;

      return PAD_UP;
   }

   if ( (lpad1_data & PAD_LEFT) && (padcountdown == 0) ) {
      if ( pad_held_down++ < 4 )
         padcountdown = 10;
      else
         padcountdown = 2;

      return PAD_LEFT;
   }

   if ( (lpad1_data & PAD_RIGHT) && (padcountdown == 0) ) {
      if ( pad_held_down++ < 4 )
         padcountdown = 10;
      else
         padcountdown = 2;

      return PAD_RIGHT;
   }

   // if up or down are NOT being pressed, reset the pad_held_down flag
   if ( !(lpad1_data & (PAD_UP | PAD_DOWN | PAD_RIGHT | PAD_LEFT)) )
      pad_held_down = 0;

	return 0;
}
