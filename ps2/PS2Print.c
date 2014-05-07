#include <stdio.h>

#include "hw.h"
#include "gs.h"
#include "gfxpipe.h"

extern unsigned char msx[];

void printch(int x, int y, unsigned couleur, unsigned char ch, int taille, int pl, int zde)
{
   unsigned char *font = &msx[(int)ch * 8];
   int i,j;
   int rectx, recty;
   
   for ( i = 0; i < 8; i++, font++ ) 
   {
      for ( j = 0; j < 8; j++ ) 
      {
         if ( (*font & (128 >> j)) )
         {
            rectx = x+(j<<0);
            
            if ( taille == 1 ) 
               recty = y + (i << 1);
            else 
               recty = y + (i << 0);
            
            gp_point(&thegp, rectx, recty, zde, couleur);
                 
            if ( pl == 1 )
               gp_point(&thegp, rectx, recty+1, zde, couleur); 
         }
      }
   }
}

void textpixel(int x, int y, unsigned color, int tail, int plein, int zdep, const char *string,...)
{
   int boucle=0;  
   char text[256];	   	
   va_list ap;			
   
   if ( string == NULL )
      return;		
		
   va_start(ap, string);		
      vsprintf(text, string, ap);	
   va_end(ap);	
   
   while ( text[boucle] != 0 ) {
      printch(x, y, color, text[boucle], tail, plein, zdep);
      boucle++;
      x += 6;
   }
}

void textCpixel(int x, int x2, int y, unsigned color, int tail, int plein, int zdep, const char *string,...)
{
   int boucle = 0;
   char text[256];
   va_list ap;		
   
   if ( string == NULL )
      return;
      
   va_start(ap, string);		
      vsprintf(text, string, ap);
   va_end(ap);
   
   while ( text[boucle] != 0 ) 
      boucle++;
   
   boucle = (x2 - x) / 2 - (boucle * 3);
   x = boucle;
   
   boucle = 0;
   
   while ( text[boucle] != 0 ) {
      printch(x, y, color, text[boucle], tail, plein, zdep);
      boucle++;
      x += 6;
   }
}
