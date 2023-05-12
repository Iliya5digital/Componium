/* prehravani ulozenych COC souboru */
/* SUMA 3/1993-3/1993 */

#include <windows.h>
#include <MMSystem.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <sndbind.h>
#include <stdc\dynpole.h>
#include <stdc\memspc.h>
#include <stdc\fileutil.h>
#include "comcompo.h"
#include "digill.h"
#include "cocload.h"

extern int Speed;

/* nastroje */
static void *NSAlloc( lword A ){return mallocSpc(A,'LIns');}
static DynPole LocDef={sizeof(NastrDef ),16,NSAlloc,freeSpc};

/* melodie */

static MPovel *PisZac=NULL;

void PustPisen( void )
{
	if( PisZac ) freeSpc(PisZac),PisZac=NULL;
	KonDyn(&NDef);
	KonDyn(&LocDef);
	KonSoub();
}

extern struct echoitem
{
	const char *Name;
	int EchoA,EchoB;
} EchoTab[];

static void ZjistiEfekty( long IS )
{
	NastrSoub *N=AccDyn(&NSou,IS);
	const char *Nam=NajdiNazev(N->NazSoub);
	struct echoitem *ei;
	for( ei=EchoTab; ei->Name; ei++ )
	{
		if( !strcmpi(ei->Name,Nam) )
		{
			int A=ei->EchoA,B=ei->EchoB;
			A*=1;if( A>255 ) A=255;
			B*=1;if( B>255 ) B=255;
			N->EffA=A;
			N->EffB=B;
			return;
		}
	}
	N->EffA=0;
	N->EffB=0;
}

int NactiPisen( FILE *f, const char *Root, const char *altRoot, int freq )
{
	PathName SR,AR;
	char *PP;
	long c;
	int ret=-1;
	ZacDyn(&LocDef);
	ZacDyn(&NDef);
	ZacSoub();

	strcpy(SR,Root);PP=strrchr(SR,'\\');if( PP ) *PP=0;
	strcpy(AR,altRoot);PP=strrchr(AR,'\\');if( PP ) *PP=0;

  c = fgetbel(f);
  if (c<0) goto Error;
	/* nastroje se do pole umistuji ridce - to nevadi, nedef. nastroje nepouzivame */
	if( AlokDyn(&LocDef,256)<0 ) goto Error;
	if( AlokDyn(&NDef,256)<0 ) goto Error;
	FreqIs=freq;
	while( --c>=0 )
	{
		long InstrN,IS;
		char CN[128],*C;
		NastrDef *N;
    InstrN = fgetbel(f);
    if (InstrN<0) goto Error;
		if( fread(&CN,sizeof(CN),1,f)!=1 ) goto Error;
		C=NajdiNazev(CN);
		Plati( (InstrN&0x3fff)<256 );
		if( InstrN&0x4000 ) N=AccDyn(&LocDef,InstrN&0x3fff);
		else N=AccDyn(&NDef,InstrN);
		AbsCesta(N->Soubory[0],C,SR);
		*N->Nazev=0;
		IS=ZavedSoub(N->Soubory[0],2);
		if( IS<0 )
    {
  		AbsCesta(N->Soubory[0],C,AR);
		  IS=ZavedSoub(N->Soubory[0],2);
		  if( IS<0 )
      {
        goto Error;
      }
    }

    {
		  int ni;
		  for( ni=0; ni<FyzOkt*12; ni++ )
		  {
			  N->IMSoubory[ni]=IS;
		  }
    }
		N->Uzit=True;
		ZjistiEfekty(IS);
		UzijSoub(IS);
	}
  c = fgetbel(f);
  if (c<0) goto Error;
	PisZac=mallocSpc(c,'MCoc');
	if( !PisZac ) goto Error;
	if( fread(PisZac,c,1,f)!=1 ) goto Error;
	ret=0;
	Error:
	if( ret<0 )
	{
		PustPisen();
	}
	else
	{
		InitFFreq(freq);
		RecalcFreq(FBufFreqDbl); /* pýepoŸ¡tej si to */
	}
	return ret;
}

/* min. implementace playeru */

static const MPovel *_Pis;

static volatile Flag KonecHrani;

Flag AutoRepeat=True;
Flag HoverFormat = False;
Flag AutoEffects=True;

Flag TestHraje( void ){return KonecHrani;}

void InitKonec( void ){KonecHrani=False;}

static long TCnt; /* citadlo dalsi udalosti */

#define GetPisW(PisB) (PisB+=2,((((unsigned char)PisB[-2])<<8)|(unsigned char)PisB[-1]))

CasT TimerADF( CasT TStep ) /* play, fwd cue */
{
  int noteOffset = HoverFormat ? -12 : 0;
  int tickCoef = HoverFormat ? 10 : 1;
	const signed char *PisB=_Pis;
	TCnt-=TStep;
	while( TCnt<=0 )
	{
		int Kan;
		int N;
		switch( *PisB++ )
		{
			/* hlasitost je -128 (Fade) .. 0 (NoteOff) ..127 */
			case MNotaZac: Kan=*PisB++,N=*PisB++,F_FHrajTon(N+noteOffset,*PisB++,Kan);break;
			case MSync: PisB++,TCnt+=GetPisW(PisB)*tickCoef;break;
			case MKonec:
				if( AutoRepeat )
				{
					PisB=PisZac;
					break;
				}
				else
				{
					KonecHrani=True;--PisB;_Pis=PisB; /* zustane MKonec */
					return 0;
				}
			case MStereo: Kan=*PisB++,N=*PisB++,F_FSetStereo(N,*PisB++,Kan);break;
			case MNastr:
        {
          int Nastr;
          Kan=*PisB++;
          Nastr = GetPisW(PisB);
          if (AutoEffects) F_FEchoNastr(Nastr,Kan,&LocDef);
          else F_FSetNastr(Nastr,Kan,&LocDef);
        }
        break;
			case MEfekt:
        {
          int Eff;
          Kan=*PisB++;
          N=(unsigned char )*PisB++;
          Eff = (unsigned char )*PisB++;
          if (!AutoEffects) AktHraj->SetEfekt(N,Eff,Kan);
        }
        break;
			default: PisB++,GetPisW(PisB);break;
		}
	}
	_Pis=PisB;
	return TCnt;
	#undef BPis
}

long TimerAD( void )
{
	TimerADF(Speed*AYBufFalc.TempStep);
	return Speed;
}

long TimerAPauza( void )
{
	return 0;
}

int ZacHrajPisen( int freq )
{
	Err ret;
	Speed=1;
	_Pis=PisZac;
	ret=F_ZacHrajFBuf(0,0,freq);
	return ret;
}

void AbortHraje( void ){}
/* nepouziva se */
Flag HrajCinnost(){return False;}
Flag TestAbort( void ){return False;}

void KonHrajPisen( void )
{
	F_KonHrajFBuf();
}