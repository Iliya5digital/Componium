/* definuje konvence pro driver alokace pameti */
/* vznikl puvodne pro Gravona */
/* SUMA 5/1994 */

/* kazdy program se musi sam postarat o vhodnou implementaci */
/* vhodny pro SUMA\STDC\MEMPOOL.C, malloc/free */

#ifndef __MEMSPC
#define __MEMSPC


#include <stddef.h>

#ifdef __cplusplus
  extern "C" {
#endif
#define newSpc(type,idtf) ((type *)mallocSpc(sizeof(type),idtf))

void  *mallocSpc( size_t size, long Idtf );
void *reallocSpc( void *mem, size_t size, long Idtf );
void freeSpc( void *mem );

void SwapEndianL(int *xx);
void SwapEndianW(short *xx);

#ifdef __cplusplus
  };
#endif

#endif
