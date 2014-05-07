/* 
 * WARNING!!: ONLY gp_gouradrect, gp_frect, gp_point, gp_texrect, gp_linerect 
 * DON'T NEED SHIFFTING LEFT COORDINATES BY 4 
 */

// GFX-Pipe by Vzzrzzn, modifications by Sjeep

#include <tamtypes.h>
#include <kernel.h>
#include "gfxpipe.h"
#include "../hw.h"
#include "../gs.h"

gfxpipe thegp;

#define GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k)                             \
                   ((u64)(lcm) | ((u64)(mxl) << 2) |                              \
                   ((u64)(mmag) << 5) | ((u64)(mmin) << 6) | ((u64)(mtba) << 9) | \
                   ((u64)(l) << 19) | ((u64)(k) << 32))

/////////////////////////////////////////////////////////
void initializeGfxPipe(unsigned long *dmatadr)
{
   dmatadr[0] = 0x0000000070000000;
   dmatadr[1] = 0;
/* dmatadr[2] = 0x1000000000008000;
   dmatadr[3] = 0x000000000000000e; */
}

/////////////////////////////////////////////////////////
unsigned int gp_bytesLeft(gfxpipe *p)
{
   unsigned int eob, cgt;
   
   eob = (unsigned int)p->curpipe;
   eob += (p->memsize >> 1);
   cgt = (unsigned int)p->curgiftag;
   
   return eob - cgt;
}

/////////////////////////////////////////////////////////
void gp_hardflush(gfxpipe *p)
{
   Dma02Wait();
   FlushCache(0);
   SendDma02(p->curpipe);
   
   if ( p->curpipe == p->dmatadrA ) {
      initializeGfxPipe(p->dmatadrB);
      
      p->curpipe = p->curdmatadr = p->dmatadrB;
      p->curgiftag = p->curdmatadr + 2;
   } else {
      initializeGfxPipe(p->dmatadrA);
      
      p->curpipe = p->curdmatadr = p->dmatadrA;
      p->curgiftag = p->curdmatadr + 2;
   }
}

/////////////////////////////////////////////////////////
void gp_checkflush(gfxpipe *p)
{
   if ( gp_bytesLeft(p) < (GP_MINSPACE << 3) ) {
      gp_hardflush(p);
   }
}

/////////////////////////////////////////////////////////
// size must be less than 1MB
int createGfxPipe(gfxpipe *pipeline, /* void *buffer,*/ int size)
{
   unsigned long* buffer=NULL;
   
   if ( size < 0x1000 )
      return 0;
   
   buffer = (unsigned long *)memalign(64,size);
   
   if ( (int)buffer & 0xf )
      return 0;
   
   pipeline->dmatadrA = (unsigned long *)buffer;
   pipeline->dmatadrB = pipeline->dmatadrA + (size >> 4);
   pipeline->memsize = size;
   
   initializeGfxPipe(pipeline->dmatadrA);
   
   pipeline->curpipe = pipeline->curdmatadr = pipeline->dmatadrA;
   pipeline->curgiftag = pipeline->dmatadrA + 2;
   
   return 1;
}

/////////////////////////////////////////////////////////
/* the rules:
    - the list is sendable via gp_hardflush after ANY gp_* function call
    - each function must advance the giftag pointer to empty space
    - a function is guaranteed GP_MINSPACE dwords to work with;
      if it needs more, it must check against the remaining size
      of the pipeline ((gp->curdmatadr + (gp->memsize / 2)) - gp->curgiftag)
    - a function must call gp_checkflush after it is done adding to the queue
    - a function must update the size field of the dmatag
    - the current dmatag is of the 'count' style after a function returns
*/

void gp_line(gfxpipe *p, unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned z, unsigned color)
{
   unsigned long dt = *(p->curdmatadr);
   
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 3);
   
   p->curgiftag[0] = 0x4400000000008001;
   p->curgiftag[1] = 0xffffffffffff5d10;
   p->curgiftag[2] = 0x0000000000000001;
   p->curgiftag[3] = 0x3f80000000000000 | color;
   
   p->curgiftag[4] = ((unsigned long)z << 32) | (y1 << 16) | x1;
   p->curgiftag[5] = ((unsigned long)z << 32) | (y2 << 16) | x2;
   
   p->curgiftag = &p->curgiftag[6]; // advance the packet pointer
   
   gp_checkflush(p);
}

/*
// bigger memory version of the above
void gp_line(gfxpipe *p, unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned color)
{
   unsigned long dt = *p->curdmatadr;
   *p->curdmatadr = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 5);
   
   p->curgiftag[0] = 0x1000000000008004;
   p->curgiftag[1] = 0xfffffffffffffffe;
   
   p->curgiftag[2] = 1;
   p->curgiftag[3] = 0;
   p->curgiftag[4] = 0x3f80000000000000 | color;
   p->curgiftag[5] = 1;
   p->curgiftag[6] = (y1 << 16) | x1;
   p->curgiftag[7] = 0xd;
   p->curgiftag[8] = (y2 << 16) | x2;
   p->curgiftag[9] = 5;
   p->curgiftag = &p->curgiftag[10];
   
   gp_checkflush(p);
} 
*/

/////////////////////////////////////////////////////////
void gp_ltriangle(gfxpipe *p, unsigned x1, unsigned y1, unsigned z1, unsigned c1,
                              unsigned x2, unsigned y2, unsigned z2, unsigned c2,
                              unsigned x3, unsigned y3, unsigned z3, unsigned c3)
{
   unsigned long dt = *(p->curdmatadr);
   
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 6);
   
   p->curgiftag[0] = 0x9400000000008001;
   p->curgiftag[1] = 0xfffffff515151d10;
   p->curgiftag[2] = 0x000000000000000a;
  
   p->curgiftag[3] = 0x3f80000000000000 | c1;
   p->curgiftag[4] = ((unsigned long)z1 << 32) | (y1 << 16) | x1;
   p->curgiftag[5] = 0x3f80000000000000 | c2;
   p->curgiftag[6] = ((unsigned long)z2 << 32) | (y2 << 16) | x2;
   p->curgiftag[7] = 0x3f80000000000000 | c3;
   p->curgiftag[8] = ((unsigned long)z3 << 32) | (y3 << 16) | x3;
   
   p->curgiftag[9] = p->curgiftag[3];
   p->curgiftag[10] = p->curgiftag[4];
   p->curgiftag = &p->curgiftag[12]; // advance the packet pointer
   
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_triangle(gfxpipe *p,  unsigned x1, unsigned y1, unsigned z1, unsigned c1,
                              unsigned x2, unsigned y2, unsigned z2, unsigned c2,
                              unsigned x3, unsigned y3, unsigned z3, unsigned c3)
{
   unsigned long dt = *(p->curdmatadr);
   
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 5);
   
   p->curgiftag[0] = 0x7400000000008001;
   p->curgiftag[1] = 0xfffffffff5151d10;
   p->curgiftag[2] = 0x000000000000000b;
   
   p->curgiftag[3] = 0x3f80000000000000 | c1;
   p->curgiftag[4] = ((unsigned long)z1 << 32) | (y1 << 16) | x1;
   p->curgiftag[5] = 0x3f80000000000000 | c2;
   p->curgiftag[6] = ((unsigned long)z2 << 32) | (y2 << 16) | x2;
   p->curgiftag[7] = 0x3f80000000000000 | c3;
   p->curgiftag[8] = ((unsigned long)z3 << 32) | (y3 << 16) | x3;
   
   p->curgiftag = &p->curgiftag[10]; // advance the packet pointer
   
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_ftriangle(gfxpipe *p, unsigned x1, unsigned y1, unsigned z1,
                              unsigned x2, unsigned y2, unsigned z2,
                              unsigned x3, unsigned y3, unsigned z3, unsigned color)
{
   unsigned long dt = *(p->curdmatadr);
   
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 4);
   
   p->curgiftag[0] = 0x5400000000008001;
   p->curgiftag[1] = 0xfffffffffff5dd10;
   p->curgiftag[2] = 0x0000000000000003;
   p->curgiftag[3] = 0x3f80000000000000 | color;
   
   p->curgiftag[4] = ((unsigned long)z1 << 32) | (y1 << 16) | x1;
   p->curgiftag[5] = ((unsigned long)z2 << 32) | (y2 << 16) | x2;
   p->curgiftag[6] = ((unsigned long)z3 << 32) | (y3 << 16) | x3;
   
   p->curgiftag = &p->curgiftag[8]; // advance the packet pointer
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_frect(gfxpipe *p, unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned z, unsigned color)
{
   unsigned long dt = *(p->curdmatadr);
   
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 4);
   
   p->curgiftag[0] = 0x6400000000008001;
   p->curgiftag[1] = 0xffffffffff55dd10;
   p->curgiftag[2] = GS_SET_PRIM(0x04, 0, 0, 0, 1, 0, 0, 0, 0);
   p->curgiftag[3] = 0x3f80000000000000 | color;
   
   p->curgiftag[4] = ((unsigned long)z << 32) | (y1 << 20) | (x1 << 4);
   p->curgiftag[5] = ((unsigned long)z << 32) | (y1 << 20) | (x2 << 4);
   p->curgiftag[6] = ((unsigned long)z << 32) | (y2 << 20) | (x1 << 4);
   p->curgiftag[7] = ((unsigned long)z << 32) | (y2 << 20) | (x2 << 4);
   
   p->curgiftag = &p->curgiftag[8]; // advance the packet pointer
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_enablezbuf(gfxpipe *p)
{
   unsigned long dt = *p->curdmatadr;
   
   *p->curdmatadr = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 2);
   
   p->curgiftag[0] = 0x1000000000008001;
   p->curgiftag[1] = 0xfffffffffffffffe;
   
   p->curgiftag[2] = 0x00070000;
   p->curgiftag[3] = 0x47;
   
   p->curgiftag = &p->curgiftag[4]; // advance the packet pointer
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_disablezbuf(gfxpipe *p)
{
   unsigned long dt = *p->curdmatadr;
   
   *p->curdmatadr = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 2);
   
   p->curgiftag[0] = 0x1000000000008001;
   p->curgiftag[1] = 0xfffffffffffffffe;
   
   p->curgiftag[2] = 0x00030000;
   p->curgiftag[3] = 0x47;
   
   p->curgiftag = &p->curgiftag[4]; // advance the packet pointer
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
/* assumes already converted data
   - vtl->numv must be strictly less than 65535 (that's ~ 1MB !)
   - format: color, point, normal 
 */

/*
void gp_drawVTL_inplace_cpn(gfxpipe *p, vecTriList *vtl)
{
   unsigned long dt = *p->curdmatadr;
   *p->curdmatadr = (dt & 0xffffffff8fff0000) | (1 << 28);
   
   p->curdmatadr = p->curgiftag;
   p->curdmatadr[0] = (0x0000000030000000 + vtl->numv + 1) | 
                      (((unsigned long)(&vtl->giftag[0]) & 0x7fffffff) << 32);
                      
   p->curdmatadr[1] = 0;
   p->curdmatadr += 2;
   p->curdmatadr[0] = 0x0000000070000000;
   p->curdmatadr[1] = 0x0000000000000000;
   
   p->curgiftag = p->curdmatadr + 2;
   gp_checkflush(p);
}
*/

/////////////////////////////////////////////////////////
/* send a byte-packed texture from RDRAM to VRAM
   - TBP = VRAM_address
   - TBW = buffer_width_in_pixels  -- dependent on pxlfmt
   - xofs, yofs in units of pixels
   - pxlfmt = 0x00 (32-bit), 0x02 (16-bit), 0x13 (8-bit), 0x14 (4-bit)
   - wpxls, hpxls = width, height in units of pixels
   - tex -- must be qword aligned !!!
 */

void gp_uploadTexture(gfxpipe *p, int TBP, int TBW, int xofs, int yofs, int pxlfmt, void *tex, int wpxls, int hpxls)
{
   int numq;
   unsigned long dt = *p->curdmatadr;
   
   *p->curdmatadr = (dt & 0xffffffff8fff0000) | (1 << 28) | ((dt & 0xffff) + 6);
   
   p->curgiftag[0] = 0x1000000000008004;
   p->curgiftag[1] = 0xfffffffffffffffe;
   
   p->curgiftag[2] = (unsigned long)( ((pxlfmt & 0x3f)<<24) | 
                     (((TBW/64) & 0x3f)<<16) | ((TBP/256) & 0x3fff) ) << 32;
   
   p->curgiftag[3] = 0x50; // BITBLTBUFF
   p->curgiftag[4] = (unsigned long)( ((yofs)<<16) | (xofs) ) << 32;
   
   p->curgiftag[5] = 0x51; // TRXPOS
   p->curgiftag[6] = ((unsigned long)hpxls << 32) | wpxls;
   
   p->curgiftag[7] = 0x52; // TRXREG
   p->curgiftag[8] = 0;
   
   p->curgiftag[9] = 0x53; // TRXDIR
   
   numq = wpxls * hpxls;
   
   switch ( pxlfmt ) {
      case 0x00: numq = (numq >> 2) + ((numq & 0x03) != 0 ? 1 : 0); break;
      case 0x02: numq = (numq >> 3) + ((numq & 0x07) != 0 ? 1 : 0); break;
      case 0x13: numq = (numq >> 4) + ((numq & 0x0f) != 0 ? 1 : 0); break;
      case 0x14: numq = (numq >> 5) + ((numq & 0x1f) != 0 ? 1 : 0); break;
      default: numq = 0;
   }
   
   p->curgiftag[10] = 0x0800000000000000 + numq; // IMAGE mode giftag.  
                                                 // Flg = 10b, nloop = numq
   p->curgiftag[11] = 0;
   
   p->curdmatadr = &p->curgiftag[12]; // set up dma tag for image transfer. 
                                      // next = tex addr, id = 11b, qwc = numq
   p->curdmatadr[0] = ((((unsigned long)tex) & 0x7fffffff) << 32) | 
                      (0x0000000030000000 + numq);
                      
   p->curdmatadr[1] = 0;
   p->curdmatadr += 2; // + 16 bytes
   
   p->curdmatadr[0] = 0x0000000070000000; // next dma tag, id = end.
   p->curdmatadr[1] = 0x0000000000000000;
   
   p->curgiftag = p->curdmatadr + 2;
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_uvftriangle(gfxpipe *p, unsigned x1, unsigned y1, unsigned z1, unsigned u1, unsigned v1,
                                unsigned x2, unsigned y2, unsigned z2, unsigned u2, unsigned v2,
                                unsigned x3, unsigned y3, unsigned z3, unsigned u3, unsigned v3,unsigned color)
{
   unsigned long dt = *(p->curdmatadr);
   
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 5);
   
   p->curgiftag[0] = 0x8400000000008001;
   p->curgiftag[1] = 0xffffffff53535310;
   p->curgiftag[2] = 0x0000000000000153;
   p->curgiftag[3] = 0x3f80000000000000 | color;
   
   p->curgiftag[4] = ((unsigned long)v1 << 16) | u1;
   p->curgiftag[5] = ((unsigned long)z1 << 32) | (y1 << 16) | x1;
   p->curgiftag[6] = ((unsigned long)v2 << 16) | u2;
   p->curgiftag[7] = ((unsigned long)z2 << 32) | (y2 << 16) | x2;
   p->curgiftag[8] = ((unsigned long)v3 << 16) | u3;
   p->curgiftag[9] = ((unsigned long)z3 << 32) | (y3 << 16) | x3;
   
   p->curgiftag = &p->curgiftag[10];        // advance the packet pointer
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
/*
void gp_stftriangle(gfxpipe *p, unsigned x1, unsigned y1, unsigned z1, float s1, float t1,
                                unsigned x2, unsigned y2, unsigned z2, float s2, float t2,
                                unsigned x3, unsigned y3, unsigned z3, float s3, float t3,unsigned color)
{
   unsigned long dt = *(p->curdmatadr);
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 5);
   
   p->curgiftag[0] = 0x8400000000008001;
   p->curgiftag[1] = 0xffffffff52525210;
   p->curgiftag[2] = 0x0000000000000153;
   p->curgiftag[3] = 0x3f80000000000000 | color;
   
   p->curgiftag[4] = (*(unsigned long *)&t1 << 32) | *(unsigned long *)&s1;
   p->curgiftag[5] = ((unsigned long)z1 << 32) | (y1 << 16) | x1;
   p->curgiftag[6] = (*(unsigned long *)&t2 << 32) | *(unsigned long *)&s2;
   p->curgiftag[7] = ((unsigned long)z2 << 32) | (y2 << 16) | x2;
   p->curgiftag[8] = (*(unsigned long *)&t3 << 32) | *(unsigned long *)&s3;
   p->curgiftag[9] = ((unsigned long)z3 << 32) | (y3 << 16) | x3;
   p->curgiftag = &p->curgiftag[10];        // advance the packet pointer
   
   gp_checkflush(p);
} 
*/

/////////////////////////////////////////////////////////
void gp_setTex(gfxpipe *p, u32 tbp, u32 tbw, u32 texwidth, u32 texheight, u32 tpsm, u32 cbp, u32 cbw, u32 cpsm,u32 filter)
{
   unsigned long dt = *p->curdmatadr;
/*
   int texwpow2 = 0, texhpow2 = 0;
   int t = 0x0400;
   int i;
 */
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 9);
/*
   for ( i=10; i>=0; i-- ) {
      if (texwidth & t) {
         texwpow2 = i;
         break;
      }
      
      t = t >> 1;
   }
   
   t = 0x0400;
   
   for ( i=10; i>=0; i-- ) {
      if (texheight & t) {
         texhpow2 = i;
         break;
      }
      
      t = t >> 1;
   }
 */
   
   p->curgiftag[0] = 0x1000000000008008;
   p->curgiftag[1] = 0xfffffffffffffffe;
   
   p->curgiftag[2] = 256 / 64;
   p->curgiftag[3] = 0x1c; // texclut  <- not really necessary but if i get lazy in future...
   p->curgiftag[4] = 0;
   p->curgiftag[5] = 0x3f; // texflush
   
   p->curgiftag[6] = GS_SET_TEXA(128, 1, 0);
   p->curgiftag[7] = 0x3b;	// texa: TA0 = 128, AEM = 1, TA1 = 0
   
// p->curgiftag[8] = 0x0000000000000040;
   
   p->curgiftag[8] = GS_SET_TEX1(0, 0,  filter,  filter, 0, 0, 0);
   p->curgiftag[9] = 0x14; // tex1_1
   
   p->curgiftag[10]= GS_SET_TEX0(tbp/256, tbw/64, tpsm, texwidth, texheight, 1, 
                                 1, cbp/256, cpsm, 1, 0, 1);
   p->curgiftag[11]= 0x06;	// tex0_1
   p->curgiftag[12]= GS_SET_CLAMP(1, 1, 0, 2^texwidth, 0, 2^texheight);
   
   p->curgiftag[13]= 0x08;	// clamp_1
   p->curgiftag[14]= 0x0000007f00000044;
   
   p->curgiftag[15]= 0x42;	// alpha_1: A = Cs, B = Cd, C = As, D = Cd
   p->curgiftag[16]= 0x0000000000000000;
   
   p->curgiftag[17]= 0x49;	// pabe
   
   p->curgiftag = &p->curgiftag[18]; // advance the packet pointer
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_texrect(gfxpipe *p, u32 x1, u32 y1, u32 u1, u32 v1, u32 x2, u32 y2, u32 u2, u32 v2, u32 z, u32 colour)
{
	unsigned long dt = *(p->curdmatadr);
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 4);
   
   p->curgiftag[0] = GIF_SET_TAG(1, 1, 0, 0, 1, 6);
   p->curgiftag[1] = 0xffffffffff535310;
   p->curgiftag[2] = GS_SET_PRIM(0x06, 0, 1, 0, 1, 0, 1, 0, 0);
   p->curgiftag[3] = 0x3f80000000000000 | colour;
   
   p->curgiftag[4] = GS_SET_UV(u1 << 4, v1 << 4);
   p->curgiftag[5] = GS_SET_XYZ(x1 << 4, y1 << 4, z);
   p->curgiftag[6] = GS_SET_UV(u2 << 4, v2 << 4);
   p->curgiftag[7] = GS_SET_XYZ(x2 << 4, y2 << 4, z);
   
   p->curgiftag = &p->curgiftag[8];
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_linerect(gfxpipe *p, unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned z, unsigned color)
{
   unsigned long dt = *(p->curdmatadr);
   
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 5);
   
   x1 <<= 4;
   x2 <<= 4;
   y1 <<= 4;
   y2 <<= 4;
   
   p->curgiftag[0] = GIF_SET_TAG(1, 1, 0, 0, 1, 8);
   p->curgiftag[1] = 0xfffffffff5555510;
	p->curgiftag[2] = GS_SET_PRIM(0x02, 0, 0, 0, 1, 0, 0, 0, 0);
   p->curgiftag[3] = 0x3f80000000000000 | color;
   p->curgiftag[4] = GS_SET_XYZ(x1,y1,z);
   p->curgiftag[5] = GS_SET_XYZ(x2,y1,z);
   p->curgiftag[6] = GS_SET_XYZ(x2,y2,z);
   p->curgiftag[7] = GS_SET_XYZ(x1,y2,z);
	p->curgiftag[8] = GS_SET_XYZ(x1,y1,z);
	p->curgiftag[9] = 0; // NOP
	
   p->curgiftag = &p->curgiftag[10]; // advance the packet pointer
   gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_point(gfxpipe *p, unsigned x1, unsigned y1, unsigned z, unsigned colour)
{
   unsigned long dt = *(p->curdmatadr);
   
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 3);
   
   p->curgiftag[0] = GIF_SET_TAG(1, 1, 0, 0, 1, 4);
   p->curgiftag[1] = 0xFFFFFFFFFFFF510;
   p->curgiftag[2] = GS_SET_PRIM(0x00, 0, 0, 0, 1, 0, 0, 0, 0);
   p->curgiftag[3] = 0x3f80000000000000 | colour;
   p->curgiftag[4] = GS_SET_XYZ(x1 << 4, y1 << 4, z);
   p->curgiftag[5] = 0; // NOP
   
   p->curgiftag = &p->curgiftag[6];
	gp_checkflush(p);
}

/////////////////////////////////////////////////////////
void gp_gouradrect(gfxpipe *p, unsigned x1, unsigned y1, unsigned c1, unsigned x2, unsigned y2, unsigned c2, unsigned z)
{
   unsigned long dt = *(p->curdmatadr);
   *(p->curdmatadr) = (dt & 0xffffffffffff0000) | ((dt & 0xffff) + 6);
   
   x1 <<= 4;
   x2 <<= 4;
   y1 <<= 4;
   y2 <<= 4;
   
   p->curgiftag[0] = GIF_SET_TAG(1, 1, 0, 0, 1, 10);
   p->curgiftag[1] = 0xFFFFFFF5151d1d10;
   p->curgiftag[2] = GS_SET_PRIM(0x04, 1, 0, 0, 1, 0, 0, 0, 0);
   p->curgiftag[3] = 0x3f80000000000000 | c1;
   p->curgiftag[4] = GS_SET_XYZ(x1, y1, z);
   p->curgiftag[5] = 0x3f80000000000000 | c2;
   p->curgiftag[6] = GS_SET_XYZ(x2, y1, z);
   p->curgiftag[7] = 0x3f80000000000000 | c2;
   p->curgiftag[8] = GS_SET_XYZ(x1, y2, z);
   p->curgiftag[9] = 0x3f80000000000000 | c1;
   p->curgiftag[10] = GS_SET_XYZ(x2, y2 ,z);
   p->curgiftag[11] = 0; // NOP
   
   p->curgiftag = &p->curgiftag[12];
	gp_checkflush(p);
}
