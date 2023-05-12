/* hrani na MIDI */
/* SUMA 6/1993-6/1993 */

#include <macros.h>
#include <tos.h>
#include <string.h>
#include <stdio.h>
#include <stdc\infosoub.h>
#include <stdc\dynpole.h>
#include <gem\interf.h>

#include "compo.h"
#include "llhraj.h"
#include "hracompo.h"
#include "ramcompo.h"
#include "digimus.h"

enum {MidiKan=16};

enum {MHan=3,MOSHan=4};

enum
{
	FreqDW=1000, /* jakou chceme frekvenci D */
	DDelay=200,
	FrqDConst=(int)(2457600L/DDelay/FreqDW), /* perioda, ve ktere se provadi kroky ridiciho hrani */
	#if !_TIMERD
		FreqD=70,
	#else
		FreqD=(int)(2457600L/DDelay/FrqDConst), /* skut. frekvence */
	#endif
	TempStep=sec/FreqD, /* krok, o ktery se meri v rid. hrani */
};

typedef struct
{
	char Nazev[32];
	int PrgNum;
	int OktPos;
	int NucTon;
	Flag Rytmika;
} MidiNastr;

/* implementace spravy nastroju */
/* pouziva stejne metody jako AY8BITLL - krasne by se objektovalo */

/* vsechna data musi byt pro vsechny syntezatory */

enum {NSyntMax=8};


typedef struct
{
	DynPole NDef;
	char Nazev[80];
	PathName OrcNam;
	Flag PlatnyOrcNam; /* Platnost nazvu */
	Flag NDZmena; /* zmena v orchestru */
	int KanOd,KanDo; /* rozsah kanalu */
	byte InitSekv[256];
	int LenIS;
	Flag Platny; /* zda je platny */
	Flag Uzity; /* zda ho chceme pouzivat */
} SyntInfo;

static void *DPmyalloc( lword L ){return myalloc(L,True);}
const static DynPole NDDef={sizeof(MidiNastr),16,DPmyalloc,myfree};
static SyntInfo NDs[NSyntMax];

static long volatile IntrCnt;
/* */

static long PrazdnyNastr( SyntInfo *S )
{
	MidiNastr n;
	long NN;
	strcpy(n.Nazev,"");
	n.PrgNum=-1;
	n.OktPos=0;
	n.NucTon=-1;
	n.Rytmika=False;
	NN=InsDyn(&S->NDef,&n);
	if( NN<0 ) return ERAM;
	return NN;
}

static void MazSNastrMidi( SyntInfo *S )
{
	S->Platny=False;
	KonDyn(&S->NDef);
}

static Err CtiSNastrMidi( const char *NNaz, SyntInfo *Snt )
{
	Err ret=EOK;
	FILE *f;
	char *R,*S;
	byte *IS;
	int C;
	MazSNastrMidi(Snt);
	Snt->Platny=True;
	ZacDyn(&Snt->NDef);
	if( !*NNaz ) return OK;
	f=fopen(NNaz,"r");
	if( !f ) return OK;
	if( !Rezerva ) if( setvbuf(f,NULL,_IOFBF,9*1024)==EOF ) {ret=ERAM;goto Konec;}
	if( AlokDyn(&Snt->NDef,32)<0 ) {ret=ERAM;goto Konec;}
	R=CtiRadek(f);if( !R ) {ret=EFIL;goto Konec;}
	strlncpy(Snt->Nazev,R,sizeof(Snt->Nazev));
	R=CtiRadek(f);if( !R ) {ret=EFIL;goto Konec;}
	IS=Snt->InitSekv;
	while( (C=SCtiCislo(&R))>=MAXERR ) *IS++=C;
	Snt->LenIS=(int)(IS-Snt->InitSekv);
	for(;;)
	{
		long NH;
		MidiNastr *N;
		R=CtiRadek(f);if( !R ) break;
		S=SCtiSlovo(&R);
		NH=PrazdnyNastr(Snt);if( NH<OK ) {ret=(Err)NH;goto Konec;}
		N=AccDyn(&Snt->NDef,NH);
		strlncpy(N->Nazev,S,sizeof(N->Nazev));
		C=SCtiCislo(&R);if( C>=MAXERR ) N->PrgNum=C;
		C=SCtiCislo(&R);if( C>=MAXERR ) N->OktPos=C;
		C=SCtiCislo(&R);if( C>=MAXERR ) N->NucTon=C;
		C=SCtiCislo(&R);if( C>=MAXERR ) N->Rytmika=C!=0;
	}
	Snt->NDZmena=False;
	ret=OK;
	Konec:
	fclose(f);
	return ret;
}

static Err FPisSNastrMidi( const char *N, SyntInfo *S )
{
	Err ret=EOK;
	long i;
	int C;
	FILE *f;
	byte *IS;
	if( !*N ) return EFIL;
	f=fopen(N,"w");
	if( !f ) return EFIL;
	if( !Rezerva ) if( setvbuf(f,NULL,_IOFBF,9*1024)==EOF ) {ret=ERAM;goto Konec;}
	if( EOF==fprintf(f,"%s\n",S->Nazev) ) {ret=EFIL;goto Konec;}
	IS=S->InitSekv;
	for( C=S->LenIS; C>0; C-- ) if( EOF==fprintf(f,"$%x",*IS++) ) {ret=EFIL;goto Konec;}
	if( EOF==fprintf(f,"\n") ) {ret=EFIL;goto Konec;}
	for( i=0; i<NDyn(&S->NDef); i++ )
	{
		MidiNastr *N=AccDyn(&S->NDef,i);
		if( N->Nazev[0] )
		{
			if
			(
				EOF==fprintf
				(
					f,
					"%s %d %d %d %d\n",
					N->Nazev,
					N->PrgNum,N->OktPos,N->NucTon,N->Rytmika
				)
			) {ret=EFIL;goto Konec;}
		}
	}
	S->NDZmena=False;
	Konec:
	fclose(f);
	return ret;
}

static Err PisSNastrMidi( const char *N, SyntInfo *S )
{
	int a;
	while( S->NDZmena )
	{
		if( !S->PlatnyOrcNam )
		{
			PathName OrcP;
			FileName OrcF;
			VsePrekresli();
			VyrobCesty(OrcP,OrcF,S->OrcNam);
			if( !EFSel(FreeString(ULOMIDFS),OrcP,OrcF,S->OrcNam) ) return ECAN;
			S->PlatnyOrcNam=True;
		}
		a=AlertRF(CLORCHA,1,JmenoOrc(N));
		if( a==1 )
		{
			Err ret=FPisSNastrMidi(S->OrcNam,S);
			if( ret<OK ) return ret;
			else Chyba(ret);
		}
		ef( a==3 ) return ECAN;
		ef( a==2 ) return EOK;
	}
	return EOK;
}

static Err CtiOrchestrMidi( SyntInfo *S )
{
	PathName OrcP;
	FileName OrcF;
	PathName OrcNew;
	VyrobCesty(OrcP,OrcF,S->OrcNam);
	VsePrekresli();
	if( EFSel(FreeString(CTIMIDFS),OrcP,OrcF,OrcNew) )
	{
		Err ret;
		VsePrekresli();
		if( S->NDZmena )
		{
			Err ret;
			ret=PisSNastrMidi(S->OrcNam,S);
			if( ret<OK ) return ret;
		}
		strcpy(S->OrcNam,OrcNew);
		S->PlatnyOrcNam=True;
		ret=CtiSNastrMidi(OrcNew,S);
		if( ret<OK ) return ret;
		return OK;
	}
	return ECAN;
}
static Err PisOrchestrMidi( SyntInfo *S )
{
	PathName OrcP;
	FileName OrcF;
	VyrobCesty(OrcP,OrcF,S->OrcNam);
	VsePrekresli();
	if( EFSel(FreeString(ULOMIDFS),OrcP,OrcF,S->OrcNam) )
	{
		Err ret;
		VsePrekresli();
		S->PlatnyOrcNam=True;
		ret=FPisSNastrMidi(S->OrcNam,S);
		if( ret<OK ) return ret;
		return OK;
	}
	return ECAN;
}

static SyntInfo *VyberSynt( const char *NNam )
{
	/* musi umet dany nastroj */
	(void)NNam;
	if( NDs[0].PlatnyOrcNam ) return NDs;
	return NULL;
}

static Err CtiOrchestrMds( void )
{
	SyntInfo *S=VyberSynt(NULL);
	if( !S ) return ECAN;
	return CtiOrchestrMidi(S);
}
static Err PisOrchestrMds( void )
{
	SyntInfo *S=VyberSynt(NULL);
	if( !S ) return ECAN;
	return PisOrchestrMidi(S);
}

static Err ZacSNastrMidi( SyntInfo *S )
{
	Err ret;
	S->Platny=True;
	S->NDef=NDDef;
	ZacDyn(&S->NDef);
	ret=CtiSNastrMidi(S->OrcNam,S);
	if( ret<OK ) {Chyba(ret);MazSNastrMidi(S);}
	return OK;
}

static Err KonSNastrMidi( SyntInfo *S )
{
	Err ret=PisSNastrMidi(S->OrcNam,S);
	if( ret!=OK ) return ret;
	MazSNastrMidi(S);
	return OK;
}

static Err ZacSNastrMds( void )
{
	int si;
	for( si=0; si<NSyntMax; si++ )
	{
		SyntInfo *S=&NDs[si];
		if( S->PlatnyOrcNam )
		{
			Err ret=ZacSNastrMidi(S);
			if( ret<EOK ) return ret;
		}
	}
	AktHraj=&Midi;
	return EOK;
}

static Err KonSNastrMds( void )
{
	int si;
	for( si=0; si<NSyntMax; si++ )
	{
		SyntInfo *S=&NDs[si];
		if( S->Platny )
		{
			Err ret=KonSNastrMidi(S);
			if( ret<EOK ) return ret;
		}
	}
	AktHraj=NULL;
	return EOK;
}
static void MazSNastrMds( void )
{
	int si;
	for( si=0; si<NSyntMax; si++ )
	{
		SyntInfo *S=&NDs[si];
		if( S->Platny ) MazSNastrMidi(S);
	}
}

static Err JinSNastrMidi( void )
{
	if( AktHraj==&Midi ) return EOK;
	return JinSNastr(&Midi,AktHraj);
}

static Flag EdNOktHor( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *F=RscTree(MIDNASD);
	IncTed(F,MIDOKTT,+4);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag EdNOktDol( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *F=RscTree(MIDNASD);
	DecTed(F,MIDOKTT,-4);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Button EdNTlac[]=
{
	EdNOktHor,MIDOKTHB,
	EdNOktDol,MIDOKTDB,
	NULL,
};

static int EdNasD( long NN, const char *Nam, Flag PovolZmJmena, const WindRect *Orig, SyntInfo *S )
{
	MidiNastr *D=AccDyn(&S->NDef,NN);
	int r;
	OBJECT *DD=RscTree(MIDNASD);
	if( PovolZmJmena ) DD[MIDNAZT].ob_flags|=EDITABLE;
	else DD[MIDNAZT].ob_flags&=~EDITABLE;
	if( !Nam ) SetMezStrTed(TI(DD,MIDNAZT),D->Nazev);
	else SetMezStrTed(TI(DD,MIDNAZT),Nam);
	SetMIntTed(TI(DD,MIDPROGT),D->PrgNum);
	SetMIntTed(TI(DD,MIDPTONT),D->NucTon);
	SetIntTed(TI(DD,MIDOKTT),D->OktPos);
	for(;;)
	{
		if( PovolZmJmena && !*D->Nazev ) r=EFForm(DD,EdNTlac,NULL,MIDNAZT,Orig);
		else r=EFForm(DD,EdNTlac,NULL,MIDPROGT,Orig);
		if( r!=MIDOKB ) return ECAN;
		Nam=MezStrTed(TI(DD,MIDNAZT));
		if( PovolJmeno(Nam) ) break;
		VsePrekresli();
		AlertRF(NAZPOVA,1);
	}
	if( strcmp(Nam,D->Nazev) ) strlncpy(D->Nazev,Nam,sizeof(D->Nazev)),S->NDZmena=True;
	r=(int)MIntTed(TI(DD,MIDPROGT));
	if( D->PrgNum!=r ) S->NDZmena=True,D->PrgNum=r;
	r=(int)IntTed(TI(DD,MIDOKTT));
	if( D->OktPos!=r ) S->NDZmena=True,D->OktPos=r;
	r=(int)MIntTed(TI(DD,MIDPTONT));
	if( D->NucTon!=r ) S->NDZmena=True,D->NucTon=r;
	r=D->NucTon>=0;
	if( D->NucTon!=r ) S->NDZmena=True,D->Rytmika=r;
	return EOK;
}

static int NajdiDefMidi( const char *NNam, int Tip, SyntInfo *S )
{
	long i;
	MidiNastr *N;
	(void)Tip;
	for( N=PrvniDyn(&S->NDef,&i); N; N=DalsiDyn(&S->NDef,N,&i) )
	{
		if( !strcmp(NNam,N->Nazev) )
		{
			if( i>0x7fff ) return ERR;
			return (int)i;
		}
	}
	return KON;
}

static void SetresNastr( SyntInfo *S )
{
	long L=NDyn(&S->NDef);
	for(;;)
	{
		MidiNastr *N;
		if( L<=0 ) break;
		N=AccDyn(&S->NDef,L-1);
		if( *N->Nazev ) break;
		L--;
	}
	S->NDef.UzitDel=L;
}

static const char *EditSNastrMidi( const char *NNam, const WindRect *Orig, SyntInfo *S )
{
	long DI=ERR;
	int ret;
	Flag Pridan=False;
	MidiNastr *N;
	if( NNam ) DI=NajdiDefMidi(NNam,-1,S);
	if( DI<OK )
	{
		DI=PrazdnyNastr(S);
		if( DI<OK ) return ERRP;
		Pridan=True;
	}
	N=AccDyn(&S->NDef,DI);
	ret=EdNasD(DI,NNam,True,Orig,S);
	if( ret>=OK ) return N->Nazev;
	if( Pridan ) N->Nazev[0]=0,SetresNastr(S);
	if( ret==ECAN ) return NULL;
	return ERRP;
}

static const char *EditSNastrMds( const char *NNam, const WindRect *Orig, Pisen *P )
{
	SyntInfo *S=VyberSynt(NNam);
	(void)P;
	if( !S ) return NNam;
	else return EditSNastrMidi(NNam,Orig,S);
}

static const char **SoupisSNastrMidi( SyntInfo *S )
{
	long I,NN;
	MidiNastr *N;
	const char **BV;
	const char **PV;
	NN=NDyn(&S->NDef);
	BV=myalloc(sizeof(*BV)*(NN+1),False);
	if( !BV ) return NULL;
	PV=BV;
	for( N=PrvniDyn(&S->NDef,&I); N; N=DalsiDyn(&S->NDef,N,&I) )
	{
		const char *Naz=N->Nazev;
		if( *Naz ) *PV++=Naz;
	}
	*PV=NULL;
	return BV;
}

enum {MaxNas=1024};

static void MergeNas( const char **B, const char **b )
{
	int C=MaxNas-1;
	while( *B ) B++,C--;
	while( *b && C>0 ) *B++=*b++,C--;
	*B++=NULL;
}

static const char **SoupisSNastrMds( Pisen *P )
{
	const char **BV=myalloc(MaxNas*sizeof(*BV),False);
	int si;
	if( !BV ) return BV;
	(void)P;
	BV[0]=NULL;
	for( si=0; si<NSyntMax; si++ )
	{
		SyntInfo *S=&NDs[si];
		if( S->Platny && S->Uzity )
		{
			const char **bv=SoupisSNastrMidi(S);
			if( !bv ) {Chyba(ERAM);break;}
			else MergeNas(BV,bv);
		}
	}
	return BV;
}

static Err ZacHNastrMidi( void ){return EOK;}
static void AktHNastrMidi( void ){}
static void KonHNastrMidi( void ){}

static int DefHNastrMidi( const char *NNam, int Tip, SyntInfo **SR )
{
	int si;
	for( si=0; si<NSyntMax; si++ )
	{
		SyntInfo *S=&NDs[si];
		if( S->Platny && S->Uzity )
		{
			long i=NajdiDefMidi(NNam,Tip,S);
			if( i>=EOK )
			{
				if( i>0x7fff ) return ERR;
				*SR=S;
				return (int)i;
			}
			if( i!=KON ) return (Err)i;
		}
	}
	return KON;
}

#define MidiIns(i) ((i)&0xfff)
#define MidiKan(i) ((i)>>12 ) /* tj tri bity - pri signed */

static int KodeInsKan( int i, SyntInfo *S )
{
	int si;
	if( i>0xfff ) return ERR;
	si=(int)(S-NDs);
	if( si>7 ) return ERR;
	return (si<<12)|i;
}


static int DefHNastrMds( const char *NNam, int Tip, int *KL, int *KH, Flag *Rytm, Pisen *P )
{
	SyntInfo *S;
	MidiNastr *MN;
	int Ins=DefHNastrMidi(NNam,Tip,&S);
	(void)P;
	if( Ins<EOK ) return Ins;
	MN=AccDyn(&S->NDef,Ins);
	*Rytm=MN->Rytmika;
	*KL=S->KanOd;
	*KH=S->KanDo;
	return KodeInsKan(Ins,S);
}

static Err VymazNastrMidi( char *N, SyntInfo *S )
{
	long NN=NajdiDefMidi(N,-1,S);
	MidiNastr *ND;
	if( NN<0 ) return ECAN;
	ND=AccDyn(&S->NDef,NN);
	ND->Nazev[0]=0,SetresNastr(S);
	S->NDZmena=True;
	return OK;
}

static Err VymazNastrMds( char *N, Pisen *P )
{
	SyntInfo *S=VyberSynt(N);
	if( !S ) return ECAN;
	(void)P;
	return VymazNastrMidi(N,S);
}

static const char *NastrInfoMidi( const char *NNam, Pisen *P )
{
	SyntInfo *S; /* ktery vystup */
	long NN=DefHNastrMidi(NNam,-1,&S);
	static char NB[128];
	char *NBF=NB;
	strcpy(NBF,NNam);
	if( NN>=0 )
	{
		MidiNastr *ND=AccDyn(&S->NDef,NN);
		NBF+=strlen(NBF)+1;
		if( ND->PrgNum>=0 ) sprintf(NBF,"Prog %3d",ND->PrgNum);
		ef( ND->NucTon>=0 ) sprintf(NBF,"R %3d",ND->NucTon);
		else strcpy(NBF,"");
	}
	else
	{
		NBF+=strlen(NBF)+1,strcpy(NBF,"");
	}
	(void)P;
	return NB;
}

/* implementace hrani */

static int MidiPut( char M )
{
	while( Bcostat(MOSHan)==0 );
	Bconout(MHan,M);
	return 0;
}

static MidiNastr *MidiSIns[MidiKan];
static SyntInfo *MidiSKan[MidiKan];
static int MidiIns[MidiKan];
static int MidiTon[MidiKan];

static void SetNastrMidi( int Nas, int Kan, DynPole *P )
{
	if( Nas>=0 )
	{
		SyntInfo *S=&NDs[MidiKan(Nas)];
		MidiSIns[Kan]=AccDyn(&S->NDef,MidiIns(Nas));
		MidiSKan[Kan]=S;
	}
	else MidiSIns[Kan]=NULL,MidiSKan[Kan]=NULL;
	(void)P;
}

static void SetStereoMidi( int VMin, int VMax, int Kan ){(void)VMin,(void)VMax,(void)Kan;}

static Flag HrajTonMidi( int Ton, int Vol, int Kan )
{
	int *MI;
	int MV=Vol>>1;
	Flag ret=False;
	MidiNastr *MN;
	Kan&=0x3f;
	MN=MidiSIns[Kan];
	if( MN && MV>0 ) /* NoteOn */
	{
		if( MN->Rytmika ) Kan=15;
		MI=&MidiIns[Kan];
		if( MN->NucTon>=0 ) Ton=MN->NucTon;
		else Ton+=MN->OktPos*12+12; /* globalni korekce */
		if( MN->PrgNum>=0 )
		{
			int PN=MN->PrgNum;
			if( *MI!=PN ) MidiPut(0xc0+Kan),MidiPut(PN),*MI=PN;
		}
		if( Ton!=0 )
		{
			MidiPut(0x90+Kan),MidiPut(Ton),MidiPut(MV); /* Note On */
			if( MN->Rytmika ) /* rytm. hned vypni - neobsazuje kanal */
			{
				MidiPut(0x90+Kan),MidiPut(Ton),MidiPut(0); /* Note Off */
				ret=True;
				MidiTon[Kan]=-1;
			}
			else MidiTon[Kan]=Ton;
		}
	}
	else
	{ /* NoteOff,FadeOut - predpoklada 1 kanal/1 ton */
		int *MT=&MidiTon[Kan];
		if( *MT>=0 ) /* vypnout muze FOut nebo NoteOff */
		{
			MidiPut(0x90+Kan),MidiPut(*MT),MidiPut(0); /* Note Off */
			*MT=-1;
		}
	}
	return ret;
}

static const char *NazevMidi( void )
{
	static char FBuf[256];
	int si;
	strcpy(FBuf,"MIDI - ");
	for( si=0; si<NSyntMax; si++ )
	{
		SyntInfo *S=&NDs[si];
		if( S->Platny && S->Uzity )
		{
			strcat(FBuf,S->Nazev);
			strcat(FBuf,", ");
			if( strlen(FBuf)>=180 ) break;
		}
	}
	FBuf[strlen(FBuf)-2]=0; /* odstran ", " */
	return FBuf;
}

/* */

void MidiIntr( void ) /* 1 kHz */
{
	IntrCnt++;
}


static Err ZacHrajMidi( CasT Cas, CasT Od )
{
	long IC=0;
	int i;
	IntrCnt=0;
	(void)Cas,(void)Od;
	for( i=0; i<MidiKan; i++ )
	{
		MidiIns[i]=-1,MidiTon[i]=-1;
		MidiSIns[i]=NULL,MidiSKan[i]=NULL;
	}
	InitKonec();
	Xbtimer(0,7,FrqDConst,IntrMSync);
	MidiPut(0xfa);
	{
		int si;
		for( si=0; si<NSyntMax; si++ )
		{
			SyntInfo *S=&NDs[si];
			if( S->PlatnyOrcNam && S->Uzity )
			{
				byte *IS;
				int C;
				IS=S->InitSekv;
				for( C=S->LenIS; C>0; C-- ) MidiPut(*IS++);
			}
		}
	}
	do
	{
		long ICN;
		while( (ICN=IntrCnt)<=IC );
		TimerADF(ICN-IC);
		IC=ICN;
	}
	while( !TestHraje() && !TestAbort() );
	/* zrusit visici tony */
	for( i=0; i<MidiKan; i++ )
	{
		int N=MidiTon[i];
		if( N>0 ) MidiPut(0x90+i),MidiPut(N),MidiPut(0);
	}
	MidiPut(0xfc);
	Xbtimer(0,0,0,ERRP);
	return EOK;
}

static Err KonHrajMidi( void ){return EOK;}

static const char OrcNazev[]="MidiVoices";

static void CtiCFGMidi( const Info *Inf )
{
	int si;
	for( si=0; si<NSyntMax; si++ )
	{
		SyntInfo *S=&NDs[si];
		const char *r=Inf ? GetText(Inf,OrcNazev,si) : NULL;
		if( !r )
		{
			strcpy(S->OrcNam,CompoRoot);
			strcat(S->OrcNam,"*.MDV");
			S->PlatnyOrcNam=False;
			S->KanOd=0;
			S->KanDo=15;
			S->Uzity=False;
		}
		else
		{
			char *K=strchr(r,' ');
			if( K ) *K++=0,sscanf(K,"%d %d %d",&S->KanOd,&S->KanDo,&S->Uzity);
			else S->KanOd=0,S->KanDo=15,S->Uzity=False;
			AbsCesta(S->OrcNam,r,CompoRoot);
			S->PlatnyOrcNam=True;
		}
	}
}
static Flag UlozCFGMidi( Info *Inf )
{
	int si;
	int ci=0;
	for( si=0; si<NSyntMax; si++ )
	{
		SyntInfo *S=&NDs[si];
		if( S->PlatnyOrcNam )
		{
			char Buf[128];
			PathName r;
			RelCesta(r,S->OrcNam,CompoRoot);
			sprintf(Buf,"%s %d %d %d",r,S->KanOd,S->KanDo,S->Uzity);
			if( !SetText(Inf,OrcNazev,Buf,ci++) ) return False;
		}
	}
	MazIPolOd(Inf,OrcNazev,ci);
	return True;
}

static Flag SpecInfoMidi( SpcInfo *SI, int i )
{
	if( i>=0 && i<NSyntMax )
	{
		SyntInfo *S=&NDs[i];
		if( S->PlatnyOrcNam )
		{
			strlncpy(SI->Naz,S->OrcNam,sizeof(SI->Naz));
			SI->KOd=S->KanOd;
			SI->KDo=S->KanDo;
			SI->Uzivat=S->Uzity;
			return True;
		}
	}
	*SI->Naz=0;
	SI->KOd=SI->KDo=0;
	SI->Uzivat=False;
	return False;
}
static Flag SpecZapMidi( SpcInfo *SI, int si )
{
	if( SI )
	{
		SyntInfo *S=&NDs[si];
		if( strcmp(S->OrcNam,SI->Naz) )
		{ /* zmena orchestru */
			Err ret=KonSNastrMidi(S);
			if( ret<EOK ) {Chyba(ret);return False;}
			strlncpy(S->OrcNam,SI->Naz,sizeof(S->OrcNam));
			S->PlatnyOrcNam=True;
			ret=ZacSNastrMidi(S);
			if( ret<EOK ) {Chyba(ret);return False;}
		}
		if( S->KanOd!=SI->KOd ) S->NDZmena=True,S->KanOd=SI->KOd;
		if( S->KanDo!=SI->KDo ) S->NDZmena=True,S->KanDo=SI->KDo;
		S->Uzity=SI->Uzivat;
		return True;
	}
	else /* vsechny vypnout */
	{
		for( si=0; si<NSyntMax; si++ )
		{
			SyntInfo *S=&NDs[si];
			S->Uzity=False;
		}
		return True;
	}
}

LLHraj Midi=
{
	1, /* synchro VSync */
	MidiKan,
	NazevMidi,
	SetNastrMidi,SetStereoMidi,
	HrajTonMidi,
	ZacHrajMidi,KonHrajMidi,
	ZacSNastrMds,KonSNastrMds,MazSNastrMds,JinSNastrMidi,
	CtiOrchestrMds,PisOrchestrMds,
	EditSNastrMds,SoupisSNastrMds,
	ZacHNastrMidi,DefHNastrMds,AktHNastrMidi,NULL,KonHNastrMidi,
	VymazNastrMds,
	NastrInfoMidi,
	CtiCFGMidi,UlozCFGMidi,
	SpecInfoMidi,SpecZapMidi,
	NULL,NULL,
	NULL,NULL,NULL,
};
