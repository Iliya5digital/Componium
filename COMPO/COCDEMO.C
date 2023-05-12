/* prehravani ulozenych COC souboru - vzor */
/* SUMA 3/1993-3/1993 */

#include <macros.h>
#include <stdlib.h>
#include <stdio.h>
#include <tos.h>
#include <dspbind.h>
#include <sndbind.h>
#include "digill.h"
#include "cocload.h"

Flag Falcon=True;

void *myalloc( long a, Flag F ){(void)F;return malloc(a);}
void myfree( void *a ){free(a);}
void *myallocSpc( long a, Flag F, long Idtf ){(void)F,(void)Idtf;return malloc(a);}
void myfreeSpc( void *a ){free(a);}
void *mallocSpc( size_t Kolik, long Idtf ){(void)Idtf;return malloc(Kolik);}
void freeSpc( void *Co ){free(Co);}

void DebugError( int Line, const char *File )
{
	(void)Line,(void)File;
}

int main()
{
	if( FGloballocksnd(CLK25K)>=0 )
	{
		FILE *f=fopen("F:\\COMPOGRA\\PREKLAD\\GRAVON.COC","rb");
		if( f )
		{
			if( NactiPisen(f,"F:\\COMPOGRA\\PREKLAD\\AVR\\*.AVR",CLK25K)>=0 )
			{
				if( ZacHrajPisen()>=0 )
				{
					while( !Bconstat(2) ){}
					Bconin(2),AbortHraje();
					KonHrajPisen();
				}
				PustPisen();
			}
			fclose(f);
		}
		FGlobalunlocksnd();
	}
	return 0;
}