/* Componium - bloky ... */
/* SUMA, 10/1992-3/1994 */

#include "../stdafx.h"
#include "macros.h"
#include <string.h>

#include "ramcompo.h"
#include "compo.h"
#include "../ComponiumView.h"

/* bloky v okne */

static void BChyba( Err err )
{
	if( err==ERR || err==ECAN || err==KON ) {}
	else Chyba(err);
}

static void ZavriZBloku( OknoInfo *OI )
{
	if( OI->ZBStopa>=0 ) ZavriS(&OI->ZBlok),OI->ZBStopa=-1;
}
static void ZavriKBloku( OknoInfo *OI )
{
	if( OI->KBStopa>=0 ) ZavriS(&OI->KBlok),OI->KBStopa=-1;
}
static void OtevriZBloku( OknoInfo *OI )
{
	OI->ZBStopa=OI->KKan;
	RozdvojS(&OI->ZBlok,&OI->Kurs);
}
static void OtevriKBloku( OknoInfo *OI )
{
	OI->KBStopa=OI->KKan;
	RozdvojS(&OI->KBlok,&OI->Kurs);
	JCtiS(&OI->KBlok);
}

static void KorektBlok( OknoInfo *OI, Flag Silny )
{
	if( OI->ZBStopa==OI->KBStopa )
	{
		StrPosT P,p;
		KdeJeS(&OI->ZBlok,&P);
		KdeJeS(&OI->KBlok,&p);
		if( CmpPos(&P,&p)>=0 ) ZavriZBloku(OI),ZavriKBloku(OI);
	}
	else
	{
		long dif=CasJeS(&OI->ZBlok)-CasJeS(&OI->KBlok);
		if( dif>0 || Silny && dif>=0 ) ZavriZBloku(OI),ZavriKBloku(OI);
	}
}
static void PodmZBloku( OknoInfo *OI )
{
	OI->ZBStopa=OI->KKan;
	RozdvojS(&OI->ZBlok,&OI->Kurs);
}

#if 0
void ZacBlok( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	ZavriZBloku(OI),OtevriZBloku(OI);
	ZavriKBloku(OI),OtevriKBloku(OI);
	KorektBlok(OI,False);
}
void KonBlok( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	if( OI->ZBStopa>=0 )
	{
		ZavriKBloku(OI),OtevriKBloku(OI);
		KorektBlok(OI,True);
	}
}
void ProzatimKonBlok( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	if( OI->ZBStopa>=0 )
	{
		ZavriKBloku(OI),OtevriKBloku(OI);
		KorektBlok(OI,False);
		if( OI->ZBStopa<0 ) ZacBlok(LHan);
		Kresli(LHan);
	}
}
void ZrusBlok( int LHan )
{
	OknoInfo *OI=OInfa[LHan];
	ZavriZBloku(OI),ZavriKBloku(OI);
}


/* bloky - logika */
/* vsude v blocich pozor na hierarchii !!! */

void MazPBlok( void )
{
	if( PisenBlok ) MazPisen(PisenBlok),PisenBlok=NULL;
}
#endif

static Flag InInterval( int K, int D, int H )
{
	return ( K>=D && K<=H ) || ( K>=H && K<=D );
}

bool CComponiumView::JeVBloku(int Kan, const StrPosT *pos, CasT cas)
{
	if( OI.ZBStopa>=0 && InInterval(Kan,OI.ZBStopa,OI.KBStopa) )
	{
		Plati(OI.KBStopa>=0);
		if( OI.ZBStopa==OI.KBStopa )
		{
			StrPosT P,p;
			KdeJeS(&OI.ZBlok,&P);
			KdeJeS(&OI.KBlok,&p);
			return CmpPos(pos,&P)>=0 && CmpPos(pos,&p)<0;
		}
		else
		{
			return cas>=CasJeS(&OI.ZBlok) && cas<CasJeS(&OI.KBlok);
		}
	}
	return False;
}

#if 0
Flag TaktBlok( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		SekvSoubor *s=&OI->ZBlok;
		CasT VT;
		ZavriZBloku(OI),ZavriKBloku(OI);
		OtevriZBloku(OI),OtevriKBloku(OI);
		VT=AktK(s)->A.VTaktu;
		while( VT>=AktK(s)->A.Takt ) VT-=AktK(s)->A.Takt;
		VT*=(TempoMin/AktK(s)->A.Tempo);
		NajdiCasS(s,CasJeS(s)-VT);
		while( TestS(s)==OK )
		{
			enum CPovel P=BufS(s)[0];
			if( P!=PTonina && P!=PNTakt && P!=PTempo && P!=PRep && P!=PERep ) break;
			else JCtiS(s);
		}
		s=&OI->KBlok;
		while( CtiS(s)==OK && BufS(s)[0]!=PTakt );
		KorektBlok(OI,True);
		return OI->ZBStopa>=0;
	}
	return False;
}
Flag JednotkaBlok( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		ZavriZBloku(OI),ZavriKBloku(OI);
		OtevriZBloku(OI),OtevriKBloku(OI);
		KorektBlok(OI,True);
		return OI->ZBStopa>=0;
	}
	return False;
}
Flag VsechnoBlok( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		ZavriZBloku(OI),ZavriKBloku(OI);
		OI->ZBStopa=0;
		RozdvojS(&OI->ZBlok,&OI->Soub[0]);
		NajdiCasS(&OI->ZBlok,0);
		OI->KBStopa=OI->NSoub-1;
		RozdvojS(&OI->KBlok,&OI->Soub[OI->KBStopa]);
		NajdiSCasS(&OI->KBlok,MAXCAS);
		KorektBlok(OI,True);
		return OI->ZBStopa>=0;
	}
	return False;
}

Flag VlozBlok( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		Flag RFlag=True;
		if( PisenBlok )
		{
			SekvSoubor b,c;
			Sekvence *S=NajdiSekv(PisenBlok,MainMel,0);
			Flag Multi=DalsiHlas(S)!=NULL;
			if( !Multi )
			{ /* jednst. rezim */
				OtevriS(&c,S,Cteni);
				RZapisS(&b,&OI->Kurs);
				ZacBlokZmenS(&b);
				while( TestS(&c)==EOK )
				{
					Err ret=KopyS(&b,&c);
					if( ret!=OK ) {BChyba(ret);RFlag=False;break;}
				}
				NormujK(AktK(&b));
				ZavriS(&b);
				ZavriS(&c);
				RZapisS(&b,&OI->Kurs);
				NormujK(AktK(&b));
				KonBlokZmenS(&b);
				ZavriS(&b);
			}
			else
			{
				int Kan=OI->KKan;
				Sekvence *H;
				CasT ZCas=CasJeS(&OI->Kurs);
				for( H=S; H && Kan<OI->NSoub && RFlag; H=DalsiHlas(H),Kan++ )
				{
					OtevriS(&c,H,Cteni);
					RZapisS(&b,&OI->Soub[Kan]);
					ZacBlokZmenS(&b);
					NajdiSCasS(&b,ZCas);
					while( TestS(&c)==EOK )
					{
						Err ret=KopyS(&b,&c);
						if( ret!=OK ) {BChyba(ret);RFlag=False;break;}
					}
					NormujK(AktK(&b));
					ZavriS(&b);
					ZavriS(&c);
					RZapisS(&b,&OI->Soub[Kan]);
					NajdiCasS(&b,ZCas);
					NormujK(AktK(&b));
					KonBlokZmenS(&b);
					ZavriS(&b);
				} /* for( H ) */
			}
			return RFlag;
		}
	}
	return False;
}
Flag ZkopirujBlok( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		Flag RFlag=True;
		KorektBlok(OI,True);
		if( OI->ZBStopa>=0 )
		{
			Pisen *NB=NovaPisen("$Blok$");
			if( !NB ) {Chyba(ERAM);return False;}
			MazPBlok();
			PisenBlok=NB;
			if( OI->ZBStopa==OI->KBStopa )
			{ /* jednst. rezim */
				SekvSoubor b,c;
				StrPosT kpos,apos;
				Sekvence *S=NovaSekv(NB,MainMel);
				if( !S ) {Chyba(ERAM);return False;}
				KdeJeS(&OI->KBlok,&kpos);
				OtevriS(&c,S,Zapis);
				ZacBlokZmenS(&c);
				RozdvojS(&b,&OI->ZBlok);
				while( KdeJeS(&b,&apos),CmpPos(&apos,&kpos)<0 )
				{
					Err ret=KopyS(&c,&b);
					if( ret!=OK ) {BChyba(ret);RFlag=False;break;}
				}
				ZavriS(&b);
				KonBlokZmenS(&c);
				ZavriS(&c);
			}
			else
			{
				int Kan;
				int ZK,KK;
				CasT ZCas=CasJeS(&OI->ZBlok);
				CasT KCas=CasJeS(&OI->KBlok);
				if( OI->ZBStopa>OI->KBStopa ) ZK=OI->KBStopa,KK=OI->ZBStopa;
				else ZK=OI->ZBStopa,KK=OI->KBStopa;
				for( Kan=ZK; Kan<=KK && RFlag; Kan++ )
				{
					SekvSoubor b,c;
					Sekvence *H=NajdiSekv(NB,MainMel,0);
					if( H )
					{
						Sekvence *NH;
						while( (NH=DalsiHlas(H))!=NULL ) H=NH;
						H=NovyHlas(H);
					}
					else H=NovaSekv(NB,MainMel);
					OtevriS(&c,H,Zapis);
					ZacBlokZmenS(&c);
					RozdvojS(&b,&OI->Soub[Kan]);
					NajdiCasS(&b,ZCas);
					while( CasJeS(&b)<KCas )
					{
						Err ret=KopyS(&c,&b);
						if( ret!=OK ) {BChyba(ret);RFlag=False;break;}
					}
					ZavriS(&b);
					KonBlokZmenS(&c);
					ZavriS(&c);
				} /* for(Kan) */
			}
			if( !RFlag ) MazPBlok();
			return RFlag;
		}
	}
	return False;
}

Flag SmazBlok( void )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		Flag RFlag=True;
		KorektBlok(OI,True);
		if( OI->ZBStopa>=0 )
		{
			if( OI->ZBStopa==OI->KBStopa )
			{ /* jednst. rezim */
				SekvSoubor b,e;
				PosT Cnt;
				StrPosT ap,ep;
				RZapisS(&b,&OI->ZBlok);
				ZacBlokZmenS(&b);
				RozdvojS(&e,&b);
				Cnt=0;
				KdeJeS(&OI->KBlok,&ep);
				while( KdeJeS(&e,&ap),CmpPos(&ap,&ep)<0 ) JCtiS(&e),Cnt++;
				ZavriS(&e);
				while( Cnt-->0 )
				{
					Err ret;
					TestS(&b);
					ret=MazS(&b);
					if( ret!=OK )
					{
						if( ret==ERR ) {AlertRF(NPOVA,1);RFlag=False;break;}
						ef( ret!=KON ) {BChyba(ret);RFlag=False;break;}
					}
				}
				KonBlokZmenS(&b);
				ZavriS(&b);
			}
			else
			{
				int Kan;
				int ZK,KK;
				CasT ZCas=CasJeS(&OI->ZBlok);
				CasT KCas=CasJeS(&OI->KBlok);
				if( OI->ZBStopa>OI->KBStopa ) ZK=OI->KBStopa,KK=OI->ZBStopa;
				else ZK=OI->ZBStopa,KK=OI->KBStopa;
				for( Kan=ZK; Kan<=KK && RFlag; Kan++ )
				{
					PosT Cnt;
					SekvSoubor b,e;
					RozdvojS(&e,&OI->Soub[Kan]);
					NajdiCasS(&e,ZCas);
					RZapisS(&b,&e);
					ZacBlokZmenS(&b);
					Cnt=0;
					while( CasJeS(&e)<KCas ) JCtiS(&e),Cnt++;
					ZavriS(&e);
					while( Cnt-->0 )
					{
						Err ret;
						TestS(&b);
						ret=MazS(&b);
						if( ret!=OK )
						{
							if( ret==ERR ) {AlertRF(NPOVA,1);RFlag=False;break;}
							ef( ret!=KON ) {BChyba(ret);RFlag=False;break;}
						}
					}
					KonBlokZmenS(&b);
					ZavriS(&b);
				} /* for(Kan) */
			}
			if( !RFlag ) MazPBlok();
			ZrusBlok(LHan);
			return RFlag;
		}
	}
	return False;
}

/* operace na bloku */

Flag PosunBlok( int Pos )
{
	int LHan=VrchniSekv();
	Flag Posun=False;
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		KorektBlok(OI,True);
		if( OI->ZBStopa>=0 )
		{
			if( OI->ZBStopa==OI->KBStopa )
			{ /* jednst. rezim */
				SekvSoubor b;
				StrPosT ap,ep;
				RZapisS(&b,&OI->ZBlok);
				ZacBlokZmenS(&b);
				Posun=True;
				KdeJeS(&OI->KBlok,&ep);
				while( KdeJeS(&b,&ap),CmpPos(&ap,&ep)<0 )
				{
					enum CPovel p;
					if( TestS(&b)!=EOK ) break;
					p=BufS(&b)[0];
					if( p<PPau )
					{
						enum CPovel np=p+Pos;
						while( np>=PPau ) np-=Okt;
						while( np<0 ) np+=Okt;
						BufS(&b)[0]=np;
						PlatiProc( PisS(&b), ==EOK );
					}
					else
					{
						PlatiProc( JCtiS(&b), ==EOK );
					}
				}
				ZavriS(&b);
				KonBlokZmenS(&b);
			}
			else
			{
				int Kan;
				int ZK,KK;
				CasT ZCas=CasJeS(&OI->ZBlok);
				CasT KCas=CasJeS(&OI->KBlok);
				if( OI->ZBStopa>OI->KBStopa ) ZK=OI->KBStopa,KK=OI->ZBStopa;
				else ZK=OI->ZBStopa,KK=OI->KBStopa;
				for( Kan=ZK; Kan<=KK; Kan++ )
				{
					SekvSoubor b;
					RZapisS(&b,&OI->Soub[Kan]);
					ZacBlokZmenS(&b);
					Posun=True;
					NajdiSCasS(&b,ZCas);
					while( CasJeS(&b)<KCas )
					{
						enum CPovel p;
						if( TestS(&b)!=EOK ) break;
						p=BufS(&b)[0];
						if( p<PPau )
						{
							enum CPovel np=p+Pos;
							while( np>=PPau ) np-=Okt;
							while( np<0 ) np+=Okt;
							BufS(&b)[0]=np;
							PlatiProc( PisS(&b), ==EOK );
						}
						else
						{
							PlatiProc( JCtiS(&b), ==EOK );
						}
					}
					KonBlokZmenS(&b);
					ZavriS(&b);
				} /* for(Kan) */
			}
		}
	}
	return Posun;
}

#endif
/* ----- */

#define JePrep(P) ( (P)>=PrepMin && (P)<PrepMax )

static void NormalizaceH( Sekvence *H, Kontext *Kont )
{
	SekvKursor h;
	Flag NutnoRadit=False;
	int ND;
	Flag NS=False;
	JednItem *B=BufK(&h);
	ZacBlokZmen(H);
	H->Uzito=True;
	{
		OtevriK(&h,H,Cteni);
		while( JCtiK(&h)==OK )
		{
			if( B[0]<=PPau )
			{
				if( !NS )
				{
					if( B[1]&Soubezna ) NS=True;
					ND=B[2];
				}
				else
				{
					if( B[1]&Soubezna )
					{
						if( B[2]<ND ) ND=B[2];
						else if( B[2]>ND ) NutnoRadit=True;
					}
					else
					{
						if( B[2]>ND ) NutnoRadit=True;
						NS=False;
					}
				}
			}
			/* test serazeni soub. not */
		}
		ZavriK(&h);
	}
	if( NutnoRadit )
	{
		OtevriK(&h,H,Zapis);
		while( TestK(&h)==OK )
		{
			if( BufK(&h)[0]<=PPau && (BufK(&h)[1]&Soubezna) )
			{
				while( JCtiK(&h)==OK && BufK(&h)[0]<=PPau && (BufK(&h)[1]&Soubezna) );
				TNormujK(&h,NULL);
			}
			else JCtiK(&h);
		}
		ZavriK(&h);
	}
	{ /* vypusœ zdvojen‚ noty */
		int PT=-1,PD=-1,PA=0;
		OtevriK(&h,H,Zapis);
		while( TestK(&h)==OK )
		{
			if( BufK(&h)[0]<=PPau )
			{
				if( BufK(&h)[0]==PT && BufK(&h)[2]==PD && (BufK(&h)[1]&~Soubezna)==PA )
				{ /* opakuje se nota */
					for(;;)
					{
						ZpatkyK(&h);
						TestK(&h);
						if( BufK(&h)[0]==PT && BufK(&h)[2]==PD && (BufK(&h)[1]&~Soubezna)==PA ) break;
					}
					MazK(&h); /* vyma§ tu soubØ§nou */
				}
				if( BufK(&h)[1]&Soubezna )
				{
					if( BufK(&h)[0]==PPau ) MazK(&h);
					else JCtiK(&h);
					PT=BufK(&h)[0],PD=BufK(&h)[2],PA=(BufK(&h)[1]&~Soubezna);
				}
				else PT=-1,JCtiK(&h);
			}
			else JCtiK(&h);
		}
		ZavriK(&h);
	}
	{
		int Tonin,TTemp,TTakt;
		int Nas,Hlas,Oktav;
		int SterMin,SterMax;
		PosuvT Posuv;
		NutnoRadit=False;
		OtevriK(&h,H,Zapis);
		MazPosuv(Posuv,&h);
		if( strcmp(NazevSekv(H),MainMel) && Kont )
		{
			SetKKontext(&h,Kont);
			Tonin=h.A.Tonina,TTemp=h.A.Tempo,TTakt=h.A.Takt;
			Nas=h.A.Nastroj,Hlas=h.A.Hlasit,Oktav=h.A.Oktava;
			SterMin=h.A.StereoMin,SterMax=h.A.StereoMax;
		}
		else
		{
			Tonin=TTemp=TTakt=-128;
			Nas=Hlas=-1;
			Oktav=0;
			SterMin=SterMax=0x7fff;
		}
		for(;;) /* test serazeni povelu, optimalizace #,b,= */
		{
			enum CPovel Pov=CPovel(0);
			enum CPovel Druh;
			while( CtiK(&h)==OK ) /* nutno zpracovat takty! */
			{
				Flag Maz=False;
				Druh=CPovel(BufK(&h)[0]);
				if( !JePrep(Druh) )
				{
					if( Druh==PVyvolej )
					{
						Sekvence *R=NajdiSekvBI(H->Pis,BufK(&h)[1],BufK(&h)[2]);
						if( R )
						{
							ZpatkyK(&h); /* korektni kontext */
							NormalizaceH(R,&h.A);
							JCtiK(&h);
						}
					}
					if( Druh!=PTakt ) break;
				}
				else
				{
					if( PovelPred(Druh,Pov)<0 ) NutnoRadit=True;
					Pov=Druh;
				}
				switch( Druh )
				{
					case PHlasit: Maz=BufK(&h)[1]==Hlas;Hlas=BufK(&h)[1];break;
					case PNastroj: Maz=BufK(&h)[1]==Nas;Nas=BufK(&h)[1];break;
					case POktava:
						if( BufK(&h)[2] ) Maz=BufK(&h)[1]==0,Oktav+=BufK(&h)[1];
						else Maz=BufK(&h)[1]==Oktav,Oktav=BufK(&h)[1];
						break;
					case PStereo: Maz=BufK(&h)[1]==SterMin && BufK(&h)[2]==SterMax;SterMin=BufK(&h)[1],SterMax=BufK(&h)[2];break;
					case PTonina: Maz=h.A.Tonina==Tonin;Tonin=h.A.Tonina;MazPosuv(Posuv,&h);break;
					case PTempo: Maz=h.A.Tempo==TTemp;TTemp=h.A.Tempo;MazPosuv(Posuv,&h);break;
					case PNTakt: Maz=h.A.Takt==TTakt;TTakt=h.A.Takt;MazPosuv(Posuv,&h);break;
					case PTakt: case PDvojTakt: MazPosuv(Posuv,&h);break;
				}
				if( Maz ) ZpatkyK(&h),MazTonF(&h);
			} /* while( ) */
			if( StatusK(&h)!=OK ) break;
			else
			{ /* skoncili prepinaci povely */
				Flag Pis=False;
				if( Druh<PPau ) /* opt. znacek */
				{
					int Ton=(Druh-TC)%Okt;
					PI *P=&Posuv[Ton];
					PI p;
					int A=BufK(&h)[1]&KBOMaskA;
					switch( A )
					{
						case KrizekA: p=TonK; break;
						case BeckoA:  p=TonB; break;
						case OdrazA:  p=TonO; break;
						default: p=TonN;break;
					}
					if( p!=TonN )
					{
						if( p==*P || p==TonO && *P==TonN ) Pis=True;
						*P=p;
					}
				}
				if( Pis )
				{
					JednotkaBuf B;
					EquJed(B,BufK(&h));
					ZpatkyK(&h);
					EquJed(BufK(&h),B);BufK(&h)[1]&=~KBOMaskA;
					PisK(&h);
				}
			}
		} /* for(;;) */
		ZavriK(&h);
	}
	if( NutnoRadit )
	{
		OtevriK(&h,H,Zapis);
		while( TestK(&h)==OK )
		{
			if( JePrep(BufK(&h)[0]) )
			{
				do
				{
					JCtiK(&h);
				}
				while( TestK(&h)==OK && JePrep(BufK(&h)[0]) );
				NormujK(&h);
			}
			else JCtiK(&h);
		}
		ZavriK(&h);
	} /* if( NutnoRadit ) */
	KonBlokZmen(H);
}

void NormalizaceF( Pisen *P, Sekvence *M, Kontext *MK )
{
	Sekvence *H,*S;
	/* v prizaku Uzito evidujeme pouziti sekvence */
	/* pri normalizaci hlavni sekv. vypustime ty nepouzite */
	for( S=PrvniSekv(P); S; S=DalsiSekv(S) ) for( H=S; H; H=DalsiHlas(H) )
	{
		H->Uzito=False;
	}
	for( H=M; H; H=DalsiHlas(H) ) NormalizaceH(H,MK);
	if( !strcmp(NazevSekv(M),MainMel) )
	{ /* optim. hlavni part */
		Sekvence *N;
		Flag MazatG=False,Mazat;
		for( S=PrvniSekv(P); S; )
		{
			Mazat=True;
			N=DalsiSekv(S);
			for( H=S; H; H=DalsiHlas(H) ) if( H->Uzito ) Mazat=False;
			if( Mazat ) MazatG=True;
			S=N;
		}
		if( MazatG /*&& AlertRF(DNEUZA,2)==1*/ )
		{
			Sekvence *N;
			for( S=PrvniSekv(P); S; )
			{
				Mazat=True;
				N=DalsiSekv(S);
				for( H=S; H; H=DalsiHlas(H) ) if( H->Uzito ) Mazat=False;
				if( Mazat ) MazSekv(S);
				S=N;
			}
		}
	}
}
