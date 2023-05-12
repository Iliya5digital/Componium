/* DYNPOLE.H - kompaktni pole s promennou velikosti */
/* SUMA 3/1993-3/1993 */

#ifndef __DYNPOLE
#define __DYNPOLE

#include "../MACROS.H"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	long VelPrvku; /* zbytek musi byt nastaven pred volanim ZacDyn */
	long AlokKrok;
	void *(*DPmalloc)( lword am );
	void (*DPfree)( void *mem );
	void *Obs;
	long AlokDel,UzitDel;
} DynPole;

void ZacDyn( DynPole *D );
void KonDyn( DynPole *D );
/* chyba - kod < 0 */
long AlokDyn( DynPole *D, long NPrvku ); /* predem si naalokujem potr. pocet */

long InsDyn( DynPole *D, const void *Prvek ); /* pri lib. opci muze dojit k chybe - i pri mazani! */
long NewDyn( DynPole *D );

int DelDyn( DynPole *D, long Ind ); /* po mazani neplati stare indexy! */

void CopyDyn(DynPole *D, const DynPole *S);

#define AccDyn(D,I) ( (void*)((char *)(D)->Obs+(I)*(D)->VelPrvku) )
#define NDyn(D) ( (D)->UzitDel )

/* Prvni ani dalsi nezkoumaji skut. obsazeni - jen NDyn */

void *PrvniDyn( const DynPole *D, long *I ); /* pri chybe vraci NULL */
void *DalsiDyn( const DynPole *D, void *A, long *I );

#ifdef __cplusplus
};
#endif

#endif
