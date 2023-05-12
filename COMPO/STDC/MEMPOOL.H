/* sprava pameti */
/* SUMA 1993 */

#ifndef __MEMPOOL
#define __MEMPOOL

#ifndef __STDDEF
#include <stddef.h>
#endif

/* pozor freeitem a alokitem musi mit velikost na stejnem miste */

typedef struct freeitem
{
	long velikost; /* vcetne hlavicky */
	struct freeitem *next; /* ve volne pameti neni rezie! */
};

typedef struct alokitem
{
	long velikost; /* vel. vcetne rezie */
	long Buf[0]; /* alokovane nemaji seznam */
};

typedef struct
{
	struct freeitem *free;
	void *MPBuf;
	long Size;
} MemPool;

int MemZac( MemPool *P, long size );
void MemKon( MemPool *P );

void *MemAlloc( MemPool *P, size_t size, long Idtf );
void MemFree( MemPool *P, void *B );
void MemRelease( MemPool *P );

long MemVolno( MemPool *P );
long MemMax( MemPool *P );
long MemPoc( MemPool *P );

#endif
