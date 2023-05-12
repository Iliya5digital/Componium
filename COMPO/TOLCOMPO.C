/* Componium - obsluha ToolBoxu */
/* SUMA 9/1993-2/1994 */

#include <macros.h>
#include <stdlib.h>
#include <gem\multiitf.h>
#include "ramcompo.h"
#include "compo.h"

static int ToolHan=-1; /* LHan toolboxu */

static Flag ToolEditB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	EdModeO=EdMode=DEditace;DskMode();
	(void)Bt,(void)Dvoj,(void)Asc;return False;
}
static Flag ToolMazB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	EdModeO=EdMode=DMazani;DskMode();
	(void)Bt,(void)Dvoj,(void)Asc;return False;
}
static Flag ToolTonB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	Flag TestM=StavMenu(Menu,VSEHLAI);
	OznacMenu(Menu,VSEHLAI,True);
	DoMenu(TONINAI,False);
	OznacMenu(Menu,VSEHLAI,TestM);
	(void)Bt,(void)Dvoj,(void)Asc;return False;
}
static Flag ToolTempB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	Flag TestM=StavMenu(Menu,VSEHLAI);
	OznacMenu(Menu,VSEHLAI,True);
	DoMenu(TEMPOI,False);
	OznacMenu(Menu,VSEHLAI,TestM);
	(void)Bt,(void)Dvoj,(void)Asc;return False;
}
static Flag ToolTaktB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	Flag TestM=StavMenu(Menu,VSEHLAI);
	OznacMenu(Menu,VSEHLAI,True);
	DoMenu(TAKTI,False);
	OznacMenu(Menu,VSEHLAI,TestM);
	(void)Bt,(void)Dvoj,(void)Asc;return False;
}

static Flag ToolPisB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	DoMenu(ULOZDOI,False);
	(void)Bt,(void)Dvoj,(void)Asc;return False;
}
Flag ToolHrajB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	DoMenu(ZAHRPI,False);
	(void)Bt,(void)Dvoj,(void)Asc;return False;
}
static Flag AskBlokRepet( int LHan )
{
	int NR=AskRepetD(2,NULL);
	if( NR>=OK )
	{
		BlokRepetice(LHan,NR);
		IgnKresli(LHan);
		return True;
	}
	return False;
}

static Flag ToolRepB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	int LHan=VrchniSekv();
	if( LHan>=0 )
	{
		OknoInfo *OI=OInfa[LHan];
		if( OI->ZBStopa>=0 ) AskBlokRepet(LHan); /* existuje blok -> lze ho editovat */
	}
	(void)Bt,(void)Dvoj,(void)Asc;return False;
}
static Flag ToolPartB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	DoMenu(NOTYI,False);
	(void)Bt,(void)Dvoj,(void)Asc;return False;
}

static Flag ToolCtiB( OBJECT *Bt, Flag Dvoj, int Asc )
{
	PovolCinMenu(False);
	if( ZavriVse() )
	{
		DoMenu(OTEVRI,False);
	}
	PovolCinMenu(True);
	(void)Bt,(void)Dvoj,(void)Asc;
	return False;
}

Flag ToolChceTopnout( int LHan, int MX, int MY )
{
	if( MX>=0 && MY>=0 )
	{
		DialStiskAPust(LHan,False,False,MX,MY);
		return False; /* netopat */
	}
	return True;
}

static void SetState( int I, Flag Yes, int Mask )
{
	WindDial *OI=OInfa[ToolHan];
	OBJECT *O=&OI->D.Form[I];
	int OS=O->ob_state,NS=OS;
	if( !Yes ) NS&=~Mask;
	else NS|=Mask;
	if( NS!=OS )
	{
		WindRect W;
		O->ob_state=NS;
		ObjcRect(OI->D.Form,I,&W);
		SendDraw(ApId,Okna[ToolHan]->handle,&W);
	}
}

void ToolSelect( int I, Flag Yes )
{
	SetState(I,Yes,SELECTED);
}
void ToolDraw( int I )
{
	WindRect W;
	WindDial *OI=OInfa[ToolHan];
	ObjcRect(OI->D.Form,I,&W);
	SendDraw(ApId,Okna[ToolHan]->handle,&W);
}

void DskMode( void )
{
	if( ToolHan>=0 )
	{
		SetState(DSKEDITB,EdModeO==DEditace,SELECTED);
		SetState(DSKMAZB,EdModeO==DMazani,SELECTED);
	}
}

void CNastavDesktop( void )
{
	/* ve skutecnosti s desktopem nic nedela - oznaceni jen z historie */
	if( ToolHan>=0 )
	{
		WindDial *OI=OInfa[ToolHan];
		OBJECT *F=OI->D.Form;
		int LHan=VrchniSekv();
		Flag SekvEnab=LHan>=0;
		static const int Desks[]=
		{
			DSKTONB,DSKTAKTB,
			DSKEDITB,DSKTEMPB,
			DSKMAZB,DSKHRAJB,DSKPISB,DSKPARTB,
			0
		};
		const int *I;
		Flag DrawF=False;
		BegUpdate();
		for( I=Desks; *I; I++ )
		{
			if( Test(&F[*I],DISABLED)==SekvEnab ) DrawF=True;
			Set01(&F[*I],DISABLED,!SekvEnab);
		}
		{
			OknoInfo *OI=OInfa[LHan];
			Flag FE=SekvEnab && OI->ZBStopa>=0;
			if( DrawF ) Set01(&F[DSKREPB],DISABLED,!FE);
			else SetState(DSKREPB,!FE,DISABLED);
		}
		if( DrawF ) ToolDraw(0);
		EndUpdate();
	}
}

static Button ToolTlac[]=
{
	{ToolEditB,DSKEDITB},
	{ToolMazB,DSKMAZB},
	{ToolTonB,DSKTONB},
	{ToolTempB,DSKTEMPB},
	{ToolTaktB,DSKTAKTB},
	{ToolPisB,DSKPISB},
	{ToolHrajB,DSKHRAJB},
	{ToolCtiB,DSKCTIB},
	{ToolRepB,DSKREPB},
	{ToolPartB,DSKPARTB},
	{NULL}
};

static Flag LogZTool( int LHan )
{
	ToolHan=-1;
	return DialLogZ(LHan);
}

static Flag LogOTool( int LHan )
{
	WindDial *OI=malloc(sizeof(WindDial));
	WindRect FD;
	Okno *O=Okna[LHan];
	if( !OI ) {ChybaRAM();return False;}
	OInfa[LHan]=OI;
	OI->LHan=LHan;
	OI->D.Form=DialCopy(RscTree(TOOLBOX));
	if( !OI->D.Form ) {ChybaRAM();goto ZrusOpen;}
	OI->DS=RscTree(TOOLBOX);
	SetNazev(O,FreeString(TOOLBFS),False);
	O->WType=MOVER|NAME;
	GetWind(0,WF_FULLXYWH,&FD);
	OI->D.Form->ob_x=FD.x;
	OI->D.Form->ob_y=FD.y+4;
	if( !DialPLogO(LHan,ToolTlac,NULL,-1,NULL) )
	{
		DialFree(OI->D.Form);
		ZrusOpen:
		free(OI);
		OInfa[LHan]=NULL;
		return False;
	}
	O->Stisk=DialStiskAPust;
	O->LogOZavri=LogZTool;
	O->LzeTopnout=ToolChceTopnout;
	O->Znak=NULL; /* nechceme zadne klavesy, ani Esc */
	ToolHan=LHan;
	return True;
}

void OtevriTool( void )
{
	WindRect W;
	GetWind(0,WF_FULLXYWH,&W);
	if( W.y+W.h<400 ) ToolHan=-1;
	else
	{
		Otevri(LogOTool,NejsouOkna);
		VsePrekresli();
		DskMode();
	}
}

void ZavriTool( void )
{
	if( ToolHan>=0 )
	{
		Zavri(ToolHan);
		ToolHan=-1;
	}
}

int ToolOkno( void )
{
	return ToolHan;
}