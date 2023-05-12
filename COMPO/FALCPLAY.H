#ifndef __FALCPLAY
#define __FALCPLAY

extern double FBufFreqDbl;

void InitFFreq( int freq );

int FGloballocksnd( void );
void FGlobalunlocksnd( void );

#ifdef __LLHRAJ
	extern LLHraj AYBufFalc;
#endif

extern int F_Mode; /* poŸet kan l… */
extern int F_Povel;
extern int F_Divis;
extern Flag F_Mode2;

extern Flag F_Display;

enum {FPNic,FPFade};

#ifdef __DYNPOLE
void F_FSetNastr( int NI, int Kan, DynPole *P );
void F_FEchoNastr( int NI, int Kan, DynPole *P );
#endif
void F_FSetStereo( int VMin, int VMax, int Kan );
void F_FSetEffect( int VMin, int VMax, int Kan );
Flag F_FHrajTon( int Ton, int Vol, int Kan );
int F_FyzTon( int Ton );

void F_EffPars( int E, const int *Pars );

Flag F_CasujHrani( void );

Err F_ZacHrajFBuf( long Del, long Od, int freq  );
Err F_KonHrajFBuf( void );

long F_BenchTune( long Val );

#endif
