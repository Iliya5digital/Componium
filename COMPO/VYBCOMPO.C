/* Componium - selektor - vyber */
/* SUMA, 10/1992-2/1994 */

#include <macros.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sttos\narod.h>
#include <gem\interf.h>
#include <gem\multiitf.h>

#include "compo.h"
#include "spritem.h"
#include "ramcompo.h"
#include "digill.h"

static int SlYMax,SlY,SlH;
static int SlY0,SlH0;
static int FIndex;
static int NFil;

typedef const char *InfoFce( const void *It );
typedef void TEditFce( void *N, int P );

typedef struct
{
	TEditFce *EditFce;
	Flag (*DelFce)( void *Co );
	InfoFce *Info;
	InfoFce *SInfo; /* vzdy jednosloupcove, nestaticke - pro sort */
	void **(*Soupis)( void );
	void (*Pust)( void **S );
	const char *EditPov,*VymazPov,*NovaPov,*NovaPovM; /* nazvy buttonu */
} VyberAkce;

static const VyberAkce *Akce;

static int SelHan; /* handle okna, ve kterem pracujeme */
static OBJECT *SelForm;
static void **Vybrano;
static void **VybPole;
static Flag Strucne;

static void CSetStrTed( OBJECT *O, const char *I, Flag C, Flag Dis, Flag Sel )
{
	TEDINFO *TI=O->ob_spec.tedinfo;
	int OS=O->ob_state;
	Flag Force=False;
	Flag Cmp=True;
	Set01(O,DISABLED,Dis),Set01(O,SELECTED,Sel);
	if( C )
	{
		Cmp= strncmp(StrTed(TI),I,TI->te_txtlen-1);
		Force= O->ob_state!=OS || Cmp;
	}
	if( Cmp ) SetStrTed(TI,I);
	if( Force )
	{
		ObjcRedraw(SelForm,(int)(O-SelForm),SelHan,NULL);
	}
}
static void FileRedraw( const void *F, OBJECT *O, Flag Compar )
{
	if( F )
	{
		const char *I=Strucne ? Akce->SInfo(F) : Akce->Info(F);
		Flag Sel=Vybrano && F==*Vybrano;
		CSetStrTed(O,I,Compar,False,Sel);
		if( !Strucne )
		{
			O+=SEL21B-SEL11B,I+=strlen(I)+1,CSetStrTed(O,I,Compar,False,Sel);
		}
	}
	else
	{
		CSetStrTed(O,"",Compar,True,False);
		if( !Strucne )
		{
			O+=SEL21B-SEL11B,CSetStrTed(O,"",Compar,True,False);
		}
	}
}

static void NastavRenDel( Flag Draw )
{
	int OSN,OSO;
	OBJECT *O=&SelForm[SELRENB];
	OBJECT *P=&SelForm[SELDELB];
	OSO=O->ob_state;
	if( Vybrano ) OSN=OSO&~DISABLED;
	else OSN=OSO|DISABLED;
	if( OSO!=OSN )
	{
		O->ob_state=OSN;
		P->ob_state=OSN;
		if( Draw )
		{
			ObjcRedraw(SelForm,SELRENB,SelHan,NULL);
			ObjcRedraw(SelForm,SELDELB,SelHan,NULL);
		}
	}
}


static int Sel1B,SelPB;
static int NFilSel;
static const char *Title;

static void InitFilSel( void )
{
	if( Strucne ) Sel1B=SEL11B,SelPB=SEL2PB;
	else Sel1B=SEL11B,SelPB=SEL1PB;
	NFilSel=SelPB-Sel1B+1;
}

static void DirRedraw( Flag Draw, Flag Compar )
{
	OBJECT *F=SelForm;
	OBJECT *O;
	const void **FP;
	static long LastFIndex;
	for( FP=VybPole,NFil=0; *FP; FP++ ) NFil++;
	if( NFil>0 ) SlH=(int)(NFilSel*(long)SlYMax/NFil);
	else SlH=SlYMax;
	if( SlH>SlYMax ) SlH=SlYMax;
	if( FIndex>NFil-NFilSel ) FIndex=NFil-NFilSel;
	if( FIndex<0 ) FIndex=0;
	{
		int NFilE=NFil-NFilSel;
		if( NFilE>0 ) SlY=(int)(FIndex*(long)(SlYMax-SlH)/NFilE);
		else SlY=0;
	}
	if( FIndex!=LastFIndex || !Draw ) Compar=False,LastFIndex=FIndex;
	F[SELTAH].ob_y=SlY;
	F[SELTAH].ob_height=SlH;
	for( O=&F[Sel1B],FP=&VybPole[FIndex]; O<=&F[SelPB]; O++ )
	{
		if( *FP ) FileRedraw(*FP++,O,Compar);
		else FileRedraw(NULL,O,Compar);
	}
	if( Draw )
	{
		if( !Compar ) ObjcRedraw(F,SELRAMB,SelHan,NULL);
		if( Draw && ( SlH!=SlH0 || SlY!=SlY0 ) ) ObjcRedraw(F,SELBAK,SelHan,NULL);
	}
	SlH0=SlH;
	SlY0=SlY;
	NastavRenDel(Draw);
}
static void SetSpeedS( const char *Text, Flag Draw )
{
	static char Buf[80];
	strcpy(Buf,Title);
	if( *Text ) strcat(Buf,": '"),strcat(Buf,Text),strcat(Buf,"'");
	if( strcmp(StrTed(TI(SelForm,SELNAZT)),Buf) )
	{
		SetStrTed(TI(SelForm,SELNAZT),Buf);
		if( Draw ) KresliNazevDial(SelForm,SELNAZT,Buf);
	}
}

static char SS[16];

static void SmazSpeedS( void )
{
	strcpy(SS,"");
	SetSpeedS(SS,True);
}

/* SInfo nesmi davat static. retez */

static int InfoCmp( const void **C, const void **c )
{
	return StrCmp(Akce->SInfo(*C),Akce->SInfo(*c),&CesPar,True);
}

static void SortVyb( const void **VybPole )
{
	int N;
	const void **FP;
	for( FP=VybPole,N=0; *FP; FP++ ) N++;
	qsort((void **)VybPole,N,sizeof(*VybPole),(int(*)(void*,void*))InfoCmp);
}
static void **VDNajdi( void *VD )
{
	void **VP;
	for( VP=VybPole; *VP; VP++ ) if( *VP==VD ) return VP;
	return NULL;
}


static long Strncmpi( const char *S, const char *s )
{
	long l=0;
	while( *S && *s )
	{
		int d=Intern(*S++)-Intern(*s++);
		if( d ) return l;
		l++;
	}
	return l;
}

static void UpravFIndex( void )
{
	if( Vybrano )
	{
		int i=(int)(Vybrano-VybPole);
		if( FIndex>i-1 ) FIndex=i=-1;
		else
		{
			int imax=i-NFilSel*1/4;
			if( FIndex<imax ) FIndex=imax;
		}
	}
}

static void VybNajdi( char *T, long TL )
{
	void **VP,**VL=NULL;
	long l,lmax=-1;
	for( VP=VybPole; *VP; VP++ )
	{
		l=Strncmpi(T,Akce->SInfo(*VP));
		if( l>lmax ) VL=VP,lmax=l;
	}
	if( lmax<0 ) lmax=0;
	if( lmax>=TL ) lmax=TL-1;
	if( VL ) strncpy(T,Akce->SInfo(*VL),lmax);
	T[lmax]=0;
	if( VL!=Vybrano )
	{
		Vybrano=VL;
		UpravFIndex();
		DirRedraw(True,True);
	}
}

static Flag SelUpP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	FIndex--;
	DirRedraw(True,True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag SelDoP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	FIndex++;
	DirRedraw(True,True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Flag SelBakP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	int Dum,Y,y;
	MysXY(&Dum,&Y);
	objc_offset(SelForm,SELBAK,&Dum,&y);
	if( Y-y<=SlY ) FIndex-=NFilSel;
	else FIndex+=NFilSel;
	DirRedraw(True,True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static Flag SelTahP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	int nfe=NFil-NFilSel;
	if( nfe<=0 ) nfe=0;
	FIndex=(int)((SlideBox(SelForm,SELBAK,SELTAH,True)*(long)nfe+500)/1000);
	DirRedraw(True,True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}
static void NastavSel( int D )
{
	OBJECT *F=SelForm;
	OBJECT *o=&F[D];
	Set1(o,SELECTED);
	Set1(o+SEL21B-SEL11B,SELECTED);
	RedrawI(F,D);
	RedrawI(F,D+SEL21B-SEL11B);
}

static Flag SelSpSP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	int A=(char)Asc;
	int S=Asc>>8;
	long l;
	char *T=SS;
	if( !Asc ) return False; /* povinne klavesou */
	l=strlen(T);
	if( S==14 )
	{
		if( l>0 )
		{
			T[l-1]=0;
			if( *T ) SetSpeedS(T,True);
			else SmazSpeedS();
			VybNajdi(SS,sizeof(SS));
		}
		return False;
	}
	if( Vybrano )
	{
		if( S==80 )
		{
			if( Vybrano[1] )
			{
				int i;
				int imax;
				Vybrano++;
				i=(int)(Vybrano-VybPole);
				imax=i-(NFilSel-3);
				if( FIndex<imax ) FIndex=imax;
				DirRedraw(True,True);
				SmazSpeedS();
				return False;
			}
		}
		ef( S==72 )
		{
			if( Vybrano>VybPole )
			{
				int i;
				Vybrano--;
				i=(int)(Vybrano-VybPole);
				if( FIndex>i-2 ) FIndex=i-2;
				DirRedraw(True,True);
				SmazSpeedS();
				return False;
			}
		}
	}
	if( A<' ' ) return False;
	if( l<sizeof(SS)-2 )
	{
		T[l+1]=0,T[l]=A;
		VybNajdi(SS,sizeof(SS));
		SetSpeedS(T,True);
		return False;
	}
	(void)Bt,(void)Dvoj;
	return False;
}

static Err Relog( void *Vyb )
{	/* zcela znovu vse nacte, ale nezavre pritom okno */
	Mouse(HOURGLASS);
	Akce->Pust(VybPole);
	VybPole=Akce->Soupis();
	if( !VybPole ) return ERAM;
	SortVyb(VybPole);
	Vybrano=VDNajdi(Vyb);
	if( !Vybrano ) Vybrano=VybPole;
	if( !*Vybrano ) Vybrano=NULL;
	FIndex=0;
	UpravFIndex();
	SS[0]=0;
	DirRedraw(True,False);
	return EOK;
}

static Flag SelPoleP( OBJECT *Bt, Flag Dvoj, int Asc )
{
	int Dial=(int)(Bt-SelForm);
	(void)Asc;
	if( Strucne )
	{
		if( Dial>=Sel1B && Dial<=SelPB )
		{
			Vybrano=&VybPole[Dial-Sel1B+FIndex];
			Plati(*Vybrano);
			if( Dvoj ) return True;
		}
	}
	else /* !Strucne */
	{
		int DP=0;
		if( Dial>=SEL21B && Dial<=SEL2PB ) Dial-=SEL21B-SEL11B,DP=1;
		if( Dial>=Sel1B && Dial<=SelPB )
		{
			Vybrano=&VybPole[Dial-Sel1B+FIndex];
			NastavSel(Dial);
			Plati(*Vybrano);
			if( Dvoj )
			{
				Err ret;
				if( !Akce->EditFce ) return True;
				Akce->EditFce(*Vybrano,DP);
				if( (ret=Relog(*Vybrano))<EOK ) {Chyba(ret);return True;}
				return False;
			}
		}
	}
	SmazSpeedS();
	NastavRenDel(True);
	return False;
}

static Flag SelDelB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	long VybI=Vybrano-VybPole;
	Plati( Akce->DelFce );
	Plati( *Vybrano );
	if( Akce->DelFce(*Vybrano) )
	{
		void **V;
		Akce->Pust(VybPole);
		VybPole=Akce->Soupis(); /* nejdriv alokuj */
		if( VybPole )
		{
			SortVyb(VybPole); /* trideni vzdy s jedn. infem */
			for( V=VybPole; *V && VybI>0; V++,VybI-- );
			if( V>VybPole ) Vybrano=--V;
			ef( *VybPole ) Vybrano=VybPole;
			else Vybrano=NULL;
			DirRedraw(True,False);
		}
		else {ChybaRAM();return True;}
	}
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Flag SelRenB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	void *Vyb=*Vybrano;
	Err ret;
	Plati( Akce->EditFce );
	Plati( Vyb );
	Akce->EditFce(Vyb,0);
	if( (ret=Relog(Vyb))<EOK ) {Chyba(ret);return True;}
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Button SelBut[]=
{
	Unselect,SELCANB,
	SelUpP,SELUP,
	SelDoP,SELDO,
	SelBakP,SELBAK,
	SelTahP,SELTAH,
	Unselect,SELOKB,
	Unselect,SELNOVB,
	SelRenB,SELRENB,
	SelDelB,SELDELB,
	SelSpSP,SELRAMB,
	SelPoleP,-1,
	NULL,
};

static FormZrychl SelAcc[]=
{
	{1,-1,-1,SELCANB},{97,-1,-1,SELCANB},
	{82,-1,-1,SELNOVB},
	{83,-1,-1,SELDELB},
	{15,-1,-1,SELRENB},
	{-1,-1,-1,SELRAMB},
	{0,0,0,0}
};

static void *VyberFull(	const char *_Title, const VyberAkce *_Akce, void *Vybr, Flag _Strucne, const WindRect *Orig )
{
	void *ret;
	Flag Ptej=False;
	Title=_Title;
	Akce=_Akce;
	Strucne=_Strucne;
	Mouse(HOURGLASS);
	VybPole=Akce->Soupis();
	if( !VybPole ) {AlertRF(RAMEA,1);return NULL;}
	InitFilSel();
	if( Akce->NovaPov==ERRP ) Akce->NovaPov=NULL,Ptej=True;
	SortVyb(VybPole); /* trideni vzdy s jedn. infem */
	Vybrano=VDNajdi(Vybr);
	if( !Vybrano ) Vybrano=VybPole;
	if( !*Vybrano ) Vybrano=NULL;
	SelForm=RscTree(VYBERD);
	SlYMax=SelForm[SELBAK].ob_height;
	SlY=0;
	SlH=SlYMax;
	SlY0=SlH0=-1;
	FIndex=0;
	UpravFIndex();
	SS[0]=0;
	DirRedraw(False,False);
	if( NFil==0 )
	{
		if( Akce->NovaPov ) ret=ERRP;
		else ret=NULL;
	}
	ef( NFil==1 && !Ptej && !Akce->NovaPov ) ret=VybPole[0];
	else
	{
		int Dial;
		OBJECT *NP;
		NP=&SelForm[SELNOVB];
		if( Akce->NovaPov ) NP->ob_flags&=~HIDETREE,NP->ob_spec.free_string=(char *)Akce->NovaPov;
		else NP->ob_flags|=HIDETREE;
		NP=&SelForm[SELRENB];
		if( Akce->EditPov ) NP->ob_flags&=~HIDETREE,NP->ob_spec.free_string=(char *)Akce->EditPov;
		else NP->ob_flags|=HIDETREE;
		NP=&SelForm[SELDELB];
		if( Akce->VymazPov ) NP->ob_flags&=~HIDETREE,NP->ob_spec.free_string=(char *)Akce->VymazPov;
		else NP->ob_flags|=HIDETREE;
		SetStrTed(TI(SelForm,SELNAZT),Title);
		Dial=WFForm(SelForm,SelBut,SelAcc,-1,Orig,&SelHan);
		if( Dial==SELNOVB ) ret=ERRP;
		ef( !Vybrano ) ret=NULL;
		else
		{
			if( Dial>=SEL11B && Dial<=SEL2PB ) Dial=SELOKB;
			if( Dial==SELOKB ) ret=*Vybrano;
			else ret=NULL;
		}
	}
	if( VybPole ) Akce->Pust(VybPole);
	else ret=NULL;
	return ret;
}
static void *Vyber( const char *Title, const VyberAkce *Akce, const WindRect *Orig )
{
	return VyberFull(Title,Akce,NULL,True,Orig);
}


static void *VyberMale( const VyberAkce *_Akce, void *DefVyb, const WindRect *Orig )
{
	void *ret;
	void **PV;
	WindRect W=*Orig;
	int P,N,Nov;
	int ls=0;
	int Def=-1;
	Polozka *Pol;
	Akce=_Akce;
	Strucne=True;
	VybPole=_Akce->Soupis();
	if( !VybPole ) {AlertRF(RAMEA,1);return NULL;}
	SortVyb(VybPole);
	for( N=0,PV=VybPole; *PV; PV++,N++ );
	Pol=myalloc(sizeof(*Pol)*(N+2),False);
	if( !Pol ) {Akce->Pust(VybPole);Chyba(ERAM);return NULL;}
	for( N=0,PV=VybPole; *PV; PV++,N++ )
	{
		Polozka *P=&Pol[N];
		int sl;
		strcpy(P->Text,"  ");
		strlncpy(&P->Text[2],Akce->SInfo(*PV),sizeof(P->Text)-3);
		strcat(P->Text," ");
		sl=(int)strlen(P->Text);
		if( sl>ls ) ls=sl;
		if( DefVyb && !strcmp(Akce->SInfo(DefVyb),Akce->SInfo(*PV)) ) Def=N;
	}
	if( Akce->NovaPovM )
	{
		Polozka *P=&Pol[N];
		strcpy(P->Text,"  ");
		strlncpy(&P->Text[1],Akce->NovaPovM,sizeof(P->Text)-2);
		Nov=N++;
		if( Def<0 ) Def=Nov;
	}
	else Nov=-1;
	if( Def<0 ) Def=0;
	Pol[N].Text[0]=0;
	if( N==0 ) ret=NULL;
	ef( N==1 )
	{
		if( Akce->NovaPov ) ret=ERRP;
		else ret=*VybPole;
	}
	else
	{
		P=PopUp(Pol,Def,&W);
		if( P>=0 )
		{
			if( P==Nov ) ret=ERRP;
			else ret=VybPole[P];
		}
		else ret=NULL;
	}
	Akce->Pust(VybPole);
	return ret;
}

static const char *StrInfo( const void *It )
{
	return (char *)It;
}

static void PustSHlasy( void **SH )
{
	for( ; *SH; SH++ ) myfree(*SH);
	myfree(SH);
}

/* vybery konkr. veci */
static const Pisen *SoupPis;
static Flag JedenHlas;

static char *SekvNazFce( Sekvence *S, Flag JedenHlas )
{
	const char *SN=NazevSekv(S);
	static char Buf[40];
	char *ND;
	int N;
	for( N=0; S->predch; S=S->predch ) N++;
	if( !strcmp(SN,MainMel) ) SN=NajdiNazev(S->Pis->NazevS);
	if( !JedenHlas || !S->dalsi ) strcpy(Buf,SN);
	else sprintf(Buf,"%s.%d",SN,N+1);
	ND=myalloc(strlen(Buf)+1,False);
	if( ND ) strcpy(ND,Buf);
	return ND;
}

static Sekvence *FZalozSekv( Pisen *P )
{
	OBJECT *F=RscTree(NOVSEKD);
	Sekvence *S;
	const char *Naz;
	SetStrTed(TI(F,NSEKVE),"");
	for(;;)
	{
		if( EFForm(F,NULL,NULL,NSEKVE,NULL)!=NSOKB ) return NULL;
		Naz=MezStrTed(TI(F,NSEKVE));
		if( *Naz && PovolJmeno(Naz) ) break;
		VsePrekresli();
		AlertRF(NAZPOVA,1);
	}
	Plati( PovolJmeno(Naz) && strcmp(Naz,MainMel) );
	if( !strcmp(Naz,NajdiNazev(P->NazevS)) ) Naz=MainMel;
	S=NajdiSekv(P,Naz,0);
	if( S ) {AlertRF(NAZEXA,1);return NULL;}
	S=NovaSekv(P,Naz);
	if( !S ) {AlertRF(RAMEA,1);return S;}
	return S;
}

static Sekvence *NajdiHSekv( const Pisen *P, const char *NazS )
{
	Sekvence *S;
	int NExt;
	const char *Ext=strrchr(NazS,'.');
	if( !Ext ) NExt=0;
	else
	{
		*(char *)Ext++=0; /* vim, ze uz to nikdo nepouzije */
		NExt=atoi(Ext)-1;
		Plati( NExt>=0 );
	}
	if( !strcmp(NazS,NajdiNazev(P->NazevS)) ) NazS=(char *)MainMel;
	S=NajdiSekv(P,NazS,NExt);
	return S;
}

static Flag DelSekv( void *_S )
{
	Sekvence *S=NajdiHSekv(SoupPis,_S);
	if( S )
	{
		while( S->predch ) S=S->predch; /* musi byt hlavni */
		if( 1==AlertRF(DELSEKA,2,NazevSekv(S)) )
		{
			VymazSekvenci(S);
			return True;
		}
	}
	return False;
}

static void **SoupisSekv( void )
{
	Sekvence *S;
	void **SVyber,**FP;
	char *NazS;
	int N;
	for( S=PrvniSekv(SoupPis),N=0; S; S=DalsiSekv(S) )
	{
		if( !JedenHlas ) N++;
		else
		{
			Sekvence *H;
			for( H=S; H; H=DalsiHlas(H) ) N++;
		}
	}
	SVyber=myalloc(sizeof(*SVyber)*(N+1),False);
	if( !SVyber ) {AlertRF(RAMEA,1);return NULL;}
	for( FP=SVyber,S=PrvniSekv(SoupPis); S; S=DalsiSekv(S) )
	{
		Sekvence *H;
		if( !JedenHlas )
		{
			*FP++=NazS=SekvNazFce(S,JedenHlas);
			*FP=NULL;
			if( !NazS ) {PustSHlasy(SVyber);Chyba(ERAM);return NULL;}
		}
		ef( S!=NajdiSekv(SoupPis,MainMel,0) ) for( H=S; H; H=DalsiHlas(H) )
		{
			*FP++=NazS=SekvNazFce(H,JedenHlas);
			*FP=NULL;
			if( !NazS ) {PustSHlasy(SVyber);Chyba(ERAM);return NULL;}
		}
	}
	*FP=NULL;
	return (void **)SVyber;
}

static VyberAkce SekvAkce=
{
	NULL, /* EditFce */
	DelSekv, /* DelFce */
	StrInfo,StrInfo,
	SoupisSekv,PustSHlasy,
};

Sekvence *VyberSekv( Pisen *P, const char *Tit, Flag _JedenHlas, const WindRect *Orig, Sekvence *Def, Flag PovolMale )
{
	Sekvence *S;
	const char *NazS;
	SoupPis=P;
	JedenHlas=_JedenHlas;
	if( PovolMale ) NazS=VyberMale(&SekvAkce,Def ? NazevSekv(Def) : NULL,Orig);
	else NazS=VyberFull(Tit,&SekvAkce,Def ? NazevSekv(Def) : NULL,True,Orig);
	if( NazS )
	{
		if( NazS==ERRP ) S=ERRP;
		else S=NajdiHSekv(P,NazS);
	}
	else S=NULL;
	if( S==ERRP ) VsePrekresli(),S=FZalozSekv(P);
	return S;
}

static const char *PisenInfo( const void *It )
{
	return NajdiNazev(((Pisen *)It)->NazevS);
}
int PocetPisni( void )
{
	Pisen **S;
	int N;
	for( S=Pisne,N=0; S<&Pisne[NMaxPisni]; S++ )
	{
		if( *S ) N++;
	}
	return N;
}
Pisen *VrchniPisen( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		return SekvenceS(&OI->Soub[0])->Pis;
	}
	return NULL;
}

static void **SoupisPisni( void )
{
	Pisen **SVyber,**S,**FP;
	int N=PocetPisni();
	SVyber=myalloc(sizeof(*S)*(N+1),False);
	if( !SVyber ) {AlertRF(RAMEA,1);return NULL;}
	for( FP=SVyber,S=Pisne; S<&Pisne[NMaxPisni]; S++ )
	{
		if( *S ) *FP++=*S;
	}
	*FP=NULL;
	return (void **)SVyber;
}

#define Vfree  ( (void (*)(void **))myfree )

static VyberAkce PisenAkce=
{
	NULL, /* EditFce */
	NULL,
	PisenInfo,PisenInfo,
	SoupisPisni,Vfree,
};

Pisen *VyberPisen( const char *Text )
{
	Pisen *P=Vyber(Text,&PisenAkce,NULL);
	if( P==ERRP ) P=NULL;
	return P;
}

static const char *FOknoInfo( const void *It )
{
	return SizeNazev(((Okno *)It)->Nazev,16);
}

static void **SoupisOken( void )
{
	int N;
	Okno **OV,**OVP,*O;
	int LHan;
	for( N=LHan=0; LHan<NMaxOken; LHan++ ) if( Okna[LHan] ) N++;
	OV=myalloc(sizeof(O)*(N+1),False);
	for( LHan=0,OVP=OV; LHan<NMaxOken; LHan++ )
	{
		O=Okna[LHan];
		if( O ) *OVP++=O;
	}
	*OVP=NULL;
	if( !OV ) return NULL;
	return (void **)OV;
}

static VyberAkce OknoAkce=
{
	NULL, /* EditFce */
	NULL, /* DelFce */
	FOknoInfo,FOknoInfo,
	SoupisOken,Vfree,
};

int VyberOkno( void )
{
	Okno *O;
	int LHan=HorniOkno();
	if( LHan>=0 ) O=Okna[LHan];
	else O=NULL;
	O=VyberFull(FreeString(VYBOKFS),&OknoAkce,O,True,NULL);
	return O ? O->LHan : -1;
}

static Pisen *NasP;

static const char *NNastrInfo( const void *It )
{
	if( !AktHraj ) return (char *)It;
	return AktHraj->NastrInfo((const char *)It,NasP);
}
static const char *SNastrInfo( const void *It )
{
	return (char *)It;
}

static void NastTEdit( void *N, int P )
{
	const char *Naz;
	(void)P;
	if( HrajOkno()>=0 ) return; /* pýi hran¡ nelze editovat */
	Naz=OEditNastr(N,NasP);
	VsePrekresli();
	if( Naz==ERRP ) Chyba(ERAM);
}

static Flag NastrDel( void *_N )
{
	char *N=_N;
	if( AlertRF(DELNASA,2,N)==1 )
	{
		Err ret=OVymazNastroj(N,NasP);
		if( ret<OK ) Chyba(ret);
		else return True;
	}
	return False;
}

static BankaItem *NastrB;

static void **SoupisNastr( void )
{
	return (void **)OSoupisNastroju(NastrB,NasP);
}

static VyberAkce NastrAkce=
{
	NastTEdit, /* EditFce */
	NastrDel, /* DelFce */
	NNastrInfo,SNastrInfo,
	SoupisNastr,Vfree,
};

static const char *FZalozNastr( Pisen *P, Banka B, Flag Strucne, const WindRect *Orig )
{
	void *N;
	const char *NST=FreeString( Strucne ? NASTRFS : EDNASFS ); 
	NastrB=B;
	NasP=P;
	RepIns:
	N=VyberFull(NST,&NastrAkce,NULL,Strucne,Orig);
	if( N==ERRP )
	{
		const char *Naz;
		VsePrekresli();
		Naz=OEditNastr("",P);
		if( Naz==ERRP ) {Chyba(ERAM);Naz=NULL;}
		if( Strucne || !Naz ) N=Naz;
		else goto RepIns;
	}
	return N;
}

static void SetFNazev( char *D, const char *Inf, int f )
{
	strlncpy(D,Inf,16);
	lenstr(D,f>=9 ? 17 : 18,' ');
	sprintf(D+strlen(D)," [F%d]",f+1);
}

static Flag BMale;
static char NastrKlav[10][32];
static int NastrBI[10];

static void **SoupisBNastr( void )
{
	char **BV,**PV;
	int NI=0;
	BankaIndex I,N=SpoctiBanku(NastrB);
	BV=myalloc(sizeof(*BV)*(N+1),False);
	if( !BV ) {AlertRF(RAMEA,1);return NULL;}
	PV=BV;
	for( I=PrvniVBance(NastrB); I>=OK; I=DalsiVBance(NastrB,I) )
	{
		int f=CisloNasF(NasP,ZBanky(NastrB,I));
		if( f>=0 && BMale )
		{
			char *D=NastrKlav[NI];
			SetFNazev(D,ZBanky(NastrB,I),f);
			NastrBI[NI]=I;
			NI++;
			*PV++=D;
		}
		else *PV++=ZBanky(NastrB,I);
	}
	*PV=NULL;
	return (void **)BV;
}

static VyberAkce NastrBAkce=
{
	NastTEdit, /* EditFce */
	NULL, /* DelFce */
	NNastrInfo,SNastrInfo,
	SoupisBNastr,Vfree,
};

const char *VyberNastroj( Pisen *P, Banka B, BankaIndex Def, Flag Strucne, const WindRect *Orig, Flag PovolMale, const char *Tit )
{ /* neni-li Strucne, chapeme jako pokyn k editaci */
	const char *S;
	if( B )
	{
		Flag Male=PovolMale && Orig && Strucne;
		char *PV=Def>=0 ? ZBanky(B,Def) : NULL;
		NastrB=B;
		NasP=P;
		BMale=Male;
		if( Male )
		{
			char Buf[32];
			int FN=PV ? CisloNasF(P,PV) : -1;
			if( FN>=0 )
			{
				SetFNazev(Buf,PV,FN);
				PV=Buf;
			}
			S=VyberMale(&NastrBAkce,PV,Orig);
		}
		else
		{
			S=VyberFull(Tit,&NastrBAkce,PV,Strucne,Orig);
		}
		if( S>=NastrKlav[0] && S<NastrKlav[lenof(NastrKlav)] )
		{
			S=ZBanky(B,NastrBI[(S-NastrKlav[0])/sizeof(*NastrKlav)]);
		}
	}
	else S=ERRP; /* neni zadna Banka */
	if( S==ERRP ) VsePrekresli(),S=FZalozNastr(P,B,Strucne,Orig);
	return S;
}

void InitVyber( void )
{
	SekvAkce.VymazPov=FreeString(DELFS);
	SekvAkce.NovaPov=FreeString(NOVYFS);
	NastrAkce.EditPov=FreeString(EDITFS);
	NastrAkce.VymazPov=FreeString(DELFS);
	NastrAkce.NovaPov=FreeString(NOVYFS);
	NastrBAkce.EditPov=FreeString(EDITFS);
	NastrBAkce.NovaPov=FreeString(JINYFS);
	NastrBAkce.NovaPovM=FreeString(JINYBKFS);
}
	
