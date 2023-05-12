/* Componium - vst/vyst operace */
/* SUMA, 9/1992-1/1994 */

#include "macros.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdc\infosoub.h"
#include "stdc\FILEUTIL.H"

#include "digill.h"
#include "ramcompo.h"
#include "compo.h"

#define DEMO 0 /* z kaz ukl d n¡ a importu */

#if !DEMO
	#define IMPORT_MID 1 /* jeçtØ nen¡ hotov */
#endif


Flag Demo=DEMO; /* pro ABOUT */

static Err Neum;
static Flag NeumSet;

static void strlncpy(char *buf, const char *s, size_t size)
{
  strncpy(buf,s,size-1);
  buf[size-1] = 0;
}
static int NeumimF( const char *Slovo, SekvKursor *s )
{
	if( !NeumSet )
	{
    int ignore = 2;
		char Sl[40];
		FileName Nz;
		strlncpy(Sl,Slovo,sizeof(Sl));
		strlncpy(Nz,NajdiNazev(s->S->Pis->NazevS),sizeof(Nz));
    // TODO: MessageBox("TODO NeumimF");
		switch( ignore )
		{
			case 1: Neum=OK;NeumSet=True;break;
			case 2: Neum=OK;break;
			default: Neum=ERR;break;
		}
	}
	return Neum;
}

static Err PisSekv( FILE *f, Sekvence *S )
{
	int c;
	SekvText *s=OtevriT(S);
	if( !s ) return ERR;
	if( EOF==fprintf(f,"%s\n{\n",ZBanky(S->Pis->NazvySekv,S->INazev)) ) return ERR;
	while( (c=CtiT(s))>=OK ) if( EOF==fputc(c,f) ) return ERR;
	if( EOF==fprintf(f,"\n}\n") ) return ERR;
	ZavriT(s);
	return OK;
}

Flag PisMelF( const char *Nam, Pisen *P )
{
	FILE *f;
	Sekvence *S,*H;
	int ret=OK;
	f=fopen(Nam,"w");
	if( !f ) goto Error;
	for( S=PrvniSekv(P); S; S=DalsiSekv(S) )
	{
		for( H=S; H; H=DalsiHlas(H) ) if( (ret=PisSekv(f,H))!=EOK ) break;
	}
	if( ret==EOK )
	{ /* jeste nazvy nastroju na klvesach */
		int i;
		for( i=0; i<NFNum; i++ )
		{
			if( EOF==fprintf(f,"#Instr %d %s\n",i,NasF(P,i)) ) {ret=ERR;break;}
		}
	}
	if( ret==EOK ) if( EOF==fprintf(f,"#Comment %s\n",P->Popis1) ) ret=ERR;
	if( ret==EOK ) if( EOF==fprintf(f,"#Comment %s\n",P->Popis2) ) ret=ERR;
	// TODO: if( ret==EOK ) ret=UlozLocaly(f,P);
	if( ret==EOK ) if( fclose(f)==EOF ) ret=ERR;
	else fclose(f);
	if( ret )
	{
		FileName FN;
		Error:
		strlncpy(FN,NajdiNazev(Nam),sizeof(FN));
    // TODO: error
		return False;
	}
	return True;
}

PathName PisenP="A:\\*.CPS";
static PathName PisenW;
static FileName PisenF="";

void NastavExtPisne( const char *E )
{
	strcpy(NajdiPExt(NajdiNazev(PisenP)),E);
}


static Flag DovolNazev( const char *W )
{
	W=NajdiNazev(W);
	if( strchr(W,'/') || strchr(W,'-') || strchr(W,':') ) return False;
	return True;
}

void VymazSekvenci( Sekvence *S )
{
  // TODO: update views as needed
  #if 0
	int LHan;
	for( LHan=NMaxOken-1; LHan>=0; LHan-- ) if( Okna[LHan] ) if( OknoJeSekv(LHan) )
	{ /* zavreni otevrenych oken */
		OknoInfo *OI=OInfa[LHan];
		if( SekvenceOI(OI)==S ) FZavri(LHan);
	}
  #endif
	MazSekv(S);
}

char *GetS( FILE *f )
{
	static char b[80];
	char *ret;
	ret=fgets(b,(int)sizeof(b),f);
	if( ret )
	{
		b[strlen(b)-1]=0;
		return b;
	}
	return ret;
}

static void Ignorovat( void ) {}

static const char CFGNam[]="COMPO.INF";

static Info ComInfo;

static const char PisNaz[]="Composition";
/*static const char PlyNaz[]="Output";*/

static void UklidSamply( void )
{
	FAktHNastr();
}

Err CtiCFG( void )
{
	return EOK;
}

void UlozCFG( void )
{
	Flag ret=True;
	Flag Pis=False;
	/*
	if( ret )
	{
		if( !SetText(&ComInfo,PlyNaz,AktHraj==&Midi ? "MIDI" : "Internal",0) ) ret=False;
	}
	*/
	if( ComInfo.Zmena && !PisInfo(ComInfo.Soub,&ComInfo) )
	{
		// TODO: cfg write error
	}
}

void KonCFG( void )
{
	FreeInfo(&ComInfo);
}
