/* digitalni hrani na AY a Falconu */
/* SUMA 3/1993-3/1994 */
/* nejnizsi uroven - pro obecne pouziti */

#include <macros.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sndbind.h>
#include <stdlib.h>
#include <sttos\samples.h>
#include <stdc\memspc.h>
#include <stdc\fileutil.h>
/*#include "stdigia.h"*/
#include "llhraj.h"
#include "digill.h"

//#include <sttos\fixedp.h>

#define _TIMERA 1 /* bezi rychly play.- ST */
#define _TIMERD 1 /* zda bezi pomaly player v preruseni - jen pri DMA */
#define _TDA 1 /* zda je TimerD ve skutecnosti A */

#define INT_XBIOS 0 /* zda jsou v preruseni xbiosy */

#define LINEAR 1 /* indikatory - logar./linear. */

#define TimerCDC ( *(char *)0xfffa1dL )
#define TimerAC  ( *(char *)0xfffa19L )

PathName NastrP;

double FreqIs; /* sem musi dat pri zac. prekladu frekv. podle FrqAConst */

static void *NDmyalloc( lword L ){return myallocSpc(L,True,'NDef');}
static void *NSmyalloc( lword L ){return myallocSpc(L,True,'NSou');}

DynPole NSou={sizeof(NastrSoub),16,NDmyalloc,myfreeSpc};
DynPole NDef={sizeof(NastrDef ),16,NSmyalloc,myfreeSpc};

/* uroven souboru se samply */

void NastrojNazev( const char *N, PathName P )
{
	PathName SP;
	if( *NastrP )
	{
		strcpy(SP,NastrP);
		NajdiNazev(SP)[0]=0;
		AbsCesta(P,N,SP);
	}
	else strcpy(P,N);
}
void VytvorNastrSoub( char *N, const char *W )
{
	PathName SP;
	strcpy(SP,NastrP);
	NajdiNazev(SP)[0]=0;
	RelCesta(N,W,SP);
}

void InitNastrP( const char *N )
{
	FileName Dum;
	char *e;
	VyrobCesty(NastrP,Dum,N);
	e=NajdiExt(NajdiNazev(NastrP));
	if( e>NastrP && e[-1]=='.' ) e--;
	strcpy(e,".AVR");
}

static long NactiSoub( const char *NazSoub, int Resol )
{
	NastrSoub n,*N=&n;
	long FS;
	PathName P;
	long SI;
	avr_t H;
	void *NObs;
	NastrojNazev(NazSoub,P);
	NObs=ZvukLoad(&H,P);
	if( !NObs )
	{
		if( errno==ENOMEM ) return ERAM;
		ef( errno==EINVAL ) return ERR;
		else return EFIL;
	}
	if( H.avr_mode!=AVR_MONO ) ZmenStereo(&H,NObs);
	if( H.avr_resolution!=16 && H.avr_resolution!=8 ) return ERR;
	if( H.avr_midinote<0 ) return ERR;
	if( Resol==1 ) 
	{
		if( H.avr_resolution!=8 )
		{
			void *NO=ZmenRozl(&H,NObs,8);
			if( !NO ) {free(NObs);return ERAM;}
			NObs=NO;
		}
	}
	else
	{
		/* Resol==2 */
		if( H.avr_resolution!=16 )
		{
			void *NO=ZmenRozl(&H,NObs,16);
			if( !NO ) {free(NObs);return ERAM;}
			NObs=NO;
		}
	}
	strcpy(N->NazSoub,NazSoub);
	N->Obs=NObs;
	FS=DelkaCSam(&H); /* delka v jedotkach, length v bytech, mono 16 bitu */
	N->Delka=FS;
	if( H.avr_looping!=AVR_NON_LOOPING )
	{
		N->Vynech=DelkaCSam(&H)-LoopEndSam(&H); /* zarucene 16 bitu, mono */
		N->Repet=(LoopEndSam(&H)-LoopFirstSam(&H));
	}
	else N->Vynech=0,N->Repet=0;
	N->Uzito=False;
	N->Resol=Resol;
	N->SamFreq=CompoFreqSam(&H)/(1000/FreqDes);
	N->BasNota=H.avr_midinote;
	N->ProFreq=0;
	/*
	N->EffA=0;
	N->EffB=0;
	*/
	N->EffA=0x80;
	N->EffB=0x80;
	SI=InsDyn(&NSou,N);
	if( SI<0 )
	{
		freeSpc(NObs);
		return ERAM;
	}
	return SI;
}

const static double FTab[12]=
{
	2093.00,2217.46,2349.31,2489.01,2637.02,2793.82,
	2959.95,3135.95,3322.43,3520.00,3729.30,3951.06,
}; /* frekvence tonu - podle fyz. tab. okt. c4-h4 (7) */

static double ZjistiFreq( int Nota )
{
	enum {BasOkt=7};
	int OMK=Nota/12-BasOkt;
	double Frq=FTab[Nota%12];
	if( OMK<0 ) Frq/=1<<-OMK;
	else Frq*=1<<OMK;
	return Frq;
}

static void SpoctiKonv( NastrSoub *N )
{
	int PT;
	int O;
	double K1;
	double FMK=ZjistiFreq(N->BasNota);
	K1=((double)N->SamFreq/FreqDes)/FreqIs*0x10000L;
	if( AktHraj==&AYBufFalc ) K1*=0x100; /* na Falconu pou§¡v me 8.24 b */
	K1/=FMK;
	for( O=0; O<FyzOkt; O++ ) for( PT=0; PT<12; PT++ )
	{
		int off=FyzOkt-O-(FyzOkt-6);
		double FT;
		if( off>=0 ) FT=FTab[PT]/(1<<off);
		else FT=FTab[PT]*(1<<-off);
		N->TonKrok[O*12+PT]=(long)( K1*FT+0.5 );
	}
	N->ProFreq=FreqIs;
}

void RecalcFreq( double Frq )
{
	NastrSoub *N;
	long i;
	FreqIs=Frq;
	for( N=PrvniDyn(&NSou,&i); N; N=DalsiDyn(&NSou,N,&i) )
	{
		if( N->Uzito && N->ProFreq!=FreqIs ) SpoctiKonv(N); /* je-li uzit, je urcite i spocitan */
	}
}

void UzijSoub( long S )
{
	NastrSoub *NS=AccDyn(&NSou,S);
	NS->Uzito=True;
}

static long NajdiSoub( const char *Nam, int Resol )
{
	NastrSoub *N;
	long i;
	for( N=PrvniDyn(&NSou,&i); N; N=DalsiDyn(&NSou,N,&i) )
	{
		if( !strcmp(N->NazSoub,Nam) && N->Resol==Resol ) return i;
	}
	return ERR;
}
long ZavedSoub( const char *Nam, int Resol ) /* resol - 1 nebo 2 - v bytech */
{	/* vynech i repet ve velikosti souboru */
	long NI=NajdiSoub(Nam,Resol);
	if( NI>=EOK ) return NI;
	return NactiSoub(Nam,Resol);
}

CasT HDelkaZneni( Pisen *P, int Nas, int Ton )
{
	/* n stroj mus¡ bìt korektnØ zaveden */
	CasT del;
	double t,r,s;
	NastrDef *D=NastrAcc(&P->LocDigi,Nas);
	NastrSoub *N=AccDyn(&NSou,D->IMSoubory[Ton]);
	if( N->Repet ) return MAXCAS;
	t=(double)sec*N->Delka/(N->SamFreq/FreqDes);
	r=ZjistiFreq(N->BasNota); /* na z kladn¡m t¢nu */
	s=ZjistiFreq(Ton);
	del=t*r/s;
	return del;
}

static void ZrusSoub( long I )
{
	NastrSoub *N=AccDyn(&NSou,I);
	if( *N->NazSoub )
	{
		freeSpc((void *)N->Obs);
		N->Obs=NULL;
		*N->NazSoub=0;
	}
}

static void SetresSoub( void )
{
	int Z,D;
	for( Z=D=0; Z<NDyn(&NSou); D++,Z++ )
	{
		NastrSoub *N,*M;
		while( N=AccDyn(&NSou,Z),!*N->NazSoub )
		{
			Plati( !N->Obs );
			Z++;
			if( Z>=NDyn(&NSou) ) break;
		}
		M=AccDyn(&NSou,D);
		*M=*N;
	}
	NSou.UzitDel=D;
	for( Z=0; Z<NDyn(&NDef); Z++ )
	{
		NastrDef *D=AccDyn(&NDef,Z);
		int ni;
		for( ni=0; ni<FyzOkt*12; ni++ )
		{
			D->IMSoubory[ni]=-1;
		}
	}
}

void ZacSoub( void )
{
	ZacDyn(&NSou);
}

void KonSoub( void )
{
	long i;
	NastrSoub *S;
	for( S=PrvniDyn(&NSou,&i); S; S=DalsiDyn(&NSou,S,&i) )
	{
		ZrusSoub(i);
	}
	KonDyn(&NSou);
}

/* celkova sprava definic nastroju */

Err ZacHNastr( Pisen *P )
{
	long i;
	NastrSoub *S;
	NastrDef *D;
	for( S=PrvniDyn(&NSou,&i); S; S=DalsiDyn(&NSou,S,&i) )
	{
		S->Uzito=False;
	}
	for( D=PrvniDyn(&NDef,&i); D; D=DalsiDyn(&NDef,D,&i) )
	{
		D->Uzit=False;
	}
	for( D=PrvniDyn(&P->LocDigi,&i); D; D=DalsiDyn(&P->LocDigi,D,&i) )
	{
		D->Uzit=False;
	}
	return EOK;
}
void FAktHNastr( void ) /* zrus vsechny nepouzite cachovane soubory */
{
	static int sem=0;
	if( sem<=0 )
	{ /* zamez rekurzi - pou§¡v  se pýi £klidu pamØti */
		long i;
		NastrSoub *S;
		sem++; /* zamez rekurzi */
		for( S=PrvniDyn(&NSou,&i); S; S=DalsiDyn(&NSou,S,&i) )
		{
			if( *S->NazSoub && !S->Uzito ) ZrusSoub(i);
		}
		sem--;
	}
}

void KonHNastr( void )
{
	if( Rezerva )
	{
		long i;
		NastrSoub *S;
		for( S=PrvniDyn(&NSou,&i); S; S=DalsiDyn(&NSou,S,&i) )
		{
			if( *S->NazSoub ) ZrusSoub(i);
		}
	}
	SetresSoub();
}

/* uroven definic nastroju */

static long Tipy[0x100]; /* obsah tehle tabulky muze byt klidne nahodny */

int NajdiDef( const char *NNam, int Tip, DynPole *P )
{
	NastrDef *N;
	long i=( Tip>=0 && Tip<(int)lenof(Tipy) ) ? Tipy[Tip] : -1;
	if( i>=0 && i<NDyn(&NDef) ) /* tip by mohl vyjit */
	{
		N=AccDyn(&NDef,i);
		if( *N->Nazev && !strcmp(NNam,N->Nazev) )
		{
			if( i<0x4000 ) return (int)i; /* Tip nam vysel */
			else return ERR;
		}
	}
	for( N=PrvniDyn(&NDef,&i); N; N=DalsiDyn(&NDef,N,&i) )
	{
		if( *N->Nazev && !strcmp(NNam,N->Nazev) )
		{
			if( Tip>=0 && Tip<(int)lenof(Tipy) ) Tipy[Tip]=i;
			if( i<0x4000 ) return (int)i;
			else return ERR;
		}
	}
	if( P )
	{
		for( N=PrvniDyn(P,&i); N; N=DalsiDyn(P,N,&i) )
		{
			if( *N->Nazev && !strcmp(NNam,N->Nazev) )
			{
				if( i<0x4000 ) return (int)i|0x4000;
				else return ERR;
			}
		}
	}
	return ERR;
}

static void NastavTip( long i, int Tip )
{
	if( Tip>=0 && Tip<(int)lenof(Tipy) ) Tipy[Tip]=i;
}
