/* import/export MIDI File - .MID souboru */
/* SUMA 12/1993-12/1993 */

#include <macros.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <gem\multiitf.h>
#include <stdc\infosoub.h>

#include "compo.h"
#include "utlcompo.h"
#include "ramcompo.h"

#define N_GM 128
#define N_NP 128

static int Quant=8; /* v 64-tin†ch */
static int PercusChannel=9;
static int PercusDura=16;
static char *DefaultVoice=NULL;

static Sekvence *Master;
static Sekvence *Drums;

typedef struct
{
	const char *Voice;
	int Offset;
	SekvKursor gk;
	Flag KursorOpened;
} GMPercus;

static const char *GmNames[N_GM];
static GMPercus *GmPercs;

/* bÿhem tvorby pouß°v°me Tempo==TempoMinFyz */
/* d°ky tomu je CasPos totÇß jako pozice poü°tan† v tic°ch */

static int MdfTempo; /* p˝edpokl†d†me spoleünÇ tempo */

static int MdfResol; /* üas vydÿlenÏ MdfResol d†v† pozici ve ütvrk†ch */

/* üas*Takt/MdfResol je pozice v intern°ch logickÏch jednotk†ch */

static const int PauDelky[]={Takt,Takt/2,Takt/4,Takt/8,Takt/16,Takt/32,0};
static const int TonDelky[]={Takt,Takt*3/4,Takt/2,Takt*3/8,Takt/4,Takt*3/16,Takt/8,Takt*3/32,Takt/16,Takt/32,0};

static int ZasadPauzu( SekvKursor *k, long Dif )
{
	const int *PD;
	int MaxD=AKontextK(k)->Takt;
	int AD=MaxD-AKontextK(k)->VTaktu;
	while( AD>=MaxD ) AD-=MaxD;
	while( AD<Takt/32 ) AD+=MaxD;
	if( Dif>AD ) Dif=AD; /* omez se na hranice taktu */
	for( PD=PauDelky; *PD; PD++ )
	{
		if( *PD<=Dif )
		{
			Err ret;
			BufK(k)[0]=PPau;
			BufK(k)[2]=*PD;
			BufK(k)[1]=BufK(k)[3]=0;
			ret=VlozK(k);if( ret<EOK ) return ret;
			break;
		}
	}
	return *PD;
}

static Err ZasadPauzy( SekvKursor *k, long dif )
{
	int dl;
	while( dif>=Takt/32 )
	{
		dl=ZasadPauzu(k,dif);
		if( dl<EOK ) return dl;
		dif-=dl;
	}
	return EOK;
}

static int AdjustCas( SekvKursor *k, long LCas, long Vynech )
{
	Err ret;
	long LAkt;
	NajdiSCasK(k,LCas);
	LAkt=CasJeK(k);
	Plati( LAkt<=LCas );
	if( TestK(k)<EOK )
	{ /* nÿkde za -> snaßÁ° p˝°pad */
		if( LAkt==LCas ) return 0; /* p˝esnÿ konec */
		ret=ZasadPauzy(k,LCas-LAkt);if( ret<EOK ) return ret;
		return EOK;
	}
	for(;;)
	{ /* najdi platnou notu */
		PlatiProc( TestK(k), ==EOK );
		if( BufK(k)[1]&Soubezna ) {PlatiProc( ZpatkyK(k),==EOK );}
		else break;
	}
	for(;;)
	{ /* najdi notu */
		if( TestK(k)<EOK ) break;
		if( DelkaKF(k)==0 ) {PlatiProc( JCtiK(k),==EOK );}
		else break;
	}
	TestK(k);
	{ /* mus°me dÿlit */
		Flag PuvPau=BufK(k)[0]==PPau;
		long DPuv=BufK(k)[2];
		long Dif=LCas-LAkt;
		PosT Sav;
		Plati( LAkt+DPuv>=LCas );
		if( PuvPau ) MazK(k); /* je to pauza - zaho‘ j° a nahra‘ */
		else
		{
			BufK(k)[1]|=Soubezna; /* udÿlej z n° soubÿßnou */
			PlatiProc( PisK(k), ==EOK );
		}
		ret=ZasadPauzy(k,Dif);if( ret<EOK ) return ret;
		LAkt+=Dif;
		Plati( LAkt==LCas );
		DPuv-=Dif;
		if( DPuv==Vynech ) return 0; /* akor†t to dopln° */
		Sav=KdeJeK(k);
		if( DPuv>Vynech )
		{
			ret=ZasadPauzy(k,DPuv-Vynech);if( ret<EOK ) return ret;
			NajdiK(k,Sav);
			return EOK;
		}
		else
		{
			ret=ZasadPauzy(k,DPuv);if( ret<EOK ) return ret;
			NajdiK(k,Sav);
			return 1; /* mus° bÏt soubÿßnÿ */
		}
	}
}


static Sekvence *SZalozHlas( Sekvence *S )
{
	SekvKursor z,d;
	Sekvence *H=S;
	Flag TaktC=False,ToniC=False,TempC=False;
	while( H->dalsi ) H=H->dalsi;
	H=NovyHlas(H);if( !H ) return NULL;
	ZacBlokZmen(H);
	OtevriK(&z,S,Cteni);
	OtevriK(&d,H,Zapis);
	NajdiSCasK(&z,0);
	while( ZpatkyK(&z)==OK )
	{
		if( TestK(&z)!=OK ) break;
		switch( BufK(&z)[0] )
		{
			case PNTakt:  if( !TaktC ) {TaktC=True;goto ZapisBuf;};break;
			case PTonina: if( !ToniC ) {ToniC=True;goto ZapisBuf;};break;
			case PTempo:  if( !TempC ) {TempC=True;goto ZapisBuf;};break;
			ZapisBuf:
				EquJed(BufK(&d),BufK(&z));
				if( VlozK(&d)!=OK ) {Chyba(ERAM);break;}
		}
	}
	NormujK(&d);
	ZavriK(&d);
	ZavriK(&z);
	KonBlokZmen(H);
	return H;
}

static const enum CPovel Pulton2Ton[12]=
{
/*C     D     E  F     G     A     H */
	TC,TC,TD,TD,TE,TF,TF,TG,TG,TA,TA,TH
};
static const int Pulton2Znacka[12]=
{
/*C              D              E      F              G               A             H */
	OdrazA,KrizekA,OdrazA,KrizekA,OdrazA,OdrazA,KrizekA,OdrazA,KrizekA,OdrazA,KrizekA,OdrazA
};

static Err FMdfInsertTon( SekvKursor *k, int Nota, Flag Napoj, int del, Flag Soubeh )
{
	Err ret;
	int O=Nota/12;
	int P=Nota%12;
	int T=Pulton2Ton[P];
	int A=Pulton2Znacka[P];
	int PuvO,NovOkt=0;
	O-=4;
	PuvO=AKontextK(k)->Oktava;
	O-=PuvO;
	while( O>=NOkt ) O--,NovOkt++;
	while( O<0 ) O++,NovOkt--;
	if( NovOkt )
	{
		while( NovOkt>2 )
		{
			BufK(k)[0]=POktava;
			BufK(k)[1]=+2;
			BufK(k)[2]=True;
			BufK(k)[3]=0;
			ret=VlozK(k);if( ret<EOK ) return ret;
			NovOkt-=2;
		}
		while( NovOkt<-2 )
		{
			BufK(k)[0]=POktava;
			BufK(k)[1]=-2;
			BufK(k)[2]=True;
			BufK(k)[3]=0;
			ret=VlozK(k);if( ret<EOK ) return ret;
			NovOkt+=2;
		}
		BufK(k)[0]=POktava;
		BufK(k)[1]=NovOkt;
		BufK(k)[2]=True;
		BufK(k)[3]=0;
		ret=VlozK(k);if( ret<EOK ) return ret;
	}
	if( Soubeh ) A|=Soubezna;
	if( Napoj ) A|=Leg;
	BufK(k)[0]=TC+T+O*Okt;
	BufK(k)[1]=A;
	BufK(k)[2]=del;
	BufK(k)[3]=0;
	ret=VlozK(k);
	return ret;
}

static Err MdfInsertTon( SekvKursor *k, int Nota, long cas, long del )
{ /* Cas i Del v Midi üasov†n° */
	Err ret;
	long LCas=(cas*(Takt/4)+MdfResol/2)/MdfResol; /* üas mÿ˝enÏ v 1/Taktin†ch */
	long LDel=(del*(Takt/4)+MdfResol/2)/MdfResol;
	/*long LDel0=LDel;*/
	const int *PD;
	int QEps=3*Quant;
	/*
	if( QEps>LDel/2 ) QEps=((int)LDel+3)/6*3; /* adaptivn° vzorec */
	if( QEps<3 ) QEps=3;
	*/
	LDel=(LCas+LDel+QEps/2)/QEps*QEps; /* kvantizace */
	LCas=(LCas+QEps/2)/QEps*QEps;
	LDel=LDel-LCas;
	while( LDel>0 )
	{
		for( PD=TonDelky; *PD; PD++ )
		{
			if( *PD<=LDel ) break;
		}
		if( !*PD ) return EOK; /* moc kr†tkÏ */
		ret=AdjustCas(k,LCas,*PD);
		if( ret<EOK ) return ret;
		/*
		ret=FMdfInsertTon(k,Nota,LDel>*PD,*PD,ret>0);
		*/
		ret=FMdfInsertTon(k,Nota,False,*PD,ret>0);
		if( ret<EOK ) return ret;
		break; /* nesnaß se legatovat */
		/*
		LCas+=*PD;
		LDel-=*PD;
		if( LDel<=LDel0/8 ) return ret;
		*/
	}
	return EOK;
}

static Err VlozJedn( SekvKursor *k, long cas, JednotkaBuf B )
{
	long LCas=cas*(Takt/4)/MdfResol; /* üas mÿ˝enÏ v 1/Taktin†ch */
	Err ret;
	ret=AdjustCas(k,LCas,0);if( ret<EOK ) return ret;
	EquJed(BufK(k),B);
	ret=VlozK(k);
	return ret;
}

static Err MdfInsertPerc( Pisen *P, int MK, int Vol, long cas )
{
	Err ret;
	int d;
	Flag Soubeh;
	GMPercus *AP=&GmPercs[MK];
	int NB;
	SekvKursor *k;
	long LCas=(cas*(Takt/4)+MdfResol/2)/MdfResol; /* üas mÿ˝enÏ v 1/Taktin†ch */
	int LDel=Takt/64*PercusDura;
	int QEps=3*Quant;
	/*int QEps=3;*/
	LCas=(LCas+QEps/2)/QEps*QEps;
	if( !AP->Voice ) return EOK;
	if( !AP->KursorOpened )
	{
		Sekvence *H;
		if( !Drums ) H=Drums=NovaSekv(P,"RhytmTrack");
		else H=SZalozHlas(Drums);
		if( !H ) return ERAM;
		ZacBlokZmen(H);
		OtevriK(&AP->gk,H,Zapis);
		KKontextK(&AP->gk)->Tempo=(int)TempoMin;
		SetKKontext(&AP->gk,KKontextK(&AP->gk));
		AP->KursorOpened=True;
	}
	k=&AP->gk;
	ret=AdjustCas(k,LCas,LDel);
	if( ret<EOK ) return ret;
	Soubeh=ret>0;
	d=Vol*2-AKontextK(k)->Hlasit;
	if( abs(d)>4	)
	{
		BufK(k)[0]=PHlasit;
		BufK(k)[1]=Vol*2;
		BufK(k)[2]=0;
		BufK(k)[3]=0;
		ret=VlozK(k);if( ret<EOK ) return ret;
	}
	NB=NajdiRetez(P->Nastroje,AP->Voice);
	if( NB<0 || AKontextK(k)->Nastroj!=NB )
	{
		NB=PridejRetez(P->Nastroje,AP->Voice);
		if( NB<EOK ) return (Err)NB;
		BufK(k)[0]=PNastroj;
		BufK(k)[1]=NB;
		BufK(k)[2]=0;
		BufK(k)[3]=0;
		ret=VlozK(k);if( ret<EOK ) return ret;
	}
	return FMdfInsertTon(k,60+AP->Offset,False,LDel,Soubeh);
}

static long getL( FILE *f )
{
	long l;
	if( 1!=fread(&l,sizeof(l),1,f) ) return -1;
	return l;
}
static long getW( FILE *f )
{
	word l;
	if( 1!=fread(&l,sizeof(l),1,f) ) return -1;
	return l;
}
static long getDel( FILE *f, long *len )
{
	long del=0;
	int c;
	do
	{
		c=fgetc(f);if( c<0 ) return -1;
		if( len ) (*len)--;
		del<<=7;
		del|=c&0x7f;
	} while( c&0x80 );
	return del;
}

static int skip( FILE *f, long len )
{
	char b[256];
	if( len>0 )
	{
		while( len>sizeof(b) )
		{
			if( 1!=fread(b,sizeof(b),1,f) ) return -1;
			len-=sizeof(b);
		}
		if( 1!=fread(b,len,1,f) ) return -1;
	}
	return 0;
}

static int GetAscii( char *Ascii, FILE *f, long l )
{
	int i;
	for( i=0; i<255 && l>0; i++ )
	{
		int c;
		c=fgetc(f),l--;if( c<0 ) return -1;
		Ascii[i]=c;
	}
	Ascii[i]=0;
	if( skip(f,l)<0 ) return -1;
	return 0;
}

static Flag Pismeno( char c )
{
	return c&0x80 || isalpha(c) || strchr("_",c);
}

static void NormujNazev( char *N )
{
	char *S=N;
	while( *S )
	{
		if( !Pismeno(*S) ) *S='_';
		S++;
	}
	while( S>N && S[-1]=='_' ) *--S=0;
}

enum {NMidKan=16};
enum {MaxBufNot=128};

struct stopa
{
	Sekvence *H;
	SekvKursor k;
	Flag NoInstr;
	long Noty[128]; /* casy NoteOnu */
	int Veloc[128];
	int Sustain; /* co je v kanalu nastaveno */
};

static Err NoteOff( Sekvence *S, struct stopa *as, int cc, int c, long Tim, int Sustain )
{
	Err ret,r;
	if( as->Noty[c]>=0 )
	{
		if( cc!=PercusChannel )
		{ /* bic° jsou zvl†Áú */
			if( as->NoInstr )
			{
				if( DefaultVoice )
				{
					BufK(&as->k)[0]=PNastroj;
					BufK(&as->k)[1]=PridejRetez(S->Pis->Nastroje,DefaultVoice);
					if( BufK(&as->k)[1]<MAXERR ) {ret=ERAM;goto SError;}
					BufK(&as->k)[2]=BufK(&as->k)[3]=0;
					r=VlozK(&as->k);if( r<EOK ) {ret=r;goto SError;}
					as->NoInstr=False;
				}
			}
			if( !as->NoInstr )
			{
				long Del=Tim-as->Noty[c];
				Del*=Sustain;
				r=MdfInsertTon(&as->k,c,as->Noty[c],Del);
				if( r<EOK ) {ret=r;goto SError;}
			}
		}
		as->Noty[c]=-1;
	}
	return EOK;
	SError: return ret;
}

static Err CtiStopu( Pisen *P, FILE *f, long nt )
{
	JednotkaBuf B;
	struct stopa Ks[NMidKan];
	char Ascii[256];
	Err ret=ERR,r;
	int s=0x90;
	Sekvence *S,*H;
	long len;
	long Tim=0; /* Midi üas */
	int cc,n;
	char Naz[32];
	if( getL(f)!='MTrk' ) goto Error;
	len=getL(f);if( len<0 ) goto Error;
	sprintf(Naz,"Track%02ld",nt+1);
	S=NovaSekv(P,Naz);
	if( !S ) {ret=ERAM;goto Error;}
	for( H=S,cc=0; cc<NMidKan; cc++ )
	{
		struct stopa *as=&Ks[cc];
		if( cc>0 )
		{
			H=NovyHlas(H);
			if( !H ) {MazSekv(S);ret=ERAM;goto Error;}
		}
		as->H=H;
		as->NoInstr=True;
		as->Sustain=1;
		ZacBlokZmen(H);
		for( n=0; n<128; n++ ) as->Noty[n]=0;
	}
	for( H=S,cc=0; H && cc<NMidKan; cc++,H=DalsiHlas(H) )
	{
		struct stopa *as=&Ks[cc];
		OtevriK(&as->k,as->H,Zapis);
		KKontextK(&as->k)->Tempo=(int)TempoMin;
		SetKKontext(&as->k,KKontextK(&as->k));
	}
	while( len>0 )
	{
		struct stopa *as=&Ks[0]; /* !!! spr†vnÿ default track !!! */
		long l;
		int c,t,vl;
		l=getDel(f,&len);if( l<0 ) goto SError;
		Tim+=l;
		c=fgetc(f);if( c<0 ) goto SError;
		len--;
		switch( c ) /* povel */
		{
			case 0xF0: /* SysEx */
				l=getDel(f,&len);if( l<1 ) goto SError;
				l--;
				if( skip(f,l)<0 ) goto SError;
				len-=l;
				c=fgetc(f);if( c!=0xf7 ) goto SError;
				len--;
				break;
			case 0xff: /* MetaEvent */
				t=fgetc(f);if( t<0 ) goto SError; /* typ ME */
				len--;
				l=getDel(f,&len);if( l<0 ) goto SError;
				switch( t )
				{
					case 1: /* n†zev skladby */
						if( GetAscii(Ascii,f,l)<0 ) goto SError;
						len-=l;
						l=0;
						strlncpy(P->Popis1,Ascii,sizeof(P->Popis1));
					break;
					case 2: /* Copyright */
						if( GetAscii(Ascii,f,l)<0 ) goto SError;
						len-=l;
						l=0;
						strlncpy(P->Popis2,Ascii,sizeof(P->Popis2));
					break;
					case 3: /* n†zev stopy */
						if( GetAscii(Ascii,f,l)<0 ) goto SError;
						len-=l;
						l=0;
						NormujNazev(Ascii);
						if( !NajdiSekv(P,Ascii,0) )
						{
							r=ZmenNazevSekv(S,Ascii);if( r<EOK ) {ret=r;goto SError;}
						}
					break;
					case 4: /* n†zev n†stroje */
						/*
						if( GetAscii(Ascii,f,l)<0 ) goto SError;
						len-=l;
						l=0;
						NormujNazev(Ascii);
						if( !NajdiSekv(P,Ascii,0) )
						{
							r=ZmenNazevSekv(S,Ascii);if( r<EOK ) {ret=r;goto SError;}
						}
						*/
					break;
					case 0x2f:
					goto Konec;
					case 0x51: /* abs. tempo */
					{
						long v;
						t=fgetc(f);if( t<0 ) goto SError;
						len--,l--;
						v=t;
						t=fgetc(f);if( t<0 ) goto SError;
						len--,l--;
						v<<=8;
						v|=t;
						t=fgetc(f);if( t<0 ) goto SError;
						len--,l--;
						v<<=8;
						v|=t;
						if( v>0 ) v=60000000L/v;
						else v=MaxTemp;
						if( v>MaxTemp ) v=MaxTemp;
						ef( v<MinTemp ) v=MinTemp;
						MdfTempo=(int)v;
					}
					break;
					case 0x58:
						B[0]=PNTakt;
						t=fgetc(f);if( t<0 ) goto SError; /* c */
						len--,l--;
						
						B[1]=t;
						t=fgetc(f);if( t<0 ) goto SError; /* j */
						len--,l--;
						t=1<<t;
						if( t>8 ) t=8;
						B[2]=t;
						B[3]=0;
						r=VlozJedn(&as->k,Tim,B);if( r<EOK ){ret=r;goto SError;}
						/*
						B[0]=PTempo;
						B[2]=B[3]=0;
						r=VlozJedn(S,Tim,B);if( r<EOK ){ret=r;goto SError;}
						*/
						t=fgetc(f);if( t<0 ) goto SError; /* cc */
						len--,l--;
						t=fgetc(f);if( t<0 ) goto SError; /* bb */
						/* bb je snad vßdycky 8 */
						/* my to stejnÿ nepouß°v†me */
						len--,l--;
					break;
					case 0x59:
						t=fgetc(f);if( t<0 ) goto SError; /* typ ME */
						len--,l--;
						if( t>-7 && t<+7 )
						{
							B[0]=PTonina;
							B[1]=t;
							t=fgetc(f);if( t<0 ) goto SError; /* typ ME */
							len--,l--;
							B[2]=t==0;
							B[3]=0;
							r=VlozJedn(&as->k,Tim,B);if( r<EOK ){ret=r;goto SError;}
						}
					break;
				}
				if( skip(f,l)<0 ) goto SError;
				len-=l;
				break;
			default:
				if( c>=0x80 )
				{
					s=c;
					c=fgetc(f);if( c<0 ) goto SError;
					len--;
				}
				cc=s&0xf;
				as=&Ks[cc];
				switch( s>>4 )
				{
					case 0x8: /* Note Off */
						if( c<0 || c>0x7f ) goto SError; /* nota */
						vl=fgetc(f);if( vl<0 ) goto SError; /* Veloc. */
						len--;
						r=NoteOff(S,as,cc,c,Tim,as->Sustain);if( r<EOK ) {ret=r;goto SError;}
					break;
					case 0x9: /* NoteOn */
						if( c<0 || c>0x7f ) goto SError; /* nota */
						vl=fgetc(f);if( vl<0 || vl>0x7f ) goto SError; /* Veloc. */
						len--;
						if( vl==0 )
						{
							r=NoteOff(S,as,cc,c,Tim,as->Sustain);if( r<EOK ) {ret=r;goto SError;}
						}
						ef( cc==PercusChannel )
						{ /* drum track */
							r=MdfInsertPerc(P,c,vl,Tim);
							if( r<EOK ) {ret=r;goto SError;}
						}
						else
						{
							as->Noty[c]=Tim;
							as->Veloc[c]=vl;
						}
					break;
					case 0xa: /* Ind. tlak */
						if( c<0 || c>0x7f ) goto SError; /* nota */
						vl=fgetc(f);if( vl<0 ) goto SError; /* val */
						len--;
					break;
					case 0xb: /* kontrol. */
						if( c<0 || c>0x7f ) goto SError; /* kontr. */
						vl=fgetc(f);if( vl<0 || vl>0x7f ) goto SError; /* val */
						len--;
						switch( c )
						{
							case 7: /* hlasitost */
								BufK(&as->k)[0]=PHlasit;
								BufK(&as->k)[1]=vl*18/10;
								BufK(&as->k)[2]=0;
								BufK(&as->k)[3]=0;
								ret=VlozK(&as->k);if( ret<EOK ) return ret;
							break;
							case 10: /* balance */
								vl=vl*2-128;
								if( vl<MinSt ) vl=MinSt;if( vl>MaxSt ) vl=MaxSt;
								BufK(&as->k)[0]=PStereo;
								BufK(&as->k)[1]=vl;
								BufK(&as->k)[2]=vl;
								BufK(&as->k)[3]=0;
								ret=VlozK(&as->k);if( ret<EOK ) return ret;
							break;
							case 64: /* sustain */
								as->Sustain=1+(vl>>5);
								/* 0x40 d†v° sustain 3 */
							break;
						}
					break;
					case 0xc: /* Prog. Chng */
					{
						const char *N;
						if( c<0 || c>0x7f ) goto SError; /* Nastr */
						N=GmNames[c];
						if( N )
						{
							BufK(&as->k)[0]=PNastroj;
							BufK(&as->k)[1]=PridejRetez(S->Pis->Nastroje,N);
							if( BufK(&as->k)[1]<MAXERR ) {ret=ERAM;goto SError;}
							BufK(&as->k)[2]=BufK(&as->k)[3]=0;
							r=VlozK(&as->k);if( r<EOK ) {ret=r;goto SError;}
							as->NoInstr=False;
						}
						else as->NoInstr=True;
					}	
					break;
					case 0xd: /* Spol. tlak */
						if( c<0 ) goto SError; /* val */
					break;
					case 0xe: /* PitchBend */
						if( c<0 ) goto SError; /* val LSB */
						c=fgetc(f);if( c<0 ) goto SError; /* val MSB */
						len--;
					break;
					default: goto SError;
				}
				break;
		}
	}
	Konec:
	if( skip(f,len)<0 ) goto SError;
	ret=EOK;
	SError:
	for( H=S,cc=0; H && cc<NMidKan; cc++,H=DalsiHlas(H) )
	{
		struct stopa *as=&Ks[cc];
		if( nt==0 && cc==0 )
		{
			NajdiK(&as->k,0);
			if( TestK(&as->k)==EOK )
			{
				BufK(&as->k)[0]=PTempo;
				BufK(&as->k)[1]=MdfTempo;
				BufK(&as->k)[2]=B[3]=0;
				r=VlozK(&as->k);if( r<EOK ) Chyba(r);
			}
		}
		ZavriK(&as->k);
		KonBlokZmen(H);
	}
	if( ret!=EOK ) MazSekv(S),S=NULL;
	else
	{ /* vymaß nepoußitÇ stopy */
		Flag Prvni=True;
		for( H=S; H; )
		{
			Sekvence *N=DalsiHlas(H);
			if( ( nt>0 || !Prvni ) && CasDelka(H,NULL)<=1 )
			{
				if( H->dalsi || H->predch ) MazHlas(H);
				else {MazSekv(H);break;}
			}
			H=N;
			Prvni=False;
		}
	}
	Error:
	if( nt==0 ) Master=S;
	return ret;
}

static long GetInt( Info *I, const char *T, int N )
{
	char *S=GetText(I,T,N);
	if( S ) return strtol(S,NULL,10);
	return -1;
}

Pisen *MdfImport( const char *Nam )
{
	Info GmVoi;
	Err ret=ERR;
	Pisen *P;
	long l,len,nt,it;
	FILE *f;
	Mouse(HOURGLASS);
	Master=NULL;
	Drums=NULL;
	DefaultVoice=NULL;
	if( CtiInfo("DEFMIDI.INF",&GmVoi) )
	{
		int i;
		long Q;
		char *T;
		Q=GetInt(&GmVoi,"Quant",0);
		if( Q<0 ) Q=8;
		if( Q<2 ) Q=2;
		if( Q>64 ) Q=64;
		Quant=(int)Q;
		Q=GetInt(&GmVoi,"PercusChannel",0);
		if( Q<0 ) Q=10;
		if( Q<1 ) Q=1;
		if( Q>16 ) Q=16;
		PercusChannel=(int)Q-1;
		Q=GetInt(&GmVoi,"PercusDuration",0);
		if( Q<0 ) Q=16;
		if( Q<1 ) Q=1;
		if( Q>64 ) Q=64;
		PercusDura=(int)Q;
		T=GetText(&GmVoi,"Default",0);
		if( T ) DefaultVoice=Duplicate(T);
		for( i=0; i<N_GM; i++ ) GmNames[i]=NULL;
		for( i=0; i<N_GM; i++ )
		{
			char *T=GetText(&GmVoi,"Prog",i);
			if( T )
			{
				char *M=strchr(T,' ');
				if( M )
				{
					char *H;
					long GMI;
					*M++=0;
					H=strchr(M,'*');
					if( H ) *H=0;
					while( H>M && H[-1]==' ' ) *--H=0;
					if( strcmp(M,"-") )
					{
						GMI=strtoul(T,NULL,10)-1;
						if( GMI>=0 && GMI<N_GM )
						{
							if( !GmNames[(int)GMI] ) GmNames[(int)GMI]=Duplicate(M);
						}
					}
				}
			}
		}
		GmPercs=myalloc(sizeof(*GmPercs)*N_NP,True);
		if( GmPercs )
		{
			for( i=0; i<N_NP; i++ )
			{
				GMPercus *AP=&GmPercs[i];
				AP->Voice=NULL;
				AP->KursorOpened=False;
			}
			for( i=0; i<N_NP; i++ )
			{
				char *T=GetText(&GmVoi,"Percus",i);
				if( T )
				{ /* p˝°klad: Percus 63 finger_snap 0 * Conga */
					char *M=strchr(T,' ');
					if( M )
					{
						char *H;
						long GMI;
						*M++=0;
						H=strchr(M,'*');
						if( H ) *H=0;
						while( H>M && H[-1]==' ' ) *--H=0;
						H=strchr(M,' '); /* offset */
						if( H ) *H++=0;
						if( strcmp(M,"-") )
						{
							GMI=strtoul(T,NULL,10)-1;
							if( GMI>=0 && GMI<N_NP )
							{
								if( !GmPercs[(int)GMI].Voice )
								{
									long O=strtol(H,NULL,10);
									if( O<-64 ) O=-64;if( O>+64 ) O=+64;
									GmPercs[(int)GMI].Voice=Duplicate(M);
									GmPercs[(int)GMI].Offset=(int)O;
								}
							}
						}
					}
				}
			}
		}
		KonInfo(&GmVoi);
	}
	else AlertRF(IERRA,1,"GMVOICES.INF");
	if( !GmPercs ) {ChybaRAM();return NULL;}
	P=NovaPisen(Nam);
	if( !P ) {ChybaRAM();return P;}
	f=fopen(Nam,"rb");
	if( !f )
	{
		Sekvence *S=NovaSekv(P,MainMel);
		if( !S ) {MazPisen(P);ChybaRAM();return NULL;}
		P->Zmena=False;
		return P;
	}
	MdfTempo=100; /* default hodnoty */
	MdfResol=192;
	if( getL(f)!='MThd' ) goto Error;
	len=getL(f);if( len<0 ) goto Error; /* delka hlav. */
	l=getW(f);if( l<0 ) goto Error; /* format 0..2 */
	nt=getW(f);if( nt<0 ) goto Error; /* pocet stop */
	l=getW(f);if( l<0 ) goto Error; /* rozliseni */
	MdfResol=(int)l;
	if( skip(f,len-6)<0 ) goto Error;
	for( it=0; it<nt; it++ )
	{
		ret=CtiStopu(P,f,it);
		if( ret<EOK ) goto Error;
	}
	fclose(f);
	/* ze stopy 1 (MasterTrack) udÿlej main */
	{ /* zav˝i bic° */
		int i;
		for( i=0; i<N_NP; i++ )
		{
			GMPercus *AP=&GmPercs[i];
			if( AP->KursorOpened )
			{
				KonBlokZmen(SekvenceK(&AP->gk));
				ZavriK(&AP->gk);
			}
		}
	}
	if( Master && !Master->dalsi )
	{
		Sekvence *S;
		Sekvence *H=Master;
		Flag NovaS=False;
		int i;
		/* vyvolej melodie */
		if( CasDelka(Master,NULL)>1 ) NovaS=True; /* stopa je obsazena */
		for( S=PrvniSekv(Master->Pis); S; S=DalsiSekv(S) ) if( S!=Master && S!=Drums )
		{
			if( NovaS )
			{
				H=SZalozHlas(Master);if( !H ) {Chyba(ERAM);break;}
			}
			NovaS=True;
			{
				SekvKursor d;
				OtevriK(&d,H,Zapis);
				NajdiSCasK(&d,0);
				BufK(&d)[0]=PVyvolej;
				BufK(&d)[1]=PridejRetez(H->Pis->NazvySekv,NazevSekv(S));
				Plati( BufK(&d)[1]>=EOK );
				BufK(&d)[2]=0;
				BufK(&d)[3]=0;
				if( VlozK(&d)!=EOK ) Chyba(ERAM);
				ZavriK(&d);
			}
		}
		ZmenNazevSekv(Master,MainMel);
		/* vyvolej bic° */
		for( i=0,S=Drums; S; S=DalsiHlas(S),i++ )
		{
			if( NovaS )
			{
				H=SZalozHlas(Master);if( !H ) {Chyba(ERAM);break;}
			}
			NovaS=True;
			{
				SekvKursor d;
				OtevriK(&d,H,Zapis);
				NajdiSCasK(&d,0);
				BufK(&d)[0]=PVyvolej;
				BufK(&d)[1]=PridejRetez(H->Pis->NazvySekv,NazevSekv(S));
				Plati( BufK(&d)[1]>=EOK );
				BufK(&d)[2]=i;
				BufK(&d)[3]=0;
				if( VlozK(&d)!=EOK ) Chyba(ERAM);
				ZavriK(&d);
			}
		}
	}
	NormalizaceF(P,Master,NULL);
	P->Zmena=False;
	if( DefaultVoice ) free(DefaultVoice),DefaultVoice=NULL;
	return P;
	Error:
	if( DefaultVoice ) free(DefaultVoice),DefaultVoice=NULL;
	fclose(f);
	MazPisen(P);
	{
		FileName FN;
		strcpy(FN,NajdiNazev(Nam));
		Transf(FN);
		switch( ret )
		{
			case ERR: AlertRF(EKFORMA,1,FN);break;
			case ERAM: ChybaRAM();break;
			default: AlertRF(IERRA,1,FN);break;
		}
	}
	{
		int i;
		for( i=0; i<N_GM; i++ )
		{
			if( GmNames[i] ) myfree(GmNames[i]),GmNames[i]=NULL;
		}
		if( GmPercs )
		{
			for( i=0; i<N_NP; i++ )
			{
				if( GmPercs[i].Voice ) myfree(GmPercs[i].Voice),GmPercs[i].Voice=NULL;
			}
			myfree(GmPercs),GmPercs=NULL;
		}
	}
	return NULL;
}
Flag MdfExport( const char *Nam, Pisen *P )
{
	(void)Nam,(void)P;
	{
		/*Error:*/
		AlertRF(NOEXPOA,1,"MID");
		return False;
	}
}
