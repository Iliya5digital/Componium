/* Componium - zachazeni se seznamy retezcu */
/* SUMA, 9/1992-1/1993 */

#include "macros.h"
#include <string.h>
#include <stdlib.h>

#include "utlcompo.h"

char *Duplicate( const char *S )
{
	char *s=new char[strlen(S)+1];
	if( !s ) return s;
	strcpy(s,S);
	return s;
}

void NovaBanka( Banka B )
{
	BankaIndex i;
	for( i=0; i<DelkaBanky; i++ )
	{
		BankaItem *b=&B[i];
		b->Retez=NULL;
		b->Pouziti=0;
	}
}

void ZrusBanku( Banka B )
{
	#ifdef __DEBUG
		For( BankaItem *, b, B, &B[DelkaBanky], b++ )
			char *s=b->Retez;
			Plati(!s);
		Next
	#endif
}

BankaIndex DalsiVBance( const BankaItem *B, BankaIndex i )
{
	for( i++; i<DelkaBanky; i++ )
	{
		const BankaItem *b=&B[i];
		const char *s=b->Retez;
		if( s ) return i;
	}
	return ERR;
}
BankaIndex MaxVBance( const BankaItem *B )
{
	int i;
	for( i=DelkaBanky-1; i>=0; i-- )
	{
		const BankaItem *b=&B[i];
		const char *s=b->Retez;
		if( s ) return i;
	}
	return -1;
}

BankaIndex PrvniVBance( const BankaItem *B ) {return DalsiVBance(B,-1);}

int SpoctiBanku( const BankaItem *B )
{
	BankaIndex i;
	int N=0;
	for( i=PrvniVBance(B); i>=OK; i=DalsiVBance(B,i) ) N++;
	return N;
}

BankaIndex NajdiRetez( const BankaItem *B, const char *S )
{
	BankaIndex i;
	for( i=0; i<DelkaBanky; i++ )
	{
		const BankaItem *b=&B[i];
		const char *s=b->Retez;
		if( s && !strcmp(s,S) ) return i;
	}
	return ERR;
}

BankaIndex PridejRetez( BankaItem *B, const char *S )
{
	BankaIndex i=NajdiRetez(B,S);
	if( i>=0 )
	{
		BankaItem *b=&B[i];
		if( b->Pouziti>=MaxPouziti ) return ERAM;
		b->Pouziti++;
		return i;
	}
	for( i=0; i<DelkaBanky; i++ )
	{
		BankaItem *b=&B[i];
		if( !b->Retez )
		{
			char *s;
			s=Duplicate(S);
			if( !s ) return ERAM;
			b->Retez=s;
			b->Pouziti=1;
			return i;
		}
	}
	return ERAM;
}

Flag ZrusRetez( BankaItem *B, BankaIndex i )
{
	BankaItem *b=B+i;
	b->Pouziti--;
	Plati( b->Pouziti>=0 );
	if( b->Pouziti<=0 )
	{
		void *f=b->Retez; /* byly potize s optimalizerem */
		b->Retez=NULL;
		myfree(f);
		return True;
	}
	else return False;
}

Flag JeVBance( const BankaItem *B, BankaIndex i )
{
	const BankaItem *b=&B[i];
	return b->Retez!=NULL;
}
char *ZBanky( const BankaItem *B, BankaIndex i )
{
	const BankaItem *b=&B[i];
	#if __DEBUG
		Plati(b->Retez);
		Plati(b->Pouziti>0);
		if( !b->Retez ) return "";
	#endif
	return b->Retez;
}

Err ZmenRetez( Banka B, BankaIndex i, const char *D )
{
	char *s=Duplicate(D);
	BankaItem *b=&B[i];
	if( !s ) return ERAM;
	myfree(b->Retez);
	b->Retez=s;
	return OK;
}

