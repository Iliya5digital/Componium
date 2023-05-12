/* Componium - obsluha nastrojove listy */
/* SUMA 2/1994-2/1994 */

#include <macros.h>
#include <stdlib.h>
#include <string.h>
#include <gem\multiitf.h>

#include "ramcompo.h"
#include "compo.h"

static int ToNHan=-1; /* LHan */

void SetToNastroj( int I, const char *Naz, Flag DrawS )
{
	OBJECT *T;
	int IT;
	Flag Draw=False;
	if( ToNHan>=0 )
	{
		WindDial *OI=OInfa[ToNHan];
		T=OI->D.Form;
	}
	else T=RscTree(TONASD);
	IT=TON1B+(TON2B-TON1B)*I;
	if( IT<=TON10B )
	{
		char Buf[60];
		char BufK[60];
		strcpy(Buf," ");
		strcpy(Buf+1,ZkratNastr(BufK,Naz,9));
		Buf[10]=0;
		if( strcmp(StrTed(TI(T,IT+TON1N-TON1B)),Buf) )
		{
			Draw=True;
			SetStrTed(TI(T,IT+TON1N-TON1B),Buf);
		}
	}
	if( ToNHan>=0 && Draw && DrawS )
	{
		WindRect W;
		WindDial *OI=OInfa[ToNHan];
		ObjcRect(OI->D.Form,IT,&W);
		SendDraw(ApId,Okna[ToNHan]->handle,&W);
	}
}

void ZobrazToN( void )
{
	int LHan=VrchniSekv();
	OBJECT *T;
	OknoInfo *SI=LHan>=0 ? OInfa[LHan] : NULL;
	if( ToNHan>=0 )
	{
		WindDial *OI=OInfa[ToNHan];
		T=OI->D.Form;
	}
	else T=RscTree(TONASD);
	{
		Pisen *P=SI ? SekvenceOI(SI)->Pis : NULL;
		int i;
		Flag Draw=False;
		for( i=0; i<10; i++ )
		{
			int IT=TON1B+(TON2B-TON1B)*i;
			char Buf[60];
			if( P )
			{
				char BufK[60];
				strcpy(Buf," ");
				strcpy(Buf+1,ZkratNastr(BufK,NasF(P,i),9));
				Buf[9]=0;
			}
			else Buf[0]=0;
			if( strcmp(StrTed(TI(T,IT+TON1N-TON1B)),Buf) )
			{
				Draw=True;
				SetStrTed(TI(T,IT+TON1N-TON1B),Buf);
			}
		}
		if( Draw && ToNHan>=0 )
		{
			ForceKresli(ToNHan);
		}
	}
}


static Flag ToNStisk( OBJECT *Bt, Flag Dvoj, int Asc )
{
	WindDial *OI=OInfa[ToNHan];
	OBJECT *D=OI->D.Form;
	int I=(int)(Bt-D);
	int LI=(I-TON1B)/(TON2B-TON1B);
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		Pisen *P=SekvenceOI(OI)->Pis;
		WindRect SR;
		ObjcRect(D,I,&SR);
		ZmenNasF(P,LI,&SR,FreeString(PRINASFS));
	}
	(void)Dvoj,(void)Asc;
	return False;
}

static Flag ToNChceTopnout( int LHan, int MX, int MY )
{
	if( MX>=0 && MY>=0 )
	{
		DialStiskAPust(LHan,False,False,MX,MY);
		return False; /* netopat */
	}
	return True;
}

static Flag LogZToN( int LHan )
{
	ToNHan=-1;
	return DialLogZ(LHan);
}

static Button ToNTlac[]=
{
	#define TonIB(i) ToNStisk,TON1B+(TON2B-TON1B)*i
	TonIB(0),TonIB(1),TonIB(2),TonIB(3),TonIB(4),
	TonIB(5),TonIB(6),TonIB(7),TonIB(8),TonIB(9),
	NULL,
};

static Flag LogOToN( int LHan )
{
	WindDial *OI=malloc(sizeof(WindDial));
	WindRect FD;
	Okno *O=Okna[LHan];
	if( !OI ) {ChybaRAM();return False;}
	OInfa[LHan]=OI;
	OI->LHan=LHan;
	OI->D.Form=DialCopy(RscTree(TONASD));
	if( !OI->D.Form ) {ChybaRAM();goto ZrusOpen;}
	OI->DS=RscTree(TONASD);
	SetNazev(O,FreeString(TONFS),False);
	O->WType=MOVER|NAME;
	GetWind(0,WF_FULLXYWH,&FD);
	OI->D.Form->ob_x=FD.w; /* pozdeji se to zaokrouhli */
	OI->D.Form->ob_y=FD.h;
	if( !DialPLogO(LHan,ToNTlac,NULL,-1,NULL) )
	{
		DialFree(OI->D.Form);
		ZrusOpen:
		free(OI);
		OInfa[LHan]=NULL;
		return False;
	}
	O->Stisk=DialStiskAPust;
	O->LogOZavri=LogZToN;
	O->LzeTopnout=ToNChceTopnout;
	O->Znak=NULL; /* nechceme zadne klavesy, ani Esc */
	ToNHan=LHan;
	return True;
}

void OtevriToN( void )
{
	WindRect W;
	GetWind(0,WF_FULLXYWH,&W);
	if( W.w<600 ) ToNHan=-1;
	else
	{
		if( ToNHan<0 ) Otevri(LogOToN,NejsouOkna);
		else Topni(ToNHan);
		VsePrekresli();
	}
}

void ZavriToN( void )
{
	if( ToNHan>=0 )
	{
		Zavri(ToNHan);
		ToNHan=-1;
	}
}

int ToNOkno( void )
{
	return ToNHan;
}
