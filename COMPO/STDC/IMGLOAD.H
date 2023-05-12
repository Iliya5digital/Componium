#ifdef __cplusplus
extern "C" {
#endif

#if _MSC_VER
//#define MAG(x) ( (((x)>>24)&0xff) | (((x)>>8)&0xff00) | (((x)<<8)&0xff0000) | (((x)<<24)&0xff000000) )
#define MAG(x) (x)
#else
#error define MAG(x)
#endif

void SwapEndianL(int *xx);
void SwapEndianW(short *xx);

#define COL_R(color) ( (int)((color>>0)&0xff) )
#define COL_G(color) ( (int)((color>>8)&0xff) )
#define COL_B(color) ( (int)((color>>16)&0xff) )
#define COL_A(color) ( (int)((color>>24)&0xff) )

#define COL_565_R(color) ( (int)(((color>>11)&0x1f)*(255.0f/31.0f)+0.5f) )
#define COL_565_G(color) ( (int)(((color>>5)&0x3f)*(255.0f/63.0f)+0.5f) )
#define COL_565_B(color) ( (int)(((color>>0)&0x1f)*(255.0f/31.0f)+0.5f) )

#define rgb888(r,g,b) ( (r)|((g)<<8)|((b)<<16)|0xff000000 )
#define rgb8888(r,g,b,a) ( (r)|((g)<<8)|((b)<<16)|((a)<<24) )
#define rgb(r,g,b) rgb888((r)*255/1000,(g)*255/1000,(b)*255/1000)

typedef unsigned long VRAMPixel;

void *IMGLoad( const char *N, int *W, int *H, int *NPlanu );

void *MTPILoad( const char *N, int *W, int *H, int *NP, int *NC, long *RGB, long Mag );
void *TPILoad( const char *N, int *W, int *H, int *NP, int *NC, long *RGB );
int MTPISave( const char *N, int W, int H, int NP, int NC, long *RGB, void *Buf, long Len, long Mag );
int TPISave( const char *N, int W, int H, int NP, int NC, long *RGB, void *Buf, long Len );

void *PANLoad( const char *N, int *W, int *H, int *NP, int *NC, long *RGB );
int PANSave( const char *N, int W, int H, int NP, int NC, long *RGB, void *Buf, long Len );

void *RAWPCCLoad( const char *N, int *W, int *H );
VRAMPixel *PCCLoad( const char *N, int *W, int *H );
void *PAC256Load( const char *N, int *W, int *H, long *RGB ); /* PixelPaket */

int SaveRAWPCC( const char *N, word *Buf, int W, int H );
int SavePCC( const char *N, word *Buf, int W, int H ); /* DC */
int SavePAC256( const char *N, byte *Buf, int W, int H, long *RGB, Flag TGA ); /* PP256 */

void *TGALoad( const char *N, int *W, int *H, int *NPlanu );
int TGASave( const char *N, int W, int H, word *Buf ); /* jen DC */
int TGA256Save( const char *N, int W, int H, void *Buf, long *RGB ); /* PixelPaket, paleta - 256 barev */
int TGALSave( const char *N, int W, int H, unsigned long *Buf ); /* TC s hloubkou Resol */
int TGANSave( const char *N, int W, int H, void *Buf, long *RGB, int NC ); /* PixelPaket, paleta - N barev */

void *KonvNBP2PP( word *Buf, int W, int H, int N ); /* N, BitPlanes->PixelPaket */

void KonvPP2BP( word *Buf, int W, int H ); /* 256, PixelPaket->BitPlanes */
void KonvBP2PP( word *Buf, int W, int H ); /* 256, PixelPaket->BitPlanes */

void KonvTransparent( word *Buf, int W, int H ); /* zmena bile (TruePaint) na 0 */
long KonvTitl( VRAMPixel *To, VRAMPixel *Buf, int W, int H ); /* vzdy opak/kolikrat - vraci delku - shrinkni si sam */

VRAMPixel *TransRadek( VRAMPixel *TBuf, int W, int H ); /* vyhleda radek */

#ifdef __cplusplus
};
#endif

