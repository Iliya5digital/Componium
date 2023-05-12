/* Componium - zachazeni s vnitrnim formatem */
/* hierarchicke sledovani souboru */
/* SUMA, 7/1993 */

#include "macros.h"
#include "utlcompo.h"

static void ZaradS( SekvSoubor *s )
{
	Pisen *P=SekvenceS(s)->Pis;
	s->next=P->SSoubory;
	P->SSoubory=s;
}
static void VypustS( SekvSoubor *s )
{
	Pisen *P=SekvenceS(s)->Pis;
	SekvSoubor **q;
	for( q=&P->SSoubory; *q!=s; q=&(*q)->next ) Plati(*q);
	*q=s->next;
	s->next=NULL;
}

void OtevriS( SekvSoubor *s, Sekvence *S, int Mode )
{
	s->SUroven=0;
	s->SHierarch=False;
	OtevriK(&s->ss[0],S,Mode);
}

void RozdvojS( SekvSoubor *s, const SekvSoubor *o )
{
	int i;
	s->SHierarch=o->SHierarch;
	s->SUroven=o->SUroven;
	for( i=0; i<=s->SUroven; i++ ) RozdvojK(&s->ss[i],&o->ss[i]);
	if( s->SHierarch ) ZaradS(s);
}
void RZapisS( SekvSoubor *s, const SekvSoubor *o )
{
	int i;
	s->SHierarch=o->SHierarch;
	s->SUroven=o->SUroven;
	for( i=0; i<=s->SUroven; i++ ) RZapisK(&s->ss[i],&o->ss[i]);
	if( s->SHierarch ) ZaradS(s);
}

void ZavriS( SekvSoubor *s )
{
	int i;
	for( i=0; i<=s->SUroven; i++ ) ZavriK(&s->ss[i]);
	if( s->SHierarch ) VypustS(s);
}

void UrovenS( SekvSoubor *s, int u )
{
	int i;
	Plati( u<=s->SUroven );
	for( i=u+1; i<=s->SUroven; i++ ) ZavriK(&s->ss[i]);
	s->SUroven=u;
}

void RezimS( SekvSoubor *s, Flag Hierarch )
{
	if( Hierarch!=s->SHierarch )
	{
		s->SHierarch=Hierarch;
		UrovenS(s,0);
		if( Hierarch ) ZaradS(s);
		else VypustS(s);
	}
}

SekvKursor *AktK( SekvSoubor *s )
{
	return &s->ss[s->SUroven];
}

JednItem *BufS( SekvSoubor *s )
{
	return s->ss[s->SUroven].Buf;
}

/* pri pruchodu sekvenci ukazuje nadrazeny Kursor na PVyvolej */

static void SetDef( SekvSoubor *s )
{
	/* nastaveni parametru podle kontextu */
	SekvKursor *k;
	if( s->SUroven<=0 ) return;
	k=&s->ss[s->SUroven];
	SetKontext(k,k-1);
	/*k->K.KteryTakt=k->A.KteryTakt=0;*/
}

Err CtiS( SekvSoubor *s )
{
	SekvKursor *k=AktK(s);
	Err ret=CtiK(k);
	if( !s->SHierarch ) return ret;
	if( ret!=EOK )
	{ /* Konec partu */
		if( s->SUroven>0 )
		{
			JednItem *B;
			SekvKursor *k=&s->ss[s->SUroven];
			int SekI=k->S->INazev;
			ZavriK(k);
			s->SUroven--;
			k=AktK(s);
			ret=TestK(k); /* preskok PVyvolej */
			if( ret==EOK )
			{
				B=k->Buf;
				Plati( B[0]==PVyvolej && B[1]==SekI );
				PlatiProc( JCtiK(k), ==EOK );
				B[0]=PKonSekv;
				if( k->A.VTaktu==0 ) k->A.VTaktu+=k->A.Takt,k->A.KteryTakt--;
				while( k->A.VTaktu>k->A.Takt ) k->A.VTaktu-=k->A.Takt,k->A.KteryTakt++;
			}
		}
	}
	else if( k->Buf[0]==PVyvolej )
	{
		if( s->SUroven<MaxSekVnor-1 )
		{
			Sekvence *S=NajdiSekvBI(k->S->Pis,k->Buf[1],k->Buf[2]);
			if( S )
			{
				JednItem *B;
				SekvKursor *n;
				s->SUroven++;
				n=AktK(s);
				OtevriK(n,S,k->Mode);
				B=BufS(s);
				*B++=PZacSekv;
				*B++=k->Buf[1];
				*B++=k->Buf[2];
				*B++=0;
				PlatiProc( ZpatkyK(k), ==EOK ); /* zpet na PVyvolej */
				SetDef(s);
			}
		}
	}
	return ret;
}

int CmpPos( const StrPosT *P, const StrPosT *p )
{
	long d;
	d=P->p[0]-p->p[0];
	if( d<0 ) return -1;else if( d>0 ) return +1;
	d=P->p[1]-p->p[1];
	if( d<0 ) return -1;else if( d>0 ) return +1;
	d=P->p[2]-p->p[2];
	if( d<0 ) return -1;else if( d>0 ) return +1;
	d=P->p[3]-p->p[3];
	if( d<0 ) return -1;else if( d>0 ) return +1;
	/* pozor - predp. MaxSekVnor==4 */
	#ifdef __TURBOC__
		#if MaxSekVnor!=4
			#error "OptimKonstr"
		#endif
	#endif
	return 0;
}

/* i fiktivni jednotky pri rek. pruchodu */

Flag Neumele( JednItem Druh )
{
	return
	(
		Druh!=PTakt && Druh!=PDvojTaktUm && Druh!=PUmPauza
	);
}

/* jen opravdove jednotky */

static Flag Skutecne( JednItem Druh )
{
	return Neumele(Druh) && Druh!=PZacSekv && Druh!=PKonSekv;
}

Err JCtiS( SekvSoubor *s )
{
	Err ret;
	if( !s->SHierarch ) return JCtiK(AktK(s));
	ret=CtiS(s);
	if( ret==OK && !Neumele(BufS(s)[0]) ) ret=CtiS(s);
	return ret;
}

Err KopyS( SekvSoubor *d, SekvSoubor *s )
{
	SekvKursor *sk=AktK(s);
	Err ret=TestK(sk);
	if( ret<EOK ) return ret;
	if( BufK(sk)[0]!=PKonSekv ) return KopyK(AktK(d),sk);
	else return ECAN;
}

Err TestS( SekvSoubor *s )
{
	SekvKursor *k=AktK(s);
	Err ret=TestK(k);
	if( !s->SHierarch ) return ret;
	if( ret!=OK )
	{
		if( s->SUroven>0 )
		{
			JednItem *B=k->Buf;
			*B++=PKonSekv;
			*B++=*B++=*B++=0;
			ret=OK;
		}
	}
	else if( k->Buf[0]==PVyvolej ) k->Buf[0]=PZacSekv;
	return ret;
}

Err ZpatkyS( SekvSoubor *s )
{
	SekvKursor *k=AktK(s);
	Err ret=ZpatkyK(k);
	if( !s->SHierarch ) return ret;
	if( ret!=OK )
	{
		if( s->SUroven>0 )
		{
			JednItem *B;
			SekvKursor *k=&s->ss[s->SUroven];
			int SekI=k->S->INazev;
			ZavriK(k);
			s->SUroven--;
			k=AktK(s);
			ret=TestK(k);
			Plati( ret==OK );
			Plati( k->Buf[0]==PVyvolej );
			Plati( k->Buf[1]==SekI );
			B=k->Buf;
			B[0]=PZacSekv;
		}
	}
	else if( k->Buf[0]==PVyvolej )
	{
		if( s->SUroven<MaxSekVnor-1 )
		{
			Sekvence *S=NajdiSekvBI(k->S->Pis,k->Buf[1],k->Buf[2]);
			if( S )
			{
				JednItem *B;
				SekvKursor *n;
				s->SUroven++;
				n=AktK(s);
				OtevriK(n,S,k->Mode);
				SetDef(s);
				NajdiCasK(n,0x7fffffffL); /* presun na konec */
				B=n->Buf;
				*B++=PKonSekv;
				*B++=k->Buf[1];
				*B++=k->Buf[2];
				*B++=0;
			}
		}
	}
	return ret;
}

CasT DelkaS( SekvSoubor *s )
{
	return DelkaK(AktK(s));
}
CasT DelkaSF( SekvSoubor *s )
{
	return DelkaKF(AktK(s));
}

Kontext *KKontextS( SekvSoubor *s )
{
	return KKontextK(&s->ss[0]);
}
Kontext *AKontextS( SekvSoubor *s )
{
	return AKontextK(AktK(s));
}

int NajdiZacRepS( SekvSoubor *s )
{
	return NajdiZacRepK(AktK(s));
}

/* pri vymazani PVyvolej za zavrou vsechny */
/* podrizene kursory, ktere ocekavaji navrat tamtez */
/* pri vymazani sekvence podobne */

/* reakce na vymazani sekvence */
void ZavriVolajici( Sekvence *S )
{
	SekvSoubor *t;
	for( t=S->Pis->SSoubory; t; t=t->next ) if( t->SHierarch )
	{
		int i;
		for( i=0; i<=t->SUroven; i++ )
		{
			SekvKursor *at=&t->ss[i];
			Sekvence *T=SekvenceK(at);
			if( T==S )
			{ /* nutno odstranit dalsi vrstvy */
				Plati( i>0 );
				UrovenS(t,i-1); /* mohlo by nekomu vadit, ze s nim hneme? !!! */
				break;
			}
		}
	}
}

/* reakce na vymazani volani sekvence */
void ZavriVolajiciZ( SekvKursor *s )
{
	Sekvence *S=SekvenceK(s);
	SekvSoubor *t;
	for( t=S->Pis->SSoubory; t; t=t->next )
	{
		int i;
		for( i=0; i<t->SUroven; i++ ) /* i==SUroven uz nema cenu zkoumat - tam uz nic nemuze vadit */
		{
			SekvKursor *at=&t->ss[i];
			Sekvence *T=SekvenceK(at);
			if( T==S && KdeJeK(at)==KdeJeK(s) )
			{ /* nutno odstranit dalsi vrstvy */
				UrovenS(t,i); /* mohlo by nekomu vadit, ze s nim hneme? !!! */
				break;
			}
		}
	}
}

Err MazS( SekvSoubor *s )
{
	Err ret;
	JednItem *B;
	ret=TestS(s);
	if( ret!=EOK ) return ret;
	B=BufS(s);
	if( B[0]==PKonSekv ) return ECAN;
	return MazK(AktK(s));
}
Err VlozS( SekvSoubor *s )
{
	if( !Skutecne(BufS(s)[0]) ) return ERR;
	return VlozK(AktK(s));
}
Err PisS( SekvSoubor *s )
{
	if( !Skutecne(BufS(s)[0]) ) return ERR;
	return PisK(AktK(s));
}

Err KdeJeS( SekvSoubor *s, StrPosT *Pos )
{
	int i;
	for( i=0; i<=s->SUroven; i++ )
	{
		Pos->p[i]=KdeJeK(&s->ss[i]);
	}
	for( ; i<MaxSekVnor; i++ ) Pos->p[i]=-1;
	return EOK;
}

CasT CasJeS( SekvSoubor *s )
{
	int i;
	CasT c=0;
	for( i=0; i<=s->SUroven; i++ )
	{
		c+=CasJeK(&s->ss[i]);
	}
	return c;
}

Err NajdiS( SekvSoubor *s, const StrPosT *Pos )
{
	Sekvence *S;
	SekvKursor *k=&s->ss[0];
	Err ret;
	UrovenS(s,0);
	if( !s->SHierarch ) return NajdiK(k,Pos->p[0]);
	for(;;)
	{
		int M;
		ret=NajdiK(k,Pos->p[s->SUroven]);
		if( ret!=EOK || s->SUroven>=MaxSekVnor-1 ) return ret;
		if( Pos->p[s->SUroven+1]<0 ) return EOK;
		ret=TestK(k);
		if( ret!=EOK || k->Buf[0]!=PVyvolej ) return ret;
		S=NajdiSekvBI(k->S->Pis,k->Buf[1],k->Buf[2]);
		if( !S ) return ret;
		s->SUroven++;
		M=k->Mode;
		k=AktK(s);
		OtevriK(k,S,M);
		SetDef(s);
	}
}

Err NajdiCasS( SekvSoubor *s, CasT Cas )
{
	Sekvence *S;
	CasT ca;
	SekvKursor *k=&s->ss[0];
	Err ret;
	if( !s->SHierarch ) return NajdiCasK(k,Cas);
	UrovenS(s,0);
	for(;;)
	{
		int M;
		ret=NajdiSCasK(k,Cas);
		if( ret!=EOK || s->SUroven>=MaxSekVnor-1 ) return ret;
		ca=CasJeK(k);
		if( ca==Cas ) return NajdiCasK(k,Cas);
		ret=TestK(k);
		if( ret!=EOK || k->Buf[0]!=PVyvolej ) return NajdiCasK(k,Cas);
		S=NajdiSekvBI(k->S->Pis,k->Buf[1],k->Buf[2]);
		if( !S ) return NajdiCasK(k,ca);
		s->SUroven++;
		M=k->Mode;
		k=AktK(s);
		OtevriK(k,S,M);
		SetDef(s);
		Cas-=ca;
	}
}

Err NajdiSCasS( SekvSoubor *s, CasT Cas )
{
	Sekvence *S;
	CasT ca;
	SekvKursor *k=&s->ss[0];
	Err ret;
	UrovenS(s,0);
	if( !s->SHierarch  ) return NajdiSCasK(k,Cas);
	for(;;)
	{
		int M;
		ret=NajdiSCasK(k,Cas);
		if( s->SUroven>=MaxSekVnor-1 || ret!=EOK ) return ret;
		ca=CasJeK(k);
		Cas-=ca;
		Plati( Cas>=0 );
		ret=TestK(k);
		if( ret!=EOK || k->Buf[0]!=PVyvolej ) return ret;
		S=NajdiSekvBI(k->S->Pis,k->Buf[1],k->Buf[2]);
		if( !S ) return ret;
		s->SUroven++;
		M=k->Mode;
		k=AktK(s);
		OtevriK(k,S,M);
		SetDef(s);
	}
}

Err NajdiTaktS( SekvSoubor *s, int Takt )
{
	Sekvence *S;
	/*int ca;*/
	SekvKursor *k=&s->ss[0];
	Err ret;
	UrovenS(s,0);
	if( !s->SHierarch  ) return NajdiTaktK(k,Takt);
	for(;;)
	{
		int M;
		ret=NajdiTaktK(k,Takt);
		if( s->SUroven>=MaxSekVnor-1 || ret!=EOK ) return ret;
		/*
		ca=AKontextK(k)->KteryTakt;
		Takt-=ca;
		*/ /* i dovnitr se cisluje stejne */
		Plati( Takt>=0 );
		ret=TestK(k);
		if( ret!=EOK || k->Buf[0]!=PVyvolej ) return ret;
		S=NajdiSekvBI(k->S->Pis,k->Buf[1],k->Buf[2]);
		if( !S ) return ret;
		s->SUroven++;
		M=k->Mode;
		k=AktK(s);
		OtevriK(k,S,M);
		SetDef(s);
	}
}

/* musi byt ve stejne sekvenci a oba stejne Hierarch */
void NajdiPosS( SekvSoubor *s, const SekvSoubor *o )
{
	int Mode=AktK(s)->Mode;
	Plati( SekvenceS(s)==SekvenceS(o) );
	Plati( s->SHierarch==o->SHierarch );
	ZavriS(s);
	if( Mode==Cteni ) RozdvojS(s,o);
	else RZapisS(s,o);
}

Err StatusS( SekvSoubor *s )
{
	if( s->SUroven>0 ) return OK;
	return (Err)s->ss[0].Status;
}

Flag BezChybyS( SekvSoubor *s )
{
	return StatusS(s)==OK || StatusS(s)==ZAC || StatusS(s)==KON;
}
