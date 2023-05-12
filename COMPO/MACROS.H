/* general purpose macros */

#if !defined(__GPMACROS)
#define __GPMACROS

#include <stdlib.h>
#include <assert.h>
#include <memory.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long lword;

enum 
{ False, True };

typedef int Flag;

typedef char PathName[256];
typedef char FileName[32];

typedef struct
{
  int x1,y1,x2,y2;
} ClipRect;

typedef struct
{
  int x,y,w,h;
} WindRect;

#ifndef __cplusplus
#define new(type) ((type *)malloc(sizeof(type)))
#endif

#ifndef __cplusplus

#define For(Type,Var,Init,End,Step) \
{ \
  Type Var;Type EF=End; \
  for( Var=Init; Var<EF; Step ) \
  {
#define NextFor } }
#define ef else if


#if defined __DEBUG
#define equ(a,b) \
    if( sizeof(a) == sizeof(b) ) memcpy( &(a), &(b), sizeof(a) )
#else
#define equ(a,b) \
    memcpy( &(a), &(b), sizeof(a) )
#endif

#define fill(a,b) memset( (a), (b), sizeof(a) )
#define cmp(a,b)  memcmp( &(a), &(b), sizeof(a) )

//#define wsizeof(a) (sizeof(a)/2)
#define diff(a,b)  ((byte *)(a)-(byte *)(b))
#define endof(a)   ((byte *)(a)+sizeof(a))

#endif

#define lenof(a)   (sizeof(a)/sizeof(*(a)))

#if defined __DEBUG || _DEBUG

#ifdef __cplusplus
extern "C" 
{
  #endif
  void DebugError( int Line, const char *File );
  #ifdef __cplusplus
};

#endif

#define Plati(flag) do {if(!(flag)) __debugbreak();} while (false)
#define PlatiProc(proc,cond) Plati( (proc) cond )
#else
#define Plati(flag)
#define PlatiProc(proc,cond) proc
#endif

#define ERRP ( (void *)-1 )

long SystemCas( void ); /* cte syst. promennou */

#endif
