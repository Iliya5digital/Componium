#ifndef __DIGIMUS
#define __DIGIMUS

typedef struct /* viz FALDIGIA.S */
{ /* poradi je zavazne - viz asm */
	long RPos,RKrk,RRep; /* 32 casti */
	unsigned long RPosF,RKrkF,RRepF; /* .16 casti */
	const short *RNas;
	byte resvd;
	byte Fade; /* pokyn k fadovani */
	byte LVol,PVol; /* hlasit. * stereo */
	byte EffC,EffD,EffA,EffB; /* efekty - zat¡m se pou§¡v  jen A,B */
	int StereoMin,StereoMax;
	const void *NasDef;
} FNastroj;

extern int F_Povel;

enum {MaxFalcKan=24};

extern FNastroj FalcKans[MaxFalcKan];

void FalcPlayDig( int NKan );

void *FalcGetPlayBuf( void );

void FalcIntrPlay();
void FalcTimerPlay( void ); /* vykone telo IntrPlay */

#endif
