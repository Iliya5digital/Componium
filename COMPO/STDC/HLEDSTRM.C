/* vyhledavaci strom */
/* SUMA 11/1993 */

#include <macros.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include "mempool.h"
#include "hledstrm.h"

#define RezieNaUzel sizeof(struct alokitem)

#define DIAG 1

MemPool Pool;

long VelStrom( Strom *S ){return S->VelStrom;}

struct uzel *FNajdiUzel( struct uzel *r, long klic )
{
	/* nenajde-li presne, da NULL */
	while( r )
	{
		long dif=klic-r->klic;
		if( dif==0 ) return r;
		ef( dif>0 ) r=r->vsyn;
		else r=r->msyn; /* dif<0 */
	}
	return r;
}

static struct uzel *FDelUzel( Strom *S, long klic )
{
	struct uzel **b=&S->Koren,*r=*b;
	while( r )
	{
		long dif=klic-r->klic;
		if( dif>0 ) b=&r->vsyn,r=*b;
		ef( dif<0 ) b=&r->msyn,r=*b;
		else
		{ /* jeste premistit potomky */
			struct uzel *v=r->vsyn,*m=r->msyn; /* r je nalezeny uzel, b na nej ukazuje */
			for(;;) /* ted s r dobublame do listu nebo jedn. stromu */
			{ /* v, m stromy, ktere je treba umistit, b mak umistujeme */
				if( !v ) {*b=m;break;}
				ef( !m ) {*b=v;break;}
				else
				{ /* oba potomci - ten s vyssi prioritou jde nahoru */
					long dif=v->prior-m->prior;
					if( dif>0 || dif==0 && (rand()&1) )
					{
						*b=v;
						b=&v->msyn;
						v=*b;
					}
					else
					{
						*b=m;
						b=&m->vsyn;
						m=*b;
					}
				}
			}
			return r;
		}
	}
	return r;
}

static void FInsUzel( Strom *S, struct uzel *a ) /* urcite neni - prave jsme ho smazali */
{ /* zapojeni jiz existujiciho uzlu - urcite neni ve stromu */
	struct uzel **b=&S->Koren,*r=*b;
	while( r )
	{
		if( a->prior>r->prior ) /* musi prijit misto r */
		{
			struct uzel **nmv=&a->vsyn,**nvm=&a->msyn; /* postupne davame do nejvets. mensiho a nejmens. vetsiho */
			*b=a;
			while( r )
			{
				long dif=a->klic-r->klic;
				if( dif>0 ) /* r je mensi */
				{
					*nvm=r;
					nvm=&r->vsyn;
					r=*nvm;
				}
				else /* dif<0 - r je vetsi */
				{
					*nmv=r;
					nmv=&r->msyn;
					r=*nmv;
				}
			}
			*nmv=*nvm=NULL;
			return;
		}
		else /* muzeme sestoupit, mame nizi ci stejnou prioritu */
		{
			long dif=a->klic-r->klic;
			if( dif>0 ) b=&r->vsyn;
			else b=&r->msyn; /* rovnost nemuze nastat */
			r=*b;
		}
	}
	/* jsme na miste - dej do listu */
	*b=a;
	a->vsyn=a->msyn=NULL;
}

void *NajdiUzel( Strom *S, long klic )
{
	struct uzel *r=FNajdiUzel(S->Koren,klic);
	if( r ) ++r;
	return r;
}

void *ZmenPrior( Strom *S, long Klic, long zmprior, Flag ZmAbsol )
{
	struct uzel *r=FDelUzel(S,Klic);
	if( !r ) return NULL;
	if( !ZmAbsol ) r->prior+=zmprior;
	else r->prior=zmprior;
	FInsUzel(S,r);
	return ++r;
}

void *UInsUzel( Strom *S, long klic, long zmprior, size_t Vel, long MaxVel, long (*Ven)( long *vsiz ) )
{
	struct uzel *NB;
	long NVel;
	for(;;) /* vyhazuj, dokud se nevejde novy uzel */
	{
		long k,vsiz;
		NVel=S->VelStrom+Vel+sizeof(struct uzel)+RezieNaUzel;
		if( MaxVel<0 || NVel<=MaxVel && (NB=MemAlloc(&Pool,sizeof(struct uzel)+Vel,'Tree'))!=NULL )
		{
			break;
		}
		ef( (k=Ven(&vsiz),DelUzel(S,k,vsiz))<0 ) /* nemame vyhazovace nebo neukazal spravne */
		{
			return NULL; /* neni koho vyhodit */
		}
	}
	S->VelStrom=NVel;
	NB->klic=klic;
	NB->prior=zmprior;
	FInsUzel(S,NB);
	return ++NB;
}

void *InsUzel( Strom *S, long klic, long zmprior, Flag zmabsol, size_t Vel, long MaxVel, Flag *UzJe, long (*Ven)( long *vsiz ) )
{
	void *r=ZmenPrior(S,klic,zmprior,zmabsol);
	if( r )
	{
		*UzJe=True;
		return r;
	}
	else
	{
		r=UInsUzel(S,klic,zmprior,Vel,MaxVel,Ven);
		*UzJe=False;
		return r;
	}
}

int DelUzel( Strom *S, long klic, size_t Vel )
{
	struct uzel *r=FDelUzel(S,klic);
	if( !r ) return -1;
	MemFree(&Pool,r);
	S->VelStrom-=Vel+sizeof(struct uzel)+RezieNaUzel;
	return 0;
}

void ZacStrom( Strom *S )
{
	S->Koren=NULL;
	S->VelStrom=0;
}

void KonStrom( Strom *S )
{
	/* uvolni vsechny uzly */
	#if defined __DEBUG && DIAG
		char EBuf[2048];
		Plati( !TestStromu(S->Koren,EBuf,LONG_MIN,LONG_MAX,LONG_MAX) )
	#endif
	while( S->Koren )
	{
		DelUzel(S,S->Koren->klic,0);
	}
	S->VelStrom=0;
}

int ZacStromy( long VelPool )
{
	srand((int)time(NULL));
	return MemZac(&Pool,VelPool);
}

void KonStromy( void )
{
	MemKon(&Pool);
}

/* diagnostika */

#if defined __DEBUG && DIAG

#include <string.h>
#include <tos.h>

int TestStromu( struct uzel *u, char *EBuf, long Min, long Max, long Prior ) /* v jakem rozmezi musi byt strom */
{ /* test koreknosti InOrder a HeapOrder ve stralde */
	int r;
	if( u )
	{
		if( u->klic<Min || u->klic>Max ) {*EBuf=0;return 1;}
		if( u->prior>Prior ) {*EBuf=0;return 3;}
		r=TestStromu(u->vsyn,EBuf,u->klic,Max,u->prior);
		if( r )
		{
			strcat(EBuf,"V");
			return r;
		}
		r=TestStromu(u->msyn,EBuf,Min,u->klic,u->prior);
		if( r )
		{
			strcat(EBuf,"M");
			return r;
		}
	}
	return 0;
}

void TestStrom( void )
{
	int i,j;
	char EBuf[1024];
	Strom S;
	long CMaxVel=Pool.free->velikost;
	for( j=0; j<1000; j++ ) /* velmi dukladne zkouseni */
	{
		if( j%100==0 ) Cconws("100\xd\xa");
		ZacStrom(&S);
		for( i=0; i<1024; i++ )
		{
			int k=random(100)-50;
			int p=random(32);
			Flag UzJe;
			/*
			if( !NajdiUzel(&S,k) )
			*/
			{
				InsUzel(&S,k,p,True,0,-1,&UzJe,(long (*)(long *))NULL);
			}
		}
		/* strom vygenerovan - ted pruchod */
		EBuf[0]=0;
		switch( TestStromu(S.Koren,EBuf,LONG_MIN,LONG_MAX,LONG_MAX) )
		{
			case 0 : break;
			case 1 :
				Cconws("Strom NOK\xd\xa");Cconws(EBuf);Cconws("\xd\xa");
				break;
			case 3 :
				Cconws("Halda NOK\xd\xa");Cconws(EBuf);Cconws("\xd\xa");
				break;
			default: Cconws("Cosi NOK\xd\xa");break;
		}
		KonStrom(&S);
		if( !Pool.free ) Cconws("NoFree\xd\xa");
		ef( Pool.free->velikost!=CMaxVel ) Cconws("Ne dost volna\xd\xa");
	}
}

#endif
static long FHloubkaStromu( struct uzel *u )
{
	if( !u ) return 0;
	else
	{
		long h=FHloubkaStromu(u->vsyn);
		long H=FHloubkaStromu(u->msyn);
		if( h>H ) H=h;
		return H+1;
	}
}
static long FPHloubkaStromu( struct uzel *u, int ur )
{
	if( !u ) return 0;
	else
	{
		long h=FPHloubkaStromu(u->vsyn,ur+1);
		long H=FPHloubkaStromu(u->msyn,ur+1);
		return h+H+ur+1;
	}
}
static long FPocetUzlu( struct uzel *u )
{
	if( !u ) return 0;
	else return FPocetUzlu(u->vsyn)+FPocetUzlu(u->msyn)+1;
}

long HloubkaStromu( Strom *S )
{
	return FHloubkaStromu(S->Koren);
}
long PHloubkaStromu( Strom *S )
{
	return FPHloubkaStromu(S->Koren,0)/FPocetUzlu(S->Koren);
}
long PocetUzlu( Strom *S )
{
	return FPocetUzlu(S->Koren);
}

