/* spolecna cast digitalnich prehravacu */
/* pro ruzne vystupy */

#ifndef __LLHRAJ
	#include "llhraj.h"
#endif
#include <stdio.h>

enum
{
	/* tj. rozsah C3 - h2 (c3) - 6 oktav */
	MinDelRep=256, /* min. delka opakovane (ukoncovaci) casti v samplu */
};

typedef struct
{
	PathName NazSoub;
	const void *Obs;
	long Delka; /* skut. delka souboru po vynechani - na F030 muze byt dlouhy */
	long Repet; /* opakovani konce - v souboru jde jen celocis. */
	long Vynech; /* vynechani od konce */
	int Resol; /* rozliseni v bytech - 1 nebo 2 */
	Freq SamFreq; /* udaje o frekvenci */
	int BasNota; /* midikey */
	double ProFreq; /* pri jake Frekvenci se prekladal TonKrok */
	lword TonKrok[12*FyzOkt]; /* predpocitane kroky pro vsechny tony (asi 300B) */
	Flag Uzito;
	int EffA,EffB; /* default hodnoty - pro automatick‚ efekty */
} NastrSoub;

/* prehravac z Componia zapouzdreny pro obecne pouziti */

#ifndef __DYNPOLE
#include <dynpole.h>
#endif

/* ST - AY verze */

extern double ABufFreqDbl;

extern LLHraj AYBufST;

/* STE/TT/Falcon */

enum
{
	FreqDW=160, /* jakou chceme frekvenci D */
	DDelay=200,
	FrqDConst=(int)(2457600L/DDelay/FreqDW), /* perioda, ve ktere se provadi kroky ridiciho hrani */
	FreqD=(int)(2457600L/DDelay/FrqDConst), /* skut. frekvence */
	TempStep=sec/FreqD, /* krok, o ktery se meri v rid. hrani */
};

/* STE/TT */

extern double SBufFreqDbl;

void InitSFreq( void );

int SGloballocksnd( void );
void SGlobalunlocksnd( void );

extern LLHraj AYBufSTE;

long S_Benchmark( void );

/* Falcon */

#include "falcplay.h"

/* import pro Falcona */

Err ZacDsp( void ); /* celk. init. DSP */
void KonDsp( void ); /* celk. deinit. DSP */

Err ZacLod( void ); /* cast. init. DSP */
void KonLod( void ); /* cast. deinit. DSP */

/* import - k obecnemu pouziti */
/* sprava orchestru */

extern DynPole NDef;
extern DynPole NSou;

extern double FreqIs; /* sem musi dat pri zac. prekladu frekv. podle FrqAConst */
extern PathName NastrP;

NastrDef *NastrAcc( DynPole *P, int NN );
int NajdiDef( const char *NNam, int Tip, DynPole *P );

void InitNastrP( const char *N );
void VytvorNastrSoub( char *N, const char *W );
void NastrojNazev( const char *N, PathName P );

void ZacSoub( void );

long ZavedSoub( const char *Nam, int Resol ); /* resol - 1 nebo 2 - v bytech */
void UzijSoub( long S );
void KonSoub( void );

Err ZacHNastr( Pisen *P );
void FAktHNastr( void ); /* zrus vsechny nepouzite cachovane soubory */
void KonHNastr( void );

Err KonSNastr( void );
void MazSNastr( void );
Err CtiOrchestr( void );
Err PisOrchestr( void );
const char *EditSNastr( const char *NNam, const WindRect *Orig, Pisen *P );
int DefHNastr( const char *NNam, int Ton, int Tip, Pisen *P );
CasT HDelkaZneni( Pisen *P, int Nas, int Ton );
const char **SoupisSNastr( Pisen *P );
void AktHNastr( void );
Err SaveHNastr( FILE *f, Pisen *P, const char *AVRPath ); /* uloz vsechny pouzite nastroje */
Err VymazNastr( char *N, Pisen *P );
Err DigiZacNastr( LLHraj *H, double Fr );

extern LLHraj *AktHraj; /* glob. promenna, urcuje, kdo prave hraje */
const char *NastrInfo( const char *NNam, Pisen *P );

#ifndef __INFSOUB
#include "stdc\infosoub.h"
#endif
void AYCtiCFG( const Info *Inf );
Flag AYUlozCFG( Info *Inf );
extern PathName OrcNam;
