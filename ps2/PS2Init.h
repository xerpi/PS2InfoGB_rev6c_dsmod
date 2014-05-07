#ifndef __INFOGB_PS2_INIT_H_
#define __INFOGB_PS2_INIT_H_

//////////////////////////////////
// External IRX modules
//////////////////////////////////
extern void *cdvd_irx;
extern int size_cdvd_irx;

//extern void *sjpcm_irx;
//extern int size_sjpcm_irx;

extern void *usbd_irx;
extern int size_usbd_irx;

extern void *usbhdfsd_irx;
extern int size_usbhdfsd_irx;

extern void *audsrv_irx;
extern int size_audsrv_irx;

extern void *ds3ps2_irx;
extern int size_ds3ps2_irx;

extern void *ds4ps2_irx;
extern int size_ds4ps2_irx;


//////////////////////////////////
// Globals declaration
//////////////////////////////////
extern int g_DispX;
extern int g_DispY;

//////////////////////////////////
// Init functions declaration
//////////////////////////////////
void PS2_Delay(int Count);
void PS2_LoadModules(void);
void PS2_Init(void);
void PS2_InitGS(void);
int  PS2_InitMC(void);

#endif
