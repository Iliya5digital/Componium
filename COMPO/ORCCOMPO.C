/* Componium - orchestry */
/* SUMA 10/1992-3/1994 */

#include <macros.h>
#include <string.h>
#include <gem\interf.h>
#include "llhraj.h"
#include "ramcompo.h"
#include "digill.h"

LLHraj *AktHraj;

/* zapouzdreni orch. specific. povelu */

const char *OEditNastr( const char *N, Pisen *P ){return AktHraj ? AktHraj->EditSNastr(N,NULL,P) : N;}
Err OVymazNastroj( char *N, Pisen *P ){return AktHraj ? AktHraj->VymazNastr(N,P) : ECAN;}
Err OZacSNastr( void )
{
	return AktHraj ? AktHraj->ZacSNastr() : EOK;
}

Err OKonSNastr( void )
{
	Err ret;
	ret=AktHraj ? AktHraj->KonSNastr() : EOK;
	if( ret!=OK ) Chyba(ret);
	return ret;
}
Err OJinSNastr( LLHraj *PrepNa, LLHraj *PrepZ )
{
	Err ret;
	if( PrepZ )
	{
		ret=PrepZ->KonSNastr();if( ret<OK ) return ret;
	}
	return PrepNa->ZacSNastr();
}
const char **OSoupisNastroju( Banka B, Pisen *P )
{
	if( AktHraj )
	{
		const char **BV,**PV,**PVO;
		BV=AktHraj->SoupisSNastr(P);
		if( !BV ) return NULL;
		for( PVO=PV=BV; *PVO; PVO++ )
		{
			const char *N=*PVO;
			if( N ) if( !B || NajdiRetez(B,N)<0 ) *PV++=N;
		}
		*PV=NULL;
		return BV;
	}
	else return NULL;
}

/* operace s retezci */

const char *JmenoOrc( const char *Nam )
{
	static FileName ON;
	char *OE;
	strcpy(ON,NajdiNazev(Nam));
	OE=NajdiExt(ON);
	if( OE>ON && OE[-1]=='.' ) OE[-1]=0;
	return ON;
}

static void MezKonvert( char *s, int z, int d )
{
	while( *s )
	{
		if( *s==z ) *s=d;
		s++;
	}
}

void SetMezStrTed( TEDINFO *T, const char *Naz )
{
	SetStrTed(T,Naz);
	MezKonvert(T->te_ptext,' ','_');
}
const char *MezStrTed( TEDINFO *T )
{
	MezKonvert(T->te_ptext,' ','_');
	return StrTed(T);
}

void SetMIntTed( TEDINFO *T, long i )
{
	if( i<0 ) SetStrTed(T,"");
	else SetIntTed(T,i);
}
long MIntTed( TEDINFO *T )
{
	if( !*StrTed(T) ) return -1;
	else return IntTed(T);
}
