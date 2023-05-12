/* Componium - zachazeni s vnitrnim formatem */
/* SUMA, 9/1992-1/1994 */

#include "macros.h"
#include <string.h>
#include "stdc\fileutil.h"
#include "utlcompo.h"

/* manipulace se sekvenci - (Blok - Sekvence - Pisen) */

void EquJed( JednItem *d, const JednItem *z )
{
	#define D ((long *)d)
	#define Z ((long *)z)
	D[0]=Z[0];
	D[1]=Z[1];
	#undef D
	#undef Z
}

/* macro - casto zrychli */
#define _LI(P,o) ( ((long *)(P))[o] )
#define _EquJed(d,z) { _LI(d,0)=_LI(z,0); _LI(d,1)=_LI(z,1); }

/* poroz - pred. sizeof(JednItem)==2 */

Flag CmpJed( const JednItem *d, const JednItem *z )
{
	#define D ((long *)d)
	#define Z ((long *)z)
	return D[0]!=Z[0] || D[1]!=Z[1];
	#undef D
	#undef Z
}

void (*ZmenaSekv)( Sekvence *S, Flag Krit );

static void OznacZmenu( Sekvence *S, Flag Krit )
{
	S->Pis->Zmena=True;
	if( ZmenaSekv && S->CntBlokZmen<=0 ) ZmenaSekv(S,Krit);
	else
	{
		S->Zmeny=True;
		if( Krit ) S->KritZmeny=True;
	}
}

Blok *NovyBlok( Blok *za )
{
	Blok *B=new(Blok);
	if( !B ) return B;
	B->prev=za;B->next=za->next;
	za->next->prev=B;
	za->next=B;
	B->Delka=0;
	return B;
}
void MazBlok( Blok *B )
{
	Blok *prev=B->prev,*next=B->next;
	myfree(B);
	prev->next=next;
	next->prev=prev;
}

static Sekvence *InitSekv( Pisen *P, const char *Nazev )
{
	Sekvence *S=new(Sekvence);
	if( !S ) return S;
	S->Zac.prev=NULL;S->Zac.next=(Blok *)&S->Kon;
	S->Kon.prev=(Blok *)&S->Zac;S->Kon.next=NULL;
	S->Cteni=S->Psani=0;
	S->Kon.Delka=S->Zac.Delka=0;
	S->_GlobDel=0;
	S->_CasDelka=0;
	S->_CasPriTempu=-1;
	S->Zmeny=False;
	S->CntBlokZmen=0;
	S->Pis=P;
	S->INazev=PridejRetez(P->NazvySekv,Nazev);
	if( S->INazev<0 ) {myfree(S);return NULL;}
	S->Soubory=NULL;
	return S;
}

static Flag IfOdstran( SekvKursor *s )
{
	switch( s->Buf[0] )
	{
		case PNastroj: return True;
		case PVyvolej: return True;
	}
	return False;
}
Flag OdstranK( SekvKursor *s )
{
	switch( s->Buf[0] )
	{
		case PNastroj: ZrusRetez(s->S->Pis->Nastroje,s->Buf[1]);return True;
		case PVyvolej:
			ZavriVolajiciZ(s);
			ZrusRetez(s->S->Pis->NazvySekv,s->Buf[1]);
			return True;
	}
	return False;
}

static void ZrusNastroje( Sekvence *S )
{
	SekvKursor s;
	Err ret;
	OtevriK(&s,S,Zapis);
	for(;;)
	{
		ret=TestK(&s);
		if( ret!=OK ) break;
		if( IfOdstran(&s) ) MazK(&s);
		else JCtiK(&s);
	}
	Plati(ret==KON);
	ZavriK(&s);
}

void ZacBlokZmen( Sekvence *S )
{
	S->CntBlokZmen++;
	if( S->CntBlokZmen==1 ) S->Zmeny=S->KritZmeny=False; /* inic. system */
}

void KonBlokZmen( Sekvence *S )
{
	S->CntBlokZmen--;
	if( S->CntBlokZmen==0 )
	{
		if( S->Zmeny )
		{
			OznacZmenu(S,S->KritZmeny);
			S->Zmeny=S->KritZmeny=False;
		}
	}
}

static void MazSekvF( Sekvence *S )
{
	Blok *B,*b;
	ZacBlokZmen(S);
	ZrusNastroje(S);
	for( B=S->Zac.next; B!=(Blok *)&S->Kon; B=b ) b=B->next,myfree(B);
	OznacZmenu(S,True);
	ZrusRetez(S->Pis->NazvySekv,S->INazev);
	ZavriVolajici(S);
	Plati( S->Cteni==0 );
	Plati( S->Psani==0 );
	Plati( !S->Soubory );
	KonBlokZmen(S);
	myfree(S);
}

Sekvence *NovaSekv( Pisen *P, const char *Nazev )
{
	Sekvence *S=InitSekv(P,Nazev),*L;
	if( !S ) return S;
	for( L=&P->Zac; L->next; L=L->next );
	L->next=S;
	S->prev=L;
	S->next=NULL;
	S->predch=NULL;
	S->dalsi=NULL;
	P->SekPtr[S->INazev]=S;
	OznacZmenu(S,True);
	return S;
}
void MazSekv( Sekvence *S )
{
	Sekvence *prev=S->prev,*next=S->next,*H;
	prev->next=next;
	if( next ) next->prev=prev;
	for( H=S->dalsi; H; H=H->dalsi ) MazHlas(H);
	S->Pis->SekPtr[S->INazev]=NULL;
	MazSekvF(S);
}

const char *NazevSekv( const Sekvence *S )
{
	return ZBanky(S->Pis->NazvySekv,S->INazev);
}
Err ZmenNazevSekv( const Sekvence *S, const char *Nazev )
{
	return ZmenRetez(S->Pis->NazvySekv,S->INazev,Nazev);
}
const char *NazevSekvPis( const Sekvence *S )
{
	static char Buf[80];
	const char *NS=NazevSekv(S);
	strcpy(Buf,NajdiNazev(S->Pis->NazevS));
	if( strcmp(NS,MainMel) )
	{
		strcat(Buf,"-");
		strcat(Buf,NS);
	}
	return Buf;
}
const char *NazevSoubPis( const Sekvence *S )
{
	static char Buf[80];
	const char *NS=NazevSekv(S);
	strcpy(Buf,S->Pis->NazevS);
	if( strcmp(NS,MainMel) )
	{
		strcat(Buf,"-");
		strcat(Buf,NS);
	}
	return Buf;
}

Sekvence *NovyHlas( Sekvence *O )
{
	Sekvence *S=InitSekv(O->Pis,NazevSekv(O));
	if( !S ) return S;
	S->predch=O;S->dalsi=O->dalsi;
	O->dalsi=S;
	if( S->dalsi ) S->dalsi->predch=S;
	S->next=NULL;
	S->prev=NULL;
	return S;
}
void MazHlas( Sekvence *S )
{
	Sekvence *L=S->dalsi,*P=S->predch;
	if( !P )
	{
		Plati(L);
		Plati(S->prev);
		L->prev=S->prev;
		L->next=S->next;
		S->prev->next=L;
		if( S->next ) S->next->prev=L;
		L->predch=NULL;
		L->Pis->SekPtr[L->INazev]=L; /* zrychl. seznam */
		MazSekvF(S);
	}
	else
	{
		P->dalsi=L;
		if( L ) L->predch=P;
		MazSekvF(S);
	}
}

Sekvence *DalsiHlas( Sekvence *S )
{
	return S->dalsi;
}

void ProhHlas( Sekvence *H, Sekvence *D )
{ /* H-D - puvodni vyznam -> udela z nej obracene (H - dolni ...) */
	Sekvence *HP=H->prev,*HN=H->next;
	Sekvence *P=H->predch,*Z=D->dalsi;
	Plati( H->dalsi==D && D->predch==H );
	D->predch=P;if( P ) P->dalsi=D;
	D->dalsi=H;H->predch=D;
	H->dalsi=Z;if( Z ) Z->predch=H;
	if( !P ) /* H byl hlavni (prvni) hlas */
	{
		D->prev=HP;if( HP ) HP->next=D; /* H mohl byt hlavni hlas, musi sedet provazani */
		D->next=HN;if( HN ) HN->prev=D;
		H->prev=H->next=NULL;
		D->Pis->SekPtr[D->INazev]=D; /* oprav zrychl. tabulku */
	}
	OznacZmenu(D,True); /* radeji v potenc. hlavnim hlase */
}

Sekvence *NajdiSekv( const Pisen *P, const char *Nazev, int Hlas )
{
	Sekvence *L;
	/* pokus pomoci pom. pointeru */
	for( L=P->Zac.next; L; L=L->next )
	{
		if( !strcmp(Nazev,NazevSekv(L) ) )
		{
			for( ; Hlas>0; L && Hlas--,L=L->dalsi );
			return (Sekvence *)L;
		}
	}
	return NULL;
}

Sekvence *NajdiSekvBI( const Pisen *P, BankaIndex I, int Hlas )
{
	Sekvence *L;
	L=P->SekPtr[I];
	if( !L )
	{
		Plati( !NajdiSekv(P,ZBanky(P->NazvySekv,I),Hlas) );
		return L;
	}
	Plati( L->INazev==I );
	for( ; Hlas>0; Hlas--,L=L->dalsi )
	{
		if( !L ) return NULL; /* nen¡ dost kan l… */
	}
	return (Sekvence *)L;
}

void NazevPisne( Pisen *P, const char *Nazev )
{
	char *E;
	strlncpy(P->NazevS,Nazev,sizeof(P->NazevS));
	E=NajdiExt(NajdiNazev(P->NazevS));
	if( E && *E && E>P->NazevS ) E[-1]=0;
	strlncpy(P->Cesta,Nazev,sizeof(P->Cesta));
}

Pisen *NovaPisen()
{
	Pisen *P=new(Pisen);
	int i;
	if( !P ) return P;
	P->Zac.next=NULL;
	P->Otevreno=0;
	P->Zmena=0;
	P->SSoubory=NULL;
	*P->Popis1=0;
	*P->Popis2=0;
	NovaBanka(P->NazvySekv);
	NovaBanka(P->Nastroje);
	for( i=0; i<(int)lenof(P->SekPtr); i++ ) P->SekPtr[i]=NULL;
	for( i=0; i<(int)lenof(P->NastrF); i++ ) P->NastrF[i][0]=0;
	ZacLocaly(P);
	NazevPisne(P,"");
	P->Efekt[0].Pars[0]=1000;
	P->Efekt[0].Pars[1]=700;
	P->Efekt[0].Pars[2]=500;
	P->Efekt[0].Pars[3]=0;
	P->Efekt[1].Pars[0]=400;
	P->Efekt[1].Pars[1]=400;
	P->Efekt[1].Pars[2]=500;
	P->Efekt[1].Pars[3]=0;
	P->Repeat=False;
	return P;
}
void MazPisen( Pisen *P )
{
	Sekvence *S,*s;
	KonLocaly(P);
	for( S=P->Zac.next; S; s=S->next,MazSekv(S),S=s );
	ZrusBanku(P->NazvySekv);
	ZrusBanku(P->Nastroje);
	#ifdef __DEBUG
		Plati( !P->SSoubory );
		For( Sekvence **, SS, P->SekPtr, &P->SekPtr[lenof(P->SekPtr)], SS++ )
			Plati( !*SS );
		Next
	#endif
	myfree(P);
}
Sekvence *PrvniSekv( const Pisen *P )
{
	return (Sekvence *)P->Zac.next;
}
Sekvence *DalsiSekv( const Sekvence *S )
{
	return (Sekvence *)S->next;
}

Flag PodHlas( Sekvence *K, Sekvence *N )
{
	for( ; N; N=DalsiHlas(N) ) if( K==N ) return True;
	return False;
}

static Flag FZavislost( Sekvence *K, BankaIndex I, int Hlas )
{
	SekvKursor p;
	Flag Zav=False;
	OtevriK(&p,K,Cteni);
	while( JCtiK(&p)==OK && !Zav )
	{
		if( p.Buf[0]==PVyvolej )
		{
			if( I==p.Buf[1] && Hlas==p.Buf[2] ) Zav=True;
			else
			{
				Sekvence *V=NajdiSekvBI(K->Pis,p.Buf[1],p.Buf[2]);
				if( V ) Zav=FZavislost(V,I,Hlas);
			}
		}
	}
	ZavriK(&p);
	return Zav;
}

Flag Zavislost( Sekvence *Kdo, Sekvence *NaKom )
{
	int Hlas;
	for( Hlas=0; NaKom->predch; NaKom=NaKom->predch,Hlas++ );
	return FZavislost(Kdo,NaKom->INazev,Hlas);
}

/* pristup k sekvencim */

static void Zacatek( SekvKursor *s )
{
	s->A=s->K;
	s->Opakuj=1;
	s->Bl=(Blok *)&s->S->Zac;
	s->Pos=0;
	s->_CasPos=0;
	s->_GlobPos=0;
	s->Uroven=0;
	s->Status=OK;
}

static void Zarad( SekvKursor *s )
{
	Sekvence *S=s->S;
	s->next=S->Soubory;
	S->Soubory=s;
}
static void Vypust( SekvKursor *s )
{
	Sekvence *S=s->S;
	SekvKursor **q;
	for( q=&S->Soubory; *q!=s; q=&(*q)->next ) Plati(*q);
	*q=s->next;
	s->next=NULL;
}

void ZacKontext( Kontext *K )
{
	K->Tempo=Tempo; /* impl. nastaveni kontextu */
	K->Takt=Takt;
	K->Tonina=-0x80;
	K->VTaktu=0;
	K->KteryTakt=0;
	K->Oktava=0;
	K->Nastroj=-0x80;
	K->Hlasit=0x80; /* polovicni hlasitost - mf */
	K->StereoMin=MinSt/2; /* default je kolem stredu */
	K->StereoMax=MaxSt/2;
	K->EffA=K->EffB=0;
}

void OtevriK( SekvKursor *s, Sekvence *S, int Mode )
{
	S->Pis->Otevreno++;
	s->S=S;
	s->Mode=Mode;
	ZacKontext(&s->K);
	Zacatek(s);
	if( Mode==Zapis ) S->Psani++;
	else S->Cteni++;
	Zarad(s);
}

void RozdvojK( SekvKursor *s, const SekvKursor *o )
{
	Sekvence *S=o->S;
	*s=*o;
	s->Mode=Cteni;
	S->Cteni++;
	S->Pis->Otevreno++;
	Zarad(s);
}
void RZapisK( SekvKursor *s, const SekvKursor *o )
{
	Sekvence *S=o->S;
	*s=*o;
	S->Psani++;
	s->Mode=Zapis;
	S->Pis->Otevreno++;
	Zarad(s);
}

#define OtevrTest() Plati( s->Mode==Zapis || s->Mode==Cteni )

void ZavriK( SekvKursor *s )
{
	Sekvence *S=s->S;
	S->Pis->Otevreno--;
	OtevrTest();
	if( s->Mode==Zapis ) S->Psani--;
	else S->Cteni--;
	s->Mode=0xff; /* zavreno */
	Vypust(s);
	/*myfree(s);*/ /* opraveno 8.10.94 */
}

static void PrevesNa( SekvKursor *s, const SekvKursor *o )
{ /* melo by zachovat kontext */
	SekvKursor *snext=s->next;
	int smode=s->Mode;
	*s=*o;
	s->next=snext;
	s->Mode=smode;
}

/* vetsinou neni AktPos ani PredPos potreba - proto makra */

#define AktPos(s) ( (s)->Pos >= (s)->Bl->Delka ? _AktPos(s) : EOK )

static Err _AktPos( SekvKursor *s )
{
	Sekvence *S=s->S;
	Blok *AB=s->Bl;
	while( s->Pos >= AB->Delka )
	{
		Plati(s->Pos <= AB->Delka);
		AB=AB->next;
		if( AB==(Blok *)&S->Kon ) return KON;
		s->Pos=0;
		s->Bl=AB;
	}
	return OK;
}

#define PredPos(s) ( (s)->Pos <= 0 ? _PredPos(s) : EOK )

static Err _PredPos( SekvKursor *s )
{
	Sekvence *S=s->S;
	Blok *AB=s->Bl;
	while( s->Pos <= 0 )
	{
		if( AB==(Blok *)&S->Zac ) return ZAC;
		AB=AB->prev;
		s->Pos=AB->Delka;
		s->Bl=AB;
	}
	return OK;
}

PosT PosDelka( Sekvence *S )
{
	return S->_GlobDel;
}

CasT DelkaSekv( Pisen *P, BankaIndex I, int Hlas, SekvKursor *Kont )
{
	Sekvence *S=NajdiSekvBI(P,I,Hlas);
	if( S ) return CasDelka(S,Kont);
	else return 0;
}

static Err ZpatkyKF( SekvKursor *s )
{
	int ret=PredPos(s);
	if( ret!=OK ) {s->Status=ret;return ret;}
	s->Pos-=Jednotka;
	s->_GlobPos-=Jednotka;
	PredPos(s);
	s->Status=OK;
	return OK;
}

static void PreskocKF( SekvKursor *s )
{
	if( s->Pos>=s->Bl->Delka ) _AktPos(s);
	s->Pos+=Jednotka;
	s->_GlobPos+=Jednotka;
}

static Flag ZpetPov( SekvKursor *s, JednotkaBuf B, int Povel )
{
	SekvKursor p;
	Err ret;
	Flag rt;
	RozdvojK(&p,s);
	for(;;)
	{
		ret=ZpatkyKF(&p);
		if( ret==ZAC ) {rt=False;break;}
		if( ret==OK )
		{
			ret=TestK(&p);
			Plati( ret==EOK );
			if( p.Buf[0]==Povel ) {rt=True;_EquJed(B,p.Buf);break;}
		}
	}
	ZavriK(&p);
	return rt;
}

static int ZpetTakt( SekvKursor *s )
{
	JednotkaBuf B;
	if( !ZpetPov(s,B,PNTakt) ) return s->K.Takt;
	else return Takt*B[1]/B[2];
}
static int ZpetTonina( SekvKursor *s )
{
	JednotkaBuf B;
	if( !ZpetPov(s,B,PTonina) ) return s->K.Tonina;
	else return B[1];
}
static int ZpetTempo( SekvKursor *s )
{
	JednotkaBuf B;
	if( !ZpetPov(s,B,PTempo) ) return s->K.Tempo;
	else return B[1];
}
static int ZpetOktava( SekvKursor *s )
{
	SekvKursor p;
	int AcOk=0;
	Err ret;
	if( s->Buf[0]==POktava && s->Buf[2] ) /* jsme na rel. oktave */
	{
		return s->A.Oktava-s->Buf[1];
	}
	RozdvojK(&p,s);
	for(;;)
	{
		ret=ZpatkyKF(&p);
		if( ret==ZAC ) break; /* zac. na oktave 0 */
		if( ret==OK )
		{
			PlatiProc( TestK(&p), ==EOK );
			if( p.Buf[0]==POktava )
			{
				AcOk+=p.Buf[1];
				if( !p.Buf[2] ) break; /* narazili jsme na absol. */
			}
		}
	}
	ZavriK(&p);
	return AcOk;
}

static int ZpetNastroj( SekvKursor *s )
{
	JednotkaBuf B;
	if( !ZpetPov(s,B,PNastroj) ) return s->K.Nastroj;
	else return B[1];
}
static int ZpetHlasit( SekvKursor *s )
{
	JednotkaBuf B;
	if( !ZpetPov(s,B,PHlasit) ) return s->K.Hlasit;
	else return B[1];
}
static void ZpetStereo( SekvKursor *s, int *Min, int *Max )
{
	JednotkaBuf B;
	if( !ZpetPov(s,B,PStereo) ) *Min=s->K.StereoMin,*Max=s->K.StereoMax;
	else *Min=B[1],*Max=B[2];
}
static void ZpetEfekt( SekvKursor *s, int *EffA, int *EffB )
{
	JednotkaBuf B;
	if( !ZpetPov(s,B,PEfekt) ) *EffA=s->K.EffA,*EffB=s->K.EffB;
	else *EffA=B[1],*EffB=B[2];
}

static CasT FZacRep( SekvKursor *r, int *_Opak, CasT *VnitrDelka )
{ /* chtelo by to udelat nerekursivne - a prehledne!!! */
	int Opak;
	int CRep=0;
	CasT ZacC=CasJeK(r),Sum;
	for(;;)
	{
		if( ZpatkyKF(r)!=OK ) {Opak=ZAC;break;}
		TestK(r);
		if( r->Buf[0]==PRep )
		{
			CRep--;
			if( CRep<0 )
			{
				Opak=r->Buf[1];
				PreskocKF(r);
				/* preskoci bez jakekoliv akce */
				/* PRep nemeni cas */
				break;
			}
		}
		else if( r->Buf[0]==PERep ) CRep++;
		PrejdiZK(r); /* tady je skryta rekurze pri CRep>0 a ERep */
	}
	Sum=ZacC-CasJeK(r);
	if( _Opak ) *_Opak=Opak;
	if( VnitrDelka ) *VnitrDelka=Sum;
	if( Opak==ZAC ) return Sum;
	return Sum*(Opak-1);
}

static CasT FDelkaRep( SekvKursor *k, int *Opak, CasT *VnitrDelka )
{
	SekvKursor r;
	CasT Cas;
	RozdvojK(&r,k);
	Cas=FZacRep(&r,Opak,VnitrDelka);
	ZavriK(&r);
	return Cas;
}

int NajdiZacRepK( SekvKursor *k )
{
	int Opak;
	FZacRep(k,&Opak,NULL);
	return Opak;
}

static CasT DelkaRep( SekvKursor *s, int *_Opak )
{
	return FDelkaRep(s,_Opak,NULL);
}

static CasT DelkaJedn( SekvKursor *s, JednItem *Pos ) /* Pos musi ukazovat na aktualni pozici v souboru */
{
	if( Pos[0]<=PPau )
	{
		if( Pos[1]&Soubezna ) return 0;
		if( s->A.Tempo==TempoMin ) return Pos[2]; /* No Tempo */
		else return Pos[2]*(TempoMin/s->A.Tempo);
	}
	else switch( Pos[0] )
	{
		case PVyvolej: return DelkaSekv(s->S->Pis,Pos[1],Pos[2],s);
		case PERep: return DelkaRep(s,NULL);
		default: return 0;
	}
}
static long LogDelkaJedn( SekvKursor *s, JednItem *Pos ) /* Pos musi ukazovat na aktualni pozici v souboru */
{
	long FD;
	if( Pos[0]<=PPau )
	{
		if( Pos[1]&Soubezna ) return 0;
		else return Pos[2];
	}
	else switch( Pos[0] )
	{
		case PVyvolej: FD=DelkaSekv(s->S->Pis,Pos[1],Pos[2],s);break;
		case PERep: FD=DelkaRep(s,NULL);break;
		default: return 0;
	}
	if( s->A.Tempo==TempoMin ) return FD; /* No Tempo */
	else return FD/(TempoMin/s->A.Tempo);
}

CasT DelkaKF( SekvKursor *s ) /* pozor - v Buf musi byt to, co na akt. pozici (pro insert plati primerene) */
{
	return DelkaJedn(s,s->Buf);
}
#define LogDelkaKF(s) LogDelkaJedn((s),(s)->Buf)

static Flag UpravVTaktu( SekvKursor *s )
{
	Flag r=False;
	while( s->A.VTaktu>=s->A.Takt ) s->A.VTaktu-=s->A.Takt,r=True,s->A.KteryTakt++;
	return r;
}
static void PrictiVTaktu( SekvKursor *s, long r )
{
	int T=s->A.Takt*4;
	r+=s->A.VTaktu;
	while( r>T ) r-=T,s->A.KteryTakt+=4;
	s->A.VTaktu=(int)r;
}

static void OdectiVTaktu( SekvKursor *s, CasT r )
{
	r=s->A.VTaktu-r;
	if( r<0 )
	{
		int T=-s->A.Takt*4;
		while( r<T ) r-=T,s->A.KteryTakt-=4;
	}
	s->A.VTaktu=(int)r;
}

/* rychlost - makro */

#define PrejdiKF(s) { if( (s)->Buf[0]<=PPau ) _PrejdiKFTon(s);else _PrejdiKF(s); }

static void _PrejdiKFTon( SekvKursor *s )
{
	s->_CasPos+=DelkaKF(s);
	PrictiVTaktu(s,LogDelkaKF(s));
}

static void _PrejdiKF( SekvKursor *s )
{
	switch( s->Buf[0] )
	{
		case PTonina: s->A.Tonina=s->Buf[1];break;
		case PTempo: s->A.Tempo=s->Buf[1];break;
		case PNTakt: s->A.Takt=Takt*s->Buf[1]/s->Buf[2];UpravVTaktu(s);break;
		case POktava:
			if( !s->Buf[2] ) s->A.Oktava=s->Buf[1];
			else s->A.Oktava+=s->Buf[1];
			break;
		case PNastroj: s->A.Nastroj=s->Buf[1];break;
		case PHlasit: s->A.Hlasit=s->Buf[1];break;
		case PStereo:
			s->A.StereoMin=s->Buf[1];s->A.StereoMax=s->Buf[2];
		break;
		case PEfekt:
			s->A.EffA=s->Buf[1];s->A.EffB=s->Buf[2];
		break;
		case PRep:
		{
			int Opak=s->Buf[1];
			int U=s->Uroven;
			if( U<MaxRepVnor )
			{
				s->RepOpak[U]=Opak;
				s->RepPos[U]=s->_CasPos;
			}
			else Opak=1;
			s->Opakuj*=Opak;
			s->Uroven++;
			break;
		}
		case PERep:
		{
			int U=s->Uroven-1;
			int Opak;
			CasT DR;
			if( U>=0 )
			{
				s->Uroven=U;
				if( U>=MaxRepVnor ) DR=0,Opak=1;
				else
				{
					Opak=s->RepOpak[U];
					DR=(s->_CasPos-s->RepPos[U])*(Opak-1);
				}
				s->_CasPos+=DR;
				s->Opakuj/=Opak;
				if( s->A.Tempo==TempoMin ) PrictiVTaktu(s,DR);
				else PrictiVTaktu(s,DR/(TempoMin/s->A.Tempo));
			}
			break;
		}
		default:
			s->_CasPos+=DelkaKF(s);
			PrictiVTaktu(s,LogDelkaKF(s));
			break;
	}
}

static SekvKursor *DopoctiCasy( SekvKursor *s )
{
	/* nutno davat pri prevesovani pozor na kontext! */
	/* pocitame jen ty, co maji stejne kontextove tempo jako s */
	/* pro ty ostatni se to stejne bude muset prepocitat */
	SekvKursor *q,*qm;
	PosT Dos=s->_GlobPos,QM;
	if( s->S->Soubory->next ) for(;;) /* jsou aspon dva soubory */
	{
		qm=NULL;
		QM=MAXPOS;
		for( q=s->S->Soubory; q; q=q->next ) if( q->K.Tempo==s->K.Tempo )
		{
			PosT qt=q->_GlobPos;
			if( qt==Dos && q!=s ) PrevesNa(q,s);
		}
		for( q=s->S->Soubory; q; q=q->next ) if( q->K.Tempo==s->K.Tempo )
		{
			PosT qt=q->_GlobPos;
			if( qt>Dos && ( qt<QM || qt==QM && q>qm ) && q!=s ) qm=q,QM=qt;
			/* finta - porovnani pointeru zajistuje ostre usporadani */
			/* mezi temi, co maji stejne _GlobPos */
		}
		if( qm )
		{
			PrevesNa(qm,s);
			while( qm->_GlobPos < QM ) PlatiProc( JCtiK(qm), ==OK );
			Dos=QM;
			s=qm;
		}
		else break;
	}
	return s;
}

void SetKontext( SekvKursor *r, SekvKursor *Kont )
{ /* pouziva se na zacatku sekvence */
	/* nastavi kontext podle akt. kontextu Kursoru Kont */
	r->K=r->A=Kont->A;
}

void SetKKontext( SekvKursor *r, const Kontext *Kont )
{ /* pouziva se na zacatku sekvence */
	/* nastavi kontext podle kontextu Kont */
	r->K=r->A=*Kont;
}

static Flag EquivVTaktu( int Takt, int VT, int vt )
{
	VT-=vt;
	if( VT<0 ) VT=-VT;
	return VT%Takt==0;
}

int CmpKontext( const Kontext *K, const Kontext *k )
{
	return
	(
		K->Tempo==k->Tempo &&
		K->Takt==k->Takt && EquivVTaktu(K->Takt,K->VTaktu,k->VTaktu) &&
		K->Tonina==k->Tonina &&
		K->Oktava==k->Oktava && K->Nastroj==k->Nastroj &&
		K->Hlasit==k->Hlasit &&
		K->StereoMin==k->StereoMin && K->StereoMax==k->StereoMax &&
		K->EffA==k->EffA && K->EffB==k->EffB
	);
}

static void SpoctiCasK( Sekvence *S, SekvKursor *Kont, SekvKursor *sk )
{
	/* v Kont nadrazeny soubor, je-li znam, */
	/* nebo v s soubor, ktery ve svem kontextu nese kontext s */
	SekvKursor r,k,*s;
	int T;
	S->_CasDelka=MAXCAS;
	OtevriK(&r,S,Cteni);
	if( Kont ) SetKontext(&r,Kont);
	else if( sk ) SetKKontext(&r,&sk->K);
	T=r.A.Tempo;
	s=DopoctiCasy(&r);
	if( s->S->_CasPriTempu!=T )
	{
		RozdvojK(&k,s);
		while( JCtiK(&k)==OK );
		k.S->_CasPriTempu=T;
		while( k.Uroven>0 )
		{
			k.Buf[0]=PERep;
			PrejdiKF(&k);
		}
		k.S->_CasDelka=k._CasPos;
		ZavriK(&k);
	}
	ZavriK(&r);
}

CasT CasDelka( Sekvence *S, SekvKursor *Kont )
{
	int T=Kont ? Kont->A.Tempo : Tempo;
	if( S->_CasPriTempu!=T ) SpoctiCasK(S,Kont,NULL);
	if( S->_CasDelka>0 ) return S->_CasDelka;
	return 1; /* sekvence nesmi byt nulova! */
}

CasT DelkaK( SekvKursor *s )
{
	Err ret=TestK(s);
	if( ret==OK ) return DelkaJedn(s,s->Buf);
	return 0;
}

Err TestK( SekvKursor *s )
{
	Err ret;
	if( s->Pos>=s->Bl->Delka )
	{
		ret=_AktPos(s);
		if( ret==OK )
		{
			JednItem *P=&s->Bl->Zac[s->Pos];
			_EquJed(s->Buf,P);
			PredPos(s);
		}
	}
	else
	{
		/* s uz bylo na legalni pozici - muzeme ho tak nechat */
		JednItem *P=&s->Bl->Zac[s->Pos];
		_EquJed(s->Buf,P);
		ret=EOK;
	}
	/* Test neovlivnuje status! */
	return ret;
}

Flag PrejdiK( SekvKursor *s, Flag Takty )
{
	if( s->Buf[0]==PDvojTakt ) s->A.VTaktu=0,s->A.KteryTakt++;
	else if( s->A.VTaktu>=s->A.Takt && UpravVTaktu(s) && Takty ) return True;
	PrejdiKF(s);
	if( s->Pos>=s->Bl->Delka ) _AktPos(s);
	s->Pos+=Jednotka;
	s->_GlobPos+=Jednotka;
	/* PredPos(s); */ /* nemuze byt potreba - s->Pos>0 ! */
	return False;
}

Err CtiK( SekvKursor *s )
{
	Err ret=TestK(s);
	if( ret==EOK && PrejdiK(s,True) )
	{
		static const int JedTakt[4]={PTakt,0,0,0};
		_EquJed(s->Buf,JedTakt);
	}
	s->Status=ret;
	return ret;
}
Err JCtiK( SekvKursor *s )
{ /* rychlost - ustredni rutina - 6 %, navic vola TestK, PrejdiK */
	Err ret=TestK(s);
	if( ret==EOK ) PrejdiK(s,False);
	s->Status=ret;
	return ret;
}
Err KopyK( SekvKursor *d, SekvKursor *s )
{
	Pisen *S,*D;
	Err ret;
	ret=JCtiK(s);
	if( ret!=OK ) return ret;
	_EquJed(d->Buf,s->Buf);
	S=s->S->Pis;
	D=d->S->Pis;
	switch( d->Buf[0] )
	{
		case PNastroj:
			d->Buf[1]=PridejRetez(D->Nastroje,ZBanky(S->Nastroje,d->Buf[1]));
			if( d->Buf[1]<OK ) return d->Buf[1];
			break;
		case PVyvolej:
			d->Buf[1]=PridejRetez(D->NazvySekv,ZBanky(S->NazvySekv,d->Buf[1]));
			if( d->Buf[1]<OK ) return d->Buf[1];
			break;
	}
	ret=VlozK(d);
	if( ret!=OK )
	{
		switch( d->Buf[0] )
		{
			case PNastroj: ZrusRetez(D->Nastroje,d->Buf[1]);break;
			case PVyvolej: ZrusRetez(D->NazvySekv,d->Buf[1]);break;
		}
		return ret;
	}
	return OK;
}

Err PrejdiZK( SekvKursor *s )
{
	Err ret=TestK(s);
	if( ret==OK )
	{
		switch( s->Buf[0] )
		{
			case PTonina: s->A.Tonina=ZpetTonina(s);break;
			case PTempo: s->A.Tempo=ZpetTempo(s);break;
			case POktava: s->A.Oktava=ZpetOktava(s);break;
			case PHlasit: s->A.Hlasit=ZpetHlasit(s);break;
			case PStereo: ZpetStereo(s,&s->A.StereoMin,&s->A.StereoMax);break;
			case PEfekt: ZpetEfekt(s,&s->A.EffA,&s->A.EffB);break;
			case PNastroj: s->A.Nastroj=ZpetNastroj(s);break;
			case PNTakt: s->A.Takt=ZpetTakt(s);UpravVTaktu(s);break;
			case PDvojTakt: s->A.KteryTakt--;break;
			case PRep:
				s->Uroven--;
				if( s->Uroven<MaxRepVnor ) s->Opakuj/=s->Buf[1];
				Plati(s->Uroven>=0);
				break;
			case PERep:
			{
				int U=s->Uroven;
				int Opak;
				CasT DR,CasR;
				if( U<MaxRepVnor )
				{
					s->Uroven++;
					DR=FDelkaRep(s,&Opak,&CasR); /* pri pocitani nesmi hrabnout pod s->Uroven */
					if( Opak>=OK )
					{
						Plati(U>=0);
						s->RepPos[U]=s->_CasPos-CasR*Opak;
						s->RepOpak[U]=Opak;
						s->Opakuj*=Opak;
						s->_CasPos-=DR;
						if( s->A.Tempo==TempoMin ) OdectiVTaktu(s,DR);
						else OdectiVTaktu(s,DR/(TempoMin/s->A.Tempo));
					}
					else s->Uroven--;
				}
				else s->Uroven++; /* urcite ma parovy zac. repetice */
				break;
			}
			default:
				s->_CasPos-=DelkaKF(s);
				OdectiVTaktu(s,LogDelkaKF(s));
				break;
		}
		while( s->A.VTaktu<=0 ) s->A.VTaktu+=s->A.Takt,s->A.KteryTakt--;
	}
	return ret;
}

Err ZpatkyK( SekvKursor *s )
{
	Err ret;
	ret=ZpatkyKF(s);
	if( ret==OK ) ret=PrejdiZK(s);
	s->Status=ret;
	return ret;
}

static Flag KritJedn( JednotkaBuf B )
{ /* jednotky, ktere za sebou naprosto zmeni casy */
	switch( B[0] )
	{
		case PRep: case PERep: case PTempo: return True;
		default: return False;
	}
}

static Err OverPovol( SekvKursor *s )
{ /* overi, zda se od akt. posize do konce vse pohybuje v mezich */
	while( JCtiK(s)==OK ) if( s->Uroven>MaxRepVnor ) return ERR;
	return OK;
}

static Err MazKF( SekvKursor *s, Flag Odstr )
{
	Blok *AB;
	PosT Pos;
	CasT dj;
	SekvKursor *q;
	JednItem *PP;
	Flag Krit;
	Err ret=AktPos(s);
	Plati(s->Mode==Zapis);
	if( ret!=OK ) goto Konec;
	AB=s->Bl;
	if( AB==(Blok *)&s->S->Kon ) {ret=KON;goto Konec;}
	Pos=s->Pos;
	PP=&AB->Zac[Pos];
	_EquJed(s->Buf,PP); /* dat do bufferu, co vymazeme */
	Krit=KritJedn(PP);
	if( !Krit ) dj=DelkaJedn(s,PP);
	else /* if( Krit ) */
	{
		SekvKursor k;
		Err rt;
		RozdvojK(&k,s);
		/*AktPos(&k);*/ /* preskok v k */ /* neni treba - s je na AktPos */
		k.Pos+=Jednotka;
		k._GlobPos+=Jednotka;
		PredPos(&k);
		rt=OverPovol(&k);
		ZavriK(&k);
		if( rt<OK ) {PredPos(s);return rt;}
		AktPos(s);
	}
	memcpy(PP,&PP[Jednotka],(AB->Delka-Pos-Jednotka)*JednVel);
	AB->Delka-=Jednotka;
	Plati(AB->Delka>=0);
	if( AB->Delka<=0 ) MazBlok(AB);
	s->S->_GlobDel-=Jednotka;
	PredPos(s);
	for( q=s->S->Soubory; q; q=q->next ) if( q->_GlobPos>s->_GlobPos )
	{
		q->_GlobPos-=Jednotka;
	}
	if( Krit ) s->S->_CasPriTempu=-1; /* neni vzdycky treba */
	else s->S->_CasDelka-=dj*s->Opakuj;
	DopoctiCasy(s);
	OznacZmenu(s->S,Krit);
	if( Odstr ) OdstranK(s); /* v bufferu musi byt to, co bylo na pos. kurs. */
	ret=OK;
	s->Status=ret;
	return ret;
	Konec:
	PredPos(s);
	s->Status=ret;
	return ret;
}

Err MazK( SekvKursor *s ){return MazKF(s,True);}
Err MazNOK( SekvKursor *s ){return MazKF(s,False);}

Err PisK( SekvKursor *s )
{
	int ret=AktPos(s);
	JednItem *PP;
	Plati(s->Mode==Zapis);
	if( ret!=OK ) {s->Status=ret;return ret;}
	UpravVTaktu(s);
	PP=&s->Bl->Zac[s->Pos];
	#if __DEBUG
	if
	(
		PP[0]==s->Buf[0] ||
		PP[0]!=PRep && PP[0]!=PERep &&
		PP[0]!=PNastroj && PP[0]!=PVyvolej &&
		s->Buf[0]!=PRep && s->Buf[0]!=PERep &&
		s->Buf[0]!=PNastroj && s->Buf[0]!=PVyvolej
	) {} else return ERR;
	#endif
	if( CmpJed(PP,s->Buf) )
	{
		Flag Krit=KritJedn(PP) || KritJedn(s->Buf);
		CasT dj,oj;
		if( !Krit ) dj=DelkaKF(s),oj=DelkaJedn(s,PP);
		_EquJed(PP,s->Buf);
		PrejdiK(s,False);
		if( Krit ) s->S->_CasPriTempu=-1; /* neni vzdycky treba */
		else s->S->_CasDelka+=(dj-oj)*s->Opakuj;
		OznacZmenu(s->S,Krit);
		DopoctiCasy(s);
	}
	else PrejdiK(s,False);
	s->Status=OK;
	return OK;
}

static int MaxDelka( Sekvence *S, Blok *B )
{
	if( B!=(Blok *)&S->Zac && B!=(Blok *)&S->Kon ) return DelkaBloku;
	return 0;
}

static void VlozBlok( SekvKursor *s )
{
	Blok *AB=s->Bl;
	PosT Pos=s->Pos;
	memcpy(&AB->Zac[Pos+Jednotka],&AB->Zac[Pos],(AB->Delka-Pos)*JednVel);
	AB->Delka+=Jednotka;
	EquJed(&AB->Zac[Pos],s->Buf);
}

static Err TrhejBlok( SekvKursor *s )
{
	Blok *AB=s->Bl;
	int Pos=s->Pos;
	Blok *NB=NovyBlok(AB);
	if( !NB ) return ERAM;
	memcpy(&NB->Zac[Jednotka],&AB->Zac[Pos],(AB->Delka-Pos)*JednVel);
	NB->Delka=AB->Delka-Pos+Jednotka;
	AB->Delka=Pos;
	EquJed(&NB->Zac[0],s->Buf);
	s->Bl=NB;
	s->Pos=0;
	return OK;
}

Err VlozK( SekvKursor *s )
{
	SekvKursor *q;
	Blok *AB;
	Flag Krit=KritJedn(s->Buf);
	CasT dj;
	Plati(s->Mode==Zapis);
	if( Krit )
	{
		SekvKursor k;
		Err rt;
		RozdvojK(&k,s);
		PrejdiKF(&k);
		rt=OverPovol(&k);
		ZavriK(&k);
		if( rt<OK ) return rt;
	}
	PredPos(s);
	AB=s->Bl;
	Plati(AB->Delka<=MaxDelka(s->S,AB));
	if( AB->Delka<MaxDelka(s->S,AB) )
	{
		VlozBlok(s);
	}
	else
	{
		Err ret=TrhejBlok(s);
		if( ret!=OK ) return ret;
	}
	s->S->_GlobDel+=Jednotka;
	if( !Krit ) dj=DelkaKF(s);
	OznacZmenu(s->S,Krit);
	{
		PrejdiK(s,False);
		for( q=s->S->Soubory; q; q=q->next ) if( q->_GlobPos>=s->_GlobPos && q!=s )
		{
			q->_GlobPos+=Jednotka;
		}
	}
	if( Krit ) s->S->_CasPriTempu=-1; /* neni vzdycky treba */
	else s->S->_CasDelka+=dj*s->Opakuj;
	DopoctiCasy(s);
	s->Status=OK;
	return OK;
}

PosT KdeJeK( SekvKursor *s )
{
	Plati( s->_GlobPos <= s->S->_GlobDel );
	return s->_GlobPos;
}
CasT CasJeK( SekvKursor *s )
{
	if( s->S->_CasPriTempu!=s->K.Tempo ) SpoctiCasK(s->S,NULL,s);
	Plati( s->_CasPos <= s->S->_CasDelka );
	return s->_CasPos;
}

Err NajdiK( SekvKursor *s, PosT Pos )
{
	Err ret=OK;
	PosT P1=s->_GlobPos/2;
	if( Pos<0 ) return ERR;
	if( Pos<P1 ) Zacatek(s);
	while( s->_GlobPos > Pos )
	{
		if( (ret=ZpatkyK(s))!=OK ) goto Ret;
	}
	while( s->_GlobPos < Pos )
	{
		if( (ret=JCtiK(s))!=OK ) goto Ret;
	}
	Ret:
	if( s->_GlobPos!=Pos ) ret=ERR;
	s->Status=ret;
	return ret;
}
Err NajdiCasK( SekvKursor *s, CasT Pos )
{
	Err ret=OK;
	CasT P1=CasJeK(s)*2/3;
	if( Pos<0 ) return ERR;
	if( Pos<P1 || Pos==0 ) Zacatek(s);
	if( Pos==0 ) goto Ret;
	while( s->_CasPos >= Pos )
	{
		if( (ret=ZpatkyK(s))!=OK )
		{
			if( ret!=ZAC ) goto Ret;
			break;
		}
	}
	while( s->_CasPos < Pos )
	{
		if( (ret=JCtiK(s))!=OK ) break;
	}
	if( s->_CasPos<Pos ) ret=ERR;
	Ret:
	s->Status=ret;
	return ret;
}
Err NajdiSCasK( SekvKursor *s, CasT Pos )
{
	Err ret=NajdiCasK(s,Pos);
	if( ret!=ERR )
	{
		for(;;)
		{
			if( s->_CasPos==Pos )
			{
				ret=JCtiK(s);
				if( ret!=OK ) {if( ret==KON ) ret=OK;break;}
				if( s->_CasPos>Pos || s->Buf[0]==PVyvolej || s->Buf[0]<=PPau ) {ret=ZpatkyK(s);break;}
				/* test na noty kvuli soubeznym */
			}
			else {ret=ZpatkyK(s);break;}
		}
	}
	s->Status=ret;
	return ret;
}

Err NajdiZTaktK( SekvKursor *s, int Num )
{
	Err ret=OK;
	int P1=s->A.KteryTakt*2/3;
	if( Num<0 ) return ERR;
	if( Num<P1 || Num==0 ) Zacatek(s);
	if( Num==0 ) goto Ret;
	while( s->A.KteryTakt>=Num )
	{
		if( (ret=ZpatkyK(s))!=OK )
		{
			if( ret!=ZAC ) goto Ret;
			break;
		}
	}
	while( s->A.KteryTakt<Num )
	{
		if( (ret=CtiK(s))!=OK ) break;
	}
	if( s->A.KteryTakt<Num ) ret=ERR;
	Ret:
	s->Status=ret;
	return ret;
}

Err NajdiTaktK( SekvKursor *s, int Num ) /* nastaveni casu */
{
	Err ret=NajdiZTaktK(s,Num);
	if( ret!=ERR )
	{
		for(;;)
		{
			if( s->A.KteryTakt==Num )
			{
				ret=CtiK(s); /* cti i takty! */
				if( ret!=EOK ) {if( ret==KON ) ret=EOK;break;}
				if( s->A.KteryTakt>Num || s->Buf[0]==PVyvolej || s->Buf[0]<=PPau ) {ret=ZpatkyK(s);break;}
				/* test na noty kvuli soubeznym */
			}
			else {ret=ZpatkyK(s);break;}
		}
	}
	s->Status=ret;
	return ret;
}

Flag BezChybyK( SekvKursor *s )
{
	return s->Status==OK || s->Status==ZAC || s->Status==KON;
}
