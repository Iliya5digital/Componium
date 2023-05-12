/* digitalni hrani na AY a Falconu */
/* SUMA 3/1993-2/1994 */
/* GEM/Componium zapouzdreni */

#include <macros.h>
#include <string.h>
#include <dspbind.h>
#include <sndbind.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sttos\samples.h>

#include "compo.h"
#include "llhraj.h"
#include "digill.h"
#include "utlcompo.h"
#include "ramcompo.h"
#include "cocload.h"

PathName OrcNam;
Flag Rezerva=False;
LLHraj *AktHraj=&AYBufFalc; /* glob. promenna, urcuje, kdo prave hraje */

const char *JmenoOrc( const char *Nam ){return Nam;}

/*
*/

NastrDef *NastrAcc( DynPole *P, int NN )
{
	if( NN&0x4000 )
	{
		Plati( P );
		return AccDyn(P,NN&0x3fff);
	}
	return AccDyn(&NDef,NN);
}

static int NajdiOptSoub( NastrDef *D, int Ton )
{
	int ni;
	int mindif=FyzOkt*12*2,dif;
	int mini=-1;
	Flag FF=False;
	for( ni=1; ni<NMSoub; ni++ )
	{
		const char *N=D->Soubory[ni];
		if( *N ) FF=True;
	}
	if( !FF ) return 0;
	for( ni=0; ni<NMSoub; ni++ )
	{
		const char *N=D->Soubory[ni];
		if( *N )
		{
			NastrSoub *NS;
			long S=ZavedSoub(D->Soubory[ni],2);
			if( S<0 ) return -1;
			NS=AccDyn(&NSou,S);
			dif=abs(Ton-NS->BasNota);
			if( dif<mindif ) mindif=dif,mini=ni;
		}
	}
	return mini;
}

int DefHNastr( const char *NNam, int Ton, int Tip, Pisen *P )
{
	int I=NajdiDef(NNam,Tip,&P->LocDigi);
	if( I>=EOK )
	{
		NastrDef *D=NastrAcc(&P->LocDigi,I);
		long S;
		int FTon=F_FyzTon(Ton);
		int ni=NajdiOptSoub(D,Ton);
		if( ni<0 ) return KON;
		S=ZavedSoub(D->Soubory[ni],2);
		if( S==ERR ) /*AlertRF(CENASFA,1,NajdiNazev(D->Soubory[ni])),*/ S=ECAN;
		if( S<EOK ) return (int)S;
		UzijSoub(S);
		D->IMSoubory[FTon]=S;
		D->Uzit=True; /* potrebujeme vedet, koho mame ukladat do COC */
	}
	else return KON;
	return I;
}

#if 0
int DefHNastr( const char *NNam, int Tip, int *KL, int *KH, Flag *Rytm, Pisen *P )
{
	int I=NajdiDef(NNam,Tip,&P->LocDigi);
	if( I>=EOK )
	{
		NastrDef *D=NastrAcc(&P->LocDigi,I);
		long S;
		if( AktHraj==&AYBufFalc ) S=ZavedSoub(D->Soubor,2);
		else S=ZavedSoub(D->Soubor,1);
		if( S==ERR ) S=ECAN;
		if( S<EOK ) return (int)S;
		UzijSoub(S);
		D->ISoub=S;
	}
	else return KON;
	*KL=0,*KH=0x7fff; /* bereme jakykoliv kanal */
	*Rytm=False; /* rytmika taky zabira cas */
	return I;
  return 0;
}
#endif

/* ---------------- editace nastroje */

/* LLHRAJ zapouzdreni sluzeb */

/* uroven orchestru - NastrDef */

const char *EditSNastr( const char *NNam, const WindRect *Orig, Pisen *P )
{
	(void)Orig;
	(void)P;
	return NNam;
}

void MazSNastr( void )
{
}

/* uroven orchestru */

const char *NastrInfo( const char *NNam, Pisen *P )
{
	(void)P;
	return NNam;
}

Err SaveHNastr( FILE *f, Pisen *P, const char *A ) /* uloz vsechny pouzite nastroje */
{
	(void)f,(void)A;
	(void)P;
	return EFIL;
}

void AktHNastr( void )
{
}

Err VymazNastr( char *N, Pisen *P )
{
	(void)N,(void)P;
	return EFIL;
}

Err KonSNastr( void )
{
	return EFIL;
}

Err CtiOrchestr( void )
{
	return EFIL;
}
Err PisOrchestr( void )
{
	return EFIL;
}

const char **SoupisSNastr( Pisen *P )
{
	(void)P;
	return NULL;
}

/* sekce DSP */

static int CAb,AbVal=-1;

#define LodSizeX 0x3e00L
#define LodSizeY 0x3e00L
#define LodSize (LodSizeX+LodSizeY)
#define DspWordSize 3

extern char LodCompo[];
extern long LodCompoSize;

Err ZacDsp( void )
{
	return EOK;
}

static long Hlasitost;
static Flag JeLod=False;

void HlasHudba( long VOut ) /* 0..1000 */
{
	(void)VOut;
}

static long FadeOutT;
static Flag JeFadeOut=False;

void ZacFadeOutHudba(int cas)
{
	if( JeLod && !TestHraje() )
	{
		FadeOutT=SystemCas()+1070;
		JeFadeOut=True;
		F_Povel=FPFade;
	}
}
void KonFadeOutHudba()
{
	if( JeFadeOut )
	{
		while( SystemCas()<FadeOutT ){}
		JeFadeOut=False;
	}
}

void FadeOutHudba( int cas )
{
	ZacFadeOutHudba(cas);
	KonFadeOutHudba();
}

Err ZacLod( void )
{
	Hlasitost=0x7FFFFFL;
	F_Povel=FPNic;
	JeLod=True;
	return EOK;
}
void KonDsp( void )
{
}
void KonLod( void )
{
	JeLod=False;
}

/* nejvyssi uroven */

void AYCtiCFG( const Info *Inf )
{
	(void)Inf;
}
Flag AYUlozCFG( Info *Inf )
{
	(void)Inf;
	return False;
}

Err DigiZacNastr( LLHraj *H, double Fr )
{
	FreqIs=Fr;
	AktHraj=H;
	return EOK;
}

Err OJinSNastr( LLHraj *Na, LLHraj *Z )
{
	(void)Na,(void)Z;
	return ECAN;
}

