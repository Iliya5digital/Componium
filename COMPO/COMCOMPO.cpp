/* Componium - prekladac */
/* SUMA 10/1992-2/1994 */

#include "../stdafx.h"
#include "macros.h"
#include <string.h>
#include <stdio.h>
/*
#include "STDC/dynbuff.h"
#include "STDC/budiky.h"

#include "compo.h"
#include "ramcompo.h"
#include "digill.h"
#include "comcompo.h"
*/

#include "ramcompo.h"

#if 0
#define PRO_JAG 0 /* generov n¡ vìstup… pro Jagu r */

/* FadeOuty: 18. 8., suggested by COMPO */

#define FADE_OUT 1 /* omezuje cvakani - vhodne pro samply */
#define ROVNOMERNE 0 /* omezuje hromadeni povelu - vhodne pro MIDI */

/* preklada se do vnitrniho formatu - je velmi podobny SMFF 0 */
/* operace pro preklad */

enum {MaxDynRep=12};

typedef struct alokkan
{
	Budik B;
	int Kanal; /* jaky je to kanal - konstantni */
	int LNastroj; /* nastroj v kanale - cislovani v banku pisne */
	int FNastroj; /* jakì je to fyz. handle - Ÿ¡slov n¡ podle AktHraj */
	struct kanprekl *LKanal; /* jaky kanal zde naposledy hral - pro dedeni */
	Flag Rytm; /* ton se sam hned vypina */
	Flag CekamLegato; /* cekam-li legato v LKanal */
	int StereoMin,StereoMax; /* stereo */
	int EffA,EffB;
	int Ton; /* <0 zadny - to by nemelo nastat? - pak by mel byt volny? */
	int PBender; /* dos. stav PB - zatim nepouzit */
	Flag FadeOut; /* uz proveden FadeOut? */
	struct alokkan **back; /* zpetny odkaz */
} AlokKan; /* informace o kanalech */

enum {NAkTon=8}; /* soubØ§nìch t¢n… v jednom kan lu */

typedef struct kanprekl
{
	Budik B; /* dedicnost */
	SekvSoubor p;
	PosuvT Pos; /* platne posuvky */
	int Opak[MaxDynRep]; /* repetice */
	int RepUr;
	int Kan;
	AlokKan *FyzK[NAkTon];
} KanPrekl;

typedef struct
{
	KanPrekl KP[MaxLogKan];
	AlokKan KA[MaxFyzKan];
	FrontaB FrontaAK; /* ve fronte jsou obsazene kanaly */
	FrontaB FrontaKP; /* ve fronte cekaji ti, co jeste hraji */
	int NKan; /* pocet stop */
	int NFyzKan; /* pocet prehravacich kanalu */
	Buffer PisBuf;
	CasT DosCas; /* cas zatim dosazeny */
	CompoErr *CErr;
	Sekvence *S;
	Flag IgnorujNDef;
	long ZbyvaSynchro;
	int ObsKan; /* prave obsazeny pocet kanalu */
	int MaxObsKan; /* max., >NFyzKan znamena preteceni */
	Flag NejakyTon; /* zda se vybec pouziva */
	FILE *GenJag; /* gen. pro Jagu ra */
} PreklInfo;

#if PRO_JAG
	static int fputw( int num, FILE *f )
	{
		int r=fputc((byte)(num>>8),f);
		if( r<0 ) return r;
		return fputc((byte)num,f);
	}
	enum {pNotaZac,pSync,pNastr,pStereo,pEfekt,pKonec};

	typedef char NasName[64];
	NasName *NasSez;

	static Flag RepExp;
	static void LoadNas( const char *Path )
	{
		PathName P;
		NasName *NS;
		FILE *f;
		if( NasSez )
		{
			if( !RepExp ) return;
			freeSpc(NasSez);
		}
		RepExp=False;
		strcpy(P,Path);
		*NajdiNazev(P)=0;
		strcat(P,"instr.s");
		NasSez=mallocSpc(sizeof(NasName)*512,'NSez');
		if( !NasSez ) return;
		NS=NasSez;
		f=fopen(P,"r");
		if( f )
		{
			const char *S;
			while( (S=CtiSlovo(f))!=NULL )
			{
				if( !strncmp(S,"i_",2) )
				{
					const char *TS;
					strcpy(*NS,S+2);
					NS++;
					TS=CtiSlovo(f);
					if( strcmp(TS,"extern") ) Chyba(EFIL);
					TS=CtiSlovo(f);
					if( strcmp(TS,S) ) Chyba(EFIL);
				}
			}
			fclose(f);
		}
		**NS=0;
	}	
	static int JagNas( const char *S )
	{
		NasName *NS;
		int i;
		Plati( NasSez );
		for( i=0,NS=NasSez; **NS; NS++,i++ )
		{
			if( !strcmp(S,*NS) ) return i;
		}
		if( !RepExp )
		{
			char Buf[256];
			sprintf(Buf,"[1][Sample not defined.|Repeat export.|%s][   OK   ]",S);
			form_alert(1,Buf);
			RepExp=True;
		}
		return 0;
	}

#endif

static void ComChyba( SekvSoubor *s, int Kan, PreklInfo *PI, const char *Alert )
{
	CompoErr *CE=PI->CErr;
	SekvSoubor k;
	CE->Kan=Kan;
	CE->S=PI->S;
	RozdvojS(&k,s);
	ZpatkyS(&k);
	CE->CPos=CasJeS(&k);
	KdeJeS(&k,&CE->PPos);
	ZavriS(&k);
	CE->Ukazat=True;
	CE->Alert=Alert;
}

static Err FZapisCekani( PreklInfo *PI, long Cas )
{
	if( Cas>0 )
	{
		while( Cas>0x7fff )
		{
			if( !ZapisW(&PI->PisBuf,(MSync<<8)|0) ) return ERAM;
			if( !ZapisW(&PI->PisBuf,0x7fff) ) return ERAM;
			Cas-=0x7fff;
			#if PRO_JAG
			if( PI->GenJag )
			{
				fputc(pSync,PI->GenJag);
				fputc(0,PI->GenJag);
				fputw(0x7fff,PI->GenJag);
			}
			#endif
		}
		if( !ZapisW(&PI->PisBuf,(MSync<<8)|0) ) return ERAM;
		if( !ZapisW(&PI->PisBuf,(int)Cas) ) return ERAM;
		#if PRO_JAG
		if( PI->GenJag )
		{
			fputc(pSync,PI->GenJag);
			fputc(0,PI->GenJag);
			fputw((int)Cas,PI->GenJag);
		}
		#endif
	}
	return EOK;
}

static Err ZapisCekani( PreklInfo *PI, long Cas )
{
	PI->ZbyvaSynchro+=Cas;
	return EOK;
}

static Err NutnoSynchro( PreklInfo *PI )
{
	Err ret=FZapisCekani(PI,PI->ZbyvaSynchro);
	PI->ZbyvaSynchro=0;
	return ret;
}

#if ROVNOMERNE

enum {SkoroCas=20,SkoroKrok=5};

static Err NutnoSkoroSynchro( PreklInfo *PI )
{
	Err ret;
	long PC=PI->ZbyvaSynchro;
	if( PC>SkoroCas ) PC-=SkoroCas;
	ef( PC>SkoroKrok ) PC=SkoroKrok;
	ret=FZapisCekani(PI,PC);
	PI->ZbyvaSynchro-=PC;
	return ret;
}

#else
#define NutnoSkoroSynchro NutnoSynchro
#endif

#if FADE_OUT

static Err FOutKan( PreklInfo *PI, AlokKan *PA )
{
	Err ret;
	Plati( PA->Ton>=0 ); /* hraje */
	ret=NutnoSynchro(PI);if( ret<EOK ) return EOK;
	if( !ZapisW(&PI->PisBuf,(MNotaZac<<8)|PA->Kanal) ) return ERAM;
	if( !ZapisW(&PI->PisBuf,(PA->Ton<<8)|0xff) ) return ERAM;
	PA->FadeOut=True;
	#if PRO_JAG
	if( PI->GenJag )
	{
		fputc(pNotaZac,PI->GenJag);
		fputc(PA->Kanal,PI->GenJag);
		fputc(PA->Ton,PI->GenJag);
		fputc(0xff,PI->GenJag);
	}
	#endif
	return EOK;
}

#endif

static Err VypKan( PreklInfo *PI, AlokKan *PA )
{
	if( PA->Ton>=0 ) /* hraje */
	{
		Err ret=NutnoSynchro(PI);if( ret<EOK ) return EOK;
		if( !PA->CekamLegato )
		{
			if( !ZapisW(&PI->PisBuf,(MNotaZac<<8)|PA->Kanal) ) return ERAM;
			if( !ZapisW(&PI->PisBuf,(PA->Ton<<8)|0) ) return ERAM;
			#if PRO_JAG
			if( PI->GenJag )
			{
				fputc(pNotaZac,PI->GenJag);
				fputc(PA->Kanal,PI->GenJag);
				fputc(PA->Ton,PI->GenJag);
				fputc(0,PI->GenJag);
			}
			#endif
			PA->Ton=-1;
		}
		VypustBCas(&PI->FrontaAK,&PA->B);
		PI->ObsKan--;
		if( PA->back ) *PA->back=NULL;
		PA->back=NULL;
	}
	return EOK;
}

static Err ZapKan( PreklInfo *PI, AlokKan *PA, int LogTon, int Vol, Flag Legato, CasT Cas )
{
	Err ret;
	while( LogTon<0 ) LogTon+=12;
	while( LogTon>127 ) LogTon-=12;
	Vol>>=1;
	if( Vol>127 ) Vol=127;
	if( Vol<1 ) Vol=1;
	ret=NutnoSynchro(PI);if( ret<EOK ) return EOK;
	if( PA->CekamLegato )
	{
		if( !ZapisW(&PI->PisBuf,(MNotaZac<<8)|PA->Kanal|0x40) ) return ERAM;
		#if PRO_JAG
		if( PI->GenJag )
		{
			fputc(pNotaZac,PI->GenJag);
			fputc(PA->Kanal+0x40,PI->GenJag);
		}
		#endif
	}
	else
	{
		if( !ZapisW(&PI->PisBuf,(MNotaZac<<8)|PA->Kanal) ) return ERAM;
		#if PRO_JAG
		if( PI->GenJag )
		{
			fputc(pNotaZac,PI->GenJag);
			fputc(PA->Kanal,PI->GenJag);
		}
		#endif
	}
	if( !ZapisW(&PI->PisBuf,(LogTon<<8)|Vol) ) return ERAM;
	#if PRO_JAG
	if( PI->GenJag )
	{
		fputc(LogTon,PI->GenJag);
		fputc(Vol,PI->GenJag);
	}
	#endif
	PI->NejakyTon=True;
	PA->Ton=LogTon;
	PA->FadeOut=False;
	if( Legato ) PA->CekamLegato=True;
	else PA->CekamLegato=False;
	if( !PA->Rytm )
	{
		ZaradBCas(&PI->FrontaAK,&PA->B,Cas);
		PI->ObsKan++;
		if( PI->MaxObsKan<PI->ObsKan ) PI->MaxObsKan=PI->ObsKan;
	}
	else
	{ /* Rytmika nazabira cas */
		/* nepocitame ji ani do kanalu - neobsazuje */
		if( !ZapisW(&PI->PisBuf,(MNotaZac<<8)|PA->Kanal) ) return ERAM;
		if( !ZapisW(&PI->PisBuf,(LogTon<<8)|0) ) return ERAM;
		#if PRO_JAG
		if( PI->GenJag )
		{
			fputc(pNotaZac,PI->GenJag);
			fputc(PA->Kanal,PI->GenJag);
			fputc(LogTon,PI->GenJag);
			fputc(0,PI->GenJag);
		}
		#endif
		PA->Ton=-1;
		if( PA->back ) *PA->back=NULL;
		PA->back=NULL;
	}
	return EOK;
}

static char SBuf[256];

static int AktDefHNastr( PreklInfo *PI, KanPrekl *K, const char *NN, int V, int Ton )
{
	int F=AktHraj->DefHNastr(NN,Ton,V,SekvenceS(&K->p)->Pis);
	if( F<EOK )
	{
		if( F==ERR )
		{
			sprintf(SBuf,FreeString(CENNASA),NN);
			ComChyba(&K->p,K->Kan,PI,SBuf);
			return ERR;
		}
		ef( F==EFIL )
		{
			ComChyba(&K->p,K->Kan,PI,FreeString(IOERRA));
			return ERR;
		}
		ef( F==KON )
		{
			if( !PI->IgnorujNDef && 1!=AlertRF(CENDNASA,1,NN) )
			{
				ComChyba(&K->p,K->Kan,PI,FreeString(IOERRA));
				return ECAN;
			}
			PI->IgnorujNDef=True;
			return -1; /* pauza nastroj! */
		}
	}
	return F;
}

static Err SetNastr( PreklInfo *PI, AlokKan *PA, KanPrekl *K, int V, int Ton )
{
	/* V je cislovani v pisni */
	/* musime nechat prehravac, at si ho ocisluje sam */
	const Pisen *P=SekvenceS(&PI->KP[0].p)->Pis;
	const char *NN;
	if( PA->LNastroj!=V )
	{
		Err ret;
		int F;
		Plati( V>=0 );
		NN=ZBanky(P->Nastroje,V);
		F=AktDefHNastr(PI,K,NN,V,Ton); /* V pomaha  hledat */
		if( F<0 ) return F;
		ret=NutnoSkoroSynchro(PI);if( ret<EOK ) return ret;
		if( !ZapisW(&PI->PisBuf,(MNastr<<8)|PA->Kanal) ) return ERAM;
		if( !ZapisW(&PI->PisBuf,F) ) return ERAM;
		#if PRO_JAG
		if( PI->GenJag )
		{
			fputc(pNastr,PI->GenJag);
			fputc(PA->Kanal,PI->GenJag);
			fputw(JagNas(NN),PI->GenJag);
		}
		#endif
		PA->LNastroj=V;
		PA->FNastroj=F;
	}
	return EOK;
}

static Err SetEfekt( PreklInfo *PI, AlokKan *PA, int EA, int EB )
{
	if( PA->EffA!=EA || PA->EffB!=EB )
	{
		Err ret;
		PA->EffA=EA,PA->EffB=EB;
		if( EA<0 ) EA=0;ef( EA>+255 ) EA=+255;
		if( EB<0 ) EB=0;ef( EB>+255 ) EB=+255;
		ret=NutnoSkoroSynchro(PI);if( ret<EOK ) return ret;
		if( !ZapisW(&PI->PisBuf,(MEfekt<<8)|PA->Kanal) ) return ERAM;
		if( !ZapisW(&PI->PisBuf,(EA<<8)|(byte)EB) ) return ERAM;
		#if PRO_JAG
		if( PI->GenJag )
		{
		}
		#endif
	}
	return EOK;
}
static Err SetStereo( PreklInfo *PI, AlokKan *PA, int VMin, int VMax )
{
	if( PA->StereoMin!=VMin || PA->StereoMax!=VMax )
	{
		Err ret;
		PA->StereoMin=VMin,PA->StereoMax=VMax;
		if( VMin<-127 ) VMin=-127;ef( VMin>+127 ) VMin=+127;
		if( VMax<-127 ) VMax=-127;ef( VMax>+127 ) VMax=+127;
		ret=NutnoSkoroSynchro(PI);if( ret<EOK ) return ret;
		if( !ZapisW(&PI->PisBuf,(MStereo<<8)|PA->Kanal) ) return ERAM;
		VMin=(byte)VMin;
		VMax=(byte)VMax;
		if( !ZapisW(&PI->PisBuf,(VMin<<8)|(byte)VMax) ) return ERAM;
		#if PRO_JAG
		if( PI->GenJag )
		{
			fputc(pStereo,PI->GenJag);
			fputc(PA->Kanal,PI->GenJag);
			fputw((VMin+VMax)/2,PI->GenJag);
		}
		#endif
	}
	return EOK;
}

#define DEDENI 1

static AlokKan *PridelKanal( PreklInfo *PI, KanPrekl *K, int Ton )
{
	Kontext *AK=AKontextS(&K->p);
	if( AK->Nastroj>=0 )
	{
		const Pisen *P=SekvenceS(&PI->KP[0].p)->Pis;
		const char *NN=ZBanky(P->Nastroje,AK->Nastroj);
		int F=AktDefHNastr(PI,K,NN,AK->Nastroj,Ton);
		AlokKan *PT,*PA,*PP;
		if( F<EOK )
		{
			if( F==-1 )
			{
				return NULL;
			}
			else return (void *)F;
		}
		PP=PT=NULL;
		#if DEDENI
			/* v prvnim pruchodu se snazime omezit prepinani nastroju a kanalu */
			for( PA=(AlokKan *)PI->FrontaAK.Volne; PA; PA=(AlokKan *)PA->B.Dalsi )
			{
				if( PA->LNastroj==AK->Nastroj ) PP=PA;
				if( PA->LKanal==K ) PT=PA;
			}
		#endif
		if( !PT ) PT=PP; /* spokoj se i se stejnym nastrojem */
		for( PA=PT ? PT : (AlokKan *)PI->FrontaAK.Volne; PA; PA=(AlokKan *)PA->B.Dalsi )
		{
			/* pri PT uspeje rovnou */
			if( !PA->CekamLegato || PA->LKanal==K )
			{ /* legato se smi pridedit jen na stejny kanal */
				Err ret;
				if( !PT || AK->Nastroj!=PA->LNastroj ) PA->CekamLegato=False;
				ret=SetNastr(PI,PA,K,AK->Nastroj,Ton);
				if( ret<EOK )
				{
					if( ret==-1 ) return NULL;
					Chyba(ret);
					return (void *)ret;
				}
				ret=SetStereo(PI,PA,AK->StereoMin,AK->StereoMax);if( ret<EOK ) {Chyba(ret);return (void *)ret;}
				ret=SetEfekt(PI,PA,AK->EffA,AK->EffB);if( ret<EOK ) {Chyba(ret);return (void *)ret;}
				return PA; /* uspesne prizazen novy kanal */
			}
		}
		PI->MaxObsKan=PI->NFyzKan+1; /* kanal se nenasel */
		return NULL;
	}
	return NULL; /* nepotrebuje kanal */
}
#endif

void MazPosuv( PosuvT Pos, SekvKursor *s )
{
	const static int PoradiK[7]={TF,TC,TG,TD,TA,TE,TH};
	const static int PoradiB[7]={TH,TE,TA,TD,TG,TC,TF};
	int i,T;
	for( i=0; i<7; i++ ) Pos[i]=TonN;
	T=s->A.Tonina;
	if( T==-128 ) T=0;
	if( T>0 )
	{
		for( i=0; i<T; i++ ) Pos[PoradiK[i]-TC]=TonK;
	}
	else if( T<0 )
	{
		T=-T;
		for( i=0; i<T; i++ ) Pos[PoradiB[i]-TC]=TonB;
	}
}

#if 0
#if FADE_OUT

#define CasFade (sec/200) /* 1/1000s je 25 vzorku DMA */

static long FadeOuty( PreklInfo *PI, long Cas )
{ /* proved ty FO, ktere se stihnou v case <=Cas */
	Err ret;
	Budik *B;
	long PBK=0;
	long CZap=0;
	for( B=PrvniBud(&PI->FrontaAK); B; B=DalsiBud(B) )
	{
		long C;
		AlokKan *AK=(AlokKan *)B;
		PBK+=B->ZaDobu;
		C=PBK-CasFade;
		Plati( AK->Ton>=0 );
		if( !AK->FadeOut && !AK->CekamLegato && C<=Cas && C>=0 )
		{
			ret=ZapisCekani(PI,C);if( ret<EOK ) return ret;
			PBK-=C;
			Cas-=C;
			CZap+=C;
			ret=FOutKan(PI,AK);if( ret<EOK ) return ret;
		}
	}
	return CZap;
}

#endif

static CasT DelkaZneni( Pisen *P, int Nas, int Ton )
{
	if( AktHraj->DelkaZneni ) return AktHraj->DelkaZneni(P,Nas,Ton);
	return MAXCAS;
}

static Err ZpracujKanal( KanPrekl *K, PreklInfo *PI, Flag PlneRep )
{
	Err ret;
	long Krok=K->B.ZaDobu;
	Budik *B;
	CasT Del=0;
	PI->DosCas+=Krok;
	{ /* simulace prechodu na nejblizsi udalost - NoteOffy, FadeOuty */
		long PBK=0;
		B=PrvniBud(&PI->FrontaAK);
		while( B )
		{ /* davno dohrane tony muzeme zrusit - prave dohrane si jeste nechame */
			PBK+=B->ZaDobu;
			if( PBK<=Krok )
			{ /* stihne dohrat - NoteOff */
				Budik *Nxt;
				#if FADE_OUT
				{
					long C=FadeOuty(PI,PBK);
					if( C<EOK ) return (Err)C;
					PBK-=C;
					Krok-=C;
				}
				#endif
				ret=ZapisCekani(PI,PBK);if( ret<EOK ) return ret;
				Krok-=PBK;
				PBK=0; /* B->ZaDobu se take srazi na nulu - VypKan */
				Nxt=DalsiBud(B);
				ret=VypKan(PI,(AlokKan *)B);if( ret<EOK ) return ret;
				B=Nxt;
			}
			else break; /* B->ZaDobu>=Krok */
		}
	}
	#if FADE_OUT
	{
		long C=FadeOuty(PI,Krok);
		if( C<EOK ) return (Err)C;
		Krok-=C;
		if( B ) B->ZaDobu-=C;
	}
	#endif
	ret=ZapisCekani(PI,Krok);if( ret<EOK ) return ret;
	B=PrvniBud(&PI->FrontaAK);
	if( B ) B->ZaDobu-=Krok;
	VypustBCas(&PI->FrontaKP,&K->B);
	if( CtiS(&K->p)==EOK )
	{
		CasT LDel;
		JednItem *B=BufS(&K->p);
		enum CPovel P=B[0];
		if( P<=PPau )
		{ /* tony a pauzy */
			Kontext *AK=AKontextS(&K->p);
			LDel=B[2]*(TempoMin/AK->Tempo);
			if( !(B[1]&Soubezna) ) Del=LDel;
			if( P<PPau )
			{ /* nota */
				const static int Pultony[7]={0,2,4,5,7,9,11};
				int T=P%Okt;
				int AO=AK->Oktava;
				int LogTon;
				if( AO<-3 ) AO=-3;
				ef( AO>+3 ) AO=+3;
				LogTon=Pultony[T]+12*(P/Okt+Okt0Ind+AO);
				switch( B[1]&KBOMaskA )
				{
					case KrizekA: K->Pos[T]=TonK;break;
					case BeckoA: K->Pos[T]=TonB;break;
					case OdrazA: K->Pos[T]=TonO;break;
				}
				switch( K->Pos[T] )
				{
					case TonK: LogTon++;break;
					case TonB: LogTon--;break;
					/* ostat. pripady - zadna posuvka */
				}
				{
					AlokKan *PAN=PridelKanal(PI,K,LogTon); /* muze precestovat */
					if( (long)PAN<0 ) return (Err)PAN;
					if( PAN )
					{
						PAN->back=NULL;
						For( AlokKan **, pa, K->FyzK, &K->FyzK[NAkTon], pa++ )
							if( !*pa )
							{
								PAN->back=pa;
								*pa=PAN;
								break;
							}
						Next
						PAN->LKanal=K;
						/* nØkter‚ n stroje nemaj¡ looping */
						/* nemus¡ se tedy hr t tak dlouho */
						if( !(B[1]&Leg) )
						{
							CasT Omez=DelkaZneni(PI->S->Pis,PAN->FNastroj,LogTon);
							if( LDel>Omez ) LDel=Omez;
						}
						ret=ZapKan(PI,PAN,LogTon,AK->Hlasit,B[1]&Leg,LDel);
						if( ret<EOK ) return ret;
					} /* je kanal */
				}
			} /* tony */
		} /* tony a pauzy */
		else switch( P )
		{
			case PTakt: case PDvojTakt:
			case PTonina: case PNTakt: case PTempo:
			case PZacSekv: case PKonSekv:
				MazPosuv(K->Pos,AktK(&K->p));
				break;
			case PRep: /* zpracovani repetic je dost slozite */
				MazPosuv(K->Pos,AktK(&K->p));
				if( K->RepUr>=MaxDynRep ) {ComChyba(&K->p,K->Kan,PI,FreeString(CEVNORA));return ERR;}
				K->Opak[K->RepUr++]=B[1];
				break;
			case PVyvolej:
				if( K->p.SUroven<MaxSekVnor-1 ) ComChyba(&K->p,K->Kan,PI,FreeString(NONEXPA));
				else ComChyba(&K->p,K->Kan,PI,FreeString(CEVNORA));
				return ERR;
			case PERep: /* zpracovani repetic je dost slozite */
				MazPosuv(K->Pos,AktK(&K->p));
				if( K->RepUr<=0 ) /* neprosli jsme zacatkem */
				{
					if( !PlneRep )
					{
						int O;
						if( K->RepUr>=MaxDynRep ) {ComChyba(&K->p,K->Kan,PI,FreeString(CEVNORA));return ERR;}
						ComChyba(&K->p,K->Kan,PI,FreeString(CEZREPA)); /* priprav se, ze nenajdes */
						ZpatkyS(&K->p);
						O=NajdiZacRepS(&K->p);
						if( O<EOK ) return ERR;
						/*else JCtiS(&K->p); /* preskoc zac. rep. */*/
						memcpy(&K->Opak[1],K->Opak,K->RepUr);
						K->RepUr++;
						K->Opak[0]=O-1;
						break;
					}
					else
					{
						ComChyba(&K->p,K->Kan,PI,FreeString(CEZREPA));
						return ERR;
					}
				}
				if( --K->Opak[K->RepUr-1]<=0 ) K->RepUr--;
				else
				{ /* vlastni vyvolani repetice */
					ComChyba(&K->p,K->Kan,PI,FreeString(CEZREPA)); /* priprav se, ze nenajdes */
					ZpatkyS(&K->p);
					if( NajdiZacRepS(&K->p)<EOK ) return ERR;
					/*else JCtiS(&K->p); /* preskoc zac. rep. */*/
				}
				break;
		}
		ZaradBCas(&PI->FrontaKP,&K->B,Del);
	}
	else /* CtiS!=EOK -> Konec kanalu */
	{
		if( K->RepUr>0 )
		{
			ComChyba(&K->p,K->Kan,PI,FreeString(CEKREPA));
			return ERR;
		}
	}
	return ret;
}

static Err UkonciTony( PreklInfo *PI )
{
	FrontaB *AK=&PI->FrontaAK;
	Err ret=EOK;
	while( PrvniBud(AK) && ret>=EOK)
	{
		ret=VypKan(PI,(AlokKan *)PrvniBud(AK));
	}
	return ret;
}

static void ZacPreklad( Sekvence *S, CasT CasOd, Kontext *K, PreklInfo *PI )
{
	int Kan;
	Sekvence *H;
	CasT MinCas=MAXCAS;
	Plati( K );
	for( Kan=0,H=S; H && Kan<MaxLogKan; Kan++,H=DalsiHlas(H) )
	{
		KanPrekl *KS=&PI->KP[Kan];
		CasT c;
		OtevriS(&KS->p,H,Cteni);
		SetKKontext(AktK(&KS->p),K);
		RezimS(&KS->p,True);
		NajdiSCasS(&KS->p,CasOd);
		c=CasJeS(&KS->p);
		NajdiCasS(&KS->p,c);
		if( TestS(&KS->p)==EOK ) if( c<MinCas ) MinCas=c;
		MazPosuv(KS->Pos,AktK(&KS->p));
		KS->Kan=Kan;
		For( AlokKan **, pa, KS->FyzK, &KS->FyzK[NAkTon], pa++ )
			*pa=NULL;
		Next
		KS->RepUr=0;
	}
	PI->NKan=Kan;
	Plati( PI->NKan>0 );
	PI->FrontaKP.Prvni=NULL;
	PI->FrontaKP.Volne=&PI->KP->B;
	PI->ZbyvaSynchro=0;
	PI->ObsKan=PI->MaxObsKan=0;
	for( Kan=0; Kan<PI->NKan-1; Kan++ )
	{
		KanPrekl *KS=&PI->KP[Kan];
		KS->B.Dalsi=&KS[1].B;
	}
	PI->KP[Kan].B.Dalsi=NULL;
	for( Kan=0; Kan<PI->NKan; Kan++ )
	{
		KanPrekl *KS=&PI->KP[Kan];
		ZaradBCas(&PI->FrontaKP,&KS->B,CasJeS(&KS->p)-MinCas);
	}
	PI->FrontaAK.Prvni=NULL;
	PI->FrontaAK.Volne=&PI->KA->B;
	for( Kan=0; Kan<PI->NFyzKan-1; Kan++ )
	{
		AlokKan *AK=&PI->KA[Kan];
		AK->B.Dalsi=&AK[1].B;
	}
	PI->KA[Kan].B.Dalsi=NULL;
	for( Kan=0; Kan<PI->NFyzKan; Kan++ )
	{
		AlokKan *AK=&PI->KA[Kan];
		AK->Kanal=Kan;
		AK->Ton=-1;
		AK->LNastroj=-1;
		AK->FNastroj=-1;
		AK->LKanal=NULL;
		AK->PBender=0;
		AK->StereoMin=AK->StereoMax=-0x7fff;
		AK->EffA=AK->EffB=-1; /* nen¡ nastaveno */
		AK->CekamLegato=False;
	}
	PI->DosCas=MinCas;
	PI->IgnorujNDef=False;
	PI->NejakyTon=False;
}

static void KonPreklad( PreklInfo *PI )
{
	int Kan;
	for( Kan=0; Kan<PI->NKan; Kan++ )
	{
		KanPrekl *KS=&PI->KP[Kan];
		ZavriS(&KS->p);
	}
	ZrusBud(&PI->FrontaKP);
	ZrusBud(&PI->FrontaAK);
}

static Err Preloz( Sekvence *S, CasT CasOd, Kontext *K, PreklInfo *PI )
{
	Err ret=EOK;
	ZacPreklad(S,CasOd,K,PI); /* inic. kanalu */
	/* prekladaci smycka */
	while( PrvniBud(&PI->FrontaKP) && ret>=EOK )
	{
		ret=ZpracujKanal((KanPrekl *)PrvniBud(&PI->FrontaKP),PI,CasOd<=0);
	} /* while( je povel ) */
	if( ret>=EOK ) ret=UkonciTony(PI);
	if( ret>=EOK )
	{
		ret=NutnoSynchro(PI);if( ret<EOK ) return ret;
		if( !ZapisW(&PI->PisBuf,(MKonec<<8)|0) ) ret=ERAM;
		if( !ZapisW(&PI->PisBuf,0) ) ret=ERAM;
		#if PRO_JAG
		if( PI->GenJag )
		{
			fputc(pKonec,PI->GenJag);
			fputc(0,PI->GenJag);
			fputc(0,PI->GenJag);
			fputc(0,PI->GenJag);
		}
		#endif
	}
	KonPreklad(PI); /* ruseni kanalu */
	return ret;
}

static PreklInfo PInfo;

static Flag ComLock=False;

static void ZrusComLock( void )
{
	ComLock=False;
}

static void ComDohraj( void ) 
{
	PustBuf(&PInfo.PisBuf);
	AktHraj->KonHNastr();
}

PathName CocPath;

Err CompoCom( Flag Hraj, Sekvence *S, CasT CasOd, Kontext *Kont, CompoErr *CE )
{
	CE->Ukazat=False;
	if( AktHraj )
	{
		Err p;
		if( ComLock ) return ECAN;
		ComLock=True;
		Mouse(HOURGLASS);
		PovolCinMenu(False);
		p=AktHraj->ZacHNastr(S->Pis);
		if( p>=OK )
		{
			PathName C;
			PInfo.CErr=CE;
			PInfo.S=S;
			InitBuf(&PInfo.PisBuf);
			PInfo.NFyzKan=AktHraj->NFyzKan;
			PInfo.GenJag=NULL;
			#if PRO_JAG
				if( !Hraj )
				{
					PathName CP,CN;
					FileName N;
					strcpy(C,S->Pis->Cesta);
					strcpy(N,NajdiNazev(C));
					strcpy(NajdiPExt(N),"");
					strcpy(NajdiPExt(NajdiNazev(C)),".BIN");
					VyrobCesty(CP,CN,C);
					if( !*CocPath ) strcpy(CocPath,CP);
					else strcpy(CP,CocPath);
					if( FSel(CocPath,CN,C) )
					{
						PInfo.GenJag=fopen(C,"wb");
						LoadNas(CocPath);
					}
					FAktHNastr();
				}
			#endif
			p=Preloz(S,CasOd,Kont,&PInfo);
			if( p>=OK && PInfo.NejakyTon )
			{
				AktHraj->AktHNastr();
				if( Hraj ) p=Zahraj(NazevSekvPis(S),PInfo.PisBuf.Buf,PInfo.DosCas,CasOd,PInfo.MaxObsKan,S->Pis);
				else
				{
					#if !PRO_JAG
					PathName CP,CN;
					/* pisen je v PInfo.PisBuf */
					/* tabulku nastroju si drzi AktHraj */
					strcpy(C,S->Pis->Cesta);
					strcpy(NajdiExt(NajdiNazev(C)),"COC");
					VyrobCesty(CP,CN,C);
					if( !*CocPath ) strcpy(CocPath,CP);
					else strcpy(CP,CocPath);
					if( FSel(CocPath,CN,C) )
					{
						FILE *f=fopen(C,"wb");
						Err ret=EFIL;
						if( f )
						{
							long c;
							if( setvbuf(f,NULL,_IOFBF,9*1024)<0 ) goto Error;
							c=PInfo.PisBuf.Ptr;
							if( !AktHraj->SaveHNastr || AktHraj->SaveHNastr(f,S->Pis,C)<EOK ) goto Error;
							if( fwrite(&c,sizeof(c),1,f)!=1 ) goto Error;
							if( fwrite(PInfo.PisBuf.Buf,PInfo.PisBuf.Ptr,1,f)!=1 ) goto Error;
							ret=EOK;
							Error:
							if( fclose(f)<0 ) ret=EFIL;
						}
						if( ret<EOK ) AlertRF(WERRA,1,NajdiNazev(C)),p=ECAN;
					}
					#else
						if( PInfo.GenJag )
						{			
							if( !AktHraj->SaveHNastr || AktHraj->SaveHNastr(PInfo.GenJag,S->Pis,C)<EOK ) {}
							fclose(PInfo.GenJag);
						}
					#endif
				}
			}
			AzDohraje(ComDohraj);
		}
		AzDohraje(ZrusComLock);
		PovolCinMenu(True);
		return p;
	}
	else return OK;
}


#endif