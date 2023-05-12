/* Componium - kresleni z vnitrniho formatu */
/* SUMA, 9/1992-10/1993 */

#include "../stdafx.h"
#include "macros.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <algorithm>

#include "spritem.h"
#include "noteImg.h"
#include "ramcompo.h"
#include "../ComponiumView.h"

using std::max;
using std::min;

typedef struct
{
  StrPosT Pos;
  WindRect WR;
} PProt; /* pro vyhledavani kursoru */

enum {NMaxItems=200};
static PProt Prot[NMaxStop][NMaxItems];
static int PPC[NMaxStop];

static Flag TedJeKursor;

enum {NOff=3}; /* pocet vrstev ve stope (horni, osnova, dolni) */

typedef struct
{
  int xpos; /* xpos kon. posl. jedn. - na xcas */
  int zpos; /* xpos zac. posl. jednotky - na zcas */
  CasT zcas;
  CasT xcas;
  Flag synchro;
} OffInfo;

enum tramsmer {TJedno,TDolu,TNahoru,TRadDolu,TRadNahoru} ; /* kterìm smØrem mus¡ dostat nohu */

typedef struct
{
  OffInfo i[NOff];
  CasT xcasspol; /* nejvyssi ze vsech kanalu */
  Err stat;
  int PredTon; /* tonina pred ctenim teto jedn. */
  int ZBlok,KBlok; /* graf. sour. bloku */
  byte PoslBlok;
  PosT RelPos; /* pocet dosud nakr. jednotek - offset */
  Flag PridanTram; /* pracovn¡ pý¡znak */
  int TramX,TramY; /* souýadnice tØl¡Ÿka noty, kter  chce tr mek, <0 znamen  § dnì tr mek */
  int NTram; /* Ÿetnost tr mku */
  tramsmer TramSmer;
} GrfStatus;

enum {MaxVec=16};
typedef struct
{
  JednotkaBuf t; /* vsechny param. */
  WindRect *W; /* kde se nakreslily */
  Flag JeKursor; /* zda na ni je kursor */
} VNota;

typedef struct
{
  VNota v[MaxVec];
  int n; /* pocet not ve vektoru */
} Vector; /* buffer na vektory not */

static Vector TonVec[NMaxStop];

static int xmax;
static GrfStatus KIStat[NMaxStop];
static StrPosT LPos[NMaxStop];
static OknoInfo *KOI; /* par. se kopiruje do glob. prom */

static word *PracScr;

static int PLHan=-1;
static WindRect PKR;

static int UpravProp( SekvSoubor *s, CasT p )
{
  if( p<0 ) return 0;
  p/=(TempoMin/TempoS(s));
  {
    long a=0;
    int rot=0;
    long Mez=12;
    while( p>=Mez )
    {
      a+=12;
      p-=Mez;
      Mez<<=1;
      rot+=1;
    }
    p=a+(p>>rot);
  }
  if( p>=xmax/2 ) p=xmax/2;
  if( p>=96 ) p=96;
  return (int)p;
}

static int PridejTaktKont( Kontext *K )
{
  int c=0;
  int vp=K->VTaktu;
  while( vp>=K->Takt ) vp-=K->Takt,c++;
  return c;
}

static int TiskCislaT( int xpos, int ypos, Kontext *K )
{
  char h[16];
  sprintf(h,"%d",K->KteryTakt+1+PridejTaktKont(K));
  return 0; // TiskS8(off,h,xpos)+4;
}
static int KrDvojCara( int xpos, int ypos, Kontext *K )
{
  int Prop=TiskCislaT(xpos,ypos,K);
  Draw(xpos-4,ypos-yosnova,&TaktSpr);
  Draw(xpos-2,ypos-yosnova,&TaktSpr);
  if( Prop<10 ) Prop=10;
  return xpos+Prop;
}
static int KrZacOktav( int xpos, int ypos )
{
  Draw(xpos,ypos-yosnova,&ZacOktSpr);
  return xpos+6;
}
static int KrKRep( int xpos, int ypos )
{
  Draw(xpos,ypos-yosnova,&EndRepSpr);
  return xpos+16;
}
static int KrZRep( int xpos, int ypos )
{
  Draw(xpos,ypos-yosnova,&ZacRepSpr);
  return xpos+16;
}


static int KrZnameni( int NZnam, int xpos, int ypos, const int *Poradi, Sprite *I )
{
  Plati(NZnam<=7);
  while( NZnam>0 && xpos<xmax )
  {
    int y=ypos-ystep*( *Poradi++ - TC )/2-ystep*9/2-2;
    Draw(xpos,y,I);
    Draw(xpos,y+ystep*Okt,I);
    NZnam--;
    xpos+=8;
  }
  return xpos;
}

static Flag UzDvojCara[NMaxStop];
static int UzRep[NMaxStop];
static int UzSek[NMaxStop];

static int KrTonina( int OKriz, int NKriz, int xpos, int Kan, Kontext *K )
{ /* NKriz<0 znamena becka */
  const static int PoradiK[7]={TF,TC,TG,TD,TA-Okt,TE,TH-Okt};
  const static int PoradiB[7]={TH-Okt,TE,TA-Okt,TD,TG-Okt,TC,TF-Okt};
  int ypos=kanypos(Kan);
  if( ( OKriz!=-128 || NKriz==0 ) && !UzDvojCara[Kan] )
  {
    xpos=KrDvojCara(xpos,ypos,K);
    UzDvojCara[Kan]=True;
  }
  if( OKriz==-128 ) OKriz=0;
  if( NKriz==-128 ) NKriz=0;
  if( NKriz>=0 )
  {
    if( OKriz>0 ) xpos=KrZnameni(OKriz-NKriz,xpos,ypos,&PoradiK[NKriz],&OdrazSpr);
    else if( OKriz<0 ) xpos=KrZnameni(-OKriz,xpos,ypos,PoradiB,&OdrazSpr);
    xpos=KrZnameni(NKriz,xpos,ypos,PoradiK,&KrizekSpr);
  }
  else if( NKriz<0 )
  {
    if( OKriz<0 ) xpos=KrZnameni(NKriz-OKriz,xpos,ypos,&PoradiK[-NKriz],&OdrazSpr);
    else if( OKriz>0 ) xpos=KrZnameni(OKriz,xpos,ypos,PoradiK,&OdrazSpr);
    xpos=KrZnameni(-NKriz,xpos,ypos,PoradiB,&BeckoSpr);
  }
  return xpos;
}

static int KrTempo( int xpos, int ypos, int Kan )
{
  (void)Kan;
  Draw(xpos-4,ypos-yosnova-2,&TempNotSpr);
  return xpos+10;
}

static int KrDvojCaraP( int xpos, int ypos, int Kan, Kontext *K )
{
  Flag *U2C=&UzDvojCara[Kan];
  if( !*U2C )
  {
    xpos=KrDvojCara(xpos,ypos,K);
    *U2C=True;
  }
  else xpos+=4;
  return xpos;
}

static int y2kan( int y )
{
  int k=(y+4+yosnova-yzac)/ydif;
  if( k<0 ) k=0;
  if( k>=NMaxStop ) k=NMaxStop-1;
  return k;
}

static int Zjistixpos( OknoInfo *OI, int FKan, int offs, CasT CasPos )
{
  enum {CDif=8};
  GrfStatus *KS=&KIStat[FKan];
  int Kan,xpos,of;
  xpos=KS->i[offs].xpos;
  for( Kan=0; Kan<OI->NStop; Kan++ )
  {
    GrfStatus *KS=&KIStat[Kan];
    for( of=0; of<NOff; of++ ) if( Kan!=FKan || of!=offs )
    {
      OffInfo *I=&KS->i[of];
      int xp;
      if( I->zcas==CasPos )
      {
        if( Kan!=FKan ) xp=I->zpos;
        else xp=I->zpos+2;
      }
      else if( I->xcas> CasPos ) xp=I->zpos+CDif;
      else if( I->xcas==CasPos ) xp=I->xpos;
      else /* xcas<CasPos */ xp=I->zpos+CDif;
      if( xpos<xp ) xpos=xp;
    }
  }
  return xpos;
}

int UrciEProp( JednItem *B )
{
  int Attr=B[1];
  int Prop;
  if( Attr&KBOMaskA ) Prop=8+16;
  else Prop=16;
  if( Attr&Leg ) Prop=Prop+10;
  if( Prop>24 ) Prop=24;
  return Prop;
}

void CComponiumView::UzavriTram( GrfStatus *KS )
{ /* nikdo to nechce dokonŸit, ale u§ je konec taktu */
  if( KS->TramX>=0 ) switch( KS->TramSmer )
  {
    case TDolu: case TRadDolu:
      switch( KS->NTram )
      {
        case 1: Draw(KS->TramX,KS->TramY+18,&Nota8DSpr);break;
        case 2: Draw(KS->TramX,KS->TramY+18,&Not16DSpr);break;
        case 3: Draw(KS->TramX,KS->TramY+18,&Not32DSpr);break;
      }
      KS->TramX=-1;
    break;
    default:
      switch( KS->NTram )
      {
        case 1: Draw(KS->TramX,KS->TramY,&Nota8HSpr);break;
        case 2: Draw(KS->TramX,KS->TramY,&Not16HSpr);break;
        case 3: Draw(KS->TramX,KS->TramY,&Not32HSpr);break;
      }
      KS->TramX=-1;
    break;
  }
}

void CComponiumView::KresliTramH( int x1, int y1, int x2, int y2, int Cet )
{
  #if 0
  long dy=(long)(y2-y1)<<16;
  int dx=x2-x1; /* x2>x1 */
  long ddy;
  long y=(long)y1<<16;
  byte *VA0=(byte *)PracScr;
  byte *VA=VA0+BPL*(long)y1; /* adresa ý dku */
  byte BMask=1<<(7-(x1&7));
  if( dy>0 ) ddy=(dy+0x10000L)/(dx+1);
  else if( dy<0 ) ddy=(dy-0x10000L)/(dx+1),y+=0xffff;
  else ddy=0;
  VA+=(x1>>3);
  for( ; x1<=x2; x1++ )
  {
    int gy=(int)(y>>16);
    if( gy<ScrH )
    {
      if( gy>=0 )
      { /* Ÿastì pý¡pad */
        byte *VAP;
        while( y1>gy ) VA-=BPL,y1--;
        while( y1<gy ) VA+=BPL,y1++;
        VAP=VA;
        *VAP|=BMask;VAP+=BPL;
        *VAP|=BMask;VAP+=BPL;
        *VAP|=BMask;
        if( Cet>1 )
        {
          int c;
          VAP+=2*BPL;
          for( c=1; c<Cet; c++ )
          {
            *VAP|=BMask;VAP+=BPL;
            *VAP|=BMask;VAP+=2*BPL;
          }
        }
      }
      else
      {
        byte *VAP;
        while( y1>gy ) VA-=BPL,y1--;
        while( y1<gy ) VA+=BPL,y1++;
        VAP=VA;
        if( VAP>=VA0 ) *VAP|=BMask;VAP+=BPL;
        if( VAP>=VA0 ) *VAP|=BMask;VAP+=BPL;
        if( VAP>=VA0 ) *VAP|=BMask;
        if( Cet>1 )
        {
          int c;
          VAP+=2*BPL;
          for( c=1; c<Cet; c++ )
          {
            if( VAP>=VA0 ) *VAP|=BMask;VAP+=BPL;
            if( VAP>=VA0 ) *VAP|=BMask;VAP+=2*BPL;
          }
        }
      }
    }
    y+=ddy;
    BMask>>=1;
    if( BMask==0 ) BMask=0x80,VA++;
  }
  (void)Cet;
  #endif
}

void CComponiumView::KresliTramD( int x1, int y1, int x2, int y2, int Cet )
{
  #if 0
  long dy=(long)(y2-y1)<<16;
  int dx=x2-x1; /* x2>x1 */
  long y=(long)y1<<16;
  long ddy;
  byte *VA=(byte *)PracScr+BPL*(long)y1; /* adresa ý dku */
  byte BMask=1<<(7-(x1&7));
  VA+=(x1>>3);
  if( dy>0 ) ddy=(dy+0x10000L)/(dx+1);
  else if( dy<0 ) ddy=(dy-0x10000L)/(dx+1),y+=0xffff;
  else ddy=0;
  for( ; x1<=x2; x1++ )
  {
    int gy=(int)(y>>16);
    if( gy>=0 && gy<ScrH )
    {
      byte *VAP;
      while( y1>gy ) VA-=BPL,y1--;
      while( y1<gy ) VA+=BPL,y1++;
      VAP=VA;
      *VAP|=BMask;VAP-=BPL;
      *VAP|=BMask;VAP-=BPL;
      *VAP|=BMask;
      if( Cet>1 )
      {
        int c;
        VAP-=2*BPL;
        for( c=1; c<Cet; c++ )
        {
          *VAP|=BMask;VAP-=BPL;
          *VAP|=BMask;VAP-=2*BPL;
        }
      }
    }
    y+=ddy;
    BMask>>=1;
    if( BMask==0 ) BMask=0x80,VA++;
  }
  (void)Cet;
  #endif
}

void CComponiumView::ZavedTram( GrfStatus *KS, int x, int y, int Cet, enum tramsmer Smer )
{
  if( !KS ) /* hned nakresli */
  {
    switch( Smer )
    {
      case TDolu: case TRadDolu:
        switch( Cet )
        {
          case 1: Draw(x,y+18,&Nota8DSpr);break;
          case 2: Draw(x,y+18,&Not16DSpr);break;
          case 3: Draw(x,y+18,&Not32DSpr);break;
        }
      break;
      default:
        switch( Cet )
        {
          case 1: Draw(x,y,&Nota8HSpr);break;
          case 2: Draw(x,y,&Not16HSpr);break;
          case 3: Draw(x,y,&Not32HSpr);break;
        }
      break;
    }
  }
  else
  {
    Flag TramSpojen=False;
    if( KS->PridanTram ) UzavriTram(KS),KS->PridanTram=False; /* na nov‚ho se neum¡me napojit */
    if( KS->TramX>=0 ) /* zkus to na nØkoho napojit */
    {
      if( KS->NTram==Cet && abs(y-KS->TramY)*2<=(x-KS->TramX) )
      {
        Flag Nahoru;
        if( KS->TramSmer==TNahoru && Smer!=TDolu ) Nahoru=True;
        else if( KS->TramSmer==TDolu && Smer!=TNahoru ) Nahoru=False;
        else if( Smer==TDolu && KS->TramSmer!=TNahoru ) Nahoru=False;
        else if( Smer==TNahoru && KS->TramSmer!=TDolu ) Nahoru=True;
        else if( Smer==TRadNahoru || KS->TramSmer==TRadNahoru ) Nahoru=True;
        else Nahoru=False;
        if( !Nahoru )
        {
          Draw(x,y+18,&Nota4DSpr);
          Draw(KS->TramX,KS->TramY+18,&Nota4DSpr);
          KresliTramD(KS->TramX,KS->TramY+42,x,y+42,Cet);
        }
        else
        {
          Draw(x,y,&Nota4HSpr);
          Draw(KS->TramX,KS->TramY,&Nota4HSpr);
          KresliTramH(KS->TramX+8,KS->TramY-1,x+8,y-1,Cet);
        }
        KS->TramX=-1;
        TramSpojen=True;
      }
    }
    if( !TramSpojen )
    {
      UzavriTram(KS);
      KS->TramX=x;
      KS->TramY=y;
      KS->NTram=Cet;
      KS->TramSmer=Smer;
      KS->PridanTram=True;
    }
  }
}

int CComponiumView::FKresliPau( WindRect *W, JednItem *B, int xpos, int ypos, int off )
{
  Sprite *I;
  int y;
  int DelVys,DelAtr;
  int Prop;
  AttrDelka(B[2],&DelVys,&DelAtr);
  switch( DelVys )
  {
    case 0: I=&Pauza1Spr; y=24;W->h=8;Prop=10; break;
    case 1: I=&Pauza1Spr; y=21;W->h=8;Prop=12; break;
    case 2: I=&Pauza4Spr; y=26;W->h=16;Prop=12; break;
    case 3: I=&Pauza8Spr; y=23;W->h=16;Prop=16; break;
    case 4: I=&Pauz16Spr; y=23;W->h=20;Prop=18; break;
    case 5: I=&Pauz32Spr; y=29;W->h=24;Prop=18; break;
    default: Plati(False); break;
  }
  W->x=xpos-2;
  W->w=12;
  W->y=ypos-y+off-2;
  Draw(xpos+2,W->y+2,I);
  if( DelAtr&Triola ) Draw(xpos+Prop,W->y+4,&TriolaSpr),Prop+=8;
  return Prop;
}

int CComponiumView::FKresliTonD( GrfStatus *KS, WindRect *W, JednItem *B, int xpos, int ypos, Flag Prapor, Flag Musi )
{
  Sprite *I;
  int DelVys,DelAtr;
  int PropPom;
  enum CPovel Druh=CPovel(B[0]);
  int y=ypos-ystep*(Druh-TC)/2;
  int Prop;
  enum tramsmer TS=Musi ? TDolu : TRadDolu;
  AttrDelka(B[2],&DelVys,&DelAtr);
  W->x=xpos-2;
  W->w=12;
  W->y=y+17;
  W->h=8;
  if( !Prapor && DelVys>2 ) DelVys=2; /* prapor nechceme */
  switch( DelVys )
  {
    case 0: I=&Nota1DSpr;Prop=12;break;
    case 1: I=&Nota2DSpr;Prop=12;break;
    case 2: I=&Nota4DSpr;Prop=12;break;
    case 3: I=NULL;ZavedTram(KS,xpos&~1,y,1,TS);Prop=16;break;
    case 4: I=NULL;ZavedTram(KS,xpos&~1,y,2,TS);Prop=16;break;
    case 5: I=NULL;ZavedTram(KS,xpos&~1,y,3,TS);Prop=16;break;
    default: Plati(False); break;
  }
  if( I ) Draw(xpos,y+18,I);
  if( Druh==TC+Okt ) Draw(xpos-4,y+21,&PomLinSpr);
  else if( Druh>=TA+NormOkt*Okt )
  { /* horni a, h */
    Draw(xpos-4,ypos-(ystep*(TA+NormOkt*Okt-TC+1))/2+24,&PomLinSpr);
  }
  if( DelAtr&Tecka )
  {
    int y=ypos-ystep*((Druh-TC+1)/2);
    Draw(xpos+12,y+21,&TeckaSpr);
    if( Prop<16 ) Prop=16;
  }
  PropPom=Prop;
  if( DelAtr&Triola )
  {
    Draw(xpos+Prop,y+18,&TriolaSpr);
    PropPom=Prop+8;
  }
  if( B[1]&Leg )
  {
    Draw(xpos+Prop-4,y+24,&LegatoSpr);
    PropPom=Prop+10; /* jeste delsi */
  }
  return PropPom;
}
int CComponiumView::FKresliTonH( GrfStatus *KS, WindRect *W, JednItem *B, int xpos, int ypos, Flag Prapor, Flag Musi )
{
  Sprite *I;
  int DelVys,DelAtr;
  enum CPovel Druh=CPovel(B[0]);
  int y=ypos-ystep*(Druh-TC)/2;
  int Prop,PropPom;
  enum tramsmer TS=Musi ? TNahoru : TRadNahoru;
  AttrDelka(B[2],&DelVys,&DelAtr);
  W->x=xpos-2;
  W->w=12;
  W->y=y+17;
  W->h=8;
  if( !Prapor && DelVys>2 ) DelVys=2; /* prapor nechceme */
  (void)KS;
  switch( DelVys )
  {
    case 0: I=&Nota1HSpr;Prop=12;break;
    case 1: I=&Nota2HSpr;Prop=12;break;
    case 2: I=&Nota4HSpr;Prop=12;break;
    case 3: I=NULL;ZavedTram(KS,xpos&~1,y,1,TS);Prop=16;break;
    case 4: I=NULL;ZavedTram(KS,xpos&~1,y,2,TS);Prop=16;break;
    case 5: I=NULL;ZavedTram(KS,xpos&~1,y,3,TS);Prop=16;break;
    default: Plati(False); break;
  }
  if( I ) Draw(xpos,y,I);
  if( Druh==TC+Okt ) Draw(xpos-4,y+21,&PomLinSpr);
  else if( Druh>=TA+NormOkt*Okt )
  { /* horni a, h */
    Draw(xpos-4,ypos-(ystep*(TA+NormOkt*Okt-TC+1))/2+24,&PomLinSpr);
  }
  PropPom=Prop;
  if( DelAtr&Tecka )
  {
    int y=ypos-ystep*((Druh-TC+1)/2);
    Draw(xpos+12,y+21,&TeckaSpr);
    if( PropPom<16 ) PropPom=16;
  }
  if( DelAtr&Triola )
  {
    Draw(xpos+Prop,y+18,&TriolaSpr);
    PropPom=Prop+8;
  }
  if( B[1]&Leg )
  {
    Draw(xpos+Prop,y+24,&LegatoSpr);
    PropPom=Prop+10; /* jeste delsi */
  }
  return PropPom;
}

static int FKresliKBO( JednItem *B, int xpos, int ypos )
{ /* vraci, kolik je treba vynechat na toto znamenko */
  Sprite *I;
  enum CPovel Druh=CPovel(B[0]);
  int y=ypos-ystep*(Druh-TC)/2;
  switch( B[1]&KBOMaskA )
  {
    case KrizekA: I=&KrizekSpr; goto KrZnam;
    case BeckoA: I=&BeckoSpr; goto KrZnam;
    case OdrazA: I=&OdrazSpr; goto KrZnam;
    KrZnam: Draw(xpos,y+2*ystep,I);return 8;
  }
  return 0;
}

int CComponiumView::FKresliTon( GrfStatus *KS, WindRect *W, JednItem *B, int xpos, int ypos )
{
  if( B[0]==PPau ) return FKresliPau(W,B,xpos,ypos,0);
  else if( B[0]-TC<TH+Okt )
  {
    int w=FKresliKBO(B,xpos,ypos);
    return FKresliTonH(KS,W,B,xpos+w,ypos,True,False)+w;
  }
  else
  {
    int w=FKresliKBO(B,xpos,ypos);
    return FKresliTonD(KS,W,B,xpos+w,ypos,True,False)+w;
  }
}

static int CmpVNota( const void *vv1, const void *vv2 )
{
  const VNota *v1 = (const VNota *)vv1, *v2 = (const VNota *)vv2;
  return v2->t[0]-v1->t[0];
}

#define CmpSwapV(v1,v2 ) {if( (v1)->t[0]<(v2)->t[0] ){VNota p=*(v1);*(v1)=*(v2);*(v2)=p;}}

void CComponiumView::SetridVec( Vector *V )
{
  switch( V->n )
  {
    case 0: case 1: return;
    case 2: CmpSwapV(&V->v[0],&V->v[1]);break;
    case 3:
      CmpSwapV(&V->v[0],&V->v[1]);
      CmpSwapV(&V->v[1],&V->v[2]);
      CmpSwapV(&V->v[0],&V->v[1]);
      break;
    default:
      qsort(V->v,V->n,sizeof(*V->v),CmpVNota);
      break;
  }
}

int CComponiumView::KresliVec( GrfStatus *KS, Vector *V, int xpos, int ypos )
{
  int W=0;
  KS->PridanTram=False;
  if( V->n==1 ) /* velmi casty pripad */
  {
    VNota *v=V->v;
    W=FKresliTon(KS,v->W,v->t,xpos,ypos); /* vcetne #/b/= */
    if( v->JeKursor ) OI.KR=*v->W;
  }
  else
  {
    int i;
    int MaxXW=0;
    int ADelka;
    int AVys=+300;
    Flag Horni=True;
    SetridVec(V);
    /* #/b/= u tonu */
    for( i=0; i<V->n; i++ )
    {
      VNota *v=&V->v[i];
      if( v->t[0]<PPau )
      {
        int XW=FKresliKBO(v->t,xpos,ypos);
        if( MaxXW<XW ) MaxXW=XW;
      }
    }
    xpos+=MaxXW;
    /* nejprve jsou pauzy */
    for( i=0; i<V->n; i++ )
    {
      VNota *v=&V->v[i];
      if( v->t[0]==PPau )
      {
        int w;
        w=FKresliPau(v->W,v->t,xpos,ypos,ystep*7);
        if( W<w ) W=w;
        if( v->JeKursor ) OI.KR=*v->W;
      }
      else break;
    }
    /* pak jsou tony - zhora dolu */
    ADelka=0;
    /* tony s nohami nahoru se obcas prostridaji s nohami dolu */
    for( ; i<V->n; i++ )
    {
      VNota *v=&V->v[i];
      int w;
      JednItem *B=v->t;
      int DelVys,DelAtr;
      AttrDelka(B[2],&DelVys,&DelAtr);
      if( DelVys==ADelka )
      {
        if( AVys-B[0]<Okt )
        {
          if( Horni ) w=FKresliTonH(KS,v->W,B,xpos,ypos,False,True);
          else w=FKresliTonD(KS,v->W,B,xpos,ypos,False,True);
        }
        else
        {
          if( AVys-B[0]>=Okt*3/2 ) Horni=True;
          if( Horni ) w=FKresliTonH(KS,v->W,B,xpos,ypos,True,True);
          else w=FKresliTonD(KS,v->W,B,xpos,ypos,True,True);
        }
      }
      else
      {
        if( ADelka>0 )
        {
          if( Horni ) Horni=False;
          else Horni=AVys-B[0]>=Okt;
        }
        ADelka=DelVys;
        if( Horni ) w=FKresliTonH(KS,v->W,B,xpos,ypos,True,True);
        else w=FKresliTonD(KS,v->W,B,xpos,ypos,True,True);
      }
      if( W<w ) W=w;
      if( v->JeKursor ) OI.KR=*v->W;
      AVys=B[0];
    }
    W+=MaxXW;
  }
  /* po nakreslen¡ t¢nu uzavýeme star‚ tr mky */
  if( !KS->PridanTram ) UzavriTram(KS);
  return W;
}

#define InitVec(V) ( (V)->n=0 )

int CComponiumView::KresliTon( GrfStatus *KS, WindRect *W, SekvSoubor *s, int xpos, int ypos, int Kan)
{
  JednItem *B=BufS(s);
  Vector *V=&TonVec[Kan];
  if( V->n<MaxVec )
  {
    VNota *v=&V->v[V->n++];
    v->W=W; /* zaznamenat WinRec - bude ho treba doplnit */
    EquJed(v->t,B);
    v->JeKursor=TedJeKursor;
  }
  if( !(B[1]&Soubezna) )
  {
    CasT Delka=DelkaSF(s);
    int PropSir=UpravProp(s,Delka);
    int Prop=KresliVec(KS,V,xpos,ypos);
    if( Prop>PropSir ) PropSir=Prop;
    InitVec(V); /* uz nakresleno doopravdy */
    return PropSir;
  }
  return 0;
}

static void KresliKomin( int ye, int x, int w, int h )
{
  #if 0
  while( --h>=0 )
  {
    HCara(off,x,x+w-1);
    off+=BPL;
  }
  #endif
}

static void TiskEff( int ye, int xpos, int EA )
{ /* nakresl¡ kom¡n */
  int H;
  int HK=7;
  H=(EA*HK+500)/1000;
  KresliKomin(ye,xpos,4,H+1);
}

static void KresliBlok( int Kan, int xz, int xk )
{
  int ypos=kanypos(Kan);
  if( xk>xmax ) xk=xmax;
  HBlok((ypos-ystep*6),xz,xk,ystep*12);
}

#define ToUpper(x) toupper(x)

#define TestCh(s) ( ToUpper((s)[-1])=='C' && ToUpper((s)[0])=='H' )

void strlncpy( char *d, const char *z, size_t l )
{
  --l;
  strncpy(d,z,l);
  d[l]=0;
}

const char *ZkratNastr( char *NK, const char *N, int Na )
{
  char *s,*sd;
  if( Na<=0 ) Na=8;
  if( (int)strlen(N)<=Na ) return N;
  strlncpy(NK,N,32);
  sd=NK+1;
  for(;;)
  {
    if( (s=strchr(sd,'_'))!=NULL )
    {
      if( s-sd>=4 )
      {
        if( TestCh(sd+3) ) sd++;
        sd[3]='.';
        sd+=4;
        strcpy(sd,s+1);
        sd++;
      }
      else sd=s+1;
    }
    else if( (s=sd+3,strlen(sd)>=5) )
    {
      if( TestCh(s) ) s++;
      *s++='.';
      *s=0;
      break;
    }
    else break;
  }
  return NK;
}

#define yhorr (ypos-yosnova+10)
#define hcele (KonecSpr.s_h-10)

static int OffsetP( enum CPovel P ) /* ve skut offset */
{
  switch( P )
  {
    case PNastroj: case PTempo: case POktava:
      return 1;
    case PHlasit: case PStereo: case PEfekt:
      return 2;
    default: /* sem patri i Rep, ERep, */
      return 0;
  }
}
static bool JeSynchro( JednItem Druh )
{
  if( Druh<=PPau ) return true;
  else if( Druh==PVyvolej ) return true;
  else return false;
}

#define YNasOff ( -4 )

void CComponiumView::KresliJedn( SekvSoubor *s, int xpos, int Kan, int offs )
{
  GrfStatus *KN=&KIStat[Kan];
  JednItem *SB=BufS(s);
  JednItem Druh=StatusS(s)!=EOK ? ERR : SB[0];
  int ypos=kanypos(Kan);
  int Prop;
  int xpos0=xpos;
  WindRect *W=NULL;
  Flag NeumeleP=Neumele(Druh);
  if( NeumeleP )
  {
    static WindRect Nouze;
    if( PPC[Kan]<NMaxItems ) W=&Prot[Kan][PPC[Kan]++].WR;
    else W=&Nouze; /* zamez pý¡stupu do NULL pointeru */
  }
  if( Druh<EOK ) /* konec */
  {
    UzavriTram(KN);
    Plati( W );
    W->x=xpos;
    W->y=yhorr;
    W->h=hcele;
    Draw(xpos-4,ypos-yosnova,&KonecSpr);
    Prop=TiskCislaT(xpos,ypos,AKontextS(s));
    if( Prop<12 ) Prop=12;
    W->w=Prop;
  }
  else if( Druh<=PPau ) /* ton */
  {
    Prop=KresliTon(KN,W,s,xpos,ypos,Kan);
    UzDvojCara[Kan]=False;
  }
  else /* Povel */
  {
    switch( Druh )
    {
      case PTakt:
        UzavriTram(KN);
        if( !UzDvojCara[Kan] )
        {
          if( Kan==0 )
          {
            int PropC=TiskCislaT(xpos,ypos,AKontextS(s));
            /* nahore */
            KIStat[Kan].i[1].xpos=xpos+PropC;
          }
          Draw(xpos-4,ypos-yosnova,&TaktSpr);
          Prop=10;
        }
        else Prop=0;
        break;
      case PDvojTakt:
        UzavriTram(KN);
        Plati( W );
        W->x=xpos;
        W->y=yhorr+6;
        W->w=8;
        W->h=hcele-6;
      case PDvojTaktUm: /* neni def. W */
        UzavriTram(KN);
        Prop=KrDvojCara(xpos,ypos,AKontextS(s))-xpos;
        UzDvojCara[Kan]=True;
        break;
      case PTonina:
      {
        int OKriz=KN->PredTon;
        if( CasJeS(s)>0 && OKriz==-128 ) OKriz=0;
        UzavriTram(KN);
        W->x=xpos-2;
        W->y=yhorr;
        W->h=hcele;
        Prop=KrTonina(OKriz,SB[1],xpos,Kan,AKontextS(s))-xpos;
        W->w=Prop+2;
        KN->PredTon=SB[1];
        if( SB[1]>=3 ) /* zasahuje i nahoru a dolu*/
        {
          KIStat[Kan].i[1].xpos=xpos+Prop;
        }
        KIStat[Kan].i[2].xpos=xpos+Prop;
        break;
      }
      case PNTakt:
      {
        int ye = (ypos-yosnova+16);
        char h[4],d[4];
        int xpo;
        sprintf(h,"%d",SB[1]);
        sprintf(d,"%d",SB[2]);
        UzavriTram(KN);
        xpo=KrDvojCaraP(xpos,ypos,Kan,AKontextS(s));
        TiskS(ye,h,xpo);
        TiskS(ye+12,d,xpo);
        TiskS(ye+36,h,xpo);
        TiskS(ye+48,d,xpo);
        Prop=xpo-xpos+10;
        W->x=xpos;
        W->y=yhorr+6;
        W->h=hcele-6;
        W->w=Prop;
        KIStat[Kan].i[2].xpos=xpos+Prop; /* zasahuje i dolu */
        break;
      }
      case PTempo:
      {
        int ye = (ypos-yosnova+YNasOff);
        char h[4];
        int xpo,xp;
        sprintf(h,"=%d",SB[1]);
        UzavriTram(KN);
        xpo=KrDvojCaraP(xpos,ypos,Kan,AKontextS(s));
        xp=KrTempo(xpo,ypos,Kan);
        Prop=TiskS(ye,h,xp)+xp-xpos+4;
        W->x=xpos;
        W->y=ypos-yosnova-2;
        W->h=16;
        W->w=Prop-4;
        break;
      }
      case POktava:
      {
        int ye = (ypos-yosnova+YNasOff+8);
        char h[16];
        int xpo;
        if( SB[2] ) sprintf(h,"%+d",SB[1]);
        else sprintf(h,"*%+d",SB[1]);
        xpo=KrZacOktav(xpos,ypos);
        Prop=xpo-xpos+TiskS8(ye,h,xpo)+4;
        W->x=xpos;
        W->y=ypos-yosnova-2;
        W->h=16;
        W->w=Prop-4;
        break;
      }
      case PNastroj:
      {
        int ye = (ypos-yosnova+YNasOff);
        const char *N=ZBanky(SekvenceS(s)->Pis->Nastroje,SB[1]);
        char BufNK[32];
        N=ZkratNastr(BufNK,N,0);
        Prop=PropS(ye,N,xpos)+8;
        W->x=xpos;
        W->y=ypos-yosnova-2;
        W->h=16;
        W->w=Prop-4;
        break;
      }
      case PHlasit:
      {
        int ye= (ypos+ystep*4-2);
        const char *N=HlasitText(SB[1]);
        Prop=TiskS8T(ye,N,xpos+1)+4;
        W->x=xpos;
        W->y=ypos+ystep*4-2;
        W->h=8;
        W->w=Prop-4;
      }
      break;
      case PStereo:
      {
        int ye = (ypos+ystep*4-2);
        char N[32];
        int LB=(SB[1]+( SB[1]>=0 ? 5 : -5) )/10;
        int PB=(SB[2]+( SB[2]>=0 ? 5 : -5) )/10;
        if( LB==PB ) sprintf(N,"%s%d",( LB<0 ? SterLS : SterPS),abs(LB));
        else
        {
          if( LB*PB>=0 )
          { /* odklon na jednu stranu */
            if( LB!=0 ) strcpy(N,LB<0 ? SterLS : SterPS);
            else strcpy(N,PB<0 ? SterLS : SterPS);
            sprintf(N+strlen(N),"%d-%d",abs(LB),abs(PB));
          }
          else
          { /* ruzna znamenka */
            sprintf(N,"%s%d",( LB<0 ? SterLS : SterPS),abs(LB));
            sprintf(N+strlen(N),"-%s%d",( PB<0 ? SterLS: SterPS ),abs(PB));
          }
        }
        Prop=TiskS8(ye,N,xpos)+4;
        W->x=xpos;
        W->y=ypos+ystep*4-2;
        W->h=8;
        W->w=Prop-4;
      }
      break;
      case PEfekt:
      {
        int ye=ypos+ystep*4-2;
        Prop=TiskS8(ye," ",xpos)+4;
        TiskEff(ye,xpos,SB[1]);
        TiskEff(ye,xpos+4,SB[2]);
        W->x=xpos;
        W->y=ye;
        W->h=8;
        W->w=Prop-4;
      }
      break;
      case PZacSekv:
      {
        BankaItem *B=SekvenceS(s)->Pis->NazvySekv;
        int ye = (ypos-yosnova+40);
        char N[40];
        int xpo;
        UzavriTram(KN);
        if( !NajdiSekvBI(SekvenceS(s)->Pis,SB[1],1) ) strcpy(N,ZBanky(B,SB[1]));
        else sprintf(N,"%s.%d",ZBanky(B,SB[1]),SB[2]+1);
        Draw(xpos,ypos-yosnova,&ZacSekSpr);
        xpo=xpos+16;
        UzDvojCara[Kan]=True;
        TiskSG(ye,N,xpo);
        xpo+=8;
        W->x=xpos;
        W->y=yhorr+20;
        W->h=hcele-32;
        W->w=(int)strlen(N)*8+xpo-xpos;
        Prop=xpo-xpos;
        break;
      }
      case PKonSekv:
      {
        int xpo;
        UzavriTram(KN);
        Draw(xpos,ypos-yosnova,&KonSekSpr);
        xpo=xpos+16;
        W->x=xpos;
        W->y=yhorr+6;
        W->h=hcele-6;
        W->w=xpo-xpos;
        Prop=xpo-xpos;
        break;
      }
      case PVyvolej:
      {
        BankaItem *B=SekvenceS(s)->Pis->NazvySekv;
        int ye = (ypos-yosnova+40);
        char N[40];
        int P;
        /* pokud NajdiSekv selze->NULL */
        UzavriTram(KN);
        if( !NajdiSekvBI(SekvenceS(s)->Pis,SB[1],1) ) strcpy(N,ZBanky(B,SB[1]));
        else sprintf(N,"%s.%d",ZBanky(B,SB[1]),SB[2]+1);
        Prop=TiskS(ye,N,xpos)+4;
        W->x=xpos;
        W->y=yhorr+6;
        W->h=hcele-6;
        W->w=Prop-4;
        P=UpravProp(s,CasJeS(s)-KN->xcasspol);
        if( P>Prop ) P=Prop;
        UzDvojCara[Kan]=False;
        break;
      }
      case PRep:
      {
        int xpo;
        int ye = (ypos-yosnova+44);
        char h[4];
        UzavriTram(KN);
        xpo=KrZRep(xpos,ypos);
        if( SB[1]!=2 )
        {
          int xp;
          sprintf(h,"%dx",SB[1]);
          xp=xpos+TiskS8(ye,h,xpos)+4;
          if( xp>xpo ) xpo=xp;
        }
        UzDvojCara[Kan]=True;
        Prop=xpo-xpos;
        W->x=xpos;
        W->y=yhorr+6;
        W->h=hcele-6;
        W->w=Prop;
        KIStat[Kan].i[2].xpos=xpos+Prop; /* zasahuje i dolu */
        break;
      }
      case PERep:
      {
        UzavriTram(KN);
        Prop=KrKRep(xpos,ypos)-xpos;
        W->x=xpos;
        W->y=yhorr+6;
        W->h=hcele-6;
        W->w=Prop;
        UzDvojCara[Kan]=True;
        KIStat[Kan].i[2].xpos=xpos+Prop; /* zasahuje i dolu */
        break;
      }
      case PUmPauza:
      {
        UzavriTram(KN);
        Prop=0;
        UzDvojCara[Kan]=True;
        break;
      }
      default: Plati(False); break;
    }
  }
  {
    GrfStatus *KS=&KIStat[Kan];
    if( TedJeKursor )
    {
      Plati( W );
      OI.KR=*W;
    }
    xpos+=Prop;
    if( KS->PoslBlok )
    {
      if( xpos0>KS->KBlok ) KS->KBlok=xpos0;
    }
    if
    (
      KS->PoslBlok && Druh==PUmPauza ||
      Druh!=PUmPauza && JeVBloku(Kan+OI.IStopa,&LPos[Kan],KS->i[offs].zcas)
    )
    { /* je blok */
      if( xpos0<KS->ZBlok ) KS->ZBlok=xpos0;
      if( xpos>KS->KBlok ) KS->KBlok=xpos;
      KS->PoslBlok=True;
    }
    else KS->PoslBlok=False;
    KS->i[offs].xpos=xpos;
    KS->i[offs].synchro=JeSynchro(Druh);
  }
}

static StrPosT PosKurs;

static void TestKurs( OknoInfo *OI, SekvSoubor *s, int Kan )
{
  enum CPovel Druh=(CPovel)BufS(s)[0];
  TedJeKursor=False;
  if( Neumele(Druh) || StatusS(s)!=OK )
  {
    StrPosT AktPos;
    AktPos=LPos[Kan];
    if( PPC[Kan]<NMaxItems ) 
    {
      PProt *PP=&Prot[Kan][PPC[Kan]];
      PP->Pos=AktPos;
    }
    if( Kan+OI->IStopa==OI->KKan )
    {
      if( !CmpPos(&PosKurs,&AktPos) ) TedJeKursor=True;
    }
  }
}

static void PreskocTakt( SekvSoubor *s )
{
  JednItem *B=BufS(s);
  if( StatusS(s)==OK && B[0]==PTakt && TestS(s)==OK )
  {
    switch( B[0] )
    {
      case PDvojTakt:
      case PTonina: case PNTakt: case PTempo:
        if( CasJeS(s)==0 ) CtiS(s);
        else B[0]=PDvojTaktUm;
        break;
      /*case PZacSekv:*/
      /*case PKonSekv:*/
      case PRep: case PERep:
        CtiS(s);
        break;
      default: B[0]=PTakt;
    }
  }
}

static void Osnova( int y )
{
  // draw clefs
  /*
  int i,j;
  lword *PS=(lword *)&PracScr[((long)BPL>>1)*y];
  lword *IM=(lword *)&Klice2Spr.Img;
  for( i=0; i<Klice2Spr.s_h; i++ )
  {
    lword t=IM[1];
    *PS++=*IM;
    for( j=BPL/4-1-1; j>=0; j-- ) *PS++=t;
    IM = (lword *)((word *)IM+Klice2Spr.s_w);
  }
  */
}

void CComponiumView::ZacniKresl(SekvSoubor *ss)
{
  int Kan;
  // TODO: erase
  //KdeJeS(&OI->Kurs,&PosKurs);
  Plati(OI.KKan>=OI.IStopa);
  Plati(OI.KKan<OI.IStopa+OI.NStop);
  for( Kan=0; Kan<OI.NStop; Kan++ )
  {
    CasT CP;
    int of;
    SekvSoubor *s=&ss[Kan];
    GrfStatus *KS=&KIStat[Kan];
    int ypos=kanypos(Kan);
    RozdvojS(s,&OI.Soub[Kan+OI.IStopa]);
    Osnova(ypos-yosnova+8);
    KS->ZBlok=0x7fff;
    KS->KBlok=0;
    KS->PoslBlok=False;
    for( of=0; of<NOff; of++ ) KS->i[of].zpos=KS->i[of].xpos=zacxpos;
    KS->i[0].xpos=KrTonina(-128,ToninaS(s),zacxpos,Kan,AKontextS(s));
    if( AKontextS(s)->VTaktu==0 && Kan==0 )
    {
      KS->i[1].xpos=KS->i[0].xpos-10+TiskCislaT(KS->i[0].xpos-10,kanypos(Kan),AKontextS(s));
    }
    KdeJeS(s,&LPos[Kan]);
    CP=CasJeS(s);
    KS->xcasspol=CP;
    KS->TramX=KS->TramY=-1;
    for( of=0; of<NOff; of++ )
    {
      KS->i[of].zcas=KS->i[of].xcas=CP;
      KS->i[of].synchro=True;
    }
    if( JeVBloku(Kan+OI.IStopa,&LPos[Kan],CP) )
    {
      if( CP==0 ) KS->ZBlok=0;
      else KS->ZBlok=zacxpos;
      KS->PoslBlok=True;
    }
    UzDvojCara[Kan]=CP<=0;
    InitVec(&TonVec[Kan]);
    /* musi byt opravdu uvnitr */
    UzRep[Kan]=0;
    UzSek[Kan]=0;
    KS->stat=StatusS(s);
    PPC[Kan]=0;
    KS->PredTon=ToninaS(s);
    CtiS(s),PreskocTakt(s);
  }
}

#if 0 // TODO: slider handling
static void MinMaxSoup( int LHan, OknoInfo *OI, SekvSoubor *ss )
{
  Okno *O=Okna[LHan];
  CasT CasKon=MAXPOS,CasZac=MAXPOS,DelMax=0;
  int Kan;
  {
    SekvSoubor *so=&OI->Soub[OI->KKan];
    Kan=OI->KKan-OI->IStopa;
    OI->ZacCas=StatusS(so)==OK ? CasJeS(so) : MAXCAS;
    /*
    OI->KonCas=StatusS(&ss[Kan])==OK || KIStat[Kan].i[0].xpos>=xmax ? CasJeS(&ss[Kan]) : MAXCAS;
    */
    /* oprava dne 28.10.94 */
    OI->KonCas=CasJeS(&ss[Kan]);
    if( OI->KonCas<OI->ZacCas ) OI->KonCas=OI->ZacCas;
  }
  for( Kan=0; Kan<OI->NStop; Kan++ )
  {
    CasT c;
    SekvSoubor *o=&OI->Soub[Kan+OI->IStopa];
    SekvSoubor *s=&ss[Kan];
    if( StatusS(o)==OK )
    {
      c=CasJeS(o);
      if( CasZac>c ) CasZac=c;
    }
    if( StatusS(s)==OK )
    {
      c=CasJeS(s);
      if( CasKon>c ) CasKon=c;
    }
    c=CasDelka(SekvenceS(o),NULL);
    if( DelMax<c ) DelMax=c;
    ZavriS(s);
  }
  for( Kan=0; Kan<OI->NStop; Kan++ )
  {
    GrfStatus *KS=&KIStat[Kan];
    UzavriTram(KS);
    if( KS->ZBlok<KS->KBlok )
    {
      KresliBlok(Kan,KS->ZBlok, KS->PoslBlok ? xmax : KS->KBlok );
    }
  }
  if( CasKon>DelMax ) CasKon=DelMax;
  if( CasZac>CasKon ) CasZac=CasKon;
  OI->CasKon=CasKon;
  OI->CasZac=CasZac;
  if( DelMax>0 )
  {
    CasT dif=CasKon-CasZac;
    long vel=1000*(long)dif/DelMax;
    long pos=DelMax>dif ? 1000*CasZac/(DelMax-dif) : 0;
    if( vel>=1000 ) vel=1000,pos=0;
    else if( vel<1 ) vel=1;
    if( pos>1000 ) pos=1000;
    else if( pos<0 ) pos=0;
    O->HSVel=(int)vel;
    O->HSPos=(int)pos;
  }
  else
  {
    O->HSVel=1000;
    O->HSPos=0;
  }
  {
    long dif=OI->NSoub-OI->NStop;
    O->VSVel=(int)( 1000*(long)OI->NStop/OI->NSoub );
    if( dif>0 ) O->VSPos=(int)( 1000*(long)OI->IStopa/dif);
    else O->VSPos=0;
  }
}

#endif

#if 0 // TODO: cursor
enum {kyoff=4,kxoff=2};

const static byte bm[8]={	0xff,0x7f,0x3f,0x1f,0x0f,0x07,0x03,0x01};
const static byte em[8]={0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff,};

static void XorRect( const WindRect *W )
{
  byte bmask,emask;
  byte *Adr=(byte *)PracScr,*A;
  int H;
  int xb=W->x;
  int xe=xb+W->w-1;
  int ye=std::max(W->y,0)+W->h;
  if( ye>=ScrH ) ye=ScrH;
  bmask=bm[xb&7];
  emask=em[xe&7];
  xb>>=3;
  xe>>=3;
  Adr+=xb+std::max(W->y,0)*((long)BPL);
  xe-=xb;
  if( xe<=0 )
  {
    bmask&=emask;
    for( H=max(W->y,0); H<ye; H++,Adr+=BPL ) *Adr^=bmask;
  }
  else
  {
    int ec;
    for( H=max(W->y,0); H<ye; H++,Adr+=BPL )
    {
      byte S=0xff;
      A=Adr;
      *A^=bmask;
      A++;
      for( ec=xe-2; ec>=0; ec-- ) *A++^=S;
      *A^=emask;
    }
  }
}


static void PleskKursor( int LHan, Flag DrawF )
{
  OknoInfo *OI=OInfa[LHan];
  if( OI->KR.x>=0 )
  {
    PKR=OI->KR;
    XorRect(&OI->KR);
    if( DrawF ) KusPrekresli(LHan,&OI->KR);
  }
}


static void SmazKursor( int LHan, Flag DrawF )
{
  if( PKR.x>=0 )
  {
    XorRect(&PKR);
    if( DrawF ) KusPrekresli(LHan,&PKR);
  }
}
#endif


int PovelPoradi( enum CPovel P )
{
  switch( P )
  {
    case PERep: return 10;
    case PKonSekv: return 10;
    case PTakt: case PDvojTakt: case PDvojTaktUm: return 20;
    case PTonina: return 30;
    case PTempo: return 40;
    case PNTakt: return 50;
    case PRep: return 60;
    case POktava: return 70;
    case PHlasit: return 80;
    case PNastroj: return 85;
    case PEfekt: return 87;
    case PStereo: return 90;
    case PZacSekv: return 100;
    default: return 110;
  }
}

int PovelPred( enum CPovel P, enum CPovel p )
{
  if( P<OK )
  {
    if( p<OK ) return 0;
    else return +1;
  }
  else if( p<OK ) return -1;
  return PovelPoradi(P)-PovelPoradi(p);
}

void CComponiumView::FKresliNoty( CDC *dc)
{
  //OknoInfo *OI=OInfa[LHan];
  SekvSoubor ss[NMaxStop];
  int xmax=200;
  /* kresleni osnovy */
  ZacniKresl(ss);
#if 0 // TODO: composition rendering
  int Kan;
  int xpos;
  /* kresleni not */
  for(;;)
  {
    CasT CP=MAXPOS,CD;
    int MKan;
    enum CPovel Pov;
    for( Kan=0; Kan<OI->NStop; Kan++ )
    {
      GrfStatus *KS=&KIStat[Kan];
      if( KS->stat==OK )
      {
        CasT c=KS->xcasspol;
        if( CP>=c )
        {
          SekvSoubor *s=&ss[Kan];
          enum CPovel PA=BufS(s)[0];
          int PP;
          CasT d;
          if( StatusS(s)==OK ) switch( PA )
          {
            case PERep: case PTakt: case PDvojTaktUm: d=0; break;
            default: d=CasJeS(s)-c;break;
          }
          else d=0,PA=ERR; /* konec kanalu */
          if( CP>c ) CP=c,CD=d,Pov=PA,MKan=Kan;
          /* stejny cas - musis se rozhodovat slozite */
          else /* CP==c */
          {
            if( d==0 && UzSek[MKan]>UzSek[Kan] ) /* CD==d */
            {
              Pov=PA,MKan=Kan;continue;
            }
            else if( d==0 && UzSek[MKan]<UzSek[Kan] ) continue;
            else if( (PP=PovelPred(Pov,PA))>=0 )
            {
              if( PP>0 ) CD=d,Pov=PA,MKan=Kan;
              else if( UzRep[MKan]>UzRep[Kan] ) CD=d,Pov=PA,MKan=Kan; /* PP==0 */
              else if( UzRep[MKan]==UzRep[Kan] )
              {
                if( d<=0 && ( PA!=PDvojTaktUm || Pov!=PDvojTaktUm ) )
                {
                  if( KIStat[MKan].i[0].xpos>KIStat[Kan].i[0].xpos ) CD=d,Pov=PA,MKan=Kan;
                }
                else
                {
                  if( KIStat[MKan].i[0].xpos<KIStat[Kan].i[0].xpos ) CD=d,Pov=PA,MKan=Kan;
                }
              }
            }
          }
        }
      }
    }
    if( CP>=MAXPOS ) break;
    else
    {
      SekvSoubor *s=&ss[MKan];
      int offs=OffsetP(Pov);
      GrfStatus *KN=&KIStat[MKan];
      xpos=Zjistixpos(OI,MKan,offs,CP);
      if( xpos>=xmax-8 ) break;
      TestKurs(OI,s,MKan);
      KN->i[offs].zcas=CP;
      KN->i[offs].zpos=xpos;
      KresliJedn(s,xpos,MKan,OI,offs);
      KN->stat=Pov<EOK ? Pov : EOK;
      if( KN->stat!=OK ) CD=0;
      KN->xcasspol=KN->i[offs].xcas=CP+CD;
      KdeJeS(s,&LPos[MKan]);
      if( Pov==PRep ) UzRep[MKan]++;
      else UzRep[MKan]=0;
      if( Pov==PZacSekv ) UzSek[MKan]++;
      else if( Pov<=PPau ) UzSek[MKan]=0;
      if( Pov==PERep )
      {
        JednItem *B=BufS(s);
        B[0]=PUmPauza,B[1]=B[2]=B[3]=0;
      }
      else CtiS(s),PreskocTakt(s);
    }
  }
  MinMaxSoup(LHan,OI,ss);
  PLHan=LHan;
  PKR.x=-1;
  //PleskKursor(LHan,False);
#endif
}


#if 0
static void ZobrazKursor( int LHan )
{
  OknoInfo *OI=OInfa[LHan];
  if( !OI->ZobrazKursor ) return;
  OI->ZobrazKursor=False;
  if( OI->KR.x<0 || OI->KR.x>=xmax-32 )
  {
    PosunCasPos(OI,CasJeS(&OI->Kurs));
    TaktVlevo(OI);
    FKresliNoty(LHan);
    if( OI->KR.x<0 || OI->KR.x>=xmax-32 )
    {
      PosunCasPos(OI,CasJeS(&OI->Kurs));
      FKresliNoty(LHan);
      if( OI->KR.x<0 || OI->KR.x>=xmax-32 )
      {
        StrPosT p;
        KdeJeS(&OI->Kurs,&p);
        NajdiS(&OI->Soub[OI->KKan],&p);
        PosunCas(OI);
        FKresliNoty(LHan);
        Plati( OI->KR.x>=0 );
      }
    }
  }
}

void KresliNoty( int LHan, WindRect *Kres )
{
  OknoInfo *OI=OInfa[LHan];
  if( OI->ZmenaObrazu )
  {
    *Kres=Okna[LHan]->Uvnitr;
    OI->ZmenaObrazu=False;
  }
  FKresliNoty(LHan);
  ZobrazKursor(LHan);
  Soupatka(LHan);
}

static void PresunKurs( int LHan, const WindRect *kr )
{
  OknoInfo *OI=OInfa[LHan];
  if( OI->KR.x!=kr->x || OI->KR.y!=kr->y || OI->KR.w!=kr->w || OI->KR.h!=kr->h )
  {
    OI->KR=*kr;
    SmazKursor(LHan,True);
    PleskKursor(LHan,True);
  }
}

static void FKresliKursor( int LHan )
{
  OknoInfo *OI=OInfa[LHan];
  int Kan=OI->KKan-OI->IStopa;
  Plati( PLHan==LHan );
  if( Kan>=0 && Kan<OI->NStop )
  {
    StrPosT KP;
    PProt *PP;
    int NP;
    KdeJeS(&OI->Kurs,&KP);
    for( PP=Prot[Kan],NP=PPC[Kan]-1; NP>=0; NP--,PP++ )
    {
      int dif=CmpPos(&PP->Pos,&KP);
      if( dif>=0 )
      {
        if( dif==0 && PP->WR.x<Okna[PLHan]->Uvnitr.w-20 )
        {
          PresunKurs(LHan,&PP->WR);
          return;
        }
        break;
      }
    }
  }
}
  
void KresliKursor( int LHan )
{
  if( LHan==PLHan ) FKresliKursor(LHan);
  else Kresli(LHan);
}

Flag HledejKursor( int LHan, int x, int y, int HMode, int *Ins )
{
  OknoInfo *OI=OInfa[LHan];
  PProt *PP,*PF=NULL,*PI=NULL;
  Flag PIZa,PFZa=False;
  int NP;
  int Kan=y2kan(y);
  if( LHan!=PLHan ) Prekresli(LHan,&Okna[LHan]->Uvnitr);
  if( Kan>=OI->NStop ) Kan=OI->NStop-1;
  for( PP=Prot[Kan],NP=PPC[Kan]-1; NP>=0; NP--,PP++ )
  {
    if( PP->WR.x<=x && x<PP->WR.x+PP->WR.w && PP->WR.y<=y && y<PP->WR.y+PP->WR.h ) PF=PP;
    if( HMode!=HKursPred )
    {
      if( PP->WR.x<=x && x<PP->WR.x+PP->WR.w ) PI=PP,PIZa=False;
      else if( PP->WR.x<=x-4 && NP>0 ) PI=PP,PIZa=True; /* jeste zname dalsiho */
    }
    else /* HKursPred */
    {
      if( PP->WR.x<=x ) PI=PP,PIZa=False;
    }
  }
  *Ins=InsNic;
  if( !PF )
  {
    if( HMode!=HKursor )
    {
      Flag ChNasIns=y-kanypos(Kan)+yosnova < 12;
      *Ins=ChNasIns ? InsNas : PIZa ? InsNota : InsSoubezna;
    }
    PF=PI;
    PFZa=PIZa;
  }
  if( !PF ) {*Ins=InsNic;return False;}
  if( PF->WR.x>=Okna[LHan]->Uvnitr.w-16 ) {*Ins=InsNic;return False;}
  ZavriS(&OI->Kurs);
  OI->KKan=Kan+OI->IStopa;
  RozdvojS(&OI->Kurs,&OI->Soub[OI->KKan]);
  if( PFZa ) PF++,NajdiS(&OI->Kurs,&PF->Pos);
  else if( *Ins==InsSoubezna )
  {
    PlatiProc( NajdiS(&OI->Kurs,&PF->Pos), ==EOK);
    if( TestS(&OI->Kurs)!=EOK || BufS(&OI->Kurs)[0]>PPau ) *Ins=InsNic;
    else JCtiS(&OI->Kurs);
  }
  else NajdiS(&OI->Kurs,&PF->Pos);
  if( TestS(&OI->Kurs)!=OK && HMode!=HKursor && *Ins!=InsSoubezna ) *Ins=InsNota;
  if( *Ins==InsNic || *Ins==InsNas )
  {
    Flag DrawF;
    int P;
    P=BufS(&OI->Kurs)[0];
    DrawF=StatusS(&OI->Kurs) || P>PPau || HMode==HKursor || HMode==HEditDvoj || HMode==HKursPred || *Ins!=InsNic;
    if( DrawF ) PresunKurs(LHan,&PF->WR);
    else SmazKursor(LHan,True),OI->KR=PF->WR;
    return True;
  }
  return False;
}

#endif

#if 0 // TODO: note editing limited redraw
enum {NotaW=18,NotaH=80,MazO=6,MazH=10,MazHO=2};

#define NMaskH 0xffff
#define OkrajH 0x8000

static word NMaskL[]=
{
  0x0000,
  0x8000,0xc000,0xe000,0xf000,0xf800,0xfc00,0xfe00,0xff00,
  0xff80,0xffc0,0xffe0,0xfff0,0xfff8,0xfffc,0xfffe,0xffff
};
static void MazENotu( WindRect *W, int x, int y, int Prop )
{
  x-=MazO;
  Prop+=MazO;
  W->x=x;
  W->y=y-MazH;
  W->w=Prop+1;
  W->h=NotaH+MazH;
  Prop-=16;
  if( Prop>16 ) Prop=16;
  if( Prop<0 ) Prop=0;
  {
    word *VA=&VRA[W->y*((long)BPL>>1)+(x>>4)];
    int jx=16-(x&0xf);
    lword HM=~((lword)NMaskH<<jx);
    lword LM=~((lword)NMaskL[Prop]<<jx);
    lword HO=(lword)OkrajH<<jx;
    lword LO=((lword)OkrajH>>Prop)<<jx;
    lword *OS=(lword *)&Klice2Spr.Img[2];
    long Step=Klice2Spr.s_w;
    int i;
    lword t;
    {
      t=*(lword *)VA;t|=~HM;*(lword *)VA=t;
      t=*(lword *)&VA[1];t|=~LM|LO;*(lword *)&VA[1]=t;
      VA+=(BPL/2);
    }
    for( i=MazH+MazHO-1-1; i>=0; i-- )
    {
      t=*(lword *)VA;t&=HM;t|=HO;*(lword *)VA=t;
      t=*(lword *)&VA[1];t&=LM;t|=LO;*(lword *)&VA[1]=t;
      VA+=(BPL/2);
    }
    for( i=Klice2Spr.s_h-1; i>=0; i-- )
    {
      t=*(lword *)VA;t&=HM;t|=HO;t|=*OS;*(lword *)VA=t;
      t=*(lword *)&VA[1];t&=HM;t|=LO;t|=*OS;*(lword *)&VA[1]=t;
      VA+=(BPL/2);
      ((word *)OS)+=Step;
    }
    for( i=NotaH-MazHO-Klice2Spr.s_h-1-1; i>=0; i-- )
    {
      t=*(lword *)VA;t&=HM;t|=HO;*(lword *)VA=t;
      t=*(lword *)&VA[1];t&=LM;t|=LO;*(lword *)&VA[1]=t;
      VA+=(BPL/2);
    }
    {
      t=*(lword *)VA;t|=~HM;*(lword *)VA=t;
      t=*(lword *)&VA[1];t|=~LM|LO;*(lword *)&VA[1]=t;
      VA+=(BPL/2);
    }
  }
}

void KresliENotu( int LHan, JednItem *B, int Prop )
{
  OknoInfo *OI=OInfa[LHan];
  WindRect W,Dum;
  int x=(OI->KR.x&~1)-kxoff + ( (B[1]&KBOMaskA) ? -4 : 4 );
  int y=kanypos(OI->KKan-OI->IStopa)&~1;
  MazENotu(&W,x,y-yosnova+ystep,Prop);
  Plati(B[0]<=PPau);
  FKresliTon(NULL,&Dum,B,W.x+MazO,y);
  KusPrekresli(LHan,&W);
}

#endif

/* init */

static int MezProp( int O )
{
  int j;
  for( j=0; (O&0x80)==0 && j<8; j++ ) O<<=1;
  return j;
}

static void SetProp( byte *Font, int i )
{
  byte *NP=&Font[i];
  byte *P=&PFont[i];
  byte O;
  int j,z;
  for( O=j=0; j<16; j++ ) O|=NP[j*0x100];
  if( O==0 ) Prop[i]=8;
  else
  {
    z=MezProp(O);
    O<<=z;
    for( j=0; j<16; j++ )
    {
      P[j*0x100]=NP[j*0x100]<<z;
    }
    z=MezProp(~O)+2;
    Prop[i]=z;
  }
}
