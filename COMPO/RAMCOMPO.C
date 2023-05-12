/* Componium - rozhrani pro MultiInterface */
/* SUMA, 9/1992-2/1994 */

#include <macros.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <tos.h>
#include <sndbind.h>
#include <linea.h>
#include <gem\multiitf.h>
#include <sttos\narod.h>
#include <sttos\cszvaz.h>
#include <sttos\samples.h>

#include "compo.h"
#include "spritem.h"
#include "ramcompo.h"
#include "digill.h"

#define CESKA_VERZE 0

int OknoMaxStop,OknoMaxStopA; /* v zavislosti na rozliseni obrazovky max. pocet stop v okne */

CmpPar CesPar;

char NazevRsc[]="COMPO.RSC";

Pisen *Pisne[NMaxPisni];

Pisen *PisenBlok;

const char *SterLS,*SterPS;

enum edmode EdMode=DEditace,EdModeO=DEditace;

PathName CompoRoot;

#ifdef __DEBUG
	void DebugError( int Line, const char *File )
	{
		if( AlertRF(PANIKA,2,NajdiNazev(File),Line)==1 )
		{
			if( VTos()>=0x104 ) wind_new();
			exit(0);
		}
	}
#endif

void Chyba( Err err )
{
	switch( err )
	{
		case ECAN: break;
		case ERAM: AlertRF(RAMEA,1);break;
		case EFIL: AlertRF(IOERRA,1);break;
		default:
			if( err<MAXERR ) AlertRF(ERRORA,1);
			break;
	}
}
void MOznamRezervu( void )
{
	AlertRF(RAMRA,1);
}

Flag LoadFont( void )
{
	void *SSP=(void *)Super(0);
	#ifdef __TURBOC__
		linea_init();
		memcpy(Font,Fonts->font[1]->fnt_dta,0x800);
		memcpy(&Font[0x800],Fonts->font[2]->fnt_dta,0x1000);
	#else
		linea0();
		memcpy(Font,la_init.li_a1[1]->font_data,0x800);
		memcpy(&Font[0x800],la_init.li_a1[2]->font_data,0x1000);
	#endif
	Super(SSP);
	return ZacGraf(&Font[0x800])>=0;
}

#define HMaxCalc(NStop) ((NStop)*ydif+(yprv-ydif))
static int NStopCalcF( int H )
{
	int ns=(H-(yprv-ydif))/ydif;
	if( ns<1 ) ns=1;
	if( ns>NMaxStop ) ns=NMaxStop;
	return ns;
}

static int NStopCalc( int H )
{
	int ns=NStopCalcF(H+ydif/2);
	if( ns>OknoMaxStop ) ns=OknoMaxStop;
	return ns;
}

static void InitMaxStop( void )
{
	WindRect Desk;
	int p;
	GetWind(0,WF_FULLXYWH,&Desk);
	WindCalc(WC_WORK,NOknoType,&Desk,&Desk);
	OknoMaxStop=NStopCalcF(Desk.h+16);p=NStopCalcF(screen.fd_h);
	if( OknoMaxStop>p ) OknoMaxStop=p;
	OknoMaxStopA=NStopCalcF(Desk.h+16-40);
}

#define image LogoImg
#include "clogimag.icn"
#undef image
#define image LogoMsk
#include "clogmask.icn"
static BITBLK LogoIBB={LogoImg,(ICON_W+15)/16*2,ICON_H,0,0,1};
static BITBLK LogoMBB={LogoMsk,(ICON_W+15)/16*2,ICON_H,0,0,0};

void ZmenVizaz( Flag Je3D )
{
	static const int Ds[]=
	{
		ABOUT,VYBERD,TONINAD,NOVSEKD,TAKTD,ATTRIBD,
		TEMPOD,REPETD,HLASITD,HRAJD,OKTAVAD,EDNASD,
		/*MIDNASD,VYSTUPD,*/STATUSD,STEREOD,NAJDIRAD,
		MIXERD,EFFPARD,LISTD,
		0
	};
	const int *D;
	for( D=Ds; *D; D++ ) Dial3D(RscTree(*D),Je3D);
}

#if CESKA_VERZE
static void ZmenCode( void (*CS)( char *S, int N ), int N )
{
	int i;
	for( i=0; i<=STATUSD; i++ ) DialCode(RscTree(i),CS,N);
	FSCode(CEKREPA,CS,N);
}
void ZmenNormu( int NTab, int OTab )
{
	if( OTab ) ZmenCode(CSStrDecode,OTab);
	if( NTab ) ZmenCode(CSStrCode,NTab);
	ZmenNormuOkna(NTab,OTab);
}
#else
void ZmenNormu( int NTab, int OTab )
{
	(void)NTab,(void)OTab;
}
#endif

Flag ZacRsc( void )
{
	TestFalcon();
	#if 1
	if( !Falcon || Zvuky()!=SndFalcon )
	{
		AlertRF(FALCONLA,1);
		return False;
	}
	#endif
	GetPath(CompoRoot);
	EndChar(CompoRoot,'\\');
	DialWindows=VTos()>=0x200; /* dial. v oknech jen na Falconu */
	Menu=RscTree(MENU);
	SterLS=FreeString(STERLFS);
	SterPS=FreeString(STERPFS);
	Desktop=NULL;
	Menu->ob_x=Menu->ob_y=0;
	menu_icheck(Menu,NOTYI,True);
	{
		OBJECT *D=RscTree(ABOUT);
		WindRect W;
		char Buf[80];
		ObjcRect(D,DSKMSKBB,&W);
		if( W.w>ICON_W )
		{
			D[DSKMSKBB].ob_spec.bitblk=&LogoMBB;
			D[DSKMSKBB].ob_width=ICON_W;
			D[DSKMSKBB].ob_height=ICON_H;
			D[DSKMSKBB].ob_x+=(W.w-ICON_W)/2;
			D[DSKMSKBB].ob_y+=(W.h-ICON_H)/2;
			D[DSMIMGBB].ob_spec.bitblk=&LogoIBB;
			D[DSMIMGBB].ob_width=ICON_W;
			D[DSMIMGBB].ob_height=ICON_H;
		}
		else D[DSKMSKBB].ob_flags|=HIDETREE;
		AutoDatum(Buf,"",__DATE__);
		SetStrTed(TI(D,ADATUMT),Buf);
		{
			char Buf[40];
			char *E;
			if( !Demo )
			{
				strcpy(Buf,StrTed(TI(D,AVERZET)));
				E=strrchr(Buf,' ');
				if( E ) sprintf(E," 1.51");
			}
			else
			{
				strcpy(Buf,"Demoversion");
			}
			SetStrTed(TI(D,AVERZET),Buf);
		}
	}
	{
		WindRect W;
		GetWind(0,WF_FULLXYWH,&W);
		if( Menu[0].ob_width>W.w ) Menu[0].ob_width=W.w;
		if( Menu[1].ob_width>W.w ) Menu[1].ob_width=W.w;
	}
	#if 0
	{
		static USERBLK UBHraj={KresliHraj,0};
		OBJECT *D=RscTree(HRAJD);
		OBJECT *G=&D[HRAVOLGB];
		G->ob_head=G->ob_tail=-1;
		G->ob_type=G_USERDEF;
		G->ob_spec.userblk=&UBHraj;
	}
	#endif
	AutoMenuKey();
	ZmenVizaz(Falcon);
	InitVyber();
	return True;
}

#undef ICON_W
#undef ICON_H
#undef ICONSIZE

#if CESKA_VERZE
static void Odveseni( void )
{
	if( CSGVazba() )
	{
		CSOdves(ApId);
	}
}
#endif

Flag ZacMulti( void )
{
	InitCTab();
	SetCesJazyk(&CesPar);
	OznamRezervu=MOznamRezervu;
	if( !LoadFont() ) {ChybaRAM();return False;}
	if( (long)Malloc(-1)<KritRez ) {ChybaRAM();KonGraf();return False;}
	For( Pisen **, P, Pisne, &Pisne[NMaxPisni], P++ )
		*P=NULL;
	Next
	PisenBlok=NULL;
	ZmenaSekv=NastavZmeny;
	#if CESKA_VERZE
	if( CSGVazba() )
	{
		ZmenCode(CSStrCode,CSTestNorm());
		if( atexit(Odveseni)>=0 )
		{
			CSZaves(ApId);
		}
	}
	#endif
	InitMaxStop(); /* musi byt pred stanovenim rozmeru oken */
	ZmenaVrchniho(-1);
	F_Divis=CLK33K;
	FGloballocksnd();
	/* jeste pred ctenim zvuku - tj. pred inic. nastroju */
	if( CtiCFG()<EOK )
	{
		KonGraf();
		return False;
	}
	ZobrazToN();
	OtevriTool();
	OtevriToN();
	{
		int LHan=VrchniSekv();
		if( LHan>=0 ) Topni(LHan);
	}
	DskMode();
	if( Zvuky()==SndFalcon )
	{
		F_Mode=8;
		F_BenchTune(-1);
	}
	return True;
}

Flag KonMulti( void )
{
	MazPBlok();
	FGlobalunlocksnd();
	KonGraf();
	return True;
}

const char *TvorNazev( const Sekvence *S, int LHan, Flag Zmena )
{
	static char CBuf[80];
	(void)LHan;
	sprintf(CBuf,"%s%s",Zmena ? "* " : "",NazevSoubPis(S));
	return CBuf;
}

void NastavNazvy( void )
{
	int LHan;
	for( LHan=0; LHan<NMaxOken; LHan++ )
	{
		Okno *O=Okna[LHan];
		if( O && OknoJeSekv(LHan) )
		{
			OknoInfo *OI=OInfa[LHan];
			Sekvence *S=SekvenceOI(OI);
			Pisen *P=S->Pis;
			SetNazev(O,TvorNazev(S,LHan,P->Zmena),True);
			OI->VNazvuZmena=P->Zmena;
		}
	}
}

Flag OknoJeSekv( int LHan )
{
	return OInfa[LHan] && Okna[LHan] && Okna[LHan]->SDruh==SDNoty;
}

int VrchniSekv( void )
{
	/* ze chceme nejvyssi sekv. */
	int LHan;
	for( LHan=HorniOkno(); LHan>=0; LHan=DalsiOkno(LHan) )
	{
		if( OknoJeSekv(LHan) ) return LHan;
	}
	return -1;
}

void PosunCas( OknoInfo *OI )
{
	int Kan;
	CasT CasPos=CasJeS(&OI->Soub[OI->KKan]);
	for( Kan=OI->IStopa; Kan<OI->NStop+OI->IStopa; Kan++ ) if( Kan!=OI->KKan )
	{
		SekvSoubor *s=&OI->Soub[Kan];
		NajdiCasS(s,CasPos);
	}
}
void PrepniCas( OknoInfo *OI )
{
	int Kan;
	CasT CasPos=CasJeS(&OI->Soub[OI->KKan]);
	for( Kan=0; Kan<OI->NSoub; Kan++ ) if( Kan!=OI->KKan )
	{
		SekvSoubor *s=&OI->Soub[Kan];
		NajdiCasS(s,CasPos);
	}
}

void Kresli( int LHan )
{
	Prekresli(LHan,&Okna[LHan]->Uvnitr);
}
void MoznaKresli( int LHan )
{
	if( OknoJeSekv(LHan) )
	{
		OknoInfo *OI=OInfa[LHan];
		if( OI->ZmenaObrazu ) Kresli(LHan);
	}
}

void IgnKresli( int LHan )
{
	ForceKresli(LHan);
	VsePrekresli();
}

void Zmenseno( int LHan )
{
	Prekresli(LHan,&Okna[LHan]->Uvnitr);
}

static CasT DelkaPred( SekvSoubor *o )
{
	CasT ret;
	SekvSoubor s;
	RozdvojS(&s,o);
	{
		CasT GCas=CasJeS(&s);
		do
		{
			ret=ZpatkyS(&s);
		} while( ret==OK && CasJeS(&s)==GCas );
		if( ret==OK ) ret=GCas-CasJeS(&s);
	}
	ZavriS(&s);
	return ret;
}

static CasT PredTakt( SekvSoubor *s )
{
	SekvSoubor p;
	CasT ret;
	RozdvojS(&p,s);
	ret=ZpatkyS(&p);
	if( ret==OK )
	{
		ret=TaktS(&p)*(TempoMin/TempoS(&p));
	}	
	else ret=0;
	ZavriS(&p);
	return ret;
}

void PosunCasPos( OknoInfo *OI, CasT CasPos )
{
	int Kan;
	for( Kan=OI->IStopa; Kan<OI->NStop+OI->IStopa; Kan++ )
	{
		SekvSoubor *s=&OI->Soub[Kan];
		NajdiCasS(s,CasPos);
	}
}

static void NastavRozmer( int LHan )
{
	WindRect W;
	AutoRozmer(&W,LHan);
	ZmenRozmer(&W,LHan);
}

void IStopaZKKan( OknoInfo *OI )
{
	int maxi=OI->NSoub-OI->NStop;
	if( OI->IStopa>maxi ) OI->IStopa=maxi;
	if( OI->KKan>OI->IStopa+OI->NStop-1 )
	{
		OI->IStopa=OI->KKan-OI->NStop+1;
	}
	if( OI->KKan<OI->IStopa )
	{
		if( OI->KKan<OI->NStop ) OI->IStopa=0;
		else OI->IStopa=OI->KKan;
	}
}

static void PrepniKKan( OknoInfo *OI, int KKan )
{
	OI->KKan=KKan;
	ZavriS(&OI->Kurs);
	RozdvojS(&OI->Kurs,&OI->Soub[KKan]);
}
CasT CasKursoru( OknoInfo *OI )
{
	return CasJeS(&OI->Kurs);
}
void NajdiKursor( OknoInfo *OI, CasT CasPos )
{
	NajdiSCasS(&OI->Kurs,CasPos);
}

void NaPoziciKurs( OknoInfo *OI )
{
	UkazKursor(OI);
	NajdiCasS(&OI->Soub[OI->KKan],CasJeS(&OI->Kurs));
	PosunCas(OI);
	TaktVlevo(OI);TaktVlevo(OI);
}

void JdiNaPozici( OknoInfo *OI, CasT CPos, int KKan )
{
	PrepniKKan(OI,KKan);
	NajdiSCasS(&OI->Kurs,CPos);
}

void SekvPriot( int LHan, int Kan, Sekvence *H )
{
	OknoInfo *OI=OInfa[LHan];
	Plati(OI->NSoub<NMaxHlasu);
	{
		int i;
		for( i=OI->NSoub-1; i>=Kan; i-- )
		{
			SekvSoubor *s=&OI->Soub[i];
			SekvSoubor *n=&s[+1];
			RozdvojS(n,s);
			ZavriS(s);
		}
	}
	{
		SekvSoubor *s=&OI->Soub[Kan];
		OtevriS(s,H,Cteni);
		SetKKontext(AktK(s),&OI->Soub[0].ss[0].K); /* kontext okna */
		if( OI->Rozvoj ) RezimS(s,True);
	}
	OI->NSoub++;
	PrepniKKan(OI,Kan);
	NajdiKursor(OI,0);
	PosunCasPos(OI,0);
	if( OI->NStop<OknoMaxStopA )
	{
		OI->NStop++;
		PosunCas(OI);
		NastavRozmer(LHan);
	}
	else
	{
		IStopaZKKan(OI);
		PosunCas(OI);
		Kresli(LHan);
	}
	UkazKursor(OI);
}
void SekvPrizav( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	CasT CP=CasKursoru(OI);
	int Kan,NKan;
	Plati(OI->NSoub>1);
	NKan=Kan=OI->KKan;
	if( NKan>OI->NSoub-2 ) NKan=OI->NSoub-2;
	else NKan=Kan+1;
	ZavriS(&OI->Soub[Kan]);
	{
		int i;
		for( i=Kan+1; i<OI->NSoub; i++ )
		{
			SekvSoubor *s=&OI->Soub[i];
			SekvSoubor *n=&s[-1];
			RozdvojS(n,s);
			ZavriS(s);
		}
	}
	if( NKan>Kan ) NKan--;
	OI->NSoub--;
	PrepniKKan(OI,NKan);
	NajdiKursor(OI,CP);
	UkazKursor(OI);
	if( OI->NStop>OI->NSoub )
	{
		OI->NStop=OI->NSoub;
		IStopaZKKan(OI);
		PosunCas(OI);
		NastavRozmer(LHan);
	}
	else
	{
		IStopaZKKan(OI);
		PosunCas(OI);
		Kresli(LHan);
	}
}

static void KKanZIStopa( OknoInfo *OI )
{
	if( OI->KKan>OI->IStopa+OI->NStop-1 )
	{
		CasT CP=CasKursoru(OI);
		PrepniKKan(OI,OI->IStopa+OI->NStop-1);
		NajdiKursor(OI,CP);
	}
	if( OI->KKan<OI->IStopa )
	{
		CasT CP=CasKursoru(OI);
		PrepniKKan(OI,OI->IStopa);
		NajdiKursor(OI,CP);
	}
}
static Flag Nahoru( OknoInfo *OI )
{
	if( OI->IStopa>0 )
	{
		OI->IStopa--;
		KKanZIStopa(OI);
		PosunCas(OI);
		return True;
	}
	return False;
}
static Flag Dolu( OknoInfo *OI )
{
	if( OI->IStopa<OI->NSoub-OI->NStop )
	{
		PrepniCas(OI);
		OI->IStopa++;
		KKanZIStopa(OI);
		PosunCas(OI);
		return True;
	}
	return False;
}
static Flag StrNahoru( OknoInfo *OI )
{
	if( OI->IStopa>0 )
	{
		PrepniCas(OI);
		OI->IStopa-=OI->NStop;
		if( OI->IStopa<0 ) OI->IStopa=0;
		KKanZIStopa(OI);
		PosunCas(OI);
		return True;
	}
	return False;
}
static Flag StrDolu( OknoInfo *OI )
{
	int imax=OI->NSoub-OI->NStop;
	if( OI->IStopa<imax )
	{
		PrepniCas(OI);
		OI->IStopa+=OI->NStop;
		if( OI->IStopa>imax ) OI->IStopa=imax;
		KKanZIStopa(OI);
		PosunCas(OI);
		return True;
	}
	return False;
}

static void KursorNahoru( OknoInfo *OI )
{
	if( OI->KKan>0 )
	{
		CasT CP=CasKursoru(OI);
		PrepniCas(OI);
		PrepniKKan(OI,OI->KKan-1);
		IStopaZKKan(OI);
		PosunCas(OI);
		NajdiKursor(OI,CP);
	}
	UkazKursor(OI);
}
static void KursorDolu( OknoInfo *OI )
{
	if( OI->KKan<OI->NSoub-1 )
	{
		CasT CP=CasKursoru(OI);
		PrepniCas(OI);
		PrepniKKan(OI,OI->KKan+1);
		IStopaZKKan(OI);
		PosunCas(OI);
		NajdiKursor(OI,CP);
	}
	UkazKursor(OI);
}

static Flag SJTaktVpravo( SekvSoubor *s )
{ /* nejvyse jeden takt v souboru */
	CasT CPO=CasJeS(s);
	NajdiSCasS(s,CPO);
	if( TestS(s)==EOK )
	{
		CasT CasPos;
		if( VTaktuS(s)<TaktS(s) ) CasPos=TaktS(s)-VTaktuS(s);
		else CasPos=TaktS(s);
		CasPos*=(TempoMin/TempoS(s));
		CasPos+=CPO;
		if( NajdiSCasS(s,CasPos)==OK ) return True;
		NajdiSCasS(s,CPO);
	}
	return False;
}
static Flag STaktVpravo( SekvSoubor *s )
{ /* aspon jeden takt v souboru */
	if( TestS(s)==EOK ) /* je kam posouvat */
	{
		CasT CPO=CasJeS(s);
		NajdiSCasS(s,CPO);
		if( TestS(s)==EOK )
		{
			CasT CasPos;
			if( VTaktuS(s)<TaktS(s) ) CasPos=TaktS(s)-VTaktuS(s);
			else CasPos=TaktS(s);
			CasPos*=(TempoMin/TempoS(s));
			CasPos+=CPO;
			if( NajdiCasS(s,CasPos)==OK )
			{
				if( CasJeS(s)==CPO ) ZpatkyS(s);
				return True;
			}
			NajdiCasS(s,CPO);
		}
	}
	return False;
}

static SekvSoubor *MinCasPos( OknoInfo *OI )
{
	CasT Min=MAXCAS,MinE=MAXCAS;
	SekvSoubor *sm=NULL,*sme=NULL;
	int Kan;
	for( Kan=OI->IStopa; Kan<OI->IStopa+OI->NStop; Kan++ )
	{
		SekvSoubor *s=&OI->Soub[Kan];
		CasT c=CasJeS(s);
		if( StatusS(s)==EOK && TestS(s)==EOK )
		{ /* jen ty, co nejsou na konci */
			if( c<Min ) Min=c,sm=s;
		}
		if( c<MinE ) MinE=c,sme=s;
	}
	if( !sm  ) sm=sme;
	Plati( sm );
	return sm;
}

void TaktVlevo( OknoInfo *OI )
{ /* aspon jeden takt vlevo (nebo na zacatek taktu) */
	CasT VT,CasPos;
	SekvSoubor *s=MinCasPos(OI);
	CasPos=CasJeS(s);
	if( VTaktuS(s)>0 )
	{
		VT=VTaktuS(s);
		while( VT>TaktS(s) ) VT-=TaktS(s);
		VT*=(TempoMin/TempoS(s));
	}
	else VT=PredTakt(s);
	{
		SekvSoubor p;
		RozdvojS(&p,s);
		if( ZpatkyS(&p)==OK )
		{
			CasT c=DelkaS(&p);
			if( VT<c ) VT=c; /* skocit aspon na predch. jedn. */
		}
		ZavriS(&p);
	}
	CasPos-=VT;
	if( CasPos<0 ) CasPos=0;
	PosunCasPos(OI,CasPos);
}

void TaktVpravo( OknoInfo *OI )
{ /* aspon jeden takt vpravo */
	SekvSoubor *s=MinCasPos(OI);
	if( STaktVpravo(s) ) PosunCasPos(OI,CasJeS(s));
}

static void JTaktVpravo( OknoInfo *OI )
{ /* nejvyse jeden takt vpravo */
	SekvSoubor *s=&OI->Soub[OI->KKan];
	if( StatusS(s)==OK )
	{
		if( SJTaktVpravo(s) ) PosunCasPos(OI,CasJeS(s));
	}
}

enum {Str=4};

static void StranaVlevo( OknoInfo *OI )
{
	int i;
	for( i=0; i<Str; i++ ) TaktVlevo(OI);
}
static void StranaVpravo( OknoInfo *OI )
{
	int i;
	for( i=0; i<Str; i++ ) TaktVpravo(OI);
}
static void NaZacatek( OknoInfo *OI )
{
	NajdiCasS(&OI->Kurs,0);
	PosunCasPos(OI,0);
}
static void NaKonec( OknoInfo *OI )
{
	SekvSoubor *s=&OI->Soub[OI->KKan];
	NajdiCasS(s,MAXCAS);
	NajdiCasS(&OI->Kurs,CasJeS(s));
	PosunCas(OI);
	TaktVlevo(OI);TaktVlevo(OI);
	UkazKursor(OI);
}

int XMax( int LHan ){return Okna[LHan]->Uvnitr.w;}

static void KonecAkordu( OknoInfo *OI )
{
	for(;;)
	{
		JednItem *B=BufS(&OI->Kurs);
		if( TestS(&OI->Kurs)!=EOK ) break;
		if( B[0]>PPau ) break;
		if( !(B[1]&Soubezna) ) break;
		if( JCtiS(&OI->Kurs)!=EOK ) break;
	}
}

static Flag KursorVpravo( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	UkazKursor(OI);
	if( JCtiS(&OI->Kurs)==EOK )
	{
		if( TestS(&OI->Kurs)==EOK && BufS(&OI->Kurs)[0]!=PKonSekv )
		{
			if( CasJeS(&OI->Kurs)>=OI->KonCas || OI->KR.x>XMax(LHan)*3/4 )
			{
				JTaktVpravo(OI);
				return True;
			}
		}
		return False;
	}
	return False;
}
static int KursorVlevo( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	UkazKursor(OI);
	if( ZpatkyS(&OI->Kurs)==EOK )
	{
		if( CasJeS(&OI->Kurs)<=OI->ZacCas || OI->KR.x<XMax(LHan)/3 )
		{
			if( CasJeS(&OI->Soub[OI->KKan])>0 )
			{
				TaktVlevo(OI);
				return 1;
			}
		}
		return 0;
	}
	return -1;
}
static Flag KTaktVlevoF( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	SekvSoubor *s=&OI->Kurs;
	CasT VT,CasPos;
	Flag ret=False;
	CasPos=CasJeS(s);
	if( CasPos>0 )
	{
		if( VTaktuS(s)>0 ) VT=VTaktuS(s)*(TempoMin/TempoS(s));
		else VT=PredTakt(s);
		CasPos=CasJeS(s)-VT;
		if( CasPos<0 ) CasPos=0;
		NajdiSCasS(s,CasPos);
		if( CasJeS(s)<=OI->ZacCas || OI->KR.x<XMax(LHan)/3 ) TaktVlevo(OI);
		ret=True;
	}
	return ret;
}
static Flag KTaktVpravoF( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	SekvSoubor *s=&OI->Kurs;
	Flag ret=False;
	if( STaktVpravo(s) )
	{
		if( CasJeS(s)>=OI->KonCas || OI->KR.x>XMax(LHan)*3/4 ) JTaktVpravo(OI);
		ret=True;
	}
	return ret;
}
static Flag KTaktVlevo( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	Flag KT=KTaktVlevoF(LHan);
	UkazKursor(OI);
	return KT;
}
static Flag KTaktVpravo( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	Flag KT=KTaktVpravoF(LHan);
	UkazKursor(OI);
	return KT;
}
static Flag KStranaVlevo( int LHan )
{
	int i;
	OknoInfo *OI=OInfa[LHan];
	Flag KT=KTaktVlevo(LHan);
	if( KT ) for( i=1; i<Str; i++ ) if( !KTaktVlevoF(LHan) ) break;
	UkazKursor(OI);
	return KT;
}
static Flag KStranaVpravo( int LHan )
{
	int i;
	OknoInfo *OI=OInfa[LHan];
	Flag KT=KTaktVpravo(LHan);
	if( KT ) for( i=1; i<Str; i++ ) if( !KTaktVpravoF(LHan) ) break;
	UkazKursor(OI);
	return KT;
}

void DoSipky( int Sip, int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	NeukazujKursor(OI);
	switch( Sip )
	{
		case WA_UPPAGE: if( StrNahoru(OI) ) Kresli(LHan); break;
		case WA_DNPAGE: if( StrDolu(OI) ) Kresli(LHan); break;
		case WA_UPLINE: if( Nahoru(OI) ) Kresli(LHan); break;
		case WA_DNLINE: if( Dolu(OI) ) Kresli(LHan); break;
		case WA_LFPAGE: StranaVlevo(OI);Kresli(LHan); break;
		case WA_RTPAGE: StranaVpravo(OI);Kresli(LHan); break;
		case WA_LFLINE: TaktVlevo(OI);Kresli(LHan); break;
		case WA_RTLINE: TaktVpravo(OI);Kresli(LHan); break;
	}
}
void DoHSlide( int Pos, int LHan )
{
	CasT CasPos=0,Cas;
	OknoInfo *OI=OInfa[LHan];
	int Kan;
	CasT dif=OI->CasKon-OI->CasZac;
	for( Kan=OI->IStopa; Kan<OI->NStop+OI->IStopa; Kan++ )
	{
		SekvSoubor *s=&OI->Soub[Kan];
		CasT c=CasDelka(SekvenceS(s),NULL);
		if( CasPos<c ) CasPos=c;
	}
	Cas=(CasPos-dif)*Pos/1000;
	Plati(Cas<=CasPos);
	PosunCasPos(OI,Cas);
	NeukazujKursor(OI);
	Kresli(LHan);
}
void DoVSlide( int Pos, int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	int maxi=OI->NSoub-OI->NStop;
	int is=(int)( Pos*(long)maxi/1000 );
	if( is<0 ) is=0;
	ef( is>maxi ) is=maxi;
	OI->IStopa=is;
	KKanZIStopa(OI);
	PosunCas(OI);
	Kresli(LHan);
}

static void OznacBlok( int LHan, int mx, int my, int BMask, Flag Cont )
{
	Okno *O=Okna[LHan];
	OknoInfo *OI=OInfa[LHan];
	int MX=mx,MY=my;
	int Ins;
	enum BlokJev Jev;
	Mouse(POINT_HAND);
	HledejKursor(LHan,mx,my,HKursPred,&Ins);
	if( !Cont || OI->ZBStopa<0 || OI->KBStopa<0 ) ZacBlok(LHan);
	ProzatimKonBlok(LHan);
	for(;;)
	{
		int OMX=MX,OMY=MY;
		Jev=CekejBlokJev(LHan,BMask);
		switch( Jev )
		{
			case BJ_KONEC: KonBlok(LHan);Kresli(LHan);return;
			case BJ_LOKR: TaktVlevo(OI);Kresli(LHan);MX=(O->Uvnitr.w-zacxpos)/4+zacxpos;break;
			case BJ_POKR: TaktVpravo(OI);Kresli(LHan);MX=O->Uvnitr.w-20;break;
			case BJ_HOKR: if( Nahoru(OI) ) Kresli(LHan);MY=0;break;
			case BJ_DOKR: if( Dolu(OI) ) Kresli(LHan);MY=O->Uvnitr.h-1;break;
			case BJ_MYS: MysVOkneXY(LHan,&MX,&MY);break;
		}
		HledejKursor(LHan,MX,MY,HKursPred,&Ins);
		if( OMX!=MX || OMY!=MY ) ProzatimKonBlok(LHan);
	}
}

Flag OknoStisk( int LHan, Flag Dvoj, Flag Dlouhe, int mx, int my )
{
	Plati( OknoJeSekv(LHan) );
	if( Dlouhe ) OznacBlok(LHan,mx,my,1,(Kbshift(-1)&3)!=0);
	else
	{
		int Ins;
		if( !Dvoj ) HledejKursor(LHan,mx,my,HKursor,&Ins);
		ef( HledejKursor(LHan,mx,my,HEditDvoj,&Ins) )
		{
			MEditace(LHan,mx,my,Dvoj);
		}
		ef( Ins==InsNas ) NasVloz(LHan);
	}
	return (Buttons()&1)^1;
}

Flag OknoStisk2( int LHan, Flag Dvoj, Flag Dlouhe, int mx, int my )
{
	int Ins;
	int VHan=VrchniSekv();
	Plati( OknoJeSekv(LHan) );
	if( LHan!=VHan ) /* bud neni vrchni vubec, nebo je jina */
	{
		Topni(LHan);
		return !Dlouhe;
	}
	switch( EdMode )
	{
		case DEditace:
			if( HledejKursor(LHan,mx,my,HEditDvoj,&Ins) )
			{
				MEditace(LHan,mx,my,Dvoj);
				EdMode=EdModeO,DskMode();
			}
			ef( Ins==InsNas ) NasVloz(LHan);
			ef( Dlouhe )
			{
				if( Ins==InsNota ) TonVloz(LHan,mx,my,False);
				if( Ins==InsSoubezna ) TonSVloz(LHan,mx,my,False);
			}
			break;
		case DMazani:
			if( HledejKursor(LHan,mx,my,HNic,&Ins) )
			{
				MazTon(OInfa[LHan]),Kresli(LHan);
			}
			break;
	}
	return (Buttons()&2)==0;
}

Flag OknoPust( int LHan, int MX, int MY ){(void)LHan,(void)MX,(void)MY;return True;}
Flag OknoPust2( int LHan, int MX, int MY ){(void)LHan,(void)MX,(void)MY;return True;}

Flag DesktopStisk( int ObIndex, Flag Dvojite, Flag Dlouhe, int MX, int MY )
{
	(void)ObIndex,(void)MX,(void)MY,(void)Dvojite;
	return !Dlouhe;
}
Flag DesktopStisk2( int ObIndex, Flag Dvojite, Flag Dlouhe, int MX, int MY )
{
	(void)ObIndex,(void)MX,(void)MY,(void)Dvojite;
	return !Dlouhe;
}

void DeskZnak( int Asc, int Scan, int Shf, Flag Buf )
{
	(void)Asc,(void)Scan,(void)Shf,(void)Buf;
}

int OknoZnak( int LHan, int Asc, int Scan, int Shf, Flag Buf )
{
	Flag Shift=Shf&(K_LSHIFT|K_RSHIFT);
	OknoInfo *OI=OInfa[LHan];
	Plati( LHan==VrchniSekv() );
	switch( Scan )
	{
		case 75: /* <- */
			if( Buf ) return 0;
			if( !Shift )
			{
				if( KursorVlevo(LHan)>0 ) break;
				else {KresliKursor(LHan);return 0;}
			}
			if( KStranaVlevo(LHan) ) break;
			return 0;
		case 77: /* -> */
			if( Buf ) return 0;
			if( !Shift )
			{
				if( KursorVpravo(LHan) ) break;
				else {KresliKursor(LHan);return 0;}
			}
			if( KStranaVpravo(LHan) ) break;
			return 0;
		case 72:
			if( Buf ) return 0;
			KursorNahoru(OI); break; /* ^ */
		case 80:
			if( Buf ) return 0;
			KursorDolu(OI);break; /* V */
		case 115: /* Ctrl <- */
			if( Buf ) return 0;
			if( KTaktVlevo(LHan) ) break;
			KresliKursor(LHan);
			return 0;
		case 116: /* Ctrl -> */
			if( Buf ) return 0;
			if( KTaktVpravo(LHan) ) break;
			KresliKursor(LHan);
			return 0;
		case 71: /* Home */
			if( Buf ) return 0;
			if( !Shift ) NaZacatek(OI);
			else NaKonec(OI);
			break;
		case 82:	
			if( Buf ) return 0;
			VlozTon(OI,False);break; /* insert */
		case 15: /* Tab */
			if( TestS(&OI->Kurs)==EOK && BufS(&OI->Kurs)[0]<=PPau )
			{
				KursorVpravo(LHan);
				VlozTon(OI,True);
			}
			break;
		case 57: /* mezera */
			KonecAkordu(OI);
			KursorVpravo(LHan);
			VlozTon(OI,False);
			break;
		case 114: case 28:
			if( Buf ) return 0;
			MEditace(LHan,0,0,True);
			break;
		case 83:
			if( Buf ) return 0;
			MazTon(OI);break; /* delete */
		case 14: if( KursorVlevo(LHan)>=0 ) {MazTon(OI);break;} return 0; /* delete */
		default:
			if( Scan>=59 && Scan<69 ) /* F1..F10 */
			{
				int LHan=VrchniSekv();
				if( LHan>=0 )
				{
					OknoInfo *OI=OInfa[LHan];
					UzijNasF(Scan-59,OI);
					return 0;
				}
			}
			else return Editace(Asc,Scan,Shf);
	}
	Kresli(LHan);
	return 0;
}

void OknoMidi( int LHan, int *Buf )
{
	(void)LHan,(void)Buf;
	/*
	Plati( LHan==VrchniSekv() );
	do
	{
		static int Stav=0;
		static int TrvajiciStav=0;
		static int Vol,Key;
		int Data=Buf[3];
		if( Data&0x80 )
		{
			if( Data<0xF8 )
			{
				Stav=Data;
				switch( Stav>>4 )
				{
					case 8:
					case 9: Vol=Key=-1;break;
				}
			}
		}
		ef( Stav&0x80 )
		{ /* data! */
			int D=Stav>>4;
			int C=Stav&0xf;
			switch( D )
			{
				case 8: /* NoteOff */
				case 9: /* NoteOn */
					if( Key<0 ) Key=Data;
					else
					{
						Vol=Data;
						if( D==9 && Vol>0 ) Editace(Key,Vol,-1);
						Key=Vol=-1;
					}
					TrvajiciStav=Stav;
					break;
				case 0xa: /* Indiv. tlak */
				case 0xb: /* Kontroler */
				case 0xc: /* PgChng */
				case 0xd: /* Spol. tlak */
				case 0xe: /* PitchBend */
					TrvajiciStav=0;
					break;
				case 0xf: /* system */
					if( C>=8 ) /* real time */
					{
					}
					else TrvajiciStav=0;
					break;
			}
		}
	}
	while( TestMidi(Buf) ); /* vyprazdni buffer */
	*/
}

void DoCas( void )
{
	int LHan;
	for( LHan=HorniOkno(); LHan>=0; LHan=DalsiOkno(LHan) )
	{
		if( OknoJeSekv(LHan) )
		{
			OknoInfo *OI=OInfa[LHan];
			if( OI->ZmenaObrazu )
			{
				PosunCas(OI);
				Kresli(LHan);
				break;
			}
		}
	}
	DoCasOkna();
}

static MFORM MKursEdit=
{7,8,1,0,1,0x0000,0x0100,0x0380,0x07C0,0x07C0,0x0380,0x3398,0x7C7C,0xFC7E,0x7C7C,0x3398,0x0380,0x07C0,0x07C0,0x0380,0x0100,0x0000,0x0000,0x0100,0x0380,0x0100,0x0100,0x0000,0x2008,0x783C,0x2008,0x0000,0x0100,0x0100,0x0380,0x0100,0x0000};
static MFORM MKursMazej=
{7,7,1,0,1,0x6006,0xF00F,0xF81F,0x7C3E,0x3E7C,0x1FF8,0x0FF0,0x0660,0x0660,0x0FF0,0x1FF8,0x3E7C,0x7C3E,0xF81F,0xF00F,0x6006,0x0000,0x6006,0x500A,0x2814,0x1428,0x0A50,0x05A0,0x0240,0x0240,0x05A0,0x0A50,0x1428,0x2814,0x500A,0x6000,0x0000};

int NotyKursor( int LHan, Flag Uvnitr, Flag Obrys )
{
	Plati( OknoJeSekv(LHan) );
	if( !Obrys ) return -1;
	if( Uvnitr )
	{
		switch( EdMode )
		{
			case DEditace: MouseUsr(&MKursEdit);return USER_DEF;
			case DMazani: MouseUsr(&MKursMazej);return USER_DEF;
			default: return ARROW;
		}
	}
	return -1;
}

void SaveScrapText( const char *Text, long Len )
{
	SaveScrapTXT(Text,Len);
}

long LoadScrapText( char *Text, long MaxLen )
{
	return LoadScrapTXT(Text,MaxLen);
}

Flag KlavZavri( int shift, int Asc )
{
	KlavInfo *MI=NajdiMenu(shift,Asc);
	if( MI && MI->Item==OZAVRI ) return True;
	return False;
}

int Kursor( Flag Uvnitr, Flag Obrys )
{
	return KursorOkno(Uvnitr,Obrys);
}
void Okrouhli( WindRect *R, int LHan )
{
	WindRect Desk;
	GetWind(0,WF_FULLXYWH,&Desk);
	WindCalc(WC_WORK,Okna[LHan]->WType,&Desk,&Desk);
	if( OknoJeSekv(LHan) )
	{
		enum {XMAT=8,YMAT=2,XOkr=0,YOkr=0};
		OknoInfo *OI=OInfa[LHan];
		CasT Cas=CasJeS(&OI->Soub[OI->KKan]);
		OI->NStop=NStopCalc(R->h);
		if( OI->NStop>OI->NSoub ) OI->NStop=OI->NSoub;
		IStopaZKKan(OI);
		R->h=HMaxCalc(OI->NStop);
		if( R->y<Desk.y+2 ) R->y=Desk.y+2;
		if( R->w>PracMFDB.fd_w-8 )
		{
			R->w=PracMFDB.fd_w-8;
		}
		R->w+=R->x;R->h+=R->y;
		R->x=(R->x+XMAT-1+XOkr)/XMAT*XMAT-XOkr;
		R->y=(R->y+YMAT-1+YOkr)/YMAT*YMAT-YOkr;
		R->w=(R->w+XMAT-1+XOkr)/XMAT*XMAT+XOkr;
		R->h=(R->h+YMAT-1+YOkr)/YMAT*YMAT+YOkr;
		R->w-=R->x;R->h-=R->y;
		PosunCasPos(OI,Cas);
	}
}

void PlneOkno( WindRect *R, int LHan )
{
	WindRect r;
	OknoInfo *OI=OInfa[LHan];
	Roztahni(&r,LHan);
	R->x=r.x;
	R->w=r.w;
	r.h=HMaxCalc(OI->NStop);
	WindCalc(WC_BORDER,Okna[LHan]->WType,&r,&r);
	R->y=r.y;
	R->h=r.h;
	OkrObrys(R,LHan);
}

void LAutoRozmer( WindRect *D, int LHan, int AsHan )
{
	WindRect R;
	Roztahni(&R,LHan);
	D->x=R.x+8*AsHan+108;
	D->y=R.y+12*AsHan;
	D->w=R.w-AsHan*8-108;
	D->h=R.h;
	WindCalc(WC_WORK,NOknoType,D,D);
	D->h=HMaxCalc(OknoMaxStopA);
	WindCalc(WC_BORDER,NOknoType,D,D);
	OkrObrys(D,LHan);
}
void AutoRozmer( WindRect *D, int LHan )
{
	int AsHan=LHan;
	int ph;
	ph=ToolOkno();if( ph>=0 && ph<LHan ) AsHan--;
	ph=ToNOkno();if( ph>=0 && ph<LHan ) AsHan--;
	LAutoRozmer(D,LHan,AsHan);
}

Flag LzeTopnout( int LHan, int MX, int MY )
{
	(void)LHan,(void)MX,(void)MY;
	return True;
}

void ZmenaVrchniho( int LHan )
{
	if( LHan>=0 && OknoJeSekv(LHan) ) ZobrazRekurzi(OInfa[LHan]);
	ZobrazToN();
}

int argc;
const char **argv;

int main( int __argc, const char *__argv[] )
{
	argc=__argc,argv=__argv;
	return MenuMain();
}
