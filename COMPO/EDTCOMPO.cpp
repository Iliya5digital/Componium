/* Componium - editace melodie */
/* SUMA, 10/1992-5/1994 */

#include "../stdafx.h"
#include <string.h>

#include "ramcompo.h"
#include "compo.h"
#include "../ComponiumView.h"

#if 0
void NastavZmeny( Sekvence *S, Flag Krit )
{
  int I;
  Sekvence *K;
  Pisen *P=S->Pis;
  (void)Krit;
  /*if( Krit )*/ for( K=PrvniSekv(P); K; K=DalsiSekv(K) )
  {
    Sekvence *H;
    if( S!=K ) for( H=K; H; H=DalsiHlas(H) ) H->_CasPriTempu=-1;
  }
  for( I=0; I<NMaxOken; I++ )
  {
    Okno *O=Okna[I];
    if( O && OknoJeSekv(I) )
    {
      OknoInfo *OIK=OInfa[I];
      if( !OIK->ZmenaObrazu )
      {
        Sekvence *K=SekvenceS(&OIK->Soub[0]);
        if( K==S || K->Pis==S->Pis ) {OIK->ZmenaObrazu=True;break;}
        /* misto testu na stejnou pisen muze byt test Zavislost(K,S) pro vsechny stopy okna */
        /* tohle je ale o hodne rychlejsi */
      }
    }
  }
}

static void ZavriOstatni( int LHan )
{
  OknoInfo *OI=OInfa[LHan];
  Sekvence *S=SekvenceOI(OI);
  int THan;
  for( THan=HorniOkno(); THan>=0; THan=DalsiOkno(THan) ) if( THan!=LHan )
  {
    OknoInfo *TI=OInfa[THan];
    if( SekvenceS(&TI->Soub[0])==S ) FZavri(THan);
  }
}
#endif

static Flag BezTrioly( int Delka )
{
  switch( Delka )
  {
    case Takt*2/3: case Takt/2*2/3: case Takt/4*2/3:
    case Takt/8*2/3: case Takt/16*2/3: case Takt/32*2/3:
      return False;
    default:
      return True;
  }
}
static Flag BezTecky( int Delka )
{
  switch( Delka )
  {
    case Takt*3/2: case Takt/2*3/2: case Takt/4*3/2:
    case Takt/8*3/2: case Takt/16*3/2: case Takt/32*3/2:
      return False;
    default:
      return True;
  }
}

static int PrepTecka( int Delka )
{
  if( BezTecky(Delka) )
  {
    if( !BezTrioly(Delka) ) Delka*=3,Delka/=2;
    Delka*=3,Delka/=2;
  }
  else Delka*=2,Delka/=3;
  return Delka;
}
static int PrepTriola( int Delka )
{
  if( BezTrioly(Delka) )
  {
    if( !BezTecky(Delka) ) Delka*=2,Delka/=3;
    Delka*=2,Delka/=3;
  }
  else Delka*=3,Delka/=2;
  return Delka;
}

static Flag PredTon( SekvKursor *s, JednItem *Buf )
{
  Flag ret;
  SekvKursor r;
  RozdvojK(&r,s);
  if( ZpatkyK(&r)!=EOK ) ret=False;
  else TestK(&r),EquJed(Buf,BufK(&r)),ret=Buf[0]<=PPau;
  ZavriK(&r);
  return ret;
}

static void KonTon( SekvKursor *k )
{
  while( JCtiK(k)==OK && BufK(k)[0]<=PPau && (BufK(k)[1]&Soubezna) );
}


static Err FVlozTon( OknoInfo *OI, Flag Soub, JednotkaBuf B )
{
  Err ret;
  SekvKursor k;
  RZapisK(&k,AktK(&OI->Kurs));
  EquJed(BufK(&k),B);
  ZacBlokZmen(SekvenceK(&k));
  ret=VlozK(&k);
  if( ret<OK ) Chyba(ret);
  else if( Soub )
  { /* opravit predch. notu */
    PlatiProc( ZpatkyK(&k), ==EOK );
    PlatiProc( ZpatkyK(&k), ==EOK ); /* dostan se na predch */
    PlatiProc( TestK(&k), ==EOK );
    BufK(&k)[1]|=Soubezna;
    PlatiProc( PisK(&k), ==EOK );
  }
  if( Soub ) KonTon(&k);
  TNormujK(&k,AktK(&OI->Kurs));
  KonBlokZmen(SekvenceK(&k));
  ZavriK(&k);
  return ret;
}

Err VlozTon( OknoInfo *OI, Flag Soub )
{
  JednotkaBuf B;
  EquJed(B,BufS(&OI->Kurs));
  if( !PredTon(AktK(&OI->Kurs),B) )
  {
    Soub=False;
    B[0]=(TC+NormOkt-1)*Okt;
    B[1]=0;
    B[2]=Takt/4;
    B[3]=0;
  }
  else
  {
    B[1]&=~KBOMaskA; /* zrusit krizky a pod., ponechat druh noty */
    if( Soub )
    {
      if( B[0]<PPau-2 ) B[0]+=2; /* horni tercie */
      else B[0]-=2; /* dolni tercie */
    }
  }
  return FVlozTon(OI,Soub,B);
}

void MazTonF( SekvKursor *s )
{
  Err ret;
  Flag Soub=True;
  TestK(s);
  if( BufK(s)[0]<=PPau )
  {
    if( !(BufK(s)[1]&Soubezna) ) Soub=False;
  }
  ret=MazK(s);
  if( ret!=OK )
  {
    if( ret!=KON ) {/*AlertRF(NPOVA,1);*/}
  }
  else
  {
    if( !Soub )
    {
      SekvKursor o;
      JednItem *B;
      RZapisK(&o,s);
      B=BufK(&o);
      while( ZpatkyK(&o)==EOK )
      {
        if( B[0]>PPau ) break;
        if( B[1]&Soubezna )
        {
          B[1]&=~Soubezna;
          PlatiProc( PisK(&o), ==EOK );
        }
        else break;
      }
      ZavriK(&o);
    }
  }
}

static Flag MidiKonvTon( SekvKursor *k, int *T, int *A, int MT )
{
  static const int Pultony[8]={0,2,4,5,7,9,11};
  PosuvT Pos;
  enum {MOkt=4};
  int Expl;
  int t;
  int o=MT/12-MOkt;
  MazPosuv(Pos,k);
  for( Expl=TonN; Expl<=TonB; Expl++ )
  for( t=0; t<Okt; t++ )
  {
    int AT=Pultony[t];
    PI Znam;
    if( Expl==TonN ) Znam=Pos[t];
    else Znam=PI(Expl);
    if( Znam==TonK ) AT++;
    if( Znam==TonB ) AT--;
    if( AT>=12 ) AT-=12,o++;
    else if( AT<0 ) AT+=12,o--;
    if( (MT%12)==AT )
    {
      *T=t+o*Okt;
      switch( Expl )
      {
        case TonK: *A=KrizekA;break;
        case TonB: *A=BeckoA;break;
        case TonO: *A=OdrazA;break;
        default: *A=0;break;
      }
      return True;
    }
  }
  return False;
}

static Flag JeTon( int a, int shf )
{
  if( shf<0 ) return True;
  if( a>='a' && a<='z' ) a-='a'-'A';
  return a>='A' && a<='H' || a=='P';
}

#if 0
int Editace( int Asc, int Scan, int Shf )
{ /* return <0 znamena, ze tu klavesu nechceme */
  /* Shf<0 -> vstup z MIDI, Asc je klavesa, Scan je velocity */
  int LHan=VrchniSekv();
  int ret=0;
  if( LHan>=0 )
  {
    OknoInfo *OI=OInfa[LHan];
    SekvKursor k;
    Err err;
    RZapisK(&k,AktK(&OI->Kurs));
    err=TestK(&k);
    if( ( err==KON || BufK(&k)[0]>PPau ) && JeTon(Asc,Shf) )
    {
      err=VlozTon(OI,False);
      if( err==OK ) err=TestK(&k);
    }
    if( err==OK )
    {
      int Druh=BufK(&k)[0];
      if( Druh<=PPau ) /* editace noty */
      {
        int Ton,Oktava;
        int Delka=BufK(&k)[2],Attr=BufK(&k)[1];
        Flag BTec=BezTecky(Delka),BTri=BezTrioly(Delka);
        if( !BTec ) Delka=PrepTecka(Delka);
        if( !BTri ) Delka=PrepTriola(Delka);
        if( Druh<PPau )
        {
          Ton=(Druh-TC)%Okt;
          Oktava=(Druh-TC)/Okt;
        }
        else Ton=Druh,Oktava=0;
        (void)Scan;
        if( Shf>=0 ) switch( Asc )
        {
          case 'c': case 'C': Ton=TC;goto TonLab;
          case 'd': case 'D': Ton=TD;goto TonLab;
          case 'e': case 'E': Ton=TE;goto TonLab;
          case 'f': case 'F': Ton=TF;goto TonLab;
          case 'g': case 'G': Ton=TG;goto TonLab;
          case 'a': case 'A': Ton=TA;goto TonLab;
          case 'h': case 'H': Ton=TH;goto TonLab;
          case 'b': case 'B': Ton=TH;goto TonLab;
          case 'p': case 'P': Ton=PPau;Attr&=~(KBOMaskA|Leg);Oktava=0;goto OktLab;
          case '1': Delka=Takt; goto OktLab;
          case '2': Delka=Takt/2; goto OktLab;
          case '4': Delka=Takt/4; goto OktLab;
          case '8': Delka=Takt/8; goto OktLab;
          case '9': Delka=Takt/16; goto OktLab;
          case '0': Delka=Takt/32; goto OktLab;
          case '.': BTec^=True; goto OktLab;
          case '3': BTri^=True; goto OktLab;
          case '/': if( Ton!=PPau ) Attr^=Leg; goto OktLab;
          case '+': if( Ton!=PPau && Oktava<NOkt-1 ) Oktava++; goto OktLab;
          case '-': if( Ton!=PPau && Oktava>0 ) Oktava--; goto OktLab;
          case '(': if( Ton!=PPau ) if( (Attr&KBOMaskA)==KrizekA ) Attr&=~KrizekA; else Attr&=~KBOMaskA,Attr|=KrizekA; goto OktLab;
          case ')': if( Ton!=PPau ) if( (Attr&KBOMaskA)==BeckoA ) Attr&=~BeckoA; else Attr&=~KBOMaskA,Attr|=BeckoA; goto OktLab;
          case '*': if( Ton!=PPau ) if( (Attr&KBOMaskA)==OdrazA ) Attr&=~OdrazA; else Attr&=~KBOMaskA,Attr|=OdrazA; goto OktLab;
          TonLab:
            if( Shf&K_LSHIFT ) Oktava=NormOkt;
            else if( Shf&K_RSHIFT ) Oktava=NormOkt-2;
            else Oktava=NormOkt-1;
          OktLab:
            if( !BTri ) Delka=PrepTriola(Delka);
            if( !BTec && Ton!=PPau ) Delka=PrepTecka(Delka);
            BufK(&k)[0]=Ton+Okt*Oktava;
            BufK(&k)[1]=Attr;
            BufK(&k)[2]=Delka;
            BufK(&k)[3]=0;
            PlatiProc(PisK(&k),==OK);
            if( Attr&Soubezna ) KonTon(&k);
            TNormujK(&k,AktK(&OI->Kurs));
          /*KresliLab:*/
            UkazKursor(OI);
            Kresli(VrchniSekv());
            break;
           default: ret=-1;
        }
        else /* MIDI */
        {
          int T,A;
          if( MidiKonvTon(&k,&T,&A,Asc) )
          {
            if( T>NOkt*Okt ) T=PPau;
            else if( T<0 ) T=PPau;
            BufK(&k)[0]=T;
            BufK(&k)[1]=(Attr&~KBOMaskA)|A;
            BufK(&k)[2]=Delka;
            BufK(&k)[3]=0;
            PlatiProc(PisK(&k),==OK);
            UkazKursor(OI);
            Kresli(VrchniSekv());
          }
        }
      }
    }
    else ret=-1;
    ZavriK(&k);
  }
  else ret=-1;
  return ret;
}
#endif

static int NajdiEdit( SekvKursor *s, SekvSoubor *z, enum CPovel Povel, CasT CasPos )
{
  /* 0 - nenalezen povel, nalezena pozice */
  /* <0 - neexeist. pozice */
  /* >0 - nalezen povel na pozici */
  int ret;
  SekvSoubor r;
  RozdvojS(&r,z);
  if( Povel!=PRep && Povel!=PERep )
  {
    NajdiSCasS(&r,CasPos);
    for(;;)
    {
      if( ZpatkyS(&r)!=OK ) break;
      if( CasJeS(&r)!=CasPos ) {JCtiS(&r);break;}
      if( TestS(&r)!=OK || BufS(&r)[0]==Povel ) break;
    }
  }
  if( CasJeS(&r)!=CasPos ) ret=-1;
  else if( TestS(&r)==EOK && BufS(&r)[0]==Povel ) ret=+1;
  else ret=0;
  RZapisK(s,AktK(&r));
  ZavriS(&r);
  TestK(s);
  return ret;
}

static int NajdiPovel( SekvKursor *s, SekvSoubor *z, enum CPovel Povel )
{ /* viz NajdiEdit */
  SekvSoubor r;
  RozdvojS(&r,z);
  NajdiSCasS(&r,CasJeS(&r));
  for(;;)
  {
    if( ZpatkyS(&r)!=OK ) break;
    PlatiProc( TestS(&r), ==EOK );
    if( BufS(&r)[0]==Povel ) break;
  }
  RZapisK(s,AktK(&r));
  ZavriS(&r);
  return TestK(s)==EOK && s->Buf[0]==Povel;
}

#if 0
int InitEdit( SekvKursor *s, JednItem Povel, int Kan, CasT CasPos, Flag Hledat )
{
  OknoInfo *OI=OInfa[VrchniSekv()];
  SekvSoubor *z;
  int ret=-1;
  /* Kan<0 znamena ediatci na pozici kursoru */
  if( Kan<0 || Kan==OI->KKan && ( Povel==PRep || Povel==PERep) )
  {
    RZapisK(s,AktK(&OI->Kurs));
    if( TestK(s)!=EOK ) return 0;
    return BufK(s)[0]==Povel;
  }
  z=&OI->Soub[Kan];
  if( Povel>=0 )
  {
    SekvSoubor zk;
    RozdvojS(&zk,z);
    NajdiSCasS(&zk,CasPos);
    if( !Hledat ) ret=NajdiEdit(s,&zk,Povel,CasPos);
    else ret=NajdiPovel(s,&zk,Povel);
    ZavriS(&zk);
  }
  else if( Povel==KON )
  {
    RZapisK(s,AktK(z));
    NajdiSCasK(s,CasPos);
  }
  else if( Povel==ZAC )
  {
    RZapisK(s,AktK(z));
    NajdiCasK(s,CasPos);
  }
  else Plati( False );
  return ret;
}
#endif

static int SNajdiEdit( SekvSoubor *s, SekvSoubor *z, enum CPovel Povel, CasT CasPos )
{
  /* 0 - nenalezen povel, nalezena pozice */
  /* <0 - neexeist. pozice */
  /* >0 - nalezen povel na pozici */
  int ret;
  SekvSoubor r;
  RozdvojS(&r,z);
  if( Povel!=PRep && Povel!=PERep )
  {
    NajdiSCasS(&r,CasPos);
    for(;;)
    {
      if( ZpatkyS(&r)!=OK ) break;
      if( CasJeS(&r)!=CasPos ) {JCtiS(&r);break;}
      if( TestS(&r)!=OK || BufS(&r)[0]==Povel ) break;
    }
  }
  if( CasJeS(&r)!=CasPos ) ret=-1;
  else if( TestS(&r)==EOK && BufS(&r)[0]==Povel ) ret=+1;
  else ret=0;
  RZapisS(s,&r);
  ZavriS(&r);
  TestS(s);
  return ret;
}

static int SNajdiPovel( SekvSoubor *s, SekvSoubor *z, enum CPovel Povel )
{ /* viz NajdiEdit */
  SekvSoubor r;
  RozdvojS(&r,z);
  NajdiSCasS(&r,CasJeS(&r));
  for(;;)
  {
    if( ZpatkyS(&r)!=OK ) break;
    PlatiProc( TestS(&r), ==EOK );
    if( BufS(&r)[0]==Povel ) break;
  }
  RZapisS(s,&r);
  ZavriS(&r);
  return TestS(s)==EOK && BufS(s)[0]==Povel;
}

#if 0
int SInitEdit( SekvSoubor *s, JednItem Povel, int Kan, CasT CasPos, Flag Hledat )
{ /* vyhled†v† strukturovanÿ */
  OknoInfo *OI=OInfa[VrchniSekv()];
  SekvSoubor *z;
  int ret=-1;
  /* Kan<0 znamena ediatci na pozici kursoru */
  if( Kan<0 || Kan==OI->KKan && ( Povel==PRep || Povel==PERep) )
  {
    RZapisS(s,&OI->Kurs);
    if( TestS(s)!=EOK ) return 0;
    return BufS(s)[0]==Povel;
  }
  z=&OI->Soub[Kan];
  if( Povel>=0 )
  {
    SekvSoubor zk;
    RozdvojS(&zk,z);
    NajdiSCasS(&zk,CasPos);
    if( !Hledat ) ret=SNajdiEdit(s,&zk,Povel,CasPos);
    else ret=SNajdiPovel(s,&zk,Povel);
    ZavriS(&zk);
  }
  else if( Povel==KON )
  {
    RZapisS(s,z);
    NajdiSCasS(s,CasPos);
  }
  else if( Povel==ZAC )
  {
    RZapisS(s,z);
    NajdiCasS(s,CasPos);
  }
  else Plati( False );
  return ret;
}
#endif

void TNormujK( SekvKursor *s, SekvKursor *Kurs ) /* s za tonem, ktery normal. */
{ /* udrzujeme, na ktere note je kursor */
  SekvKursor k;
  int p,P;
  Flag Sort;
  PosT Pos=KdeJeK(s);
  JednItem *b;
  ZacBlokZmen(SekvenceK(s));
  RZapisK(&k,s);
  b=BufK(&k);
  do
  {
    Flag NS=False;
    Sort=False;
    NajdiK(&k,Pos);
    P=0;
    for(;;)
    {
      if( ZpatkyK(&k)!=OK ) break;
      if( TestK(&k)!=OK ) break;
      if( b[0]<=PPau )
      {
        if( !(b[1]&Soubezna) )
        {
          if( NS ) break; /* druh† nesoub. nota */
          NS=True;
        }
        p=b[2]*(PPau+1)+b[0];
        if( p<P )
        {
          JednotkaBuf B;
          Flag NaKurs;
          Sort=True;
          NaKurs=( Kurs && KdeJeK(&k)==KdeJeK(Kurs) );
          EquJed(B,k.Buf);MazNOK(&k); /*  neodstranuj - o to starame sami */
          PlatiProc( TestK(&k), ==EOK);
          if( BufK(&k)[1]&Soubezna ) B[1]|=Soubezna;
          else B[1]&=~Soubezna;
          BufK(&k)[1]|=Soubezna;
          PlatiProc( PisK(&k), ==EOK);
          EquJed(BufK(&k),B);
          if( VlozK(&k)!=OK ) OdstranK(&k),Chyba(ERAM);
          else if( NaKurs ) JCtiK(Kurs);
        }
        P=p;
      }
      else break; /* nen° nota */
    }
  } while( Sort );
  ZavriK(&k);
  KonBlokZmen(SekvenceK(s));
}

void NormujK( SekvKursor *s )
{
  SekvKursor k;
  int _PovUs[PrepMax-PrepMin],i;
  enum CPovel p,Pov;
  Flag Sort;
  CasT CasPos=CasJeK(s);
  #define PovUs (_PovUs-PrepMin)
  for( i=PrepMin; i<PrepMax; i++ ) PovUs[i]=False;
  ZacBlokZmen(SekvenceK(s));
  RZapisK(&k,s);
  NajdiSCasK(&k,CasPos);
  while( CasJeK(&k)==CasPos )
  {
    if( ZpatkyK(&k)!=OK ) break;
    if( TestK(&k)!=OK ) break;
    p=CPovel(k.Buf[0]);
    if( p>=PrepMin && p<PrepMax && p!=PRep && p!=PERep )
    {
      if( PovUs[p] || p==PTonina && CasJeK(&k)==0 && k.Buf[1]==0 )
      {
        if( p!=POktava ) MazTonF(&k); /* nemaz oktavy */
      }
      else PovUs[p]=True;
    }
  }
  do
  {
    Sort=False;
    Pov=PrepMax;
    NajdiSCasK(&k,CasPos);
    while( CasJeK(&k)==CasPos )
    {
      if( ZpatkyK(&k)!=OK ) break;
      if( TestK(&k)!=OK ) break;
      p=CPovel(k.Buf[0]);
      if( p>=PrepMin && p<PrepMax )
      {
        if( PovelPoradi(p)>PovelPoradi(Pov) )
        {
          JednotkaBuf B;
          Sort=True;
          EquJed(B,k.Buf);
          MazNOK(&k);
          JCtiK(&k);
          EquJed(k.Buf,B);
          if( VlozK(&k)!=OK )
          {
            OdstranK(&k);
            Chyba(ERAM);
          }
        }
        Pov=p;
      }
    }
  } while( Sort );
  ZavriK(&k);
  KonBlokZmen(SekvenceK(s));
  #undef PovUs
}

void KonecEdit( SekvKursor *s, OknoInfo *OI )
{
  NormujK(s);
  ZavriK(s);
  if( OI ) UkazKursor(OI);
}

#if 0
void MazFPovel( int Kan, JednItem Povel )
{
  SekvKursor r;
  OknoInfo *OI=OInfa[VrchniSekv()];
  Flag ret;
  ret=InitEdit(&r,Povel,Kan,CasJeS(&OI->Kurs),False);
  if( ret>0 ) MazTonF(&r);
  KonecEdit(&r,OI);
}

void MazPovel( JednItem Povel )
{
  OknoInfo *OI=OInfa[VrchniSekv()];
  int Kan;
  Flag Kurs=False; /* kanal s kursorem az posledni - kvuli pocitani repetic */
  for( Kan=PrvniKEdit(OI); Kan>=0; Kan=DalsiKEdit(OI,Kan) )
  {
    if( Kan==OI->KKan ) Kurs=True;
    else MazFPovel(Kan,Povel);
  }
  if( Kurs ) MazFPovel(OI->KKan,Povel);
}

void MazTon( OknoInfo *OI )
{
  SekvSoubor *s=&OI->Kurs;
  SekvKursor k;
  Plati(OI==OInfa[VrchniSekv()]);
  if( TestS(s)==OK ) switch( BufS(s)[0] )
  {
    case PTonina: case PTempo: case PNTakt: case PDvojTakt:
      MazPovel(BufS(s)[0]);
      return;
  }
  RZapisK(&k,AktK(s));
  MazTonF(&k);
  NormujK(&k);
  ZavriK(&k);
}
#endif

static Err KorektVloz( SekvKursor *s )
{
  Err err=VlozK(s);
  if( err!=OK )
  {
    if( err==ERR ) {/*AlertRF(NPOVA,1);*/}
    else Chyba(err);
  }
  return err;
}

static void PovelEdit( JednotkaBuf B, int Kan, CasT CasPos, Flag Hledat )
{
#if 0
  OknoInfo *OI=OInfa[VrchniSekv()];
  int KKan=Kan>=0 ? Kan : OI->KKan;
  SekvKursor k;
  Flag IE;
  JednotkaBuf BOld;
  ZacBlokZmen(SekvenceS(&OI->Soub[KKan]));
  IE=InitEdit(&k,B[0],Kan,CasPos,Hledat);
  if( IE==0 )
  {
    if( StatusK(&k)==ERR ) goto KonecL;
    EquJed(BufK(&k),B);
    KorektVloz(&k);
    BOld[0]=ERR;
  }
  else if( IE>0 )
  {
    EquJed(BOld,BufK(&k));
    EquJed(BufK(&k),B);
    PlatiProc(PisK(&k),==OK);
  }
  KonecL: KonecEdit(&k,OI);
  KonBlokZmen(SekvenceS(&OI->Soub[KKan]));
  #endif
}

void ZmenaToniny( int NKriz, Flag Dur, int Kan, CasT CasPos, Flag Hledat )
{
  JednotkaBuf Buf;
  Buf[0]=PTonina;
  Buf[1]=NKriz;
  Buf[2]=Dur;
  Buf[3]=0;
  PovelEdit(Buf,Kan,CasPos,Hledat);
}
void ZmenaTaktu( int H, int D, int Kan, CasT CasPos, Flag Hledat )
{
  JednotkaBuf Buf;
  Buf[0]=PNTakt;
  Buf[1]=H;
  Buf[2]=D;
  Buf[3]=0;
  PovelEdit(Buf,Kan,CasPos,Hledat);
}

void ZmenaTempa( int H, int Kan, CasT CasPos, Flag Hledat )
{
  JednotkaBuf Buf;
  Buf[0]=PTempo;
  Buf[1]=H;
  Buf[2]=Buf[3]=0;
  PovelEdit(Buf,Kan,CasPos,Hledat);
}
void ZmenaHlasit( int H, int Kan, CasT CasPos )
{
  JednotkaBuf Buf;
  (void)Kan;
  Buf[0]=PHlasit;
  Buf[1]=H;
  Buf[2]=Buf[3]=0;
  PovelEdit(Buf,-1,CasPos,False);
}
void ZmenaOktavy( int O, Flag Rel, int Kan, CasT CasPos )
{
  JednotkaBuf Buf;
  (void)Kan;
  Buf[0]=POktava;
  Buf[1]=O;
  Buf[2]=Rel;
  Buf[3]=0;
  PovelEdit(Buf,-1,CasPos,False);
}
void ZmenaSterea( int Min, int Max, int Kan, CasT CasPos )
{
  JednotkaBuf Buf;
  (void)Kan;
  Buf[0]=PStereo;
  Buf[1]=Min;
  Buf[2]=Max;
  Buf[3]=0;
  PovelEdit(Buf,-1,CasPos,False);
}
void ZmenaEfektu( int EffA, int EffB, int Kan, CasT CasPos )
{
  JednotkaBuf Buf;
  (void)Kan;
  Buf[0]=PEfekt;
  Buf[1]=EffA;
  Buf[2]=EffB;
  Buf[3]=0;
  PovelEdit(Buf,-1,CasPos,False);
}

#if 0
void VyvolaniS( Sekvence *S )
{
  SekvKursor k;
  OknoInfo *OI=OInfa[VrchniSekv()];
  Sekvence *K=SekvenceOI(OI);
  Sekvence *H;
  int NH;
  for( H=S,NH=0; H->predch; H=H->predch,NH++ );
  if( K==S || Zavislost(S,K) || PodHlas(S,K) || PodHlas(K,S) )
  {
    AlertRF(REKURA,1);
  }
  else
  {
    RZapisK(&k,AktK(&OI->Kurs));
    BufK(&k)[0]=PVyvolej;
    BufK(&k)[1]=S->INazev;
    BufK(&k)[2]=NH;
    BufK(&k)[3]=0;
    if( VlozK(&k)!=OK ) Chyba(ERAM);
    else
    {
      PridejRetez(S->Pis->NazvySekv,NazevSekv(S)); /* nealokuje - nemuze dojit k chybe */
      /* ted by jeste mohlo najit kursor! */
      /* NajdiK(&OI->Kurs,KdeJeK(&k)); */
    }
    KonecEdit(&k,OI);
  }
}
void ZmenaVyvolaniS( Sekvence *S )
{
  SekvKursor k;
  OknoInfo *OI=OInfa[VrchniSekv()];
  Sekvence *K=SekvenceOI(OI);
  Sekvence *H;
  int NH;
  for( H=S,NH=0; H->predch; H=H->predch,NH++ );
  if( K==S || Zavislost(S,K) || PodHlas(S,K) || PodHlas(K,S) )
  {
    AlertRF(REKURA,1);
  }
  else
  {
    RZapisK(&k,AktK(&OI->Kurs));
    Plati( TestK(&k)==EOK && BufK(&k)[0]==PVyvolej );
    {
      BankaItem *B=SekvenceK(&k)->Pis->NazvySekv;
      int NN=PridejRetez(B,NazevSekv(S));
      if( NN>MAXERR )
      {
        ZrusRetez(B,BufK(&k)[1]); /* zrus stare */
        BufK(&k)[1]=NN;
        BufK(&k)[2]=NH;
        PlatiProc( PisK(&k),==EOK );
      }
      else Chyba(ERAM);
    }
    KonecEdit(&k,OI);
  }
}
void ZmenaNastroje( const char *S, int Kan, CasT CasPos )
{
  int LHan=VrchniSekv();
  if( LHan>=0 )
  {
    OknoInfo *OI=OInfa[LHan];
    SekvKursor k;
    RZapisK(&k,AktK(&OI->Kurs));
    if( TestK(&k)!=EOK || BufK(&k)[0]!=PNastroj )
    {
      BankaItem *B=SekvenceK(&k)->Pis->Nastroje;
      BufK(&k)[0]=PNastroj;
      BufK(&k)[1]=PridejRetez(B,S);
      if( BufK(&k)[1]<MAXERR ) Chyba(ERAM);
      else
      {
        BufK(&k)[2]=BufK(&k)[3]=0;
        if( KorektVloz(&k)!=OK )
        {
          ZrusRetez(B,BufK(&k)[1]);
          Chyba(ERAM);
        }
      }
    }
    else
    {
      BankaItem *B=SekvenceK(&k)->Pis->Nastroje;
      int NN=PridejRetez(B,S);
      if( NN>MAXERR )
      {
        ZrusRetez(B,BufK(&k)[1]);
        BufK(&k)[1]=NN;
        PlatiProc(PisK(&k),==OK);
      }
      else Chyba(ERAM);
    }
    KonecEdit(&k,OI);
  }
  (void)CasPos,(void)Kan;
}
#endif

static Err SZacRepet( OknoInfo *OI, int Opak, SekvSoubor *dle )
{
  SekvKursor k;
  Err ret;
  RZapisK(&k,AktK(dle));
  BufK(&k)[0]=PRep;
  BufK(&k)[1]=Opak;
  BufK(&k)[2]=0;
  BufK(&k)[3]=0;
  ret=KorektVloz(&k);
  KonecEdit(&k,OI);
  return ret;
}
static Err SKonRepet( OknoInfo *OI, SekvSoubor *dle )
{
  SekvKursor k;
  Err ret;
  RZapisK(&k,AktK(dle));
  BufK(&k)[0]=PERep;
  BufK(&k)[1]=0;
  BufK(&k)[2]=0;
  BufK(&k)[3]=0;
  ret=KorektVloz(&k);
  KonecEdit(&k,OI);
  return ret;
}
static void SEdZacRepet( OknoInfo *OI, int Opak, SekvSoubor *dle )
{
  SekvKursor k;
  RZapisK(&k,AktK(dle));
  k.Buf[0]=PRep;
  k.Buf[1]=Opak;
  k.Buf[2]=0;
  k.Buf[3]=0;
  PlatiProc( PisK(&k), ==OK);
  KonecEdit(&k,OI);
}

#if 0
void ZacRepet( int Opak, int Kan, CasT CasPos )
{
  OknoInfo *OI=OInfa[VrchniSekv()];
  SZacRepet(OI,Opak,&OI->Kurs);
  (void)CasPos,(void)Kan;
}
void KonRepet( int Kan, CasT CasPos )
{
  OknoInfo *OI=OInfa[VrchniSekv()];
  SKonRepet(OI,&OI->Kurs);
  (void)CasPos,(void)Kan;
}
void EdZacRepet( int Opak, int Kan, CasT CasPos )
{
  OknoInfo *OI=OInfa[VrchniSekv()];
  SEdZacRepet(OI,Opak,&OI->Kurs);
  (void)CasPos,(void)Kan;
}
void BlokRepetice( int LHan, int Opak )
{
  OknoInfo *OI=OInfa[LHan];
  Plati(VrchniSekv()>=0);
  if( OI->ZBStopa>=0 )
  {
    int D=OI->ZBStopa;
    int H=OI->KBStopa;
    if( D==H )
    { /* jednost. rezim */
      if( SZacRepet(OI,Opak,&OI->ZBlok)==OK )
      {
        if( SKonRepet(OI,&OI->KBlok)==OK )
        {
          JCtiS(&OI->KBlok);
        }
      }
    }
    else
    { /* vicest. rezim */
      int Kan;
      if( D>H ) H^=D,D^=H,H^=D;
      for( Kan=D; Kan<=H; Kan++ )
      {
        SekvSoubor k;
        Err rz;
        RozdvojS(&k,&OI->Soub[Kan]);
        NajdiCasS(&k,CasJeS(&OI->ZBlok));
        rz=SZacRepet(OI,Opak,&k);
        ZavriS(&k);
        if( rz==OK )
        {
          RozdvojS(&k,&OI->Soub[Kan]);
          NajdiCasS(&k,CasJeS(&OI->KBlok));
          SKonRepet(OI,&k);
          ZavriS(&k);
        }
        else if( rz==ERAM ) break;
      }
    } /* provedena editace */
  }
}
#endif

void Predtakti( int Kan, CasT CasPos )
{
  JednotkaBuf Buf;
  Buf[0]=PDvojTakt;
  Buf[1]=0;
  Buf[2]=0;
  Buf[3]=0;
  PovelEdit(Buf,Kan,CasPos,False);
}

static int ZacTempo( Sekvence *S )
{
  int ret=Tempo;
  SekvSoubor s;
  OtevriS(&s,S,Cteni);
  while( CasJeS(&s)==0 )
  {
    if( JCtiS(&s)!=OK ) break;
    if( BufS(&s)[0]==PTempo ) ret=BufS(&s)[1];
  }
  ZavriS(&s);
  return ret;
}
static void ZacTakt( Sekvence *S, int *HH, int *DD )
{
  SekvSoubor s;
  OtevriS(&s,S,Cteni);
  *HH=*DD=4;
  while( CasJeS(&s)==0 )
  {
    if( JCtiS(&s)!=OK ) break;
    if( BufS(&s)[0]==PNTakt ) *HH=BufS(&s)[1],*DD=BufS(&s)[2];
  }
  ZavriS(&s);
}

#if 0
int HlavniTempo( int LHan )
{
  if( LHan>=0 )
  {
    OknoInfo *OI=OInfa[LHan];
    Pisen *P=SekvenceOI(OI)->Pis;
    Sekvence *S=NajdiSekv(P,MainMel,0);
    if( S ) return ZacTempo(S);
  }
  return Tempo;
}
void HlavniTakt( int LHan, int *HH, int *DD )
{
  *HH=*DD=4;
  if( LHan>=0 )
  {
    OknoInfo *OI=OInfa[LHan];
    Pisen *P=SekvenceOI(OI)->Pis;
    Sekvence *S=NajdiSekv(P,MainMel,0);
    if( S ) ZacTakt(S,HH,DD);
  }
}
#endif

void InitTonina( Sekvence *D, SekvSoubor *s )
{
  Flag TaktC=False,ToniC=False,TempC=False;
  SekvKursor d;
  SekvSoubor z;
  RozdvojS(&z,s);
  OtevriK(&d,D,Zapis);
  while( ZpatkyS(&z)==OK )
  {
    if( TestS(&z)!=OK ) break;
    switch( BufS(&z)[0] )
    {
      case PNTakt:  if( !TaktC ) {TaktC=True;goto ZapisBuf;};break;
      case PTonina: if( !ToniC ) {ToniC=True;goto ZapisBuf;};break;
      case PTempo:  if( !TempC ) {TempC=True;goto ZapisBuf;};break;
      ZapisBuf:
        EquJed(BufK(&d),BufS(&z));
        if( VlozK(&d)!=OK )  {Chyba(ERAM);break;}
    }
  }
  ZavriS(&z);
  NormujK(&d);
  ZavriK(&d);
}

#if 0
void ZalozHlas( void )
{
  int LHan=VrchniSekv();
  if( LHan>=0 )
  {
    OknoInfo *OI=OInfa[LHan];
    if( OI->NSoub<NMaxHlasu )
    {
      Sekvence *D=SekvenceS(&OI->Soub[OI->KKan]);
      Sekvence *H=NovyHlas(D);
      SekvSoubor t;
      if( !H ) {Chyba(ERAM);return;}
      ZrusBlok(LHan);
      ZavriOstatni(LHan);
      OtevriS(&t,D,Cteni);
      NajdiSCasS(&t,0);
      InitTonina(H,&t);
      ZavriS(&t);
      SekvPriot(LHan,OI->KKan+1,H);
    }
  }
}

void ProhodHlas( void )
{
  int LHan=VrchniSekv();
  if( LHan>=0 )
  {
    OknoInfo *OI=OInfa[LHan];
    if( OI->KKan>0  )
    {
      SekvSoubor *s,*n;
      Sekvence *H=SekvenceS(&OI->Soub[OI->KKan]); /* bude z nej horni */
      Sekvence *D=SekvenceS(&OI->Soub[OI->KKan-1]); /* bude z nej dolni */
      SekvSoubor pom;
      ZrusBlok(LHan);
      ZavriOstatni(LHan);
      s=&OI->Soub[OI->KKan];
      RozdvojS(&pom,s);ZavriS(s);
      n=&OI->Soub[OI->KKan-1];
      RozdvojS(s,n);ZavriS(n);
      RozdvojS(n,&pom);ZavriS(&pom);
      OI->KKan--;
      IStopaZKKan(OI);
      PosunCas(OI);
      UkazKursor(OI);
      ProhHlas(D,H);
      Kresli(LHan);
    }
  }
}

void ZrusHlas( void )
{
  int LHan=VrchniSekv();
  if( LHan>=0 )
  {
    OknoInfo *OI=OInfa[LHan];
    Sekvence *H=SekvenceS(&OI->Kurs);
    Plati(H);
    if( PosDelka(H)<=0 || 1==AlertRF(ZHLASA,2,OI->KKan+1,NazevSekvPis(H)) )
    {
      ZavriOstatni(LHan);
      ZrusBlok(LHan);
      if( OI->NSoub>1 )
      {
        CasT CasPos=CasKursoru(OI);
        SekvPrizav(LHan);
        MazHlas(H);
        NajdiKursor(OI,CasPos);
      }
      else
      {
        FZavri(LHan);
        MazSekv(H);
      }
    }
  }
}

static void OtevriKSekv( int LHan )
{
  OknoInfo *OI=OInfa[LHan];
  Pisen *P=SekvenceOI(OI)->Pis;
  SekvSoubor *s=&OI->Kurs;
  Sekvence *S;
  char *NS;
  int Je;
  Plati( BufS(s)[0]==PVyvolej || BufS(s)[0]==PZacSekv );
  S=NajdiSekvBI(P,BufS(s)[1],0);
  if( !S )
  {
    NS=ZBanky(P->NazvySekv,BufS(s)[1]);
    Plati(NS);
    S=NovaSekv(P,NS);
    if( !S ) {Chyba(ERAM);return;}
  }
  if( S )
  {
    Je=JeOtevrena(S,&AktK(s)->A);
    if( Je>=0 ) Topni(Je);
    else
    {
      WindRect W;
      KursorRect(&W,LHan);
      SekvOtevri(P,S,NULL,-1,-1,&W,&AktK(s)->A);
    }
  }
}
#endif

static int DelkaZlomek( int Z, Flag Tec, Flag Tri )
{
  Z=1<<Z;
  Z=Takt/Z;
  if( Tec ) Z*=3,Z/=2;
  if( Tri ) Z*=2,Z/=3;
  return Z;
}
static int ZlomekDelka( int D )
{
  int Z,Dum;
  AttrDelka(D,&Z,&Dum);
  return Z;
}

#if 0
static void MEditujTon( int LHan, int OMX, int OMY )
{
  OknoInfo *OI=OInfa[LHan];
  int Druh0,MDelka0;
  int Prop;
  int MDruh,MDelka;
  Flag Tec,Tri;
  SekvKursor k;
  RZapisK(&k,AktK(&OI->Kurs) );
  PlatiProc( TestK(&k), ==OK );
  Druh0=BufK(&k)[0];
  MDelka0=ZlomekDelka(BufK(&k)[2]);
  Tri=!BezTrioly(BufK(&k)[2]);
  Tec=!BezTecky(BufK(&k)[2]);
  Plati(Druh0<=PPau);
  Prop=UrciEProp(BufK(&k));
  Mouse(Off);
  do
  {
    int MX,MY;
    MysVOkneXY(LHan,&MX,&MY);
    MDruh=Druh0+(OMY-MY)/(ystep/2);
    if( MDruh>PPau ) MDruh=PPau;
    if( MDruh<TC ) MDruh=TC;
    MDelka=(MX-OMX)/(ystep*2)+MDelka0;
    if( MDelka<0 ) MDelka=0;
    else if( MDelka>5 ) MDelka=5;
    BufK(&k)[0]=MDruh;
    BufK(&k)[2]=DelkaZlomek(MDelka,Tec,Tri);
    KresliENotu(LHan,BufK(&k),Prop);
  } while( Buttons()&3 );
  if( MDruh==PPau ) BufK(&k)[2]=DelkaZlomek(MDelka,False,Tri);
  PlatiProc( PisK(&k), ==OK );
  Mouse(On);
  if( BufK(&k)[1]&Soubezna ) KonTon(&k);
  TNormujK(&k,AktK(&OI->Kurs));
  ZavriK(&k);
  Kresli(LHan);
}

static void EAttrib( int LHan )
{
  OknoInfo *OI=OInfa[LHan];
  Flag Tec,Tri,Lg;
  int VI,DI;
  int ZDelka;
  WindRect K;
  SekvKursor k;
  RZapisK(&k,AktK(&OI->Kurs));
  PlatiProc( TestK(&k), ==OK );
  Plati(BufK(&k)[0]<=PPau);
  ZDelka=ZlomekDelka(BufK(&k)[2]);
  Tri=!BezTrioly(BufK(&k)[2]);
  Tec=!BezTecky(BufK(&k)[2]);
  Lg=(BufK(&k)[1]&Leg)!=0;
  if( Tri ) DI=2;
  else if( Tec ) DI=1;
  else DI=0;
  switch( BufK(&k)[1]&KBOMaskA )
  {
    case KrizekA: VI=1;break;
    case BeckoA: VI=2;break;
    case OdrazA: VI=3;break;
    default: VI=0;break;
  }
  KursorRect(&K,LHan);
  AttribD(BufK(&k)[0]<PPau,&VI,&DI,&Lg,&K);
  Tec=DI==1;
  Tri=DI==2;
  BufK(&k)[2]=DelkaZlomek(ZDelka,Tec,Tri);
  {
    int A=BufK(&k)[1]&~(KBOMaskA|Leg);
    int AS;
    if( VI==1 ) AS=KrizekA;
    else if( VI==2 ) AS=BeckoA;
    else if( VI==3 ) AS=OdrazA;
    else AS=0;
    if( Lg ) AS|=Leg;
    BufK(&k)[1]=A|AS;
  }
  PlatiProc( PisK(&k), ==OK );
  ZavriK(&k);
  Kresli(LHan);
}

void MEditace( int LHan, int mx, int my, Flag Dvoj )
{
  OknoInfo *OI=OInfa[LHan];
  Err err;
  err=TestS(&OI->Kurs);
  if( err==OK )
  {
    if( BufS(&OI->Kurs)[0]<=PPau )
    {
      if( !Dvoj ) MEditujTon(LHan,mx,my);
      else EAttrib(LHan);
    }
    else
    {
      WindRect K;
      Plati( LHan==VrchniSekv() );
      KursorRect(&K,LHan);
      switch( BufS(&OI->Kurs)[0] )
      {
        case PNastroj: ZmenNas(!Dvoj);break;
        case PTempo: TempoD(True,&K,!Dvoj);break;
        case PNTakt: TaktD(True,&K,!Dvoj);break;
        case PTonina: ToninaD(True,&K,!Dvoj);break;
        case PHlasit: HlasitD(!Dvoj);break;
        case POktava: OktavaD(!Dvoj);break;
        case PStereo: StereoD(!Dvoj);break;
        case PEfekt: EfektD(!Dvoj);break;
        case PZacSekv: case PVyvolej:
          if( Dvoj ) OtevriKSekv(LHan);
          else ZmenVyvolejS();
          break;
        case PRep: EdRepetD();break;
      }
    }
  }
}

void NasVloz( int LHan )
{
  Kresli(LHan);
  ZmenNas(True);
}
#endif

static void FTonVloz( int LHan, int mx, int my, Flag Dvoj, Flag Soub )
{
  #if 0
  OknoInfo *OI=OInfa[LHan];
  Err ret;
  enum CPovel Druh;
  JednotkaBuf B;
  Mouse(HOURGLASS);
  if( !PredTon(AktK(&OI->Kurs),B) ) B[2]=Takt/4;
  Druh=(kanypos(OI->KKan-OI->IStopa)-my-2)*2/ystep+TC+7;
  if( Druh<TC ) Druh=TC;
  if( Druh>PPau ) Druh=PPau;
  B[0]=Druh;
  B[1]=B[3]=0;
  ZacBlokZmen(SekvenceS(&OI->Kurs));
  ret=FVlozTon(OI,Soub,B);
  Kresli(LHan);
  if( ret>=OK )
  {
    if( OI->KR.x<0 || OI->KR.x>XMax(LHan)-32 )
    {
      UkazKursor(OI);
      Kresli(LHan);
    }
    MEditace(LHan,mx,my,Dvoj);
  }
  KonBlokZmen(SekvenceS(&OI->Kurs));
  #endif
}

void TonVloz( int LHan, int mx, int my, Flag Dvoj )
{
  FTonVloz(LHan,mx,my,Dvoj,False);
}

void TonSVloz( int LHan, int mx, int my, Flag Dvoj )
{
  FTonVloz(LHan,mx,my,Dvoj,True);
}
