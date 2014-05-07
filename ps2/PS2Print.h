#ifndef _PS2_PRINT_H__
#define _PS2_PRINT_H__

// Print white, centered text
#define TextOutC2(x1, x2, y, str, z) textCpixel((x1), (x2), (y) + 4,\
            GS_SET_RGBA(255,255,255,128),0,0,(z),(str))

// Print white, left-aligned text
#define TextOut(x, y, str, z) textpixel((x), (y) + 4, \
            GS_SET_RGBA(255,255,255,128),0,0, (z), (str))

void printch(int x, int y, unsigned couleur, unsigned char ch, int taille, int pl, int zde);
void textpixel(int x, int y, unsigned color, int tail, int plein, int zdep, const char *string,...);
void textCpixel(int x, int x2, int y, unsigned color, int tail, int plein, int zdep, const char *string,...);

#endif
