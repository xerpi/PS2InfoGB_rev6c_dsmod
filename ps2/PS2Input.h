#ifndef __INFOGB_PS2_INPUT_H_
#define __INFOGB_PS2_INPUT_H_


// by xerpi
struct SS_BUTTONS
{
    u8 select   : 1;
    u8 L3       : 1;
    u8 R3       : 1;
    u8 start    : 1;    
    u8 up       : 1;        
    u8 right    : 1;
    u8 down     : 1;
    u8 left     : 1;


    u8 L2       : 1;
    u8 R2       : 1;
    u8 L1       : 1;
    u8 R1       : 1;
    u8 triangle : 1;    
    u8 circle   : 1;
    u8 cross    : 1;    
    u8 square   : 1;

    u8 PS       : 1;
    u8 not_used : 7;
};

struct SS_ANALOG
{
    u8 x;
    u8 y;
};

struct SS_DPAD_SENSITIVE
{
    u8 up;
    u8 right;
    u8 down;
    u8 left;
};

struct SS_SHOULDER_SENSITIVE
{
    u8 L2;
    u8 R2;
    u8 L1;
    u8 R1;
};

struct SS_BUTTON_SENSITIVE
{
    u8 triangle;
    u8 circle;
    u8 cross;
    u8 square;
};

struct SS_MOTION
{
    s16 acc_x;
    s16 acc_y;
    s16 acc_z;
    s16 z_gyro;
};

struct SS_GAMEPAD
{
    u8                             hid_data;
    u8                             unk0;
    struct SS_BUTTONS              buttons;
    u8                             unk1;
    struct SS_ANALOG               left_analog;
    struct SS_ANALOG               right_analog;
    u32                            unk2;
    struct SS_DPAD_SENSITIVE       dpad_sens;
    struct SS_SHOULDER_SENSITIVE   shoulder_sens;
    struct SS_BUTTON_SENSITIVE     button_sens;
    u16                            unk3;
    u8                             unk4;
    u8                             status;
    u8                             power_rating;
    u8                             comm_status;
    u32                            unk5;
    u32                            unk6;
    u8                             unk7;
    struct SS_MOTION               motion;
} __attribute__((packed, aligned(32)));



struct ds4_input {
    u8 report_ID;
    u8 leftX;
    u8 leftY;
    u8 rightX;
    u8 rightY;
    

    u8 dpad     : 4;
    u8 square   : 1;
    u8 cross    : 1;
    u8 circle   : 1;
    u8 triangle : 1;
  
    u8 L1      : 1;
    u8 R1      : 1;
    u8 L2      : 1;
    u8 R2      : 1;
    u8 share   : 1;
    u8 options : 1;
    u8 L3      : 1;
    u8 R3      : 1;
    
    u8 PS     : 1;
    u8 tpad   : 1;
    u8 cnt1   : 6;
    
    u8 Ltrigger;
    u8 Rtrigger;
    
    u8 cnt2;
    u8 cnt3;
    
    u8 battery;
    
    s16 accelX;
    s16 accelY;
    s16 accelZ;
    
    union {
        s16 roll;
        s16 gyroZ;
    };
    union {
        s16 yaw;
        s16 gyroY;
    };
    union {
        s16 pitch;
        s16 gyroX;
    };
    
    u8 unk1[5];
    
    u8 battery_level : 4;
    u8 usb_plugged   : 1;
    u8 headphones    : 1;    
    u8 microphone    : 1;    
    u8 padding       : 1;
    
    u8 unk2[2];
    u8 trackpadpackets;
    u8 packetcnt;
    
    u32 finger1ID     : 7;
    u32 finger1active : 1;
    u32 finger1X      : 12;
    u32 finger1Y      : 12;
    
    u32 finger2ID     : 7;
    u32 finger2active : 1;
    u32 finger2X      : 12;
    u32 finger2Y      : 12;
    
} __attribute__((packed, aligned(32)));


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
extern struct SS_GAMEPAD ds3Input;
extern struct ds4_input ds4Input;

//////////////////////////////////
// Function declaration
//////////////////////////////////
bool PS2_SaveButtonsConfig(const char *path);
bool PS2_LoadButtonsConfig(const char *path);
void PS2_UpdateInput();
int  PS2_UpdateInputMenu();
int  PS2_SetupPad();
void read_dualshock();
void update_dualshock();

#endif
