/*
 *  Copyright (C) 2010  Krystian Karaœ <k4rasq@gmail.com>
 *
 *  I release it for fully free use by anyone, anywhere, for any purpose.
 *  But I still retain the copyright, so no one else can limit this release.
 */
 
#ifndef __PS2_INFOGB_GUI_H_
#define __PS2_INFOGB_GUI_H_

////////////////////////////////////////////////////////////////
// Types for DisplayMessageBox
////////////////////////////////////////////////////////////////
#define MSG_VOID           0  // Wait for push Start (return true only)
#define MSG_FATAL          1  // Halt
#define MSG_ACCEPT_ABORT   2  // Wait for push Start or X (return true/false)

////////////////////////////////////////////////////////////////
// Functions declaration
////////////////////////////////////////////////////////////////
bool DisplayMessageBox(const char *StrMsg, int Type);
void DisplayIntroBox(const char *StrMsg);
void guiFadeOut(int pa);
void guiFadeIn(int pa);
void Load_CNF(const char *ConfigPath);
void Save_CNF(const char *ConfigPath);
void MenuOptions();
void MenuIngame();
void MenuBrowser();

////////////////////////////////////////////////////////////////
// Globals declaration
////////////////////////////////////////////////////////////////
extern unsigned long dwKeyPad1;
extern unsigned long dwKeyPad2;

extern const char *ConfigPath;
extern const char *ButtonsConfigPath;

extern int auto_ROM_flag;

#endif
