/* Componium - prehravani */
/* SUMA 10/1992-10/1993 */

#include <string.h>
#include <stdlib.h>
#include <macros.h>
#include <sndbind.h>

#include "ramcompo.h"
#include "digill.h"
#include "comcompo.h"
#include "compo.h"

static int HrajHan=-1;

/* prazdna implementace playeru */

static const MPovel *_Pis,*PisZac;

static volatile CasT MaxCas;
static volatile Flag DoRewFlag; /* zadost o previnuti - pro preruseni */
static volatile CasT UplynulCas;
static volatile Flag KonecHrani;
static CasT CasZac,CasDel;
static Flag Paused;
static CasT Speed; /* 1== norm. play v int. jednot. */
static Flag Rewind; /* je treba vyhledat spravnou pozici */

static Flag AutoRepeat=False;

static DynPole PlyLocal;

static Flag JeFadeOut;
static long FadeOutT;

int SystemCas();

static Flag OknoAbort( Flag Abort )
{
	if( JeFadeOut )
	{
		if( SystemCas()>=FadeOutT ) return True;
	}
	else
	{
		if( Abort )
		{
			FadeOutT=SystemCas()+1070;
			JeFadeOut=True;
			F_Povel=FPFade;
		}
	}
	return False;
}

int HrajOkno( void )
{
	return HrajHan;
}

Flag HrajeSeVOkne( void )
{
	return HrajHan>=0;
}

Flag TestHraje( void ){return KonecHrani;}
Flag TestAbort( void )
{
	#if 0
		if( Vrchni==HrajHan && (TestEvntKeyb()>>8)==1 ) return OknoAbort(True);
	#endif
	return OknoAbort(False);
}

void InitKonec( void )
{
	F_Povel=FPNic;
	JeFadeOut=False;
	KonecHrani=False;
}

static long TCnt; /* citadlo dalsi udalosti */

CasT TimerADF( CasT TStep ) /* play, fwd cue */
{
	const MPovel *Pis=_Pis;
	#define BPis ((signed char *)Pis )
	UplynulCas+=TStep;
	TCnt-=TStep;
	while( TCnt<=0 )
	{
		int Kan;
		int N;
		switch( *BPis++ )
		{
			/* hlasitost je -128 (Fade) .. 0 (NoteOff) ..127 */
			case MNotaZac: Kan=*BPis++,N=*BPis++,AktHraj->HrajTon(N,*BPis++,Kan);break;
			case MSync: BPis++,TCnt+=*Pis++;break;
			case MKonec:
				if( AutoRepeat )
				{
					Pis=PisZac,UplynulCas=0,MaxCas=0;
					break;
				}
				else
				{
					KonecHrani=True;--BPis;_Pis=Pis;
					return 0;
					/* zustane MKonec */
				}
			case MStereo: Kan=*BPis++,N=*BPis++,AktHraj->SetStereo(N,*BPis++,Kan);break;
			case MEfekt: Kan=*BPis++,N=*BPis++,AktHraj->SetEfekt(N,*BPis++,Kan);break;
			case MNastr: Kan=*BPis++,AktHraj->SetNastr(*Pis++,Kan,&PlyLocal);break;
			default: BPis++,Pis++;break;
		}
	}
	_Pis=Pis;
	return TCnt;
	#undef BPis
}

static void TimerRepl( CasT Pos ) /* play, pro rewind */
{ /* simuluje dojeti na akt. pozici */
	CasT Pos0=Pos;
	typedef struct kinfo
	{
		int Nas;
		int SMin,SMax;
		int EffA,EffB;
	};
	struct kinfo KI[MaxFyzKan];
	int i;
	const MPovel *Pis;
	#define BPis ((signed char *)Pis )
	/* nejprve dojed visici tony */
	Pis=_Pis;
	for(;;)
	{
		int Kan;
		int N,V;
		switch( *BPis++ )
		{
			case MNotaZac:
				Kan=*BPis++,N=*BPis++,V=*BPis++;
				if( V==0 ) AktHraj->HrajTon(N,V,Kan); /* jen Note Off */
				break;
			case MSync: BPis++,TCnt+=*Pis++;break;
			case MKonec: goto ZacSim;
			default: BPis++,Pis++;break;
		}
	}
	ZacSim:
	Pis=PisZac;
	/* simuluj prubeh */
	for( i=0; i<AktHraj->NFyzKan; i++ )
	{
		KI[i].Nas=-1;
		KI[i].EffA=KI[i].EffB=-1;
		KI[i].SMin=-0x100,KI[i].SMax=0x100;
	}
	while( Pos>0 )
	{
		int Kan;
		switch( *BPis++ )
		{
			case MSync: BPis++,Pos-=*Pis++;break;
			case MKonec: --BPis; _Pis=Pis;goto KonecLab;/* zustane MKonec */
			case MStereo: Kan=*BPis++,KI[Kan].SMin=*BPis++,KI[Kan].SMax=*BPis++;break;
			case MEfekt: Kan=*BPis++,KI[Kan].EffA=*BPis++,KI[Kan].EffB=*BPis++;break;
			case MNastr: Kan=*BPis++,KI[Kan].Nas=*Pis++;break;
			default: BPis++,Pis++;break;
		}
	}
	_Pis=Pis;
	for( i=0; i<AktHraj->NFyzKan; i++ )
	{
		struct kinfo *ki=&KI[i];
		if( ki->Nas>=0 ) AktHraj->SetNastr(ki->Nas,i,&PlyLocal);
		else AktHraj->SetNastr(-1,i,&PlyLocal);
		if( ki->SMin>-0x100 ) AktHraj->SetStereo(ki->SMin,ki->SMax,i);
		if( ki->EffA>=0 ) AktHraj->SetEfekt(ki->EffA,ki->EffB,i);
	}
	TCnt=0;
	UplynulCas=Pos0-Pos+CasZac;
	KonecLab:
	if( AktHraj->ResetPau ) AktHraj->ResetPau();
	#undef BPis
}

long TimerAD( void )
{
	long S=Speed*AktHraj->TempStep;
	TimerADF(S);
	return S;
}

long TimerAPauza( void )
{
	long S=Speed*AktHraj->TempStep;
	if( !DoRewFlag )
	{
		UplynulCas+=S;
		if( UplynulCas<CasZac ) UplynulCas=CasZac;
		ef( UplynulCas>CasDel ) UplynulCas=CasDel;
		MaxCas=0;
		return S;
	}
	else
	{
		long p=UplynulCas-CasZac;
		if( p<0 ) p=0;
		TimerRepl(p);
		DoRewFlag=False;
		MaxCas=0;
	}
	return 0;
}

static Err FyzHraj( const MPovel *Pis, CasT Del, CasT Od )
{
	Err ret;
	PisZac=_Pis=Pis;
	UplynulCas=Od;
	CasZac=Od,CasDel=Del;
	DoRewFlag=False;
	MaxCas=0;
	Speed=1;
	Paused=False; /* fixed error: 15. 8. 1994 - reported by COMPO */
	Rewind=False; /* fixed error: 15. 8. 1994 - reported by COMPO */
	TCnt=1;
	ret=AktHraj->ZacHraj(Del,Od);
	//if( ret==ERR ) AlertRF(SNDERRA,1),ret=ECAN;
	return ret;
}

void SetLocal( DynPole *P )
{
	PlyLocal=*P; /* stejn‚ vlastnosti */
	ZacDyn(&PlyLocal);
	CopyDyn(&PlyLocal,P);
}
void ResetLocal( void )
{
	KonDyn(&PlyLocal);
}

#if 0
/* max. rozmery indikatoru */
enum {IByte=20}; /* musi byt nas. lwordu - take viz SPRITEM.S */
enum {IH=100};

static word IScr[IByte/2*IH];
static MFDB IMFDB={IScr,IByte*8,IH,IByte/2,0,1,0,0,0,};

static int K_GVol[MaxFyzKan];
static ClipRect K_CRA[2];
static WindRect K_OR,K_CR,K_SR;

int cdecl KresliHraj( PARMBLK *pb )
{
	K_OR=*(WindRect *)&pb->pb_x;
	K_CR=*(WindRect *)&pb->pb_xc;
	if( K_OR.w>IByte*8 ) K_OR.w=IByte*8;
	if( K_OR.h>IH ) K_OR.h=IH;
	WindSect(&K_CR,&K_OR);
	wtocr(&K_CRA[1],&K_CR);
	K_SR=K_CR;
	K_SR.x-=K_OR.x,K_SR.y-=K_OR.y;
	wtocr(&K_CRA[0],&K_SR);
	if( AktHraj && AktHraj->GetVol )
	{
		static int ci[2]={LRED,0}; /* LRED je v AES.H def. jako 10 */
		if( ClipOK(&K_CRA[0]) && ClipOK(&K_CRA[1]) )
		{
			int i,N;
			memset(IScr,0,sizeof(IScr));
			N=AktHraj->GetVol(K_GVol);
			if( N<=8 ) for( i=0; i<N; i++ )
			{
				int Vol=K_GVol[i];
				int AV=K_OR.h-1-(int)( (long)Vol*(K_OR.h-1)/1000 );
				int h;
				word *HP=&IScr[IByte/2*AV+i];
				for( h=AV; h<K_OR.h; h++,HP+=IByte/2 ) *HP=0x7ffe;
			}
			else for( i=0; i<N; i++ )
			{
				int Vol=K_GVol[i];
				int AV=K_OR.h-1-(int)( (long)Vol*(K_OR.h-1)/1000 );
				int h;
				byte *HP=&((byte *)IScr)[IByte*AV+i];
				for( h=AV; h<K_OR.h; h++,HP+=IByte ) *HP=0x7e;
			}
		}
		if( IMFDB.fd_nplanes==screen.fd_nplanes ) vro_cpyfm(GHandle,S_ONLY,&K_CRA[0].x1,&IMFDB,&screen);
		else vrt_cpyfm(GHandle,MD_REPLACE,&K_CRA[0].x1,&IMFDB,&screen,ci);
	}
	return 0;
}

#endif

static void DoRew( void )
{
	if( Speed && Paused && Rewind )
	{
		Rewind=False;
		DoRewFlag=True;
		while( DoRewFlag ); /* pockej na rewind */
	}
}

static void DoPause( void )
{
	if( !Paused && AktHraj->Pause )
	{
		Rewind=False;
		DoRewFlag=False;
		AktHraj->Pause(),Paused=True;
	}
}
static void DoContinue( void )
{
	if( Paused && AktHraj->Continue ) AktHraj->Continue(),Paused=False;
}

static Flag HraPly( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *WD=OInfa[HrajHan];
	DUnsel(WD,HRAREWB);
	DUnsel(WD,HRAFWDB);
	Speed=1;
	DoRew();
	if( !Test(Bt,SELECTED) ) DSel(WD,HRAPLYB);
	DoContinue();
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag HraRew( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *WD=OInfa[HrajHan];
	DUnsel(WD,HRAPLYB);
	DUnsel(WD,HRAFWDB);
	DoPause();
	Rewind=True;
	Speed=-10;
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag HraFwd( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *WD=OInfa[HrajHan];
	if( !Paused ) Speed=3;
	else Speed=10,Rewind=True,DUnsel(WD,HRAPLYB);
	DUnsel(WD,HRAREWB);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Flag HraEnd( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *WD=OInfa[HrajHan];
	(void)Bt,(void)Dvoj,(void)Asc;
	DUnsel(WD,HRAPLYB);
	DUnsel(WD,HRAFWDB);
	DUnsel(WD,HRAREWB);
	DUnsel(WD,HRAENDB);
	DoPause();
	Speed=0;
	return False;
}

static Flag HraPau( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *WD=OInfa[HrajHan];
	if( Test(Bt,SELECTED) )
	{
		Speed=0;
		DoPause();
		if( Test(&WD->D.Form[HRAFWDB],SELECTED) ) Rewind=True;
		DUnsel(WD,HRAFWDB);
		DUnsel(WD,HRAREWB);
	}
	else
	{
		Speed=1;
		DoRew();
		DoContinue();
	}
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Flag HraRep( OBJECT *Bt, Flag Dvoj, int Asc )
{
	AutoRepeat=Test(Bt,SELECTED);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Button DialTlac[]=
{
	{HraPly,HRAPLYB},
	{HraFwd,HRAFWDB},
	{HraRew,HRAREWB},
	{HraEnd,HRAENDB},
	{HraRep,HRAOPAKB},
	{NULL},
};

#if 0
static Flag TestDial( void )
{
	WindDial *WD=OInfa[HrajHan];
	int MX,MY,mkm,mkk;
	graf_mkstate(&MX,&MY,&mkm,&mkk);
	if( mkm&1 )
	{
		int FHan=wind_find(MX,MY);
		if( FHan==WD->D.DHandle )
		{
			DoDialTlac(&WD->D,&WD->S,MX,MY,False);
			DoDialPust(&WD->D,&WD->S);
			return WD->S.Konec;
		}
	}
	return False;
}
#endif

Flag HrajCinnost( void )
{ /* zpracovani mesagi */
	WindDial *WD=OInfa[HrajHan];
	#if 0
	int M[8];
	Okno *O=Okna[HrajHan];
	/*Mouse(ARROW);*/
	while( TestMesag(M) )
	{
		if(	M[0]==WM_CLOSED && O->handle==M[3] )
		{
			if( OknoAbort(True) ) return True;
		}
		if( M[0]==WM_TOPPED ) {Topni(HrajHan);return False;}
		HandlePrekresli(M); /* co nezpracujes sam, posli dal */
	}
	#endif
	if( OknoAbort(False) ) return True;
	#if 0
	if( Vrchni==HrajHan && TestDial() ) return True;
	if( AktHraj->GetVol ) ObjcRedraw(WD->D.Form,HRAVOLGB,WD->D.DHandle,NULL);
	#endif
	if( AktHraj->GetCas )
	{
		SetCas(WD->D.Form,UplynulCas-AktHraj->GetCas(),WD->D.DHandle);
	}
	return False;
}

void TiskCas( TEDINFO *T, CasT Cas )
{
	char Buf[80];
	long Min=Cas/TempoMinFyz;
	long Sec10=(Cas%TempoMinFyz)/(TempoMinFyz/(60*10));
	sprintf(Buf,"%ld:%02ld.%01ld",Min,Sec10/10,Sec10%10);
	SetStrTed(T,Buf);
}

static void CasujHrani( int LHan )
{
	Plati( LHan==HrajHan );
	if( AktHraj->CasHraj() )
	{
		Zavri(LHan); /* ukonŸ¡me okno */
	}
	OknoAbort(False);
}

void AzDohraje( void (*Proc)( void ) )
{
	if( HrajHan<0 ) Proc();
	else PoZavreni(HrajHan,Proc);
}

static void FyzKonHrani( void )
{
	Chyba( AktHraj->KonHraj() );
}
static void Dohrano( int LHan )
{
	Plati( LHan==HrajHan );
	HrajHan=-1;
	PlatiProc( DialLogZ(LHan), ==True );
}

static Flag ZavriHrani( int LHan )
{
	Plati( LHan==HrajHan );
	if( OknoAbort(True) )
	{
		Dohrano(LHan);
		return True;
	}
	return False;
}
static void FZavriHrani( int LHan )
{
	Plati( LHan==HrajHan );
	while( !OknoAbort(True) ) {}
	Dohrano(LHan);
}

Err Zahraj( const char *Naz, const MPovel *Pis, CasT Del, CasT Od, int MaxKan, Pisen *P )
{
	Err ret;
	OBJECT *F=RscTree(HRAJD);
	if( HrajHan>=0 ) return ECAN;
	if( !*P->Popis1 ) SetStrTed(TI(F,HRAPIST),Naz);
	else SetStrTed(TI(F,HRAPIST),P->Popis1);
	SetStrTed(TI(F,HRAPOPT),P->Popis2);
	TiskCas(TI(F,HRADELT),Del);
	#define IN3D   0x400
	#define LOOK3D 0x200
	/*if( AktHraj==&Midi ) SetLocal(&P->LocMidi);
	else */ SetLocal(&P->LocDigi);
	if( AktHraj->GetVol ) AktHraj->GetVol(NULL); /* init */
	SetCas(F,Od,-1);
	if( MaxKan>AktHraj->NFyzKan )
	{
		char Buf[10];
		sprintf(Buf,">%d",AktHraj->NFyzKan);
		SetStrTed(TI(F,HRAMKANT),Buf);
	}
	else SetIntTed(TI(F,HRAMKANT),MaxKan);
	/* plat¡ jen pro Falcona! */
	{
		int Div;
		if( MaxKan>AktHraj->NFyzKan ) MaxKan=AktHraj->NFyzKan;
		ef( MaxKan<1 ) MaxKan=1;
		F_Mode=MaxKan;
		{
			long BenchK=F_BenchTune(-1); /* poŸet vzork… za sekundu */
			long EsF;
			long EsDiv;
			int DivKan=MaxKan;
			/* s poŸtem kan l… m¡rnØ kles  n roŸnost */
			if( DivKan>8 )
			{	
				DivKan=8+((DivKan-8)*3+1)/4;
			}
			EsF=BenchK/DivKan;
			/* f = 25.175e6 / (256*(k+1)) */
			/* k = 25.175e6 / (256*f) - 1 */
			/* potýebujeme zaokrouhlovat nahoru */
			EsDiv=(25175000L+256*EsF-1)/(256*EsF)-1;
			if( EsDiv<CLK50K ) EsDiv=CLK50K;
			ef( EsDiv>CLK8K ) EsDiv=CLK8K;
			/* nØkter‚ frekvence CODEC nezvl d  */
			if( EsDiv==6 ) EsDiv++;
			ef( EsDiv==8 ) EsDiv++;
			ef( EsDiv==10 ) EsDiv++;
			Div=(int)EsDiv;
		}
		F_Divis=Div;
		InitFFreq();
		RecalcFreq(FBufFreqDbl); /* pýepoŸ¡tej si to */
	}
	SetStrTed(TI(F,HRAVYS1T),AktHraj->Nazev());
	Set01(&F[HRAPOST],DISABLED,!AktHraj->GetCas);
	AutoRepeat=P->Repeat;
	if( Od>0 ) AutoRepeat=False;
	Set01(&F[HRAOPAKB],SELECTED,AutoRepeat);F[HRAOPAKB].ob_flags&=~IN3D;
	Set1(&F[HRAPLYB],SELECTED);F[HRAPLYB].ob_flags&=~IN3D;
	Set0(&F[HRAREWB],SELECTED);F[HRAREWB].ob_flags&=~IN3D;
	Set0(&F[HRAFWDB],SELECTED);F[HRAFWDB].ob_flags&=~IN3D;
	Set0(&F[HRAENDB],SELECTED);F[HRAENDB].ob_flags&=~IN3D;
	Set01(&F[HRAOPAKB],DISABLED,Od>0);
	PovolCinMenu(False);
	HrajHan=OtevriCDial(F,DialTlac,NULL,-1,NULL,False);
	if( HrajHan>=0 )
	{
		Okno *O=Okna[HrajHan];
		VsePrekresli();
		if( AktHraj==&AYBufFalc )
		{
			F_EffPars(0,P->Efekt[0].Pars);
			F_EffPars(1,P->Efekt[1].Pars);
		}
		ret=FyzHraj(Pis,Del,Od);
		if( ret<EOK )
		{ /* zruç okno, nØco se pokazilo */
			Zavri(HrajHan);
			HrajHan=-1;
		}
		else
		{
			O->LogOZavri=ZavriHrani;
			O->FLogOZavri=FZavriHrani;
			O->DoCas=CasujHrani;
			AzDohraje(FyzKonHrani);
		}
	}
	AzDohraje(ResetLocal);
	/*if( Od>0 ) P->Repeat=AutoRepeat;*/
	PovolCinMenu(True);
	return ret;
}

