/* Componium - alokace pameti */
/* SUMA, 9/1992-3/1996 */

#include "macros.h"
#include <string.h>
#include <stdlib.h>

#include "utlcompo.h"
#include "stdc/memSpc.h"


void *myallocSpc( long size, Flag Trvale, long Idtf )
{
	void *P=malloc(size);
	return P;
}
void myfreeSpc( void *P )
{
	free(P);
}

void *mallocSpc( size_t Kolik, long Idtf )
{
	(void)Idtf;
	return myallocSpc(Kolik,False,Idtf);
}
void freeSpc( void *Co )
{
	myfreeSpc(Co);
}
void *myalloc( long Kolik, Flag Trvale )
{
	return myallocSpc(Kolik,Trvale,'____');
}
void myfree( void *Co )
{
	myfreeSpc(Co);
}
