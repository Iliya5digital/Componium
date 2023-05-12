/* Componium - obsluha menu */
/* SUMA, 10/1992-11/1995 */

#include <macros.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <tos.h>
#include <sttos\samples.h>
#include <stdc\dynpole.h>
#include <gem\interf.h>
#include <gem\multiitf.h>

#include "compo.h"
#include "digill.h"
#include "ramcompo.h"

/* pr ce s jednoduchìmi tah tky v oknØ */

static void AddSlide( OBJECT *T, int Bak, int Sld, Flag Vert, int Offs )
{
	int Pos,Pos0;
	Pos0=Pos=PosSlide(T,Bak,Sld,True);
	Pos+=Offs;
	if( Pos<0 ) Pos=0;
	ef( Pos>1000 ) Pos=1000;
	if( Pos0!=Pos )
	{
		SetSlideBox(T,Bak,Sld,Vert,Pos,True);
	}
}

static void DoSlideBak( OBJECT *T, int Bak, int Sld, Flag Vert )
{
	if( MysSlide(T,Bak,Sld,Vert)<=PosSlide(T,Bak,Sld,Vert) )
	{
		AddSlide(T,Bak,Sld,Vert,-100);
	}
	else
	{
		AddSlide(T,Bak,Sld,Vert,+100);
	}
}
static void DoSlideTah( OBJECT *DT, int Bak, int Sld, Flag Vert )
{
	int Pos;
	Pos=SlideBox(DT,Bak,Sld,Vert);
	SetSlideBox(DT,Bak,Sld,Vert,Pos,True);
}

/* ----- */

static void NaPozici( OknoInfo *OI, CasT CasPos )
{
	CasT CasDel;
	CasDel=CasDelka(SekvenceS(&OI->Kurs),NULL);
	if( CasPos>CasDel ) CasPos=CasDel;
	NajdiSCasS(&OI->Kurs,CasPos);
	NaPoziciKurs(OI);
}

void ZobrazRekurzi( OknoInfo *OI )
{
	Flag Rekur=OI->Rozvoj;
	menu_icheck(Menu,NOTYI,Rekur);
	if( ToolOkno()>=0 )
	{
		ToolSelect(DSKPARTB,!Rekur);
	}
}

static void NastavRekurzi( Flag Rekur, int LHan )
{
	int i;
	OknoInfo *OI;
	if( LHan<0 ) return;
	OI=OInfa[LHan];
	if( OI->Rozvoj!=Rekur ) /* normal. bool. */
	{
		/* vypnout rekurzi */
		for( i=0; i<OI->NSoub; i++ ) RezimS(&OI->Soub[i],Rekur);
		RezimS(&OI->Kurs,Rekur);
		/*
		if( OI->ZBStopa>=0 ) RezimS(&OI->ZBlok,Rekur);
		if( OI->KBStopa>=0 ) RezimS(&OI->KBlok,Rekur);
		*/ /* bloky vzdy nerekursivni */
		OI->Rozvoj=Rekur;
		NajdiCasS(&OI->Soub[OI->KKan],CasJeS(&OI->Soub[OI->KKan]));
		PosunCas(OI);
	}
	ZobrazRekurzi(OI);
}

static Pisen *AktPisen;
static Sekvence *_S;
static Pisen *_P;
static WindRect *_Obr;
static WindRect *_Orig;
static const Kontext *_Kont;
static CasT _KCas;
static int _KKan;

void SupsPrac( int LHan, const ClipRect *A, const WindRect *W )
{
	(void)LHan;
	DoSupsPrac(A,W,&PracMFDB);
}

static Flag LSekvOtevri( int LHan )
{
	Okno *O=Okna[LHan];
	OknoInfo *OI=myalloc(sizeof(OknoInfo),True);
	if( !OI ) return False;
	OInfa[LHan]=OI;
	OI->LHan=LHan;
	SetNazev(O,TvorNazev(_S,LHan,_P->Zmena),False);
	OI->VNazvuZmena=_P->Zmena;
	{
		Sekvence *H;
		int I;
		for( I=0,H=_S; H && I<NMaxHlasu; H=DalsiHlas(H),I++ )
		{
			SekvSoubor *s=&OI->Soub[I];
			OtevriS(s,H,Cteni);
			if( _Kont ) SetKKontext(AktK(s),_Kont);
		}
		OI->NSoub=I;
		OI->IStopa=0;
		OI->NStop=min(OI->NSoub,OknoMaxStopA);
		OI->KR.x=OI->KR.y=-1;
		OI->ZobrazKursor=True;
		OI->ZBStopa=OI->KBStopa=-1;
		OI->Rozvoj=False;
		_S->Zmeny=False;
		O->WType=NOknoType;
		/* definice metod */
		O->Kresleni=KresliNoty;
		O->SupsPrac=SupsPrac;
		O->DoSipky=DoSipky;
		O->Stisk=OknoStisk;
		O->Stisk2=OknoStisk2;
		O->Znak=OknoZnak;
		O->DoMidi=OknoMidi;
		O->Pust=NULL;
		O->Pust2=NULL;
		O->Okrouhli=Okrouhli;
		O->Plne=PlneOkno;
		O->Zmenseno=Zmenseno;
		O->LzeTopnout=LzeTopnout;
		O->DoHSlide=DoHSlide;
		O->DoVSlide=DoVSlide;
		O->LogOZavri=LogZavri;
		O->FLogOZavri=FLogZavri;
		O->Kursor=NotyKursor;
		/* spec. implem. */
		O->Spec=OI;
		O->SDruh=SDNoty;
		if( _KKan>=0 && OI->NSoub>_KKan )
		{
			OI->KKan=_KKan;
			RozdvojS(&OI->Kurs,&OI->Soub[OI->KKan]);
			IStopaZKKan(OI);
			NastavRekurzi(StavMenu(Menu,NOTYI),LHan);
			NaPozici(OI,_KCas);
		}
		else
		{
			OI->KKan=0;
			RozdvojS(&OI->Kurs,&OI->Soub[0]);
			NastavRekurzi(StavMenu(Menu,NOTYI),LHan);
			NajdiSCasS(&OI->Kurs,0);
		}
		if( !_Obr ) AutoRozmer(&O->DefO,LHan);
		else O->DefO=*_Obr,OkrObrys(&O->DefO,LHan);
		if( _Orig && _Orig->x>=0 ) GrowBox(_Orig,&O->DefO);
	}
	return True;
}

static Flag LogOtevri( int LHan )
{
	Pisen *P;
	Sekvence *S;
	if( AktPisen ) P=AktPisen;
	else P=VyberPisen(FreeString(SEKVZFS));
	if( !P ) return False;
	S=VyberSekv(P,FreeString(SEKVFS),False,NULL,NULL,False);
	if( !S ) return False;
	_S=S;
	_P=P;
	_Obr=_Orig=NULL;
	_KCas=-1;
	_KKan=-1;
	_Kont=NULL;
	return LSekvOtevri(LHan);
}

void NejsouOkna( void ) {AlertRF(NOKNA,1);}
void ChybaRAM( void ) {AlertRF(RAMEA,1);}

Flag SekvOtevri( Pisen *P, Sekvence *S, WindRect *Obr, CasT KCas, int KKan, WindRect *Orig, const Kontext *Kont )
{
	_S=S;
	_P=P;
	_Obr=Obr;
	_Orig=Orig;
	_KCas=KCas;
	_KKan=KKan;
	_Kont=Kont;
	return Otevri(LSekvOtevri,NejsouOkna);
}

void FLogZavri( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	int I;
	for( I=0; I<OI->NSoub; I++ ) ZavriS(&OI->Soub[I]);
	ZavriS(&OI->Kurs);
	ZrusBlok(LHan);
	OI->NSoub=0;
	myfree(OI);
	OInfa[LHan]=NULL;
	/* Okno pousti az FyzZavri */
}

static int OkenPisne( const Pisen *P )
{
	int N=0;
	int i;
	for( i=0; i<NMaxOken; i++ ) if( Okna[i] && OknoJeSekv(i) )
	{ /* zavreni otevrenych oken */
		OknoInfo *OI=OInfa[i];
		if( SekvenceOI(OI)->Pis==P ) N++;
	}
	return N;
}

Flag LogZavri( int LHan )
{
	Plati( OknoJeSekv(LHan) )
	{
		OknoInfo *OI=OInfa[LHan];
		Pisen *P=SekvenceOI(OI)->Pis;
		if( OkenPisne(P)<=1 )
		{
			Flag ret;
			PovolCinMenu(False);
			if( P->Zmena )
			{
				int a;
				do
				{
					VsePrekresli();
					a=AlertRF(CLOSEA,1,NajdiNazev(P->NazevS));
					if( a==1 ) ret=UlozFPisen(P,False);
					ef( a==2 ) ret=True;
					else ret=False;
				}
				while( a==1 && P->Zmena );
			}
			else ret=True;
			if( ret )
			{
				FLogZavri(LHan);
				PlatiProc( ZavriFFPisen(P), ==True);
			}
			PovolCinMenu(True);
			return ret;
		}
		FLogZavri(LHan);
		return True;
	}
}

static int ListHan=-1;
#define ListOkno() (ListHan)

static int AboutHan=-1;

static void ZrusAbout( void )
{
	AboutHan=-1;
}
void OtevriAbout( void )
{
	if( AboutHan<0 )
	{
		AboutHan=OtevriCDial(RscTree(ABOUT),NULL,NULL,-1,NULL,False);
		PoZavreni(AboutHan,ZrusAbout);
	}
	else Topni(AboutHan);
}

static void VZavri( void )
{
	if( Vrchni>=0 )
	{
		if( OknoJeSekv(Vrchni) || Vrchni==AboutHan ) Zavri(Vrchni);
	}
}

int JeOtevrena( Sekvence *S, const Kontext *K )
{
	int LHan;
	for( LHan=0; LHan<NMaxOken; LHan++ )
	{
		Okno *O=Okna[LHan];
		if( O && OknoJeSekv(LHan) )
		{
			OknoInfo *OI=OInfa[LHan];
			SekvSoubor *s=&OI->Soub[0];
			if( SekvenceS(s)==S /*&& ( !K || CmpKontext(K,KKontextS(s)) )*/ ) return LHan;
			(void)K;
		}
	}
	return -1;
}

static Flag TestKurs( JednotkaBuf B, int Povel, Flag Hledat )
{
	SekvSoubor k;
	Flag ret=False;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		ret=SInitEdit(&k,Povel,OI->KKan,CasKursoru(OI),Hledat);
		if( ret>0 )
		{
			StrPosT pos,kpos;
			KdeJeS(&k,&pos);
			KdeJeS(&OI->Kurs,&kpos);
			if( !Hledat && CmpPos(&pos,&kpos) )
			{
				/* pýesuneme kursor na novou posici */
				NajdiS(&OI->Kurs,&pos);
				UkazKursor(OI);
				KresliKursor(LHan);
			}
			EquJed(B,BufS(&k));
		}
		ZavriS(&k);
	}
	return ret;
}

int PrvniKEdit( OknoInfo *OI )
{
	if( StavMenu(Menu,VSEHLAI) ) return 0;
	else return OI->KKan;
}
int DalsiKEdit( OknoInfo *OI, int Kan )
{
	if( StavMenu(Menu,VSEHLAI) && ++Kan<OI->NSoub ) return Kan;
	return -1;
}

/* ------------- */

const char *TempoText( int Tempo );

/* viz tez HlasitD */
const static char HlasS[][4]={"ppp","pp","p","mp","mf","f","ff","fff"};
const static int HlasUr[]=   { 1*2,  16*2,32*2,52*2,80*2,96*2,112*2,127*2};
/* hodnoty podle definice MIDI */

enum {NHlas=(int)lenof(HlasS)};

static int HlasUroven( int H ) /* zjisteni indexu textu */
{
	int MI=0;
	int MIN=0x7fff;
	int I;
	for( I=0; I<(int)lenof(HlasUr); I++ )
	{
		int d=abs(H-HlasUr[I]);
		if( d<MIN ) MIN=d,MI=I;
	}
	Plati( MI>=0 && MI<NHlas );
	return MI;
}

static int UrovenHlas( int H ) /* zjisteni vykonu podle indexu */
{
	if( H<0 ) H=0;
	ef( H>=NHlas ) H=NHlas-1;
	return HlasUr[H];	
}

const char *HlasitText( int H )
{
	return HlasS[HlasUroven(H)];
}


/* ------------- */

static int TonNK;
static Flag TonDur;

#define TonNicS "  -----   "

static Flag TonDurPD( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *DT=RscTree(TONINAD);
	OBJECT *PDD=RscTree(TONDURPD);
	int ret;
	WindRect O;
	ObjcRect(DT,TONDRPDT,&O);
	ret=DPopUp(PDD,TonNK+DURCI-1,&O);
	if( ret>0 )
	{
		TonNK=ret-(DURCI-1);
		TonDur=True;
		SetStrTed(TI(DT,TONMLPDT),TonNicS);
		SetStrTed(TI(DT,TONDRPDT),PDD[ret+1].ob_spec.free_string);
		RedrawI(DT,TONMLPDT);
		RedrawI(DT,TONDRPDT);
	}
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag TonMolPD( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *DT=RscTree(TONINAD);
	OBJECT *PDD=RscTree(TONMOLPD);
	int ret;
	WindRect O;
	ObjcRect(DT,TONMLPDT,&O);
	ret=DPopUp(PDD,TonNK+MOLAI-1,&O);
	if( ret>0 )
	{
		TonNK=ret-(MOLAI-1);
		TonDur=False;
		SetStrTed(TI(DT,TONMLPDT),PDD[ret+1].ob_spec.free_string);
		SetStrTed(TI(DT,TONDRPDT),TonNicS);
		RedrawI(DT,TONMLPDT);
		RedrawI(DT,TONDRPDT);
	}
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Button TonTlac[]=
{
	{TonDurPD,TONDRPDT},
	{TonMolPD,TONMLPDT},
	{NULL},
};

void ToninaD( Flag Hledat, const WindRect *K, Flag Male )
{
	Flag Dur;
	int NK,NEK;
	JednotkaBuf B;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		if( TestKurs(B,PTonina,Hledat) ) Dur=B[2],NK=B[1];
		else Dur=True,NK=ERR;
		if( !Male || NK>4 || NK<-4 )
		{
			OBJECT *DT=RscTree(TONINAD);
			if( NK<=ERR ) NK=0;
			if( Dur )
			{
				OBJECT *PDD=RscTree(TONDURPD);
				SetStrTed(TI(DT,TONDRPDT),PDD[NK+DURCI].ob_spec.free_string);
				SetStrTed(TI(DT,TONMLPDT),TonNicS);
			}
			else
			{
				OBJECT *PDM=RscTree(TONMOLPD);
				SetStrTed(TI(DT,TONDRPDT),TonNicS);
				SetStrTed(TI(DT,TONMLPDT),PDM[NK+MOLAI].ob_spec.free_string);
			}
			TonNK=NK,TonDur=Dur;
			if( FForm(DT,TonTlac,NULL,K)==TONOKB ) NEK=TonNK,Dur=TonDur;
			else NEK=ECAN;
		}
		else
		{
			int P;
			OBJECT *DP=RscTree(DURMOLPD);
			WindRect W;
			Plati( NK>=-4 && NK<=4 );
			Plati( K );
			MysXY(&W.x,&W.y);
			W.y-=8,W.x=0,W.w=640,W.h=16;
			WindSect(&W,K);
			P=DPopUp(DP,NK+4,&W);
			if( P>=0 ) NEK=P-4;
			else NEK=ERR;
		}
		if( NEK>ERR )
		{
			int Kan;
			Mouse(HOURGLASS);
			for( Kan=PrvniKEdit(OI); Kan>=0; Kan=DalsiKEdit(OI,Kan) )
			{
				ZmenaToniny(NEK,Dur,Kan,CasKursoru(OI),Hledat);
			}
			NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
			IgnKresli(LHan);
		}
	}
}

static Flag TaktNahoru( OBJECT *Bt, Flag Dvoj, int Asc )
{
	IncTed(RscTree(TAKTD),THORT,9);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag TaktDolu( OBJECT *Bt, Flag Dvoj, int Asc )
{
	DecTed(RscTree(TAKTD),THORT,2);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag DTaktNahoru( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *F=RscTree(TAKTD);
	TEDINFO *Ted=TI(F,TDOLT);
	int T=(int)IntTed(Ted);
	if( T<8 )
	{
		T*=2;
		SetIntTed(Ted,T);
		RedrawI(F,TDOLT);
	}
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag DTaktDolu( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *F=RscTree(TAKTD);
	TEDINFO *Ted=TI(F,TDOLT);
	int T=(int)IntTed(Ted);
	if( T>2 )
	{
		T/=2;
		SetIntTed(Ted,T);
		RedrawI(F,TDOLT);
	}
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Button TaktBut[]=
{
	{TaktNahoru,THORHB},
	{TaktDolu,THORDB},
	{DTaktNahoru,TDOLHB},
	{DTaktDolu,TDOLDB},
	{NULL},
};

static FormZrychl TaktKlav[]=
{
	{72,-1,-1,THORHB,},{80,-1,-1,THORDB,},
	{77,-1,-1,TDOLHB,},{75,-1,-1,TDOLDB,},
	{1,-1,-1,TAKTCANB,},
	{0,0,0,0,}
};

static OBJECT *ToolObj( void )
{
	int LHan=ToolOkno();
	if( LHan>=0 )
	{
		WindDial *OI=OInfa[LHan];
		return OI->D.Form;
	}
	return NULL;
}

void TaktD( Flag Hledat, const WindRect *K, Flag Male )
{
	OBJECT *DT=RscTree(TAKTD);
	int H,D;
	JednotkaBuf B;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		if( TestKurs(B,PNTakt,True) ) H=B[1],D=B[2];
		else H=D=4;
		if( Male )
		{
			static Polozka Pol[]={"  2/8 ","  3/8 ","  6/8 ","  2/4 ","  3/4 ","  4/4 ",""};
			static char PolD[]="  x/y ";
			int P=0,i;
			WindRect W;
			PolD[2]=H+'0',PolD[4]=D+'0';
			for( i=0; i<(int)lenof(Pol); i++ )
			{
				if( !strcmp(PolD,Pol[i].Text) ) {P=i;break;}
			}
			Plati( K );
			MysXY(&W.x,&W.y);
			W.y-=8,W.x=0,W.w=640,W.h=16;
			WindSect(&W,K);
			P=PopUp(Pol,P,&W);
			if( P>=0 )
			{
				const char *Pl=Pol[P].Text;
				H=Pl[2]-'0';
				D=Pl[4]-'0';
			}
			else H=D=ERR;
		}
		else
		{
			SetIntTed(TI(DT,THORT),H);
			SetIntTed(TI(DT,TDOLT),D);
			if( FForm(DT,TaktBut,TaktKlav,K)==TAKTOKB )
			{
				H=(int)IntTed(TI(DT,THORT));
				D=(int)IntTed(TI(DT,TDOLT));
			}
			else H=D=ERR;
		}
		if( H>=EOK )
		{
			int Kan;
			Mouse(HOURGLASS);
			for( Kan=PrvniKEdit(OI); Kan>=0; Kan=DalsiKEdit(OI,Kan) )
			{
				ZmenaTaktu(H,D,Kan,CasKursoru(OI),Hledat);
			}
			NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
			IgnKresli(LHan);
		}
	}
}

static Flag TempBakP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(TEMPOD);
	int Val;
	if( MysSlide(T,TMPBKGB,TMPSLDB,False)<=PosSlide(T,TMPBKGB,TMPSLDB,False) )  AddTed(T,TEMPOT,-10,MinTemp,MaxTemp);
	else AddTed(T,TEMPOT,10,MinTemp,MaxTemp);
	Val=(int)(1000*(IntTed(TI(T,TEMPOT))-MinTemp)/(MaxTemp-MinTemp));
	SetSlideBox(T,TMPBKGB,TMPSLDB,False,Val,True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Flag TempTahP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(TEMPOD);
	int Tmp,Val;
	Tmp=SlideBox(T,TMPBKGB,TMPSLDB,False);
	Tmp=(int)((Tmp*(long)(MaxTemp-MinTemp)+500)/1000)+MinTemp;
	if( Tmp<MinTemp ) Tmp=MinTemp;
	if( Tmp>MaxTemp ) Tmp=MaxTemp;
	SetIntTed(TI(T,TEMPOT),Tmp);
	Val=(int)(1000*(IntTed(TI(T,TEMPOT))-MinTemp)/(MaxTemp-MinTemp));
	SetSlideBox(T,TMPBKGB,TMPSLDB,False,Val,True);
	RedrawI(T,TEMPOT);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag TempoNahoru( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(TEMPOD);
	int Val;
	IncTed(T,TEMPOT,MaxTemp);
	Val=(int)(1000*(IntTed(TI(T,TEMPOT))-MinTemp)/(MaxTemp-MinTemp));
	SetSlideBox(T,TMPBKGB,TMPSLDB,False,Val,True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag TempoDolu( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(TEMPOD);
	DecTed(T,TEMPOT,MinTemp);
	SetSlideBox(T,TMPBKGB,TMPSLDB,False,(int)(1000*(IntTed(TI(T,TEMPOT))-MinTemp)/(MaxTemp-MinTemp)),True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Button TempoBut[]=
{
	{TempoNahoru,TEMPHB},
	{TempoDolu,TEMPDB},
	{TempTahP,TMPSLDB},
	{TempBakP,TMPBKGB},
	{NULL},
};
static FormZrychl TempoKlav[]=
{
	{75,-1,-1,TEMPDB,},{77,-1,-1,TEMPHB,},
	{1,-1,-1,TEMPCANB,},
	{0,0,0,0,}
};
void TempoD( Flag Hledat, const WindRect *K, Flag Male )
{
	OBJECT *DT=RscTree(TEMPOD);
	TEDINFO *TI=TI(DT,TEMPOT);
	JednotkaBuf B;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		int H;
		OknoInfo *OI=OInfa[LHan];
		if( TestKurs(B,PTempo,True) ) H=B[1];
		else H=Tempo;
		if( Male )
		{
			enum {KrokTemp=5,NPol=(MaxTemp-MinTemp)/KrokTemp};
			Polozka *Pol;
			int P=0,i;
			WindRect W;
			P=(H-MinTemp)/KrokTemp;
			H=ERR;
			Pol=myalloc(sizeof(Polozka)*NPol+1,False);
			if( !Pol ) ChybaRAM();
			else
			{
				for( i=0; i<NPol; i++ )
				{
					sprintf(Pol[i].Text,"  %d",MinTemp+i*KrokTemp);
				}
				strcpy(Pol[i].Text,"");
				Plati( K );
				MysXY(&W.x,&W.y);
				W.y-=8,W.x=0,W.w=640,W.h=16;
				WindSect(&W,K);
				P=PopUp(Pol,P,&W);
				if( P>=0 )
				{
					const char *Pl=Pol[P].Text;
					H=(int)atol(&Pl[2]);
				}
				myfree(Pol);
			}
		}
		else
		{
			SetIntTed(TI,H);
			SetSlideBox(DT,TMPBKGB,TMPSLDB,False,(int)((1000L*(H-MinTemp))/(MaxTemp-MinTemp)),False);
			if( EFForm(DT,TempoBut,TempoKlav,TEMPOT,K)==TEMPOKB ) H=(int)IntTed(TI);
			else H=ERR;
		}
		if( H>=0 )
		{
			int Kan;
			if( H>MaxTemp ) H=MaxTemp;
			ef( H<MinTemp ) H=MinTemp;
			Mouse(HOURGLASS);
			for( Kan=PrvniKEdit(OI); Kan>=0; Kan=DalsiKEdit(OI,Kan) )
			{
				ZmenaTempa(H,Kan,CasKursoru(OI),Hledat);
			}
			NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
			IgnKresli(LHan);
		}
	}
}

static void SetLPTed( TEDINFO *T, int Val )
{
	char Buf[20];
	if( Val<0 ) sprintf(Buf,"%s%d",FreeString(STERLFS),-Val);
	ef( Val>0 ) sprintf(Buf,"%s%d",FreeString(STERPFS),+Val);
	else Buf[0]=0;
	SetStrTed(T,Buf);
}
static int LPTed( TEDINFO *T )
{
	int ret;
	const char *S=StrTed(T);
	if( !*S ) return 0;
	if( *S==*FreeString(STERLFS) ) ret=-atoi(S+1);
	ef( *S==*FreeString(STERPFS) ) ret=atoi(S+1);
	else return 0;
	if( ret<MinSt ) ret=MinSt;
	if( ret>MaxSt ) ret=MaxSt;
	return ret;
}

static void SetLPSlid( OBJECT *DT, int BKB, int SLB, int Val, Flag Draw )
{
	SetSlideBox(DT,BKB,SLB,False,(int)((1000L*(Val-MinSt))/(MaxSt-MinSt)),Draw);
	SetLPTed(TI(DT,SLB),Val);
	if( Draw ) RedrawI(DT,SLB);
}

static void AddLPTed( OBJECT *T, int I, int S )
{
	TEDINFO *A=TI(T,I);
	int V=LPTed(A);
	int V0=V;
	V+=S;
	if( V>MaxSt ) V=MaxSt;
	ef( V<MinSt ) V=MinSt;
	if( V!=V0 ) SetLPTed(A,V);
}

static void FSterBak( int B, int S )
{
	OBJECT *T=RscTree(STEREOD);
	if( MysSlide(T,B,S,False)<=PosSlide(T,B,S,False) ) AddLPTed(T,S,-10);
	else AddLPTed(T,S,10);
	SetLPSlid(T,B,S,LPTed(TI(T,S)),True);
}

static void FSterTah( int B, int S )
{
	OBJECT *T=RscTree(STEREOD);
	int Tmp=SlideBox(T,B,S,False);
	Tmp=(int)((Tmp*(long)(MaxSt-MinSt)+500)/1000)+MinSt;
	if( Tmp<MinSt ) Tmp=MinSt;
	if( Tmp>MaxSt ) Tmp=MaxSt;
	Tmp/=10;
	Tmp*=10;
	SetLPSlid(T,B,S,Tmp,True);
}

static Flag SterBBak( OBJECT *Bt, Flag Dvoj, int Asc )
{
	FSterBak(STRBKBB,STRSLBB);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag SterTBak( OBJECT *Bt, Flag Dvoj, int Asc )
{
	FSterBak(STRBKTB,STRSLTB);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag SterBTah( OBJECT *Bt, Flag Dvoj, int Asc )
{
	FSterTah(STRBKBB,STRSLBB);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag SterTTah( OBJECT *Bt, Flag Dvoj, int Asc )
{
	FSterTah(STRBKTB,STRSLTB);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Flag SterTPra( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(STEREOD);
	AddLPTed(T,STRSLTB,+1);
	SetLPSlid(T,STRBKTB,STRSLTB,LPTed(TI(T,STRSLTB)),True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag SterTLev( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(STEREOD);
	AddLPTed(T,STRSLTB,-1);
	SetLPSlid(T,STRBKTB,STRSLTB,LPTed(TI(T,STRSLTB)),True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag SterBPra( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(STEREOD);
	AddLPTed(T,STRSLBB,+1);
	SetLPSlid(T,STRBKBB,STRSLBB,LPTed(TI(T,STRSLBB)),True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag SterBLev( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(STEREOD);
	AddLPTed(T,STRSLBB,-1);
	SetLPSlid(T,STRBKBB,STRSLBB,LPTed(TI(T,STRSLBB)),True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Button SterBut[]=
{
	{SterTTah,STRSLTB},{SterBTah,STRSLBB},
	{SterTBak,STRBKTB},{SterBBak,STRBKBB},
	{SterTLev,STRLFTB},{SterBLev,STRLFBB},
	{SterTPra,STRRTTB},{SterBPra,STRRTBB},
	{NULL},
};

void StereoD( Flag Male )
{
	WindRect K;
	OBJECT *DT=RscTree(STEREOD);
	int B,T; /* basy, trebly */
	JednotkaBuf J;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		if( TestKurs(J,PStereo,True) ) B=J[1],T=J[2];
		else B=MinSt,T=MaxSt;
		SetLPSlid(DT,STRBKBB,STRSLBB,B,False);
		SetLPSlid(DT,STRBKTB,STRSLTB,T,False);
		(void)Male;
		KursorRect(&K,LHan);
		if( FForm(DT,SterBut,NULL,&K)==STEOKB )
		{
			int B=(int)LPTed(TI(DT,STRSLBB));
			int T=(int)LPTed(TI(DT,STRSLTB));
			Mouse(HOURGLASS);
			ZmenaSterea(B,T,OI->KKan,CasKursoru(OI));
			NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
			IgnKresli(LHan);
		}
	}
}

/* Efekty */

static Flag EffBak( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(EFFPARD);
	int I=(int)(Bt-T);
	(void)Bt,(void)Dvoj,(void)Asc;
	DoSlideBak(T,I,I+1,True);
	return False;
}
static Flag EffTah( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *DT=RscTree(EFFPARD);
	int I=(int)(Bt-DT);
	(void)Bt,(void)Dvoj,(void)Asc;
	DoSlideTah(DT,I-1,I,True);
	return False;
}

static Button EffBut[]=
{
	{EffBak,EFFABAKB},{EffTah,EFFABAKB+1},
	{EffBak,EFFBBAKB},{EffTah,EFFBBAKB+1},
	{NULL},
};

void EfektD( Flag Male )
{
	WindRect K;
	OBJECT *DT=RscTree(EFFPARD);
	int EffA,EffB;
	JednotkaBuf J;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		if( TestKurs(J,PEfekt,True) ) EffA=J[1],EffB=J[2];
		else EffA=0,EffB=0;
		SetSlideBox(DT,EFFABAKB,EFFABAKB+1,True,1000-EffA,False);
		SetSlideBox(DT,EFFBBAKB,EFFBBAKB+1,True,1000-EffB,False);
		(void)Male;
		KursorRect(&K,LHan);
		if( FForm(DT,EffBut,NULL,&K)==EFFOKB )
		{
			int EffA=1000-PosSlide(DT,EFFABAKB,EFFABAKB+1,True);
			int EffB=1000-PosSlide(DT,EFFBBAKB,EFFBBAKB+1,True);
			Mouse(HOURGLASS);
			ZmenaEfektu(EffA,EffB,OI->KKan,CasKursoru(OI));
			NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
			IgnKresli(LHan);
		}
	}
}

static Flag MixBak( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(MIXERD);
	int I=(int)(Bt-T);
	(void)Bt,(void)Dvoj,(void)Asc;
	DoSlideBak(T,I,I+1,True);
	return False;
}
static Flag MixTah( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *DT=RscTree(MIXERD);
	int I=(int)(Bt-DT);
	(void)Bt,(void)Dvoj,(void)Asc;
	DoSlideTah(DT,I-1,I,True);
	return False;
}

#define MixPole(grp) \
	{MixBak,grp+(EFSBAK1B-EFSAGRPB)}, \
	{MixBak,grp+(EFSBAK2B-EFSAGRPB)}, \
	{MixBak,grp+(EFSBAK3B-EFSAGRPB)}, \
	{MixBak,grp+(EFSBAK4B-EFSAGRPB)}, \
	{MixTah,grp+(EFSBAK1B-EFSAGRPB)+1}, \
	{MixTah,grp+(EFSBAK2B-EFSAGRPB)+1}, \
	{MixTah,grp+(EFSBAK3B-EFSAGRPB)+1}, \
	{MixTah,grp+(EFSBAK4B-EFSAGRPB)+1}

static Button MixBut[]=
{
	MixPole(EFSAGRPB),
	MixPole(EFSBGRPB),
	{NULL},
};

static const int EfsBaks[4]={EFSBAK1B,EFSBAK2B,EFSBAK3B,EFSBAK4B};

static void SetEfekt( OBJECT *DT, int I, EfektT *Efekt )
{
	int i;
	SetStrTed(TI(DT,I+(EFSTXT1T-EFSAGRPB)),"DLY");
	SetStrTed(TI(DT,I+(EFSTXT2T-EFSAGRPB)),"FDB");
	SetStrTed(TI(DT,I+(EFSTXT3T-EFSAGRPB)),"GAI");
	SetStrTed(TI(DT,I+(EFSTXT4T-EFSAGRPB)),"BAL");
	for( i=0; i<4; i++ )
	{
		int B=I+EfsBaks[i]-EFSAGRPB;
		SetSlideBox(DT,B,B+1,True,1000-Efekt->Pars[i],False);
	}
	DT[I+(EFSBAK4B-EFSAGRPB)].ob_flags|=HIDETREE;
	DT[I+(EFSTXT4T-EFSAGRPB)].ob_flags|=HIDETREE;
}
static Flag GetEfekt( OBJECT *DT, int I, EfektT *Efekt )
{
	Flag Zmena=False;
	int i;
	for( i=0; i<4; i++ )
	{
		int B=I+EfsBaks[i]-EFSAGRPB;
		int E=1000-PosSlide(DT,B,B+1,True);
		if( Efekt->Pars[i]!=E ) Efekt->Pars[i]=E,Zmena=True;
	}
	return Zmena;
}

static void MixerD( void )
{
	OBJECT *DT=RscTree(MIXERD);
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		Pisen *P=SekvenceOI(OI)->Pis;
		SetEfekt(DT,EFSAGRPB,&P->Efekt[0]);
		SetEfekt(DT,EFSBGRPB,&P->Efekt[1]);
		if( FForm(DT,MixBut,NULL,NULL)==EFSOKB )
		{
			Flag Zmena=False;
			Zmena|=GetEfekt(DT,EFSAGRPB,&P->Efekt[0]);
			Zmena|=GetEfekt(DT,EFSBGRPB,&P->Efekt[1]);
			if( Zmena ) P->Zmena=Zmena;
		}
	}
}

/* ---- */

enum {MaxHlas=LMaxVol};

static void SetHlas( OBJECT *F, int H, Flag Draw )
{
	SetStrTed(TI(F,HLAST),HlasitText(H));
	SetIntTed(TI(F,HLASCT),H);
	if( Draw ) RedrawI(F,HLAST),RedrawI(F,HLASCT);
	SetSlideBox(F,HLASBKGB,HLASSLDB,False,(int)((1000L*H)/MaxHlas),Draw);
}
static void AddHlas( OBJECT *F, int Ad )
{
	int V=(int)IntTed(TI(F,HLASCT));
	if( Ad<0 && V>0 || Ad>0 && V<MaxHlas )
	{
		V+=Ad;
		if( V<0 ) V=0;
		ef( V>MaxHlas ) V=MaxHlas;
		SetHlas(F,V,True);
	}
}
static int GetHlas( void )
{
	OBJECT *F=RscTree(HLASITD);
	return (int)IntTed(TI(F,HLASCT));
}
static Flag HlasNahoruP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *F=RscTree(HLASITD);
	AddHlas(F,1);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Flag HlasDoluP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *F=RscTree(HLASITD);
	AddHlas(F,-1);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag HlasTahP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(HLASITD);
	int Tmp;
	Tmp=SlideBox(T,HLASBKGB,HLASSLDB,False);
	Tmp=(int)((Tmp*(long)MaxHlas+500)/1000);
	if( Tmp<0 ) Tmp=0;
	if( Tmp>MaxHlas ) Tmp=MaxHlas;
	SetHlas(T,Tmp,True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag HlasBakP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(HLASITD);
	if( MysSlide(T,HLASBKGB,HLASSLDB,False)<=PosSlide(T,HLASBKGB,HLASSLDB,False) )  AddHlas(T,-MaxHlas/10);
	else AddHlas(T,MaxHlas/10);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Button HlasitTlac[]=
{
	{HlasNahoruP,HLASNB},
	{HlasDoluP,HLASDB},
	{HlasTahP,HLASSLDB},
	{HlasBakP,HLASBKGB},
	{NULL},
};
static FormZrychl HlasitKlav[]=
{
	{75,-1,-1,HLASDB,},{77,-1,-1,HLASNB,},
	{1,-1,-1,HLASCANB,},
	{0,0,0,0,}
};
void HlasitD( Flag Male )
{
	OBJECT *DT=RscTree(HLASITD);
	int H;
	JednotkaBuf B;
	WindRect K;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		if( TestKurs(B,PHlasit,True) ) H=B[1];
		else H=MaxHlas/2;
		SetHlas(DT,H,False);
		KursorRect(&K,LHan);
		if( !Male )
		{
			if( EFForm(DT,HlasitTlac,HlasitKlav,HLASCT,&K)==HLASOKB )
			{
				H=GetHlas();
			}
			else H=ERR;
		}
		else
		{
			static Polozka Pol[]=
			{
				"  ppp ","  pp  ","  p   ","  mp  ",
				"  mf  ","  f   ","  ff  ","  fff ",""
			};
			int P;
			P=PopUp(Pol,HlasUroven(H),&K);
			if( P>=0 )
			{
				H=UrovenHlas(P);
				Plati( H<LMaxVol );
			}
			else H=ERR;
		}
		if( H>=EOK )
		{
			ZmenaHlasit(H,OI->KKan,CasKursoru(OI));
			NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
			IgnKresli(LHan);
		}
	}
}

static Flag OktNahoru( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(OKTAVAD);
	IncTed(T,OKTT,+3);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag OktDolu( OBJECT *Bt, Flag Dvoj, int Asc )
{
	OBJECT *T=RscTree(OKTAVAD);
	DecTed(T,OKTT,-3);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Button OktavTlac[]=
{
	{OktNahoru,OKTNB},
	{OktDolu,OKTDB},
	{NULL},
};
static FormZrychl OktavKlav[]=
{
	{75,-1,-1,OKTDB,},{77,-1,-1,OKTNB,},
	{-1,-1,'a',OKTABSB},{-1,-1,'A',OKTABSB},
	{-1,-1,'r',OKTRELB},{-1,-1,'R',OKTRELB},
	{1,-1,-1,OKTCANB,},
	{0,0,0,0,}
};

void OktavaD( Flag Male )
{
	OBJECT *DT=RscTree(OKTAVAD);
	TEDINFO *TI=TI(DT,OKTT);
	int O;
	Flag Rel;
	JednotkaBuf B;
	WindRect K;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		SekvKursor k;
		Err ret;
		ret=InitEdit(&k,POktava,-1,CasKursoru(OI),False);
		if( ret>0 )
		{
			EquJed(B,BufK(&k));
			O=B[1],Rel=B[2];
		}
		else O=0,Rel=True;
		ZavriK(&k);
		KursorRect(&K,LHan);
		if( !Male )
		{
			SetIntTed(TI,O);
			Set01(&DT[OKTRELB],SELECTED,Rel);
			Set01(&DT[OKTABSB],SELECTED,!Rel);
			if( FForm(DT,OktavTlac,OktavKlav,&K)==OKTOKB )
			{
				O=(int)IntTed(TI);
				Rel=Test(&DT[OKTRELB],SELECTED);
			}
			else O=ERR;
		}
		else /* PopUp */
		{
			static Polozka Pol[]={"  -3 ","  -2 ","  -1 ","   0 ","  +1 ","  +2 ","  +3 ",""};
			int P=PopUp(Pol,O+3,&K);
			if( P>=0 ) O=P-3;
			else O=ERR;
		}
		if( O>MAXERR )
		{ 
			ZmenaOktavy(O,Rel,OI->KKan,CasKursoru(OI));
			NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
			IgnKresli(LHan);
		}
	}
}

void UzijNasF( int FNum, OknoInfo *OI )
{
	const char *S;
	Pisen *P=SekvenceOI(OI)->Pis;
	Plati( FNum>=0 && FNum<NFNum );
	S=NasF(P,FNum);
	if( !*S )
	{
		WindRect K;
		char Buf[80];
		KursorRect(&K,OI->LHan);
		sprintf(Buf,FreeString(PRINASFS),FNum+1);
		ZmenNasF(P,FNum,&K,Buf);
		S=NasF(P,FNum);
	}
	if( S && *S )
	{
		ZmenaNastroje(S,OI->KKan,CasKursoru(OI));
		NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
		IgnKresli(OI->LHan);
	}
}

void ZmenNasF( Pisen *P, int FNum, const WindRect *SR, const char *FText )
{
	const char *S;
	int Df;
	Plati( FNum>=0 && FNum<NFNum );
	S=P->NastrF[FNum];
	if( *S ) Df=NajdiRetez(P->Nastroje,S);
	else Df=-1;
	{
		WindRect K;
		char Buf[80];
		Plati( SR );
		K=*SR;
		sprintf(Buf,FText,FNum+1);
		S=VyberNastroj(P,P->Nastroje,Df,True,&K,False,Buf);
		if( S )
		{
			SetNasF(P,FNum,S);
			SetToNastroj(FNum,S,True);
		}
	}
}

void ZmenNas( Flag PovolMale )
{
	const char *S;
	BankaIndex Df;
	JednotkaBuf B;
	WindRect K;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		Pisen *P=SekvenceOI(OI)->Pis;
		if( TestKurs(B,PNastroj,False) ) Df=B[1];
		else Df=-1;
		KursorRect(&K,LHan);
		S=VyberNastroj(P,P->Nastroje,Df,True,&K,PovolMale,FreeString(NASTRFS));
		if( S )
		{
			ZmenaNastroje(S,OI->KKan,CasKursoru(OI));
			NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
			IgnKresli(LHan);
		}
	}
}

void KursorRect( WindRect *W, int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	Okno *O=Okna[LHan];
	if( OI->KR.x>=0 && OI->KR.y>=0 )
	{
		WindRect F;
		*W=OI->KR;
		W->x+=O->Uvnitr.x;
		W->y+=O->Uvnitr.y;
		GetWind(0,WF_FULLXYWH,&F);
		WindSect(W,&F);
	}
	else W->x=W->y=W->w=W->h=-1;
}

static void VyvolejS( void )
{
	Sekvence *S;
	WindRect K;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		KursorRect(&K,LHan);
		S=VyberSekv(SekvenceOI(OI)->Pis,FreeString(VYVSEKFS),True,&K,NULL,False);
		if( S )
		{
			VyvolaniS(S);
			IgnKresli(LHan);
		}
	}
}

void ZmenVyvolejS( void )
{
	Sekvence *S;
	WindRect K;
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		SekvKursor s;
		KursorRect(&K,LHan);
		if( InitEdit(&s,PVyvolej,-1,CasKursoru(OI),False) )
		{
			S=NajdiSekvBI(SekvenceOI(OI)->Pis,s.Buf[1],s.Buf[2]);
			ZavriK(&s);
			Plati( S );
			S=VyberSekv(SekvenceOI(OI)->Pis,FreeString(VYVSEKFS),True,&K,S,True);
			if( S )
			{
				ZmenaVyvolaniS(S);
				IgnKresli(LHan);
			}
		}
		else ZavriK(&s);
	}
}

static void PredtaktiD( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		int Kan;
		for( Kan=PrvniKEdit(OI); Kan>=0; Kan=DalsiKEdit(OI,Kan) )
		{
			Predtakti(Kan,CasKursoru(OI));
		}
		NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
		Kresli(LHan);
	}
}

static Flag RepUpP ( OBJECT *Bt, Flag Dvoj, int Asc )
{
	(void)Bt,(void)Dvoj,(void)Asc;
	IncTed(RscTree(REPETD),REPET,99);
	return False;
}
static Flag RepDoP ( OBJECT *Bt, Flag Dvoj, int Asc )
{
	(void)Bt,(void)Dvoj,(void)Asc;
	DecTed(RscTree(REPETD),REPET,1);
	return False;
}

static Button RepButt[]=
{
	{RepUpP,REPHB,},
	{RepDoP,REPDB,},
	{NULL},
};

static FormZrychl RepKlav[]=
{
	{75,-1,-1,REPDB},
	{77,-1,-1,REPHB},
	{1,-1,-1,REPCANB},
	{0,0,0,0},
};

int AskRepetD( int Opak, const WindRect *Orig )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OBJECT *DT=RscTree(REPETD);
		SetIntTed(TI(DT,REPET),Opak);
		if( EFForm(DT,RepButt,RepKlav,REPET,Orig)==REPOKB )
		{
			return (int)IntTed(TI(DT,REPET));
		}
	}
	return ECAN;
}

void EdRepetD( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		JednotkaBuf B;
		if( TestKurs(B,PRep,False) )
		{
			WindRect K;
			int NR;
			KursorRect(&K,LHan);
			NR=AskRepetD(B[1],&K);
			if( NR!=ECAN )
			{
				EdZacRepet(NR,OI->KKan,CasKursoru(OI));
				NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
				IgnKresli(LHan);
			}
		}
	}
}
static void ZacRepetD( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		WindRect K;
		int NR;
		KursorRect(&K,LHan);
		NR=AskRepetD(2,&K);
		if( NR!=ECAN )
		{
			ZacRepet(NR,OI->KKan,CasKursoru(OI));
			NajdiSCasS(&OI->Kurs,CasJeS(&OI->Kurs));
			IgnKresli(LHan);
		}
	}
}
static void KonRepetD( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		KonRepet(OI->KKan,CasKursoru(OI));
		Kresli(LHan);
	}
}

static FormZrychl AttrKlav[]=
{
	{-1,-1,'.',TECKAB,},
	{-1,-1,'3',TRIOLAB,},
	{-1,-1,'0',ZADDELB,},
	{-1,-1,'_',ZADLEGB,},	{-1,-1,'/',ZADLEGB,},
	{-1,-1,'#',KRIZEKBB,},{-1,-1,'+',KRIZEKBB,},{-1,-1,'(',KRIZEKBB,},
	{-1,-1,'B',BECKOBB,},{-1,-1,'-',BECKOBB,},{-1,-1,')',BECKOBB,},
	{-1,-1,'=',ODRAZBB,},{-1,-1,'*',ODRAZBB,},
	{-1,-1,' ',ZADVYSB,},
	{1,-1,-1,ATTRCANB,},
	{0,0,0,0,}
};

#define SelectIf(O,F) Set01((O),SELECTED,(F))
#define EnableIf(O,F) Set01((O),DISABLED,!(F))

void AttribD( Flag Ton, int *_VI, int *_DI, Flag *Lg, const WindRect *K )
{
	OBJECT *AtD=RscTree(ATTRIBD);
	int VI=*_VI,DI=*_DI;
	SelectIf(&AtD[ZADDELB],DI==0);
	SelectIf(&AtD[TECKAB],DI==1);
	SelectIf(&AtD[TRIOLAB],DI==2);
	SelectIf(&AtD[ZADVYSB],VI==0);
	SelectIf(&AtD[KRIZEKBB],VI==1);
	SelectIf(&AtD[BECKOBB],VI==2);
	SelectIf(&AtD[ODRAZBB],VI==3);
	SelectIf(&AtD[ZADLEGB],*Lg);
	EnableIf(&AtD[ZADVYSB],Ton);
	EnableIf(&AtD[KRIZEKBB],Ton);
	EnableIf(&AtD[BECKOBB],Ton);
	EnableIf(&AtD[ODRAZBB],Ton);
	EnableIf(&AtD[ZADLEGB],Ton);
	if( FForm(AtD,NULL,AttrKlav,K)==ATTROKB )
	{
		if( Test(&AtD[TECKAB],SELECTED) ) DI=1;
		ef( Test(&AtD[TRIOLAB],SELECTED) ) DI=2;
		else DI=0;
		if( Test(&AtD[KRIZEKBB],SELECTED) ) VI=1;
		ef( Test(&AtD[BECKOBB],SELECTED) ) VI=2;
		ef( Test(&AtD[ODRAZBB],SELECTED) ) VI=3;
		else VI=0;
		*Lg=Test(&AtD[ZADLEGB],SELECTED);
		*_VI=VI,*_DI=DI;
	}
}

static void CompoHraj( Flag Hraj, Flag OdKurs )
{
	Sekvence *SH;
	int LHan;
	Mouse(HOURGLASS);
	PovolCinMenu(False);
	LHan=HrajOkno();
	if( LHan>=0 ) FZavri(LHan);
	VsePrekresli();
	LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		SH=SekvenceOI(OI);
		if( SH )
		{
			CompoErr CErr;
			CasT Cas=( OdKurs ? CasJeS(&OI->Kurs) : 0 );
			Err ret=CompoCom(Hraj,SH,Cas,KKontextS(&OI->Soub[0]),&CErr);
			if( ret<OK )
			{
				if( CErr.Ukazat )
				{ /* zobrazit misto chyby - S je urcite vrchni */
					int Je=JeOtevrena(CErr.S,NULL);
					if( Je<0 )
					{
						int VHan=VrchniSekv();
						OknoInfo *VI;
						SekvOtevri(SH->Pis,CErr.S,NULL,CErr.CPos,CErr.Kan,NULL,NULL);
						VI=OInfa[VHan];
						NajdiS(&VI->Kurs,&CErr.PPos);
						IgnKresli(VHan);
					}
					else
					{
						OknoInfo *OI=OInfa[Je];
						Topni(Je);
						JdiNaPozici(OI,CErr.CPos,CErr.Kan);
						IStopaZKKan(OI);
						UkazKursor(OI);
						NaPozici(OI,CErr.CPos);
						NajdiS(&OI->Kurs,&CErr.PPos);
						IgnKresli(Je);
					}
				}
				else VsePrekresli();
				switch( ret )
				{
					case ERR: AlertF(CErr.Alert,1);break;
					default: Chyba(ret);break;
				}
			}
		}
	}
	PovolCinMenu(True);
}

static void PrepNastr( LLHraj *P )
{
	Err ret;
	if( AktHraj )
	{
		if( AktHraj!=P ) ret=P->JinSNastr();
		else ret=OK;
	}
	else ret=P->ZacSNastr();
	if( ret<OK ) Chyba(ret);
}
static void ZrusNastr( void )
{
	Err ret;
	if( AktHraj ) ret=AktHraj->KonSNastr();
	else ret=OK;
	if( ret<OK ) Chyba(ret);
}

static void StatusD( void )
{
	OBJECT *O=RscTree(STATUSD);
	int LHan=VrchniSekv();
	OknoInfo *OI=LHan>=0 ? OInfa[LHan] : NULL;
	long Fre=(long)Malloc(-1);
	Pisen *P=NULL;
	int r;
	long rb;
	SetIntTed(TI(O,STARAMT),(Fre+512)/1024);
	rb=F_BenchTune(-1);
	SetIntTed(TI(O,STABENT),rb/2990);
	if( !OI )
	{
		SetStrTed(TI(O,STAPOP1T),"");
		SetStrTed(TI(O,STAPOP2T),"");
		SetStrTed(TI(O,STASKLT),"");
		SetStrTed(TI(O,STAPARTT),"");
		SetStrTed(TI(O,STANASTT),"");
		O[STAPOP1T].ob_flags&=~EDITABLE;
		O[STAPOP2T].ob_flags&=~EDITABLE;
	}
	else
	{
		P=SekvenceOI(OI)->Pis;
		SetStrTed(TI(O,STASKLT),SizeNazev(P->NazevS,TI(O,STASKLT)->te_txtlen-1));
		SetStrTed(TI(O,STAPOP1T),P->Popis1);
		SetStrTed(TI(O,STAPOP2T),P->Popis2);
		O[STAPOP1T].ob_flags|=EDITABLE;
		O[STAPOP2T].ob_flags|=EDITABLE;
		SetIntTed(TI(O,STAPARTT),SpoctiBanku(P->NazvySekv));
		SetIntTed(TI(O,STANASTT),SpoctiBanku(P->Nastroje));
	}
	r=WFForm(O,NULL,NULL,OI ? STAPOP1T : -1,NULL,NULL);
	if( r==STAOKB )
	{
		if( P )
		{
			if( strcmp(P->Popis1,StrTed(TI(O,STAPOP1T))) || strcmp(P->Popis2,StrTed(TI(O,STAPOP2T))) )
			{
				strlncpy(P->Popis1,StrTed(TI(O,STAPOP1T)),sizeof(P->Popis1));
				strlncpy(P->Popis2,StrTed(TI(O,STAPOP2T)),sizeof(P->Popis2));
				P->Zmena=True;
			}
		}
	}
}

static void NajdiTaktD( int LHan )
{
	if( LHan>=0 )
	{
		OBJECT *O=RscTree(NAJDIRAD);
		OknoInfo *OI=OInfa[LHan];
		SetStrTed(TI(O,NRACIST),"");
		if( EFForm(O,NULL,NULL,NRACIST,NULL) )
		{
			int RB=(int)IntTed(TI(O,NRACIST));
			if( RB>0 )
			{
				NajdiTaktS(&OI->Kurs,RB-1);
				NaPoziciKurs(OI);
				Kresli(LHan);
			}
		}
	}
}

static void DoEditNas( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		Pisen *P=SekvenceOI(OI)->Pis;
		VyberNastroj(P,P->Nastroje,-1,False,NULL,False,FreeString(EDNASFS));
	}
	else
	{
		VyberNastroj(NULL,NULL,-1,False,NULL,False,FreeString(EDNASFS)); /* bez predvyberu */
	}
}

Flag ZavriVse( void )
{
	Pisen **P;
	int LHan;
	LHan=ListOkno();
	if( LHan>=0 ) Zavri(LHan); /* zavýi */
	LHan=HrajOkno();
	if( LHan>=0 ) Zavri(LHan); /* zahaj zav¡r n¡ */
	for( P=&Pisne[NMaxPisni-1]; P>=Pisne; P-- )	if( *P )
	{
		if( !ZavriFPisen(*P) ) return False;
	}
	VsePrekresli();
	LHan=HrajOkno();
	if( LHan>=0 ) FZavri(LHan); /* dokonŸi zav¡r n¡ */
	LHan=AboutHan;
	if( LHan>=0 ) Zavri(LHan);
	VsePrekresli();
	return True;
}

static Flag ZavriAbout( void )
{
	int LHan;
	LHan=AboutHan;
	if( LHan>=0 ) FZavri(LHan);
	VsePrekresli();
	return True;
}

typedef struct
{
	PathName name;
} ListItem;

static void *LDmyalloc( lword L ){return myallocSpc(L,True,'List');}

static DynPole ListData={sizeof(ListItem),16,LDmyalloc,myfreeSpc};
static Flag ListPaused;

static long ListI;

static void UkazListI( OBJECT *D, int DrawHan )
{
	char Buf[256];
	const char *Str;
	if( ListI<0 ) ListI=0;
	ef( ListI>NDyn(&ListData) ) ListI=NDyn(&ListData);
	if( ListPaused ) Str=FreeString(LSTSTPFS);
	else Str=FreeString(LSTPLYFS);
	sprintf(Buf,Str,ListI+1,NDyn(&ListData));
	if( SetStrTed(TI(D,LSTNUMT),Buf) && DrawHan>=0 )
	{
		DRedrawI(D,LSTNUMT,DrawHan);
	}
	if( ListI<NDyn(&ListData) )
	{
		ListItem *I=AccDyn(&ListData,ListI);
		strcpy(Buf,NajdiNazev(I->name));
	}
	else
	{
		strcpy(Buf,"");
	}
	if( SetStrTed(TI(D,LSTNAMT),Buf) && DrawHan>=0 )
	{
		DRedrawI(D,LSTNAMT,DrawHan);
	}
}

static void ListPauza( void )
{
	WindDial *WD=OInfa[ListHan];
	Okno *O=Okna[ListHan];
	int LHan=HrajOkno();
	if( LHan>=0 ) Zavri(LHan);
	Plati( ListHan>=0 );
	O->DoCas=NULL;
	ListPaused=True;
	UkazListI(WD->D.Form,WD->D.DHandle);
}

static Flag ListNew( OBJECT *Bt, Flag Dvoj, int Asc )
{
	KonDyn(&ListData);
	ZacDyn(&ListData);
	ListPauza();
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static PathName ListW,ListP;
static FileName ListN;

static Flag ListLoad( OBJECT *Bt, Flag Dvoj, int Asc )
{
	FILE *f;
	WindDial *WD=OInfa[ListHan];
	KonDyn(&ListData);
	ZacDyn(&ListData);
	ListPauza();
	if( !*ListP )
	{
		strcpy(ListP,PisenP);
		strcpy(NajdiPExt(NajdiNazev(ListP)),".PLS");
	}
	if( EFSel(FreeString(PLAYLSFS),ListP,ListN,ListW) )
	{
		f=fopen(ListW,"r");
		if( f )
		{
			char Buf[256];
			while( fgetl(Buf,(int)sizeof(Buf),f) )
			{
				ListItem *IT;
				long NI=NewDyn(&ListData);
				if( NI<0 ) {ChybaRAM();break;}
				IT=AccDyn(&ListData,NI);
				strlncpy(IT->name,Buf,sizeof(IT->name));
			}
			fclose(f);
		}
	}
	UkazListI(WD->D.Form,WD->D.DHandle);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag ListSave( OBJECT *Bt, Flag Dvoj, int Asc )
{
	FILE *f;
	if( !*ListP )
	{
		strcpy(ListP,PisenP);
		strcpy(NajdiPExt(NajdiNazev(ListP)),".PLS");
	}
	if( EFSel(FreeString(PLAYLSFS),ListP,ListN,ListW) )
	{
		f=fopen(ListW,"w");
		if( f )
		{
			long I;
			ListItem *IT;
			for( IT=PrvniDyn(&ListData,&I); IT; IT=DalsiDyn(&ListData,IT,&I) )
			{
				fprintf(f,"%s\n",IT->name);
			}
			fclose(f);
		}
	}
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag ListInsert( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *WD=OInfa[ListHan];
	static PathName ITP;
	PathName ITW;
	FileName ITN;
	if( !*ITP )
	{
		strcpy(ITP,PisenP);
	}
	*ITN=0;
	if( WEFSel(FreeString(PLAYLSFS),ITP,ITN,ITW) )
	{
		int F;
		DTA d,*od=Fgetdta();
		Fsetdta(&d);
		for( F=Fsfirst(ITW,FA_READONLY|FA_HIDDEN); F>=0; F=Fsnext() )
		{
			long NI=NewDyn(&ListData);
			PathName W;
			ListItem *IT;
			if( NI<0 ) {ChybaRAM();return False;}
			IT=AccDyn(&ListData,NI);
			strcpy(W,ITW);
			strcpy(NajdiNazev(W),d.d_fname);
			strlncpy(IT->name,W,sizeof(IT->name));
		}
		Fsetdta(od);
	}
	UkazListI(WD->D.Form,WD->D.DHandle);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static void ListCas( int LHan )
{
	Plati( LHan==ListHan );
	if( HrajOkno()<0 )
	{
		if( ListI<NDyn(&ListData) )
		{
			WindDial *WD=OInfa[ListHan];
			ListItem *IT=AccDyn(&ListData,ListI);
			Pisen *LP;
			PovolCinMenu(False);
			Mouse(HOURGLASS);
			VsePrekresli();
			UkazListI(WD->D.Form,WD->D.DHandle);
			LP=CtiPisen(IT->name);
			if( LP )
			{
				CompoErr CE;
				Sekvence *S=NajdiSekv(LP,MainMel,0);
				if( S )
				{
					Err ret;
					SekvKursor k;
					Kontext *K;
					OtevriK(&k,S,Cteni);
					K=KKontextK(&k);
					ret=CompoCom(True,S,0,K,&CE);
					/* spusœ hran¡ */
					ZavriK(&k);
					if( ret<EOK ) Chyba(ret);
				}
				MazPisen(LP);
			}
			ListI++;
			PovolCinMenu(True);
		}
		else ListPauza();
	}
}

static void ZrusListData( void )
{
	int LHan=HrajOkno();
	if( LHan>=0 ) Zavri(LHan);
	KonDyn(&ListData);
	ListHan=-1;
}

static Flag ListPlay( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *WD=OInfa[ListHan];
	Okno *O=Okna[ListHan];
	Plati( ListHan>=0 );
	O->DoCas=ListCas;
	ListPaused=False;
	UkazListI(WD->D.Form,WD->D.DHandle);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Flag ListStop( OBJECT *Bt, Flag Dvoj, int Asc )
{
	(void)Bt,(void)Dvoj,(void)Asc;
	ListPauza();
	return False;
}
static Flag ListNext( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *WD=OInfa[ListHan];
	int LHan=HrajOkno();
	if( LHan>=0 ) Zavri(LHan);
	else ListI++;
	(void)Bt,(void)Dvoj,(void)Asc;
	UkazListI(WD->D.Form,WD->D.DHandle);
	return False;
}
static Flag ListPrev( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *WD=OInfa[ListHan];
	int LHan=HrajOkno();
	if( LHan>=0 ) Zavri(LHan),ListI-=2;
	else ListI--;
	UkazListI(WD->D.Form,WD->D.DHandle);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Button LstTlac[]=
{
	ListNew,LSTNEWB,ListLoad,LSTLOADB,
	ListSave,LSTSAVEB,ListInsert,LSTINSB,
	ListPlay,LSTPLAYB,ListStop,LSTSTOPB,
	ListPrev,LSTPREVB,ListNext,LSTNEXTI,
	NULL,
};

static FormZrychl LstKlav[]=
{
	{82,-1,-1,LSTINSB,},
	{0,0,0,0,}
};

static void PlayList( void )
{
	if( ListHan>=0 ) Topni(ListHan);
	else
	{
		OBJECT *D=RscTree(LISTD);
		ZacDyn(&ListData);
		ListI=0;
		ListPaused=True;
		ZavriAbout();
		UkazListI(D,-1);
		ListHan=OtevriDial(D,LstTlac,LstKlav,-1,NULL,False);
		if( ListHan>=0 )
		{
			PoZavreni(ListHan,ZrusListData);
		}
	}
}

static void UsporadejOkna( void )
{
	int LHan;
	int AsHan=0;
	int THan=ToolOkno();
	int NHan=ToNOkno();
	int HHan=HrajOkno();
	int IHan=ListOkno();
	for( LHan=DolniOkno(); LHan>=0; )
	{
		if( OknoJeSekv(LHan) )
		{
			WindRect W;
			LAutoRozmer(&W,LHan,AsHan);
			ZmenRozmer(&W,LHan);
			AsHan++;
			LHan=VyssiOkno(LHan);
		}
		ef( LHan==HHan || LHan==IHan )
		{ /* tohle nech tam co je */
			LHan=VyssiOkno(LHan);
		}
		ef( LHan==NHan )
		{
			Okno *O=Okna[LHan];
			WindRect W=O->DefO;
			WindRect FD;
			GetWind(0,WF_FULLXYWH,&FD);
			W.x=FD.x+FD.w-W.w;
			W.y=(FD.y+FD.h-W.h)&~1;
			if( O->Pohyb ) O->Pohyb(LHan,&W);
			else ZmenRozmer(&W,LHan);
			LHan=VyssiOkno(LHan);
		}
		ef( LHan==THan || !Okna[LHan]->LogOZavri )
		{
			Okno *O=Okna[LHan];
			WindRect W=O->DefO;
			WindRect FD;
			GetWind(0,WF_FULLXYWH,&FD);
			W.x=FD.x;
			W.y=(FD.y+4)&~1;
			if( O->Pohyb ) O->Pohyb(LHan,&W);
			else ZmenRozmer(&W,LHan);
			LHan=VyssiOkno(LHan);
		}
		else
		{
			int NLHan=VyssiOkno(LHan); /* LHan prestane platit po Zavri */
			Zavri(LHan);
			LHan=NLHan;
		}
	}
	LHan=HorniOkno();
	if( LHan>=0 && !OknoJeSekv(LHan) )
	{
		for( LHan=DalsiOkno(LHan); LHan>=0; LHan=DalsiOkno(LHan) )
		{
			if( OknoJeSekv(LHan) ) {Topni(LHan);break;}
		}
	}
	LHan=HorniOkno();
	if( LHan>=0 && LHan!=Vrchni ) Topni(LHan);
}

static void OtevriSExt( Flag Nova )
{
	Pisen *P;
	PovolCinMenu(False);
	ZavriAbout();
	P=OtevriPisen(Nova);
	if( P )
	{
		Sekvence *S=NajdiSekv(P,MainMel,0);
		Flag SO;
		int Je;
		AktPisen=P;
		if( S )
		{
			Je=JeOtevrena(S,NULL);
			if( Je>=0 ) Topni(Je);
			else SO=SekvOtevri(P,S,NULL,-1,-1,NULL,NULL);
		}
		else SO=Otevri(LogOtevri,NejsouOkna);
		if( !SO ) ZavriFPisen(P);
	}
	PovolCinMenu(True);
}

static void VrchRect( WindRect *R )
{
	Plati( VrchniSekv()>=0 );
	KursorRect(R,VrchniSekv());
}

void DoMenu( int I, Flag IsMouse )
{
	Err ret;
	WindRect W;
	(void)IsMouse;
	if( PovoleneMenu(Menu,I) ) switch( I )
	{
		case KONECI:
			PovolCinMenu(False);
			UlozCFG();
			if( ZavriVse() )
			{
				Koneci();
				if( Konec )
				{
					ret=KonSNastr();
					if( ret<OK ) Chyba(ret);
					KonCFG();
				}
			}
			PovolCinMenu(True);
			break;
		case OZAVRI: VZavri(); break;
		case OHLASI: if( PocetPisni()>0 ) {AktPisen=NULL;Otevri(LogOtevri,NejsouOkna);break;}
		case IMPORTI:
			NastavExtPisne(".MID");
			OtevriSExt(False);
		break;
		case NOVAI:
			NastavExtPisne(".CPS");
			OtevriSExt(True);
		break;
		case OTEVRI:
			NastavExtPisne(".CPS");
			OtevriSExt(False);
		break;
		case ULOZI: UlozPisen();break;
		case ULOZDOI:
			if( Kbshift(-1)&3 ) CompoHraj(False,False); /* se shiftem preklad */
			else UlozJako(); /* jinak uloz do */
			break;
		case STATUSI: StatusD();break;
		case PROHODI: ProhodOkna(OknoJeSekv);break;
		case VYBERI:
		{
			int LHan=VyberOkno();
			if( LHan>=0 ) Topni(LHan);
			break;
		}
		case USPORI: UsporadejOkna();break;
		case NOTYI:
		{
			int LHan=VrchniSekv();
			if( LHan>=0 )
			{
				NastavRekurzi(!StavMenu(Menu,NOTYI),LHan);
				Kresli(LHan);
			}
			break;
		}
		case NHLASI: ZalozHlas();break;
		case ZHLASI: ZrusHlas();break;
		case PHLASI: ProhodHlas();break;
		case NASTROJI: ZmenNas(False);break;
		case VYVSEKI: VyvolejS();break;
		case VSEHLAI: PrepniMenu(Menu,I); break;
		case ZTONINAI: VrchRect(&W);ToninaD(False,&W,False);break;
		case ZTAKTI: VrchRect(&W);TaktD(False,&W,False);break;
		case ZTEMPOI: VrchRect(&W);TempoD(False,&W,False);break;
		case TONINAI: ToninaD(True,NULL,False);break;
		case TAKTI: TaktD(True,NULL,False);break;
		case TEMPOI: TempoD(True,NULL,False);break;
		case HLASITI: HlasitD(False);break;
		case OKTAVAI: OktavaD(False);break;
		case STEREOI: StereoD(False);break;
		case EFEKTI: EfektD(False);break;
		case MIXERI: MixerD();break;
		case ZACREPI: ZacRepetD();break;
		case KONREPI: KonRepetD();break;
		case PREDTI: PredtaktiD();break;
		case ZAHRPI: CompoHraj(True,False);break;
		case ZAHRSI: CompoHraj(True,True);break;
		case LISTI: PlayList();break;
		/*
		case NORMALI:
		{
			int LHan;
			Normalizace();
			for( LHan=HorniOkno(); LHan>=0; LHan=DalsiOkno(LHan) ) MoznaKresli(LHan);
			break;
		}
		*/
		case SELALLI: VsechnoBlok(); Kresli(VrchniSekv());break;
		case BLKOPI: if( ZkopirujBlok() ) Kresli(VrchniSekv());break;
		case BLKTAKTI:if( TaktBlok() && ZkopirujBlok() && SmazBlok() ) Kresli(VrchniSekv());break;
		case BLKJEDNI:if( JednotkaBlok() && ZkopirujBlok() && SmazBlok() ) Kresli(VrchniSekv());break;
		case BLMAZI: if( ZkopirujBlok() && SmazBlok() ) Kresli(VrchniSekv());break;
		case BLVLOZI: if( VlozBlok() ) Kresli(VrchniSekv());break;
		case BLKODZNI: ZrusBlok(VrchniSekv());Kresli(VrchniSekv());break;
		case SHUPI: if( PosunBlok(+1) ) Kresli(VrchniSekv());break;
		case SHDOI: if( PosunBlok(-1) ) Kresli(VrchniSekv());break;
		case NAJDIRAI: NajdiTaktD(VrchniSekv());break;
		/*case KONFIGI: DoKonfig();break;*/
		case EDITNASI: DoEditNas();break;
		case CTIORCHI: if( AktHraj ) Chyba(AktHraj->CtiOrchestr());break;
		case PISORCHI: if( AktHraj ) Chyba(AktHraj->PisOrchestr());break;
		case SAVECFGI: UlozCFG();break;
		default:
			OtevriAbout();
		break;
	}
}

static int PocetOken( void )
{
	int N;
	int LHan;
	for( N=LHan=0; LHan<NMaxOken; LHan++ ) if( Okna[LHan] ) N++;
	return N;
}
static Flag VrchniZmena( void )
{
	const Pisen *P=VrchniPisen();
	if( !P ) return False;
	return P->Zmena;
}
static long NutnoKreslit( void )
{
	int LHan;
	if( HrajOkno()>=0 ) return 100;
	if( ListOkno()>=0 ) return 300;
	for( LHan=0; LHan<NMaxOken; LHan++ ) if( Okna[LHan] )
	{
		if( OknoJeSekv(LHan) )
		{
			OknoInfo *OI=OInfa[LHan];
			if( OI->ZmenaObrazu ) return 300;
		}
	}
	return -1;
}

void NastavMenu( Flag IsMouse )
{
	Flag F,G; /* musi byt normal. Boolean - pouzivame & */
	Flag H=HrajOkno()>=0;
	int N;
	OknoInfo *VI;
	BegUpdate();
	(void)IsMouse;
	F=JeOkno;
	PovolMenu(Menu,OZAVRI,F && Vrchni!=ToolOkno() && Vrchni!=ToNOkno() );
	F=VolneOkno()>=0;
	N=PocetPisni();
	G=N<NMaxPisni;
	PovolMenu(Menu,OTEVRI,F&G);
	PovolMenu(Menu,IMPORTI,F&G);
	PovolMenu(Menu,NOVAI,F&G);
	G=N>0;
	PovolMenu(Menu,OHLASI,F&G);
	G=VrchniZmena();
	PovolMenu(Menu,ULOZI,G);
	N=PocetOken();
	F=N>1;
	PovolMenu(Menu,VYBERI,F);
	PovolMenu(Menu,PROHODI,F || N>0 && Vrchni<0);
	PovolMenu(Menu,USPORI,N>0);
	N=VrchniSekv();
	VI=OInfa[N];
	F=N>=0;
	{
		static Flag Povoleno=-1; /* ani True, ani False */
		if( F!=Povoleno )
		{
			const static int SekvItems[]=
			{
				ZACREPI,KONREPI,PREDTI,
				VSEHLAI,NHLASI,ZHLASI,
				VYVSEKI,NASTROJI,HLASITI,OKTAVAI,STEREOI,
				EFEKTI,MIXERI,
				ZTAKTI,ZTEMPOI,ZTONINAI,TAKTI,TEMPOI,TONINAI,
				BLKTAKTI,BLKJEDNI,SELALLI,NAJDIRAI,
				ZAHRSI,ZAHRPI,ULOZDOI,NOTYI,
				0
			};
			const int *I;
			for( I=SekvItems; *I; I++ ) PovolMenu(Menu,*I,F);
			Povoleno=F;
		}
		PovolMenu(Menu,PHLASI,F && VI->KKan>0);
	}
	G=F && VI->ZBStopa>=0;
	PovolMenu(Menu,BLKOPI,G);
	PovolMenu(Menu,BLKODZNI,G);
	PovolMenu(Menu,BLMAZI,G);
	PovolMenu(Menu,SHUPI,G);
	PovolMenu(Menu,SHDOI,G);
	PovolMenu(Menu,BLVLOZI, F && PisenBlok );
	F=AktHraj!=NULL;
	PovolMenu(Menu,CTIORCHI, !H && F && AktHraj->CtiOrchestr );
	PovolMenu(Menu,PISORCHI, !H && F && AktHraj->PisOrchestr );
	PovolMenu(Menu,EDITNASI, !H && F && AktHraj->EditSNastr  );
	EndUpdate();
	CNastavDesktop();
	Casovac=NutnoKreslit();
	{
		int LHan;
		for( LHan=0; LHan<NMaxOken; LHan++ ) if( OknoJeSekv(LHan) )
		{
			OknoInfo *OI=OInfa[LHan];
			Sekvence *S=SekvenceOI(OI);
			Pisen *P=S->Pis;
			if( OI->VNazvuZmena!=P->Zmena )
			{
				SetNazev(Okna[LHan],TvorNazev(S,LHan,P->Zmena),True);
				OI->VNazvuZmena=P->Zmena;
			}
		}
	}
}
