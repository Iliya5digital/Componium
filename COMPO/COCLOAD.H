#ifndef __COCLOAD
#define __COCLOAD

/* import z prehravace */

#ifndef __COMPOUTL
typedef int Err;
#endif

#include "..\compo\falcplay.h"
#include <stdio.h>

/* vlastni sluzby */

extern Flag AutoRepeat;
extern Flag HoverFormat;
extern Flag AutoEffects;

int NactiPisen( FILE *f, const char *Root, const char *altRoot, int freq );
void PustPisen( void );

int ZacHrajPisen( int freq );
void KonHrajPisen( void );
Flag TestHraje( void );

void AbortHraje( void );

void HlasHudba( long Vol ); /* 0..1000 */
void ZacFadeOutHudba( int Time );
void KonFadeOutHudba( void );
void FadeOutHudba( int Time );

#endif
