/* digitalni hrani na Falconu */
/* SUMA 3/1993-3/1994 */
/* nejnizsi uroven - pro obecne pouziti */

#include <stdlib.h>
#include <stdio.h>
#include <sndbind.h>
#include <macros.h>
#include <sttos\dsplolev.h>
#include <sttos\sysvar.h>
#include "faldigia.h"
#include "llhraj.h"
#include "digill.h"
#include "3D\GRAVON\sndGiGOut.h"

/* FadeOuty: 18. 8., suggested by COMPO */

#define _TIMERD 1 /* zda bezi pomaly player v preruseni */
#define _TDA 1 /* zda je TimerD ve skutecnosti A */

#define TimerCDC ( *(char *)0xfffffa1dL )
#define TimerAC  ( *(char *)0xfffffa19L )

#define FBRamPomer 128

double FBufFreqDbl;

static const short FTichoObs[]={0};
static const NastrSoub FTichoSoub={"",FTichoObs,1};

FNastroj FalcKans[MaxFalcKan];

/* zakladni implementace hrani */
/* hlavni hraci procedury */
/* implementace pro F030 */


void F_FSetEffect( int A, int B, int Kan )
{ /* A a B jsou od 0 do 255 */
	FNastroj *N=&FalcKans[Kan];
	N->EffA=A;
	N->EffB=B;
}

void F_FSetNastr( int NI, int Kan, DynPole *P )
{
	if( NI>=0 )
	{
		const NastrDef *NN=NastrAcc(P,NI);
		FalcKans[Kan].NasDef=NN;
	}
	else
	{
		FalcKans[Kan].NasDef=NULL;
	}
}

void F_FEchoNastr( int NI, int Kan, DynPole *P )
{ /* automatick‚ efekty */
	FNastroj *N=&FalcKans[Kan];
	const NastrSoub *NS;
	if( NI>=0 )
	{
		const NastrDef *NN=NastrAcc(P,NI);
	  N->NasDef=NN;
		NS=AccDyn(&NSou,NN->IMSoubory[0]);
	}
	else
  {
	  N->NasDef=NULL;
    NS=&FTichoSoub;
  }
	N->EffA=NS->EffA;
	N->EffB=NS->EffB;
}

void F_FSetStereo( int VMin, int VMax, int Kan )
{
	FNastroj *K=&FalcKans[Kan];
	K->StereoMin=VMin,K->StereoMax=VMax;
}

static int FadeSpd=80;

int F_FyzTon( int Ton )
{
	Ton-=CisMinOkt*12;
	while( Ton<0 ) Ton+=12;
	while( Ton>=FyzOkt*12 ) Ton-=12;
	return Ton;
}

static void FFSetNastr( int Kan, int Vol, int Ton, Flag Legato )
{ /* verze pro Falcona */
	FNastroj *N=&FalcKans[Kan];
	if( Vol>0 ) /* Note On */
	{ /* stereo tabulky a hlasitost */
		/* Ton je 7 bit unsigned , max-min je 8 bit signed (max 0x101) */
		int v;
		long Rep,Krk,RepF;
		const NastrDef *NS=N->NasDef;
		const NastrSoub *NI;
		#define KrokLog 24
		Ton=F_FyzTon(Ton);
		if( !NS ) NI=&FTichoSoub;
		else NI=AccDyn(&NSou,NS->IMSoubory[Ton]);
		Krk=NI->TonKrok[Ton]; /* 32(.KrokLog) bit… */
		/* musime poslat hlasitosti */
		/* Ton je 7 bit unsigned , max-min je 8 bit unsigned (max 0xfe) */
		v=Ton*(N->StereoMax-N->StereoMin)/(12*FyzOkt)+N->StereoMin;
		/* v je 8 bitu signed */
		/* 8b * 7b je 15 b unsigned, musi vyjit 8b unsigned */
		N->LVol=((0x80-v)*Vol)>>7;
		N->PVol=((0x80+v)*Vol)>>7;
		N->Fade=0;
		Rep=NI->Repet; /* Rep musi byt 32.0 */
		RepF=0;
		if( (Krk>>KrokLog)>(NI->Delka-NI->Vynech) ) Krk=0;
		if( Rep>NI->Delka-NI->Vynech ) Rep=0;
		if( Krk==0 || Kan==0 && Krk>0 && Rep<=(Krk>>KrokLog) ) /* v ost. kanalech smi byt Rep==0 */
		{
			Rep=Krk>>KrokLog;
			RepF=(Krk&((1L<<KrokLog)-1))<<(32-KrokLog);
		}
		if( !Legato )
		{
			N->RNas = (short *)NI->Obs+(NI->Delka-NI->Vynech); /* konec nastroje */
			N->RPos = -(NI->Delka-NI->Vynech); /* offset vzdy zaporny */
			N->RPosF = 0;
		}
		N->RKrk = Krk>>KrokLog;
		N->RKrkF = (Krk&((1L<<KrokLog)-1))<<(32-KrokLog);
		N->RRep=Rep;
		N->RRepF=RepF;
	}
	ef( Vol==0 )
	{ /* Note Off */
		N->RNas=FTichoObs+lenof(FTichoObs); /* konec nastroje */
		N->RPos=-(int)lenof(FTichoObs); /* offset vzdy zaporny */
		N->RKrk=0;
		N->RRep=0;
		N->RPosF=0;
		N->RKrkF=0;
		N->RRepF=0;
		N->Fade=0;
	}
	else /* FadeOut */
	{
		N->Fade=FadeSpd;
	}
}

Flag F_FHrajTon( int Ton, int Vol, int Kan )
{
	FFSetNastr(Kan&0x3f,Vol,Ton,Kan&0x40);
	return False;
}

static Flag Pauza;

static CasT DelkaSkl;

/* oprava podle reklamace COMPO - 11.8. */

Flag F_Display=True;

Flag F_Mode2 = False;
int F_Mode=8; /* poŸet kan l… */
int F_Divis=CLK25K; /* ý¡zen¡ frekvence */

int F_Povel; /* r…zn‚ povely */

const int sampleSizeTgt = sizeof(short)*2;

static __inline short Sat16b( int x ) 
{
  if (x<SHRT_MIN) return SHRT_MIN;
  if (x>SHRT_MAX) return SHRT_MAX;
  return x;
}

const int volMusic = 2;

enum {EchoBufSize=4*1024};

struct EchoBuffer
{
  int buffer[2][EchoBufSize];

  int accum[2]; // accumulate left/right input in this sample
  int bufPos;
};

static struct EchoBuffer EchoA,EchoB;

static int EffPars[2][4]=
{
	{1000,700,1000,0},
	{ 330,400,1000,0},
};


static void EchoAddInput(struct EchoBuffer *echo, int lData, int rData)
{
  echo->accum[0] += lData, echo->accum[1]+= rData;
}

static void EchoSample(struct EchoBuffer *echo, int *effPars, int *lOut, int *rOut)
{
  // perform echo simulation
  int lIn = echo->accum[0];
  int rIn = echo->accum[1];
  int delay = effPars[0]; // in samples
  int feedback = effPars[1]; // in 1/1000
  int gain = effPars[2]; // in 1/1000
  int bufPos = echo->bufPos;

  int bufL = echo->buffer[0][bufPos];
  int bufR = echo->buffer[1][bufPos];

  delay = (int)(delay*FBufFreqDbl/32780);

  echo->buffer[0][(bufPos+delay)%EchoBufSize] = lIn+bufL*feedback/1000;
  echo->buffer[1][(bufPos+delay)%EchoBufSize] = rIn+bufR*feedback/1000;

  *lOut += bufL*gain/1000;
  *rOut += bufR*gain/1000;

  // reset input data (prepare next sample)
  echo->accum[0] = echo->accum[1] = 0;
  echo->bufPos= (bufPos+1)%EchoBufSize;
}

static Flag Echos = True;

static void MixChannels(void *buffer, size_t bufferSize)
{
  short *buffer16 = (short *)buffer;
  size_t s;
  size_t bufferSamples = bufferSize/sampleSizeTgt;
  for (s=0; s<bufferSamples; s++)
  {
    int left = 0;
    int right = 0;
    int c;
    for (c=0; c<MaxFalcKan; c++)
    {
      FNastroj *nastr = &FalcKans[c];
      short sample = nastr->RNas[nastr->RPos];
      long long pos64 = (((long long)nastr->RPos)<<32)|nastr->RPosF;
      long long krk64 = (((long long)nastr->RKrk)<<32)|nastr->RKrkF;
      pos64 += krk64;
      if (pos64>=0) // positive means over end, loop or terminate
      {
        long long rep64 = (((long long)nastr->RRep)<<32)|nastr->RRepF;
        pos64 -= rep64;
        if (pos64>=0) // if there is no repeat, return to the last sample
        {
          pos64 -= krk64;
        }
      }
      nastr->RPos = pos64>>32, nastr->RPosF = (unsigned)pos64;

      {
        int lVol = sample*nastr->LVol/255;
        int rVol = sample*nastr->PVol/255;
        left += lVol;
        right += rVol;

        EchoAddInput(&EchoA,lVol*nastr->EffA/255,rVol*nastr->EffA/255);
        EchoAddInput(&EchoB,lVol*nastr->EffB/255,rVol*nastr->EffB/255);
      }
    }
    EchoSample(&EchoA,EffPars[0],&left,&right);
    EchoSample(&EchoB,EffPars[1],&left,&right);

    buffer16[s*2+0] = Sat16b(left>>volMusic);
    buffer16[s*2+1] = Sat16b(right>>volMusic);
  }
}


enum doplnmode {DNormal,DZac,DKonec,DPauza};

static volatile Flag TimerKonec;

static size_t CompoMixerCallback(void *buffer, size_t bufferSize, int bufferFreq)
{
	enum doplnmode Mode = DNormal;
  // each buffer process high-level commands (song)
  int bufferOffset = 0;
  // we want 200 Hz freq. or better for high-level
  size_t chunk = bufferFreq/200/sampleSizeTgt*sampleSizeTgt;
  if (chunk>bufferSize) chunk = bufferSize;
  if (bufferSize==0) return 0;
  while (bufferOffset+chunk<=bufferSize)
  {
    if( Mode==DNormal )
    {
      // measure time based on samples processed
	    TimerADF(chunk/sampleSizeTgt*sec/bufferFreq); /* Zac i Kon - ticho */
    }

    // loop through all channels
    MixChannels((char *)buffer+bufferOffset,chunk);
    bufferOffset += chunk;
  }
  
  // now mix the channels
  return bufferOffset;
}

#if _TIMERD
	static int FGetVol( int GVol[MaxFyzKan] )
	{
		(void)GVol;
		return 8;
	}
	
	static CasT FGetCas( void ) /* zpozdeni */
	{
		return 0;
	}
#else
	#define FGetCas NULL
	#define FGetVol NULL
#endif

static Flag MamSnd=False;

#define DMA_TEST 1

#if DMA_TEST
	long TBuf[0x1000];
#endif

static int LockDivis;

int FGloballocksnd( void )
{
  return 0;
}

void FGlobalunlocksnd( void )
{
}

static int Locallocksnd( void )
{
	return 0;
}
static void Localunlocksnd( void )
{
}

void InitFFreq( int freq  )
{
	long FS;
  long FBRamFreq = freq/FBRamPomer;
	FBufFreqDbl=freq;
	AYBufFalc.TempStep=(sec/FBRamFreq);
	/* Fadeout m  prob¡hat 20 ms */
	/* za 20 ms dostaneme 0.02*FBRamFreq vzork… */
	/* rychlost Fadeoutu je tedy 1/(0.02*FBRamFreq) */
	/* neboli FadeSpd=50/FBRamFreq */
	/* oŸek vanì maxim ln¡ fadeout je 0.005 */
	/* m…§eme tedy j¡t nahoru o 7b */
	FS=FBRamFreq/50; /* na tolika vzorc¡ch prob¡h  fade-out */
	if( FS>FBRamPomer ) FS=FBRamPomer;
	FS=(64*128)/FS;
	/* FS by vych z¡ pod 128 */
	if( FS>127 ) FS=127;
	FadeSpd=(int)FS;
}

static Err ZacF030( int freq )
{
	InitFFreq(freq);
	if( Locallocksnd()<0 ) return ERR;
	return EOK;
}

static void VypniTimer( void )
{
}

static void PustZvuk( void )
{
	Localunlocksnd();
}

#if _TIMERD

static void IntrAbort( void )
{
	TimerKonec=True;
}

static void FPause( void )
{
	Pauza=True;
}

static void FContinue( void )
{
	Pauza=False;
}
static void FResetPau( void )
{ /* je v supervizoru */
}
#else
static void FPause( void ){}
static void FContinue( void ){}
static void FResetPau( void ){}

#endif


void F_EffPars( int E, const int *Pars )
{
	int i;
	int *P=EffPars[E];
	long P0;
	for( i=0; i<4; i++ ) P[i]=Pars[i];
	/* par. 0 (delay) z vis¡ na frekvenci */
	P0=(long)(P[0]*FBufFreqDbl/32780L);
	if( P0>1000 ) P0=1000;
	P[0]=(int)P0;
}

static long SetEffPars( void )
{
	return 0;
}

Err F_ZacHrajFBuf( CasT Cas, CasT Od, int freq )
{
	Err ret;
	int Kan;
	TimerKonec=False;
	Pauza=False;
	if( (ret=ZacF030(freq))<EOK ) return ret;
	if( (ret=ZacLod())>=EOK )
	{
		(void)Od;
		DelkaSkl=Cas;
		for( Kan=0; Kan<MaxFalcKan; Kan++ )
		{
			F_FSetStereo(-MinSt,MaxSt,Kan);
			F_FHrajTon(0,0,Kan);
			F_FSetEffect(0,0,Kan);
			F_FSetNastr(-1,Kan,NULL); /* ticho */
		}
		InitKonec();
		TimerKonec=False;
		SetEffPars();
    StartSoundWaveOut(freq,CompoMixerCallback);
		return 0;
	}
	return ret;
}

Flag F_CasujHrani( void )
{
	return False;
}
/* nejvyssi uroven */

Err F_KonHrajFBuf( void )
{
  EndSoundWaveOut();
	PustZvuk();
	KonLod();
	return EOK;
}

static Err ZacSNastrFBuf( void )
{
	return DigiZacNastr(&AYBufFalc,FBufFreqDbl);
}

static Err JinSNastrFBuf( void )
{
	return OJinSNastr(&AYBufFalc,AktHraj);
}

static const char *NazevFBuf( void )
{
	static char FBuf[60];
	sprintf(FBuf,"DSP %.1f Hz; %s",FBufFreqDbl,JmenoOrc(OrcNam));
	return FBuf;
}

static Err FZacHrajFBuf( CasT Del, CasT Od )
{
	Err ret;
	ret=F_ZacHrajFBuf(Del,Od,(int)FBufFreqDbl);
	return ret;
}

LLHraj AYBufFalc=
{
	0,
	MaxFalcKan,
	NazevFBuf,
	F_FSetNastr,F_FSetStereo,F_FSetEffect,
	F_FHrajTon,
	FZacHrajFBuf,F_CasujHrani,F_KonHrajFBuf, /* Zac muze byt Prek */
	ZacSNastrFBuf,KonSNastr,MazSNastr,JinSNastrFBuf,
	CtiOrchestr,PisOrchestr,EditSNastr,SoupisSNastr,
	ZacHNastr,DefHNastr,AktHNastr,SaveHNastr,KonHNastr,VymazNastr,
	NastrInfo,
	AYCtiCFG,AYUlozCFG,
	NULL,NULL,
	FGetVol,FGetCas,
	FPause,FContinue,FResetPau,
	HDelkaZneni,
};

int Speed = 1;