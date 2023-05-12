/* strom obsahujici uzly libovolne velikosti */
/* uzel nenese inforamci o velkosti! */
/* jedna se o straldu - TREAP */

#ifndef __HLEDSTRM
#define __HLEDSTRM
#ifndef __STDDEF
#include <stddef.h>
#endif

typedef struct uzel /* generalizace */
{
	struct uzel *vsyn,*msyn; /* interni struktura - vetsi, mensi */
	long klic;
	long prior;
	long buf[0]; /* ve skut. tady je vlastni obsah uzlu */
};

typedef struct
{
	struct uzel *Koren;
	long VelStrom; /* celk. velikost pameti obsazena stromem */
} Strom;

#ifdef __MEMPOOL
	extern MemPool Pool;
#endif

void ZacStrom( Strom *S );
void KonStrom( Strom *S );
long VelStrom( Strom *S );


void *UInsUzel( Strom *S, long klic, long zmprior, size_t Vel, long MaxVel, long (*Ven)( long *vsiz ) );
/* NULL - neni RAM, jinak vrati, kde alokoval buf. */
/* nesmi existovat - zalozime novy */
/* v TipVen funkce pro vyhazovani (nepovinna) */

void *InsUzel( Strom *S, long klic, long zmprior, Flag zmabsol, size_t Vel, long MaxVel, Flag *UzJe, long (*Ven)( long *vsiz ) );
/* NULL - neni RAM, jinak vrati, kde alokoval buf. */
/* pokud uzel existuje, zmeni mu prioritu (absolutne ci inkrementalne) */
/* pokud ne, tak je-li treba nekoho vyhodi (horsiho) */
/* a zavede novy uzel s danou prioritou */
int DelUzel( Strom *S, long Klic, size_t Vel ); /* velikost kvuli pocitani celk. velikosti */
void *NajdiUzel( Strom *S, long Klic );
void *ZmenPrior( Strom *S, long Klic, long zmprior, Flag ZmAbsol );

int ZacStromy( long VelPool );
void KonStromy( void );

long HloubkaStromu( Strom *S );
long PHloubkaStromu( Strom *S );
long PocetUzlu( Strom *S );

void TestStrom( void ); /* test celeho systemu */
int TestStromu( struct uzel *u, char *EBuf, long Min, long Max, long Prior );
/* v jakem rozmezi musi byt strom */
/* test jednoho stromu */

#endif
