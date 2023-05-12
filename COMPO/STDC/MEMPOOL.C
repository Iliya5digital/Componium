/* sprava pameti */
/* SUMA 11/1993 */

#include <stdlib.h>
#include <limits.h>
#include "mempool.h"

#include <macros.h>
#include <stdio.h>

#define REPORT 0

void *MemAlloc( MemPool *P, size_t size, long Idtf )
{
	/* best match */
	long mdif=LONG_MAX;
	struct freeitem **i,**b=NULL;
	struct freeitem *A;
	struct alokitem *a;
	size+=sizeof(struct alokitem)+3;
	size&=~3; /* long align */
	#if __DEBUG
		#if REPORT
		{
			char Id[5];
			char Buf[80];
			Id[0]=(char)(Idtf>>24);
			Id[1]=(char)(Idtf>>16);
			Id[2]=(char)(Idtf>>8);
			Id[3]=(char)(Idtf>>0);
			Id[4]=0;
			sprintf(Buf,"Malloc %4s: %6ld",Id,size);
			DebugError(0,Buf);
		}
		#endif
		size+=sizeof(struct alokitem)*2;
	#endif
	if( size<sizeof(struct freeitem) ) size=sizeof(struct freeitem);
	for( i=&P->free; *i; i=&(*i)->next )
	{
		long dif=(*i)->velikost-size;
		if( dif>=0 )
		{
			if( dif==0 ) {b=i;break;}
			if( dif>sizeof(struct freeitem) && mdif>dif )
			{
				mdif=dif;
				b=i;
			}
		}
	}
	if( !b ) return NULL;
	A=*b;
	if( A->velikost>size ) /* urcite zbyva aspon sizeof(freeitem) */
	{
		struct freeitem *next=A->next;
		long vel=A->velikost;
		a=(struct alokitem *)A;
		a->velikost=size;
		A=(struct freeitem *)( ((char *)A)+size );
		*b=A;
		A->velikost=vel-size;
		A->next=next;
	}
	else /* presne sedici blok */
	{
		*b=A->next;
		a=(struct alokitem *)A;
		a->velikost=size;
	}
	#if __DEBUG
		(++a)->velikost='MGAL';
		(++a)->velikost=Idtf;
	#else
		(void)Idtf;	
	#endif
	return ++a;
}

void MemFree( MemPool *P, void *B )
{
	struct freeitem **i;
	struct alokitem *A=B;
	struct freeitem *L=NULL;
	#if __DEBUG
		A-=2;
		if( A->velikost!='MGAL' )
		{
			char Buf[80];
			sprintf(Buf,"!Free %lx %s",(long)(A+2),__FILE__);
			DebugError(__LINE__,Buf);
			return;
		}
	#endif
	A--;
	/* free bloky usporadane vzestupne podle adresy */
	for( i=&P->free; ; )
	{
		struct freeitem *I=*i;
		if( !I || (char *)I>=(char *)A )
		{
			if( L && ((char *)L)+L->velikost==(char *)A )
			{
				/* napojit na predchazejiciho */
				L->velikost+=A->velikost;
			}
			else
			{
				L=(struct freeitem *)A;
				Plati( A->velikost>=sizeof(struct freeitem) );
				*i=L;
				/* pozor - L a A se prekryvaji - velikost na stejnem miste */
				L->next=I;
			}
			/* ted jeste spojit */
			Plati( L );
			if( L->next )
			{
				if( (char *)L->next==((char *)L)+L->velikost ) /* napojit na dalsiho */
				{
					L->velikost+=L->next->velikost;
					L->next=L->next->next;
				}
			}
			break;
		}
		L=I;
		i=&(*i)->next;
	}
}

int MemZac( MemPool *P, long size )
{
	P->MPBuf=P->free=malloc(size);
	if( !P->free ) return -1;
	P->free->velikost=size;
	P->free->next=NULL;
	P->Size=size;
	return 0;
}

void MemRelease( MemPool *P )
{
	P->free=P->MPBuf;
	P->free->velikost=P->Size;
	P->free->next=NULL;
}

void MemKon( MemPool *P )
{
	free(P->MPBuf);
}

long MemVolno( MemPool *P )
{
	long r=0;
	struct freeitem *i;
	for( i=P->free; i; i=i->next ) r+=i->velikost;
	return r;
}
long MemPoc( MemPool *P )
{
	long r=0;
	struct freeitem *i;
	for( i=P->free; i; i=i->next ) r++;
	return r;
}

long MemMax( MemPool *P )
{
	long r=0;
	struct freeitem *i;
	for( i=P->free; i; i=i->next )
	{
		if( i->velikost>r ) r=i->velikost;
	}
	return r;
}

