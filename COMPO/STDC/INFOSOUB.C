/* prace s textovymi infosoubory */
/* SUMA 2/1993 */

#include "../macros.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "infosoub.h"
#include "memspc.h"

enum {Krok=32};

Flag NovInfo( const char *Soub, Info *D )
{
	D->Buf=(InfoPol *)mallocSpc(Krok*sizeof(*D->Buf),'In');
	D->Vyhrazeno=Krok;
	if( !D->Buf ) return False;
	D->Uzito=0;
	D->Zmena=False;
	strcpy(D->Soub,Soub);
	return True;
}
static Flag PriAlok( Info *D, size_t Unit )
{
	if( D->Vyhrazeno<D->Uzito+Unit )
	{
		size_t NV=(D->Vyhrazeno+Unit+Krok-1)/Krok*Krok;
		void *N=mallocSpc(NV*sizeof(*D->Buf),'In');
		if( !N ) return False;
		D->Vyhrazeno=NV;
		memcpy(N,D->Buf,D->Uzito*sizeof(*D->Buf));
		freeSpc(D->Buf);
		D->Buf=(InfoPol *)N;
	}
	return True;
}
static InfoPol *Pridej( Info *D, const InfoPol *R )
{
	InfoPol *r;
	if( !PriAlok(D,1) ) return NULL;
	r=&D->Buf[D->Uzito++];
	*r=*R;
	D->Zmena=True;
	return r;
}
InfoPol *NajdiIPol( const Info *D, const char *N, int Index )
{
	int i;
	InfoPol *R;
	for( i=0; (size_t)i<D->Uzito; i++ )
	{
		R=D->Buf+i;
		if( !strcmpi(R->Nazev,N) && --Index<0 )
		{
			return R;
		}
	}
	return NULL;
}

void FreeInfo( Info *D )
{
	if( D->Buf ) freeSpc(D->Buf),D->Buf=NULL;
}

char *GetText( const Info *D, const char *Naz, int Index )
{
	InfoPol *R=NajdiIPol(D,Naz,Index);
	if( R ) return R->Buf;
	else return NULL;
}

Flag VlozIPol( Info *D, const InfoPol *P, int Index )
{
	InfoPol *N=NajdiIPol(D,P->Nazev,Index);
	if( N )
	{
		if( !D->Zmena ) D->Zmena=strcmp(P->Buf,N->Buf);
		*N=*P;
		return True;
	}
	if( Index>0 )
	{
		N=NajdiIPol(D,P->Nazev,Index-1);
		if( !N ) return False; /* spatna hodn. Indexu */
		D->Zmena=True;
	}
	return Pridej(D,P)!=NULL;
}

Flag SetText( Info *D, const char *Naz, const char *Obs, int Index )
{
	InfoPol I;
	strcpy(I.Buf,Obs);
	strcpy(I.Nazev,Naz);
	return VlozIPol(D,&I,Index);
}

void MazIPolOd( Info *D, const char *Naz, int Index )
{
	int i;
	for( i=0; (size_t)i<D->Uzito; i++ )
	{
		InfoPol *R=&D->Buf[i];
		if( !strcmp(R->Nazev,Naz) && --Index<0 ) R->Nazev[0]=0,D->Zmena=True;
	}
}

static char *fgetline( char *B, int L, FILE *f )
{
	char *r=fgets(B,L,f);
	char *e;
	if( !r ) return r;
	e=B+strlen(B)-1;
	if( *e=='\n' ) *e=0;
	return r;
}
Flag CtiInfo( const char *Soub, Info *D )
{
	FILE *f;
	char Buf[256];
	InfoPol B;
	if( !NovInfo(Soub,D) ) return False;
	#ifndef __MSDOS__
		f=fopen(Soub,"r");
	#else
		f=fopen(Soub,"rt");
	#endif
	if( !f ) return True;
	while( fgetline(Buf,(int)sizeof(Buf),f) )
	{
		char *S=strchr(Buf,' ');
		if( !S ) continue;
		*S++=0;
		if( strlen(Buf)>=DelInfoNaz ) continue;
		if( strlen(S)>=DelInfoObs ) continue;
		strcpy(B.Nazev,Buf);
		strcpy(B.Buf,S);
		if( !Pridej(D,&B) ) goto Error;
	}
	fclose(f);
	D->Zmena=False;
	return True;
	Error:
	fclose(f);
	FreeInfo(D);
	return False;
}

Flag PisInfo( const char *Soub, Info *D )
{
	FILE *f;
	size_t i;
	#ifndef __MSDOS__
		f=fopen(Soub,"w");
	#else
		f=fopen(Soub,"wt");
	#endif
	if( !f ) return False;
	for( i=0; i<D->Uzito; i++ )
	{
		InfoPol *R=&D->Buf[i];
		if( *R->Nazev ) fprintf(f,"%s %s\n",R->Nazev,R->Buf);
	}
	fclose(f);
	D->Zmena=False;
	return True;
}

Flag KonInfo( Info *D )
{
	if( D->Zmena && !PisInfo(D->Soub,D) ) return False;
	FreeInfo(D);
	return True;
}

