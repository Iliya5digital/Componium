/* digitalni hrani na AY a Falconu */
/* SUMA 3/1993-2/1994 */
/* GEM/Componium zapouzdreni */

#include <macros.h>
#include <string.h>
#include <dspbind.h>
#include <sndbind.h>
#include <stdlib.h>
#include <errno.h>
#include <gem\multiitf.h>
#include <stdc\infosoub.h>
#include <sttos\narod.h>
#include <sttos\samples.h>

#include "compo.h"
#include "llhraj.h"
#include "digill.h"
#include "utlcompo.h"
#include "ramcompo.h"

PathName OrcNam;
static Flag ZdaPisOrc;
static Flag NDZmena;

static int PrazdnyNastr( Pisen *P, Flag ForceL )
{
	NastrDef n,*N=&n;
	Flag Global;
	int NN;
	int ni;
	strcpy(N->Nazev,"");
	for( ni=0; ni<NMSoub; ni++ )
	{
		strcpy(N->Soubory[ni],"");
	}
	for( ni=0; ni<FyzOkt*12; ni++ )
	{
		N->IMSoubory[ni]=-1;
	}
	if( !P ) Global=True;
	ef( ForceL ) Global=False;
	else Global=AlertRF(LOKGLOBA,1)==2;
	if( Global )
	{
		if( NDyn(&NDef)>=0x4000 ) return ERAM;
		NN=(int)InsDyn(&NDef,N);
		if( NN<0 ) return ERAM;
	}
	else
	{
		if( NDyn(&P->LocDigi)>=0x4000 ) return ERAM;
		NN=(int)InsDyn(&P->LocDigi,N);
		if( NN<0 ) return ERAM;
		NN|=0x4000;
	}
	return NN;
}


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
		if( S==ERR ) AlertRF(CENASFA,1,NajdiNazev(D->Soubory[ni])),S=ECAN;
		if( S<EOK ) return (int)S;
		UzijSoub(S);
		D->IMSoubory[FTon]=S;
		D->Uzit=True; /* potrebujeme vedet, koho mame ukladat do COC */
	}
	else return KON;
	return I;
}

/* ---------------- editace nastroje */

static int EdNHan;
static NastrDef *NasD;
static const char *NasNam;
static char FName[NMSoub][sizeof(NasD->Soubory[0])];

static void SetFNamTed( TEDINFO *T, const char *N )
{
	while( strlen(N)>T->te_txtlen )
	{
		const char *n;
		n=strchr(N,'\\');
		if( !n || n==N ) N++;
		else N=n;
	}
	SetStrTed(T,N);
}

static const char *NastrojExt( void )
{
	if( *NastrP ) return NajdiPExt(NajdiNazev(NastrP));
	else return ".AVR";
}

static Flag EdNasSoub( OBJECT *Bt, Flag Dvoj, int Asc )
{
	NastrDef *D=NasD;
	const char *Nam=NasNam;
	PathName P,W;
	FileName FN;
	char Buf[80];
	OBJECT *DD=RscTree(EDNASD);
	int IS=(int)(Bt-DD)-EDNSOU1T;
	if( IS<0 ) IS=0;
	ef( IS>NMSoub-1 ) IS=NMSoub-1;
	VsePrekresli();
	*W=0;
	{
		int IIS;
		for( IIS=IS; IIS>=0; IIS-- )
		{
			if( *FName[IIS] )
			{
				NastrojNazev(FName[IIS],W);
				break;
			}
		}
	}
	if( !*W ) NastrojNazev(NastrojExt(),W);
	VyrobCesty(P,FN,W);
	if( *Nam ) sprintf(Buf,"%s '%s'",FreeString(DEFNASFS),Nam);
	else strcpy(Buf,FreeString(VYBNASFS));
	if( EFSel(Buf,P,FN,W) )
	{
		VytvorNastrSoub(FName[IS],W);
		SetFNamTed(TI(DD,EDNSOU1T+IS),FName[IS]);
		SendODraw(EdNHan,DD,EDNSOU1T+IS);
		strlncpy(D->Soubory[IS],FName[IS],sizeof(D->Soubory[IS]));
		if( !*D->Nazev )
		{
			avr_t H;
			void *Obs=ZvukLoad(&H,W);
			if( Obs )
			{
				char NBuf[32];
				strlncpy(NBuf,H.avr_name,9);
				strcat(NBuf,H.avr_xname);
				if( !strcmp(NBuf,"???") ) *NBuf=0;
				else strcpy(D->Nazev,NBuf);
				free(Obs);
				if( H.avr_midinote<0 ) AlertRF(CENASFA,1);
			}
			else
			{
				*D->Nazev=0;
				if( errno==EINVAL ) AlertRF(CENASFA,1);
				ef( errno==ERAM ) ChybaRAM();
			}
			SetMezStrTed(TI(DD,EDNASTT),D->Nazev);
			SendODraw(EdNHan,DD,EDNASTT);
		}
	}
	VsePrekresli();
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

static Button EdNTlac[]=
{
	{EdNasSoub,EDNSOU1T},
	{EdNasSoub,EDNSOU2T},
	{EdNasSoub,EDNSOU3T},
	{EdNasSoub,EDNSOU4T},
	{NULL}
};

static int EdNasD( int NN, Pisen *P, const char *Nam, Flag PovolZmJmena, const WindRect *Orig )
{
	NastrDef *D=NastrAcc(&P->LocDigi,NN);
	NastrDef DO=*D;
	OBJECT *DD=RscTree(EDNASD);
	NasD=D;
	NasNam=Nam;
	if( PovolZmJmena ) DD[EDNASTT].ob_flags|=EDITABLE;
	else DD[EDNASTT].ob_flags&=~EDITABLE;
	if( !*D->Nazev )
	{
		strlncpy(D->Nazev,Nam,sizeof(D->Nazev));
	}
	for(;;)
	{
		int r;
		int ni;
		SetMezStrTed(TI(DD,EDNASTT),D->Nazev);
		for( ni=0; ni<NMSoub; ni++ )
		{
			strcpy(FName[ni],D->Soubory[ni]);
		}
		SetFNamTed(TI(DD,EDNSOU1T),D->Soubory[0]);
		SetFNamTed(TI(DD,EDNSOU2T),D->Soubory[1]);
		SetFNamTed(TI(DD,EDNSOU3T),D->Soubory[2]);
		SetFNamTed(TI(DD,EDNSOU4T),D->Soubory[3]);
		if( PovolZmJmena ) r=WFForm(DD,EdNTlac,NULL,EDNASTT,Orig,&EdNHan);
		else r=WFForm(DD,EdNTlac,NULL,-1,Orig,&EdNHan);
		if( r!=EDNOKB ) {*D=DO;return ECAN;}
		else
		{
			const char *Naz=MezStrTed(TI(DD,EDNASTT));
			long IND;
			Flag Zmena;
			if( !*Naz || !PovolJmeno(Naz) )
			{
				VsePrekresli();
				AlertRF(NAZPOVA,1);
				continue;
			}
			IND=NajdiDef(Naz,-1,&P->LocDigi);
			if( IND>=EOK && IND!=NN )
			{
				VsePrekresli();
				AlertRF(NAZEXA,1);
				continue;
			}
			strlncpy(D->Nazev,Naz,sizeof(D->Nazev));
			Zmena=False;
			if( strcmp(D->Nazev,DO.Nazev) ) Zmena=True;
			for( ni=0; ni<NMSoub; ni++ )
			{
				strlncpy(D->Soubory[ni],FName[ni],sizeof(D->Soubory[ni]));
				if( strcmp(D->Soubory[ni],DO.Soubory[ni]) ) Zmena=True;
			}
			if( Zmena )
			{
				if( NN<0x4000 ) NDZmena=True; /* neni lokalni! */
				else
				{
					Plati( P );
					P->Zmena=True;
					NastavNazvy();
				}
			}
			return EOK;
		}
	}
}

/* LLHRAJ zapouzdreni sluzeb */

/* uroven orchestru - NastrDef */

const char *EditSNastr( const char *NNam, const WindRect *Orig, Pisen *P )
{
	int DI=ERR;
	int ret;
	Flag Pridan=False;
	NastrDef *N;
	if( NNam ) DI=NajdiDef(NNam,-1,&P->LocDigi);
	if( DI<EOK )
	{
		DI=PrazdnyNastr(P,False);
		if( DI<EOK ) return ERRP;
		Pridan=True;
	}
	N=NastrAcc(&P->LocDigi,DI);
	ret=EdNasD(DI,P,NNam,True,Orig);
	if( ret>=EOK ) return N->Nazev;
	if( Pridan ) N->Nazev[0]=0;
	if( ret==ECAN ) return NULL;
	return ERRP;
}

void MazSNastr( void )
{
	KonDyn(&NDef);
	KonSoub();
}

static Err CtiSNastr( const char *NNaz )
{
	Err ret;
	FILE *f;
	MazSNastr();
	ZacDyn(&NDef);
	if( !*NNaz ) return EOK;
	InitNastrP(NNaz);
	f=fopen(NNaz,"r");
	if( !f ) return EOK;
	if( !Rezerva ) if( setvbuf(f,NULL,_IOFBF,9*1024)==EOF ) {ret=ERAM;goto Konec;}
	if( AlokDyn(&NDef,32)<0 ) {ret=ERAM;goto Konec;}
	for(;;)
	{
		int NH;
		int ni;
		NastrDef *N;
		char *R,*S;
		R=CtiRadek(f);if( !R ) break;
		S=SCtiSlovo(&R);
		NH=PrazdnyNastr(NULL,False);if( NH<EOK ) {ret=(Err)NH;goto Konec;}
		N=NastrAcc(NULL,NH); /* urcite globalni */
		strlncpy(N->Nazev,S,sizeof(N->Nazev));
		for( ni=0; ni<4; ni++ )
		{
			S=SCtiSlovo(&R);
			if( S ) strlncpy(N->Soubory[ni],S,sizeof(N->Soubory[ni]));
			else strcpy(N->Soubory[ni],"");
		}
	}
	NDZmena=False;
	ret=EOK;
	Konec:
	fclose(f);
	return ret;
}

static Err FPisSNastr( const char *N )
{
	Err ret=EOK;
	long i;
	FILE *f;
	if( !*N ) return EFIL;
	InitNastrP(N);
	f=fopen(N,"w");
	if( !f ) return EFIL;
	if( !Rezerva ) if( setvbuf(f,NULL,_IOFBF,9*1024)==EOF ) {ret=ERAM;goto Konec;}
	for( i=0; i<NDyn(&NDef); i++ )
	{
		NastrDef *N=AccDyn(&NDef,i);
		if( N->Nazev[0] )
		{
			int ni;
			if( EOF==fprintf(f,"%s",N->Nazev) ) {ret=EFIL;goto Konec;}
			for( ni=0; ni<NMSoub; ni++ )
			{
				if( *N->Soubory[ni] )
				{
					if( EOF==fprintf(f," %s",N->Soubory[ni]) ) {ret=EFIL;goto Konec;}
				}
			}
			if( EOF==fprintf(f,"\n") ) {ret=EFIL;goto Konec;}
		}
	}
	NDZmena=False;
	Konec:
	fclose(f);
	return ret;
}

static Err PisSNastr( const char *N )
{
	int a;
	if( !NDZmena ) return EOK;
	do
	{
		a=AlertRF(CLORCHA,1,JmenoOrc(N));
		if( a==1 )
		{
			Err ret=FPisSNastr(OrcNam);
			if( ret<EOK ) return ret;
		}
		ef( a==3 ) return ECAN;
	} while( a==1 && NDZmena );
	return EOK;
}

static const char OrcNazev[]="AVRVoices";

/* uroven orchestru */

const char *NastrInfo( const char *NNam, Pisen *P )
{
	int NN=NajdiDef(NNam,-1,&P->LocDigi);
	static char NB[128];
	char *NBF=NB;
	strcpy(NBF,NNam);
	if( NN>=0 )
	{
		NastrDef *ND=NastrAcc(&P->LocDigi,NN);
		NBF+=strlen(NBF)+1;
		if( NN>=0x4000 ) *NBF++='#';
		strcpy(NBF,SizeNazev(ND->Soubory[0],20));
	}
	else
	{
		NBF+=strlen(NBF)+1,strcpy(NBF,"");
	}
	return NB;
}
static Flag CopySampleToDir( const char *Dir, const char *File )
{
	int t;
	PathName D;
	strcpy(D,Dir);
	Dcreate(D);
	EndChar(D,'\\');
	strcat(D,NajdiNazev(File));
	strcpy(NajdiPExt(NajdiNazev(D)),".SSS");
	t=open(D,O_RDONLY);
	if( t>=0 )
	{
		close(t);
		return False;
	}
	else
	{
		avr_t h;
		void *bf=ZvukLoad(&h,File);
		if( !bf ) ChybaRAM();
		else
		{
			int r;
			if( h.avr_resolution!=8 )
			{
				void *nbf=ZmenRozl(&h,bf,8);
				if( !nbf )
				{
					ChybaRAM();
					myfreeSpc(bf);
				}
				bf=nbf;
			}
			if( bf )
			{
				r=ZvukSave(D,&h,bf);
				if( r<0 ) Chyba(EFIL);
				myfreeSpc(bf);
			}
		}
		return True;
	}
}

#if 0 /* verze pro Falcon */

Err SaveHNastr( FILE *f, Pisen *P, const char *AVRPath ) /* uloz vsechny pouzite nastroje */
{
	long i,I;
	char C[128];
	PathName SR;
	PathName SamP;
	char *PP;
	NastrDef *N;
	/* uloz jen pouzite NastrDefy */
	/* oindexuj je ovsem stejne jako dosud */
	Plati( P )
	strcpy(SamP,AVRPath);
	strcpy(NajdiNazev(SamP),"SSS");
	I=NDyn(&P->LocDigi); /* lokalni pouzivame vsechny */
	for( N=PrvniDyn(&NDef,&i); N; N=DalsiDyn(&NDef,N,&i) )
	{
		if( N->Uzit ) I++;
	}
	if( fwrite(&I,sizeof(I),1,f)!=1 ) return EFIL;
	for( N=PrvniDyn(&NDef,&i); N; N=DalsiDyn(&NDef,N,&i) )
	{
		if( N->Uzit )
		{
			PathName NP;
			memset(C,0,sizeof(C));
			NastrojNazev(N->Soubory[0],NP);
			strcpy(C,NajdiNazev(NP));
			strcpy(NajdiPExt(C),".SSS");
			if( fwrite(&i,sizeof(i),1,f)!=1 ) return EFIL;
			if( fwrite(&C,sizeof(C),1,f)!=1 ) return EFIL;
			CopySampleToDir(SamP,NP);
		}
	}
	strcpy(SR,P->Cesta);
	PP=strrchr(SR,'\\');
	if( PP ) *PP=0;
	for( N=PrvniDyn(&P->LocDigi,&i); N; N=DalsiDyn(&P->LocDigi,N,&i) )
	{
		long c=i+0x4000;
		memset(C,0,sizeof(C));
		RelCesta(C,N->Soubory[0],SR);
		strcpy(NajdiPExt(C),".SSS");
		if( fwrite(&c,sizeof(c),1,f)!=1 ) return EFIL;
		if( fwrite(&C,sizeof(C),1,f)!=1 ) return EFIL;
		CopySampleToDir(SamP,N->Soubory[0]);
	}
	return EOK;
}

#else /* verze pro Jagu r */

static void UlozJagNastroj( NastrDef *N, const char *AVRPath )
{
	PathName SamP;
	PathName NP;
	strcpy(SamP,AVRPath);
	strcpy(NajdiNazev(SamP),"SSS");
	if( *N->Soubory[1] )
	{
		char Buf[256];
		sprintf(Buf,"[1][Export pro Jagu r.|Nepou§¡vejte multisamply.|('%s')][   OK   ]",N->Nazev);
		form_alert(1,Buf);
	}
	NastrojNazev(N->Soubory[0],NP);
	if( CopySampleToDir(SamP,NP) )
	{
		FILE *sez;
		FILE *fs,*ft;
		PathName Seznam;
		PathName PP;
		strcpy(Seznam,AVRPath);
		strcpy(NajdiNazev(Seznam),"instrum.inc");
		sez=fopen(Seznam,"a");
		if( sez )
		{
			FileName sss_n;
			strcpy(sss_n,NajdiNazev(NP));
			strcpy(NajdiPExt(sss_n),"");
			strlwr(sss_n);
			fprintf(sez,"$(M)\\%s.stp: $*.sss | -ii $*.stp s_%s\n",sss_n,N->Nazev);
			fprintf(sez,"$(M)\\%s.sss: | -ii $*.sss i_%s\n",sss_n,N->Nazev);
			fclose(sez);
		}
		strcpy(PP,AVRPath);
		strcpy(NajdiNazev(PP),"instr.s");
		ft=fopen(PP,"r");
		if( ft ) fclose(ft);
		fs=fopen(PP,"a");
		if( fs )
		{
			if( !ft ) fprintf(fs,"data\nI_List::\n");
			fprintf(fs,"dc.l i_%s\n",N->Nazev);
			fprintf(fs,"extern i_%s\n",N->Nazev);
			fclose(fs);
		}
	}
	#if 0
	{
		int ii;
		fprintf(f,"i_%s::\n",N->Nazev);
		for( ii=0; ii<NMSoub; ii++ )
		{
			FileName LNazev;
			strcpy(LNazev,NajdiNazev(N->Soubory[ii]));
			strcpy(NajdiPExt(LNazev),"");
			fprintf(f,"\textern sss_%s\n",LNazev);
			fprintf(f,"\tdc.l sss_%s\n",LNazev);
		}
	}
	#endif
}

Err SaveHNastr( FILE *f, Pisen *P, const char *AVRPath ) /* uloz vsechny pouzite nastroje */
{
	long i;
	NastrDef *N;
	/* uloz jen pouzite NastrDefy */
	Plati( P )
	for( N=PrvniDyn(&NDef,&i); N; N=DalsiDyn(&NDef,N,&i) )
	{
		if( N->Uzit ) UlozJagNastroj(N,AVRPath);
	}
	for( N=PrvniDyn(&P->LocDigi,&i); N; N=DalsiDyn(&P->LocDigi,N,&i) )
	{
		if( N->Uzit ) UlozJagNastroj(N,AVRPath);
	}
	(void)f;
	return EOK;
}

#endif /* verze pro Falcon */

void AktHNastr( void )
{
	long MemF=(long)Malloc(-1);
	if( MemF<RezVel*6 ) FAktHNastr(); /* zacni vyhazovat, co nepotrebujes */
}

Err VymazNastr( char *N, Pisen *P )
{
	int NN=NajdiDef(N,-1,&P->LocDigi);
	NastrDef *ND;
	if( NN<0 ) return ECAN;
	ND=NastrAcc(&P->LocDigi,NN);
	ND->Nazev[0]=0;
	if( NN<0x4000 ) NDZmena=True; /* neni lokalni! */
	else
	{
		Plati( P );
		P->Zmena=True;
		NastavNazvy();
	}
	return EOK;
}

Err KonSNastr( void )
{
	Err ret=PisSNastr(OrcNam);
	if( ret!=EOK ) return ret;
	MazSNastr();
	AktHraj=NULL;
	return EOK;
}

Err CtiOrchestr( void )
{
	PathName OrcP;
	FileName OrcF;
	PathName OrcNew;
	VyrobCesty(OrcP,OrcF,OrcNam);
	VsePrekresli();
	if( EFSel(FreeString(CTINASFS),OrcP,OrcF,OrcNew) )
	{
		Err ret;
		VsePrekresli();
		if( NDZmena )
		{
			Err ret;
			ret=PisSNastr(OrcNam);
			if( ret<EOK ) return ret;
		}
		strcpy(OrcNam,OrcNew);
		ZdaPisOrc=True;
		ret=CtiSNastr(OrcNew);
		if( ret<EOK ) return ret;
		return EOK;
	}
	return ECAN;
}
Err PisOrchestr( void )
{
	PathName OrcP;
	FileName OrcF;
	VyrobCesty(OrcP,OrcF,OrcNam);
	VsePrekresli();
	if( EFSel(FreeString(ULONASFS),OrcP,OrcF,OrcNam) )
	{
		Err ret;
		VsePrekresli();
		ZdaPisOrc=True;
		ret=FPisSNastr(OrcNam);
		if( ret<EOK ) return ret;
		return EOK;
	}
	return ECAN;
}

const char **SoupisSNastr( Pisen *P )
{
	long I,NN;
	NastrDef *N;
	const char **BV;
	const char **PV;
	NN=NDyn(&NDef);
	if( P )
	{
		NN+=NDyn(&P->LocDigi);
	}
	BV=myalloc(sizeof(const char *)*(NN+1),False);
	if( !BV ) return NULL;
	PV=BV;
	for( N=PrvniDyn(&NDef,&I); N; N=DalsiDyn(&NDef,N,&I) )
	{
		const char *Naz=N->Nazev;
		if( *Naz ) *PV++=Naz;
	}
	if( P )
	{
		for( N=PrvniDyn(&P->LocDigi,&I); N; N=DalsiDyn(&P->LocDigi,N,&I) )
		{
			const char *Naz=N->Nazev;
			if( *Naz ) *PV++=Naz;
		}
	}
	*PV=NULL;
	return BV;
}

/* sekce DSP */

static int CAb;
static void *CompoLod=NULL;
static long CLSize;

#define LodSizeX 0x3e00L
#define LodSizeY 0x3e00L
#define LodSize (LodSizeX+LodSizeY)
#define DspWordSize 3

static PathName LodName;
#define LodNName "COMPO.LOD"

static Flag IsLod=False;

static Err FZacLod( void )
{
	long ax,ay;
	if( !CompoLod ) return ECAN;
	if( Dsp_Lock()<0 ) {AlertRF(DSPERRA,1);return ECAN;}
	Dsp_Available(&ax,&ay);
	if( ax<LodSizeX || ay<LodSizeY )
	{
		Dsp_FlushSubroutines();
		Dsp_Available(&ax,&ay);
		if( ax<LodSizeX || ay<LodSizeY ) {Dsp_Unlock();AlertRF(DSPERRA,1);return ECAN;}
	}
	Dsp_Reserve(LodSizeX,LodSizeY);
	Dsp_ExecProg(CompoLod,CLSize,CAb);
	IsLod=True;
	return EOK;
}
void FKonLod( void )
{
	if( IsLod )
	{
		long in,out;
		in=1;
		Dsp_BlkUnpacked(&in,1L,&out,0L); /* deinit. proc */
		Dsp_Unlock();
		IsLod=False;
	}
}

Err ZacDsp( void )
{
	void *Buf;
	if( Zvuky()!=SndFalcon ) return ECAN;
	if( !CompoLod )
	{
		AbsCesta(LodName,LodNName,CompoRoot);
		Buf=myallocSpc(LodSize*DspWordSize,True,'LOD8');
		if( !Buf ) return ERAM;
		CAb=Dsp_RequestUniqueAbility();
		CLSize=Dsp_LodToBinary(LodName,Buf);
		if( CLSize<0 ) {AlertRF(IERRA,1,SizeNazev(LodName,25));free(Buf);return ECAN;}
		CompoLod=Buf;
	}
	return EOK;
}

Err ZacLod( void )
{
	return FZacLod();
}

void KonDsp( void )
{
	if( CompoLod ) myfreeSpc(CompoLod),CompoLod=NULL;
}
void KonLod( void )
{
	FKonLod();
}

/* nejvyssi uroven */

void AYCtiCFG( const Info *Inf )
{
	const char *r=Inf ? GetText(Inf,OrcNazev,0) : NULL;
	if( !r )
	{
		strcpy(OrcNam,CompoRoot);
		strcat(OrcNam,"*.ATV");
		ZdaPisOrc=False;
	}
	else
	{
		AbsCesta(OrcNam,r,CompoRoot),ZdaPisOrc=True;
	}
	F_Divis=CLK33K;
	InitFFreq();
}
Flag AYUlozCFG( Info *Inf )
{
	PathName r;
	RelCesta(r,OrcNam,CompoRoot);
	return !ZdaPisOrc || SetText(Inf,OrcNazev,r,0);
}

Err DigiZacNastr( LLHraj *H, double Fr )
{
	Err ret;
	ZacDyn(&NDef);
	ZacSoub();
	ret=CtiSNastr(OrcNam);
	if( ret<EOK ) {Chyba(ret);MazSNastr();}
	FreqIs=Fr;
	AktHraj=H;
	return EOK;
}


