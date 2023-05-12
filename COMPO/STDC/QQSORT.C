#include <macros.h>
#include "qqsort.h"

#define QQSwap(a,b) {long p=(long)*(a);*(a)=*(b);*(b)=(void *)p;}

void _qqsort( void **PP, void **EP, long (*CmpP)( void *H, void *h ) )
{
	long PEP=(long)EP;
	if( EP<=PP+2 )
	{
		if( EP==PP+2 )
		{
			if( CmpP(PP[0],PP[1])>0 ) QQSwap(PP,PP+1);
		}
	}
	else
	{
		void **P=PP+((EP-PP)>>1);
		EP--;
		if( CmpP(*EP,*P)<0 ) QQSwap(EP,P);
		if( CmpP(*PP,*P)<0 )
		{
			QQSwap(PP,P);
		}
		/* P je minimalni */
		ef( CmpP(*PP,*EP)>0 ) QQSwap(PP,EP);
		/* v PP je stredni - tj. pivot */
		if( EP==PP+2 ) /* jen tri prvky */
		{
			QQSwap(PP,P);
		}
		else
		{
			P=PP+1;
			do
			{
				for(;;)
				{
					if( P>EP ) goto TotBreak;
					if( CmpP(*PP,*P)<=0 ) break;
					P++;
				}
				for(;;)
				{
					if( CmpP(*PP,*EP)>0 )
					{
						QQSwap(P,EP);
						EP--,P++;
						break;
					}
					EP--;
					if( P>EP ) goto TotBreak;
				}
			}
			while( P<=EP );
			TotBreak:;
			_qqsort(PP,P,CmpP);
			_qqsort(P,(void **)PEP,CmpP);
		}
	}
}

void qqsort( void **PP, void **EP, long (*CmpP)( void *H, void *h ) )
{
	if( EP>PP ) _qqsort(PP,EP,CmpP);
}
