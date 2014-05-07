#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <stdio.h>
#include <loadfile.h>
#include <iopcontrol.h>
#include <iopheap.h>

#include <sbv_patches.h>
#include <libmc.h>

#include "PS2Init.h"
#include "PS2Input.h"
#include "PS2InfoGB.h"


#include "lib/libcdvd/ee/cdvd_rpc.h"

extern "C" {
   #include "hw.h"
   #include "gs.h"
   #include "gfxpipe.h"
}

#include "common.h"

////////////////////////////////////////////////////////////////
int g_DispX, g_DispY;

static int mc_Type, mc_Free, mc_Format;
static const int numModulesElements = 5;
static char *ModulesList[numModulesElements] = {
   "rom0:SIO2MAN",
   "rom0:MCMAN",
   "rom0:MCSERV",
   "rom0:PADMAN",
   "rom0:LIBSD"
};

////////////////////////////////////////////////////////////////
void PS2_Delay(int Count)
{
   int i, ret;

   for (i = 0; i < Count; i++) {
      ret = 0x1000000;
      while ( ret-- ) {
         asm("nop\nnop\nnop\nnop");
      }
   }
}

////////////////////////////////////////////////////////////////
void PS2_LoadModules(void)
{
   int ret = 0;

   for ( int i = 0; i < numModulesElements; i++ ) {
      ret = SifLoadModule(ModulesList[i], 0, NULL);
      if ( ret < 0 ) {
         printf("Failed to load module: %s\n", ModulesList[i]);
      }
   }

   SifExecModuleBuffer(&cdvd_irx, size_cdvd_irx,0, NULL, &ret);
   if (ret < 0) {
      printf("Failed to load module: LIBCDVD.IRX\n");
   }

   SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &ret);
   if (ret < 0) {
     printf("Failed to load module: USBD.IRX\n");
   }

   SifExecModuleBuffer(&usbhdfsd_irx, size_usbhdfsd_irx, 0, NULL, &ret);
   if (ret < 0) {
      printf("Failed to load module: USBHDFSD.IRX\n");
   }

   SifExecModuleBuffer(&audsrv_irx, size_audsrv_irx, 0, NULL, &ret);
	if (ret < 0) {
      printf("Failed to load module: audsrv.irx\n");
	}
    
    SifExecModuleBuffer(&ds3ps2_irx, size_ds3ps2_irx, 0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: ds3ps2.irx\n");
    }

    SifExecModuleBuffer(&ds4ps2_irx, size_ds4ps2_irx, 0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: ds4ps2.irx\n");
    }

}

////////////////////////////////////////////////////////////////
void PS2_InitGS(void)
{
   DmaReset();

   if ( pal_ntsc() == 3 ) {
      g_DispX = 75;
      g_DispY = 40;
      GS_InitGraph(GS_PAL, GS_NONINTERLACE);
      GS_SetDispMode(g_DispX, g_DispY, SCREEN_W, SCREEN_H);
      g_SndSampler = 44100 / 50;
   } else {
      g_DispX = 80;
      g_DispY = 7;
      GS_InitGraph(GS_NTSC, GS_NONINTERLACE);
      GS_SetDispMode(g_DispX, g_DispY, SCREEN_W, SCREEN_H);
      g_SndSampler = 44100 / 60;
   }

   GS_SetEnv(SCREEN_W, SCREEN_H, 0, 0x50000, GS_PSMCT32, 0xA0000, GS_PSMZ16S);
   install_VRstart_handler();
	createGfxPipe(&thegp, /*(void *)0xF00000,*/ 0x50000);
	gp_hardflush(&thegp);
}

////////////////////////////////////////////////////////////////
int PS2_InitMC(void)
{
	int ret;

	if(mcInit(MC_TYPE_MC) < 0) {
		printf("Failed to initialise memcard server!\n");
		SleepThread();
	}

	// Since this is the first call, -1 should be returned.
	mcGetInfo(0, 0, &mc_Type, &mc_Free, &mc_Format);
	mcSync(0, NULL, &ret);
	printf("mcGetInfo returned %d\n",ret);
	printf("Type: %d Free: %d Format: %d\n\n", mc_Type, mc_Free, mc_Format);

	// Assuming that the same memory card is connected, this should return 0
	mcGetInfo(0,0,&mc_Type,&mc_Free,&mc_Format);
	mcSync(0, NULL, &ret);
	printf("Port 0 - mcGetInfo returned %d\n",ret);
	printf("Type: %d Free: %d Format: %d\n\n", mc_Type, mc_Free, mc_Format);

	mcGetInfo(1,0,&mc_Type,&mc_Free,&mc_Format);
	mcSync(0, NULL, &ret);
	printf("Port 1 - mcGetInfo returned %d\n",ret);
	printf("Type: %d Free: %d Format: %d\n\n", mc_Type, mc_Free, mc_Format);

	return (int)(mc_Free*1000);
}

////////////////////////////////////////////////////////////////
void PS2_Init(void)
{
   int i;

   SifIopReset("rom0:UDNL rom0:EELOADCNF",0);
	while(!SifIopSync());

	fioExit();
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifExitCmd();

	SifInitRpc(0);
	FlushCache(0);
	FlushCache(2);

   sbv_patch_enable_lmb();           // not sure we really need to do this again
   sbv_patch_disable_prefix_check(); // here, but will it do any harm?

   PS2_LoadModules();
   PS2_Delay(5); // KarasQ: we have to wait a while
   PS2_InitGS();
   PS2_InitMC();
   PS2_SetupPad();

   CDVD_Init();

   // Sound init
   g_SndBuff = (short int *)malloc(sizeof(short int)*g_SndSampler*2*2);

   for ( i = 0; i < g_SndSampler*4; i++ )
      g_SndBuff[i] = 0;
}
