#ifndef __INFOGB_PS2_INPUT_H_
#define __INFOGB_PS2_INPUT_H_

//////////////////////////////////
// type/struct definisions
//////////////////////////////////
typedef struct _ButtonsConfig {
   unsigned int gb_Ta;
   unsigned int gb_Tb;
   unsigned int gb_Tselect;
   unsigned int gb_Tstart;
   unsigned int gb_a;
   unsigned int gb_b;
   unsigned int gb_select;
   unsigned int gb_start;
   unsigned int browser_start;
   unsigned int turbo_on;        // Turbo buttons ON/OFF flag
} ButtonsConfig;

//////////////////////////////////
// Globals declaration 
//////////////////////////////////
extern ButtonsConfig g_BntCnf;

extern unsigned long dwKeyPad1;
extern unsigned long dwKeyPad2;

//////////////////////////////////
// Function declaration
//////////////////////////////////
bool PS2_SaveButtonsConfig(const char *path);
bool PS2_LoadButtonsConfig(const char *path);
void PS2_UpdateInput();
int  PS2_UpdateInputMenu();
int  PS2_SetupPad();

#endif
