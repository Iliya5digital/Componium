/* Componium - prevody - vnitrni format <-> text */
/* SUMA, 9/1992-3/1993 */

#include "macros.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "stdc\fileutil.h"

#include "utlcompo.h"

const char MainMel[]="$Compo$";

static int ChybaF( const char *S, SekvKursor *s ) {(void)S,(void)s;return ERR;}
int (*ChybaC)( const char *S, SekvKursor *s )=ChybaF;

static char KonvTon( int T )
{
  switch( T )
  {
    case TC: return 'C';
    case TD: return 'D';
    case TE: return 'E';
    case TF: return 'F';
    case TG: return 'G';
    case TA: return 'A';
    default: return 'H';
  }
}

void AttrDelka( int Delka, int *DelVys, int *Attr )
{ 
  switch( Delka )
  {
    #define DelkaCase(i) \
      case (Takt>>(i))*3/2: *DelVys=i;*Attr=Tecka;break; \
      case (Takt>>(i)): *DelVys=i;*Attr=0;break; \
      case (Takt>>(i))*2/3: *DelVys=i;*Attr=Triola;break;
    DelkaCase(0);
    DelkaCase(1);
    DelkaCase(2);
    DelkaCase(3);
    DelkaCase(4);
    DelkaCase(5);
    default: Plati(False);
  }
}

#define pis(t,a) (t)->SSBuf[(t)->SSPtr++]=(a)

static void zapis( SekvText *t, const char *T )
{
  for( ; *T; T++ ) pis(t,*T);
}
static void piscislo( SekvText *t, int C )
{
  char c[10];
  sprintf(c,"%d",C);
  zapis(t,c);
}

static void PisAttrDelku( SekvText *t, int Attr, int Delka )
{
  int DelAtt,DelVys;
  AttrDelka(Delka,&DelVys,&DelAtt);
  switch( Attr&KBOMaskA )
  {
    case KrizekA: pis(t,'#');break;
    case BeckoA: pis(t,'b');break;
    case OdrazA: pis(t,'=');break;
  }
  switch( DelVys )
  {
    case 0: pis(t,'1');break;
    case 1: pis(t,'2');break;
    case 2: pis(t,'4');break;
    case 3: pis(t,'8');break;
    case 4: pis(t,'1');pis(t,'6');break;
    case 5: pis(t,'3');pis(t,'2');break;
    default: Plati(False);break;
  }
  if( DelAtt&Triola ) pis(t,'3');
  if( DelAtt&Tecka ) pis(t,'.');
  if( Attr&Soubezna ) pis(t,'|');
  if( Attr&Leg ) pis(t,'_');
}

SekvText *OtevriT( Sekvence *S )
{
  SekvText *t=new(SekvText);
  if( !t ) return t;
  OtevriK(&t->SS,S,Cteni);
  t->SSPtr=t->SSCte=0;
  t->NovyRad=True;
  t->Indent=1;
  return t;
}
void ZavriT( SekvText *t )
{
  ZavriK(&t->SS);
  myfree(t);
}

static void Odsad( SekvText *t )
{
  int i;
  for( i=0; i<t->Indent; i++ ) pis(t,'\t');
}

int CtiT( SekvText *t )
{
  int ret;
  Plati( t->SSCte<=t->SSPtr );
  if( t->SSCte >= t->SSPtr )
  {
    ret=CtiK(&t->SS);
    switch( ret )
    {
      case KON: return KON;
      default: return ret;
      case OK:
      {
        t->SSCte=t->SSPtr=0;
        if( t->NovyRad )
        {
          t->NovyRad=False;
          Odsad(t);
        }
        else pis(t,' ');
        if( t->SS.Buf[0]<PPau )
        {
          JednItem B=t->SS.Buf[0];
          int Attr=t->SS.Buf[1];
          int Delka=t->SS.Buf[2];
          int Ton=(B-TC)%Okt;
          int Oktava=(B-TC)/Okt;
          if( Oktava<NormOkt )
          {
            pis(t,KonvTon(Ton));
            while( ++Oktava<NormOkt ) pis(t,'-');
          }
          else
          {
            pis(t,KonvTon(Ton)+'a'-'A');
            while( Oktava-->NormOkt ) pis(t,'+');
          }
          PisAttrDelku(t,Attr,Delka);
        }
        else
        {
          int Delka,Attr;
          switch( t->SS.Buf[0] )
          {
            case PPau:
              Attr=t->SS.Buf[1];
              Delka=t->SS.Buf[2];
              zapis(t,"P ");
              PisAttrDelku(t,Attr,Delka);
              break;
            case PDvojTakt:
              zapis(t,"||");
            case PTakt:
              pis(t,'\n');
              t->NovyRad=True;
              break;
            case PTonina:
            {
              const static char DurToninaK[8][4]={"C","G","D","A","E","H","F#","C#"};
              const static char DurToninaB[8][4]={"C","F","Bb","Eb","Ab","Db","Gb","Cb"};
              const static char MolToninaK[8][4]={"A","E","H","F#","C#","G#","D#","A#"};
              const static char MolToninaB[8][4]={"A","D","G","C","F","Bb","Eb","Ab"};
              int NK=t->SS.Buf[1];
              if( !t->SS.Buf[2] )
              {
                zapis(t,"Minor ");
                zapis(t,NK>0 ? MolToninaK[NK] : MolToninaB[-NK]);
              }
              else
              {
                zapis(t,"Major ");
                zapis(t,NK>0 ? DurToninaK[NK] : DurToninaB[-NK]);
              }
              break;
            }
            case PNTakt:
              zapis(t,"Measure ");
              piscislo(t,t->SS.Buf[1]);
              pis(t,' ');
              piscislo(t,t->SS.Buf[2]);
              break;
            case PTempo:
              zapis(t,"Tempo ");
              piscislo(t,t->SS.Buf[1]);
              break;
            case PNastroj:
              zapis(t,"Instr ");
              zapis(t,ZBanky(t->SS.S->Pis->Nastroje,t->SS.Buf[1]));
              break;
            case PHlasit:
              zapis(t,"Vol ");
              piscislo(t,t->SS.Buf[1]);
              break;
            case POktava:
              if( t->SS.Buf[2] ) zapis(t,"OctR ");
              else zapis(t,"OctA ");
              piscislo(t,t->SS.Buf[1]);
              break;
            case PStereo:
              zapis(t,"Stereo ");
              piscislo(t,t->SS.Buf[1]);
              pis(t,' ');
              piscislo(t,t->SS.Buf[2]);
            break;
            case PEfekt:
              zapis(t,"Effect ");
              piscislo(t,t->SS.Buf[1]);
              pis(t,' ');
              piscislo(t,t->SS.Buf[2]);
            break;
            case PVyvolej:
              zapis(t,"Part ");
              zapis(t,ZBanky(t->SS.S->Pis->NazvySekv,t->SS.Buf[1]));
              zapis(t,".");
              piscislo(t,t->SS.Buf[2]);
            break;
            case PRep:
              zapis(t,"Rep ");
              piscislo(t,t->SS.Buf[1]);
              pis(t,'\n');
              t->Indent++;
              t->NovyRad=True;
              break;
            case PERep:
              if( !t->NovyRad ) zapis(t,"\n");
              t->Indent--;
              Odsad(t);
              zapis(t,"Rep~");
              break;
            default: zapis(t,"??? ");break;
          }
        }
        Plati(t->SSPtr<sizeof(t->SSBuf)-16);
      }
    }
  }
  return t->SSBuf[t->SSCte++];
}

#undef pis

/* Cteni souboru */

#define PismenoCislo(c) ( Pismeno(c) || isdigit(c) )

static Flag Pismeno( char c )
{
  return c&0x80 || isalpha(c) || strchr("_()[]@$&!:+-",c);
}

static int Mezera( FILEEmul *f )
{
  int c;
  while( (c=fgetc(f))!=EOF && isspace(c) ) {}
  return c;
}

char *CtiRadek( FILEEmul *f )
{
  int c;
  enum {LS=256};
  static char Slovo[LS];
  int S=0;
  c=Mezera(f);
  if( c==EOF ) return NULL;
  do
  {
    if( S<LS-1 && c!='\r') Slovo[S++]=c;
  }
  while( c=fgetc(f),c!=EOF && c!='\n' );
  Slovo[S++]=0;
  return Slovo;
}

char *CtiSlovo( FILEEmul *f )
{
  int c;
  enum {LS=32};
  static char Slovo[LS];
  int S=0;
  c=Mezera(f);
  if( c==EOF ) return NULL;
  do
  {
    if( S<LS-1 ) Slovo[S++]=c;
  }
  while( c=fgetc(f),c!=EOF && !isspace(c) );
  Slovo[S++]=0;
  return Slovo;
}

long CtiLCislo( FILEEmul *f )
{
  char *S=CtiSlovo(f);
  char *p;
  long l;
  if( !S ) return ERR;
  l=strtol(S,&p,10);
  if( p!=S+strlen(S) ) return ERR;
  return l;
}

Flag PovolJmeno( const char *N )
{
  if( !Pismeno(*N++) ) return False;
  for( ; *N; N++ ) if( !PismenoCislo(*N) ) return False;
  return True;
}
Flag SPovolJmeno( const char *N )
{
  return PovolJmeno(N) || !strcmp(N,MainMel) || !strcmp(N,"$Melodie$");
  /* preklad !!! */
}

static int KonvDelka( int Delka, int Attr )
{
  if( Delka%10==3 )
  {
    Delka/=10;
    Attr|=Triola;
  }
  switch( Delka )
  {
    default: return -1;
    case 1: case 2: case 4: case 8: case 16: case 32: break;
  }
  Delka=Takt/Delka;
  if( Attr&Triola ) Delka*=2,Delka/=3;
  if( Attr&Tecka ) Delka*=3,Delka/=2;
  return Delka;
}

enum {Be=1,Krizek,Odraz};

static int ZpracujTon( char *S, int *Attrib )
{
  int Ton;
  *Attrib=0;
  switch( *S++ )
  {
    case 'c': case 'C': Ton=TC;break;
    case 'd': case 'D': Ton=TD;break;
    case 'e': case 'E': Ton=TE;break;
    case 'f': case 'F': Ton=TF;break;
    case 'g': case 'G': Ton=TG;break;
    case 'a': case 'A': Ton=TA;break;
    case 'h': case 'H': Ton=TH;break;
    case 'b': case 'B': Ton=TH;*Attrib=Be;break;
    default: return ERR;
  }
  switch ( *S++ )
  {
    case 'b': *Attrib=Be;break;
    case '#': *Attrib=Krizek;break;
    case '=': *Attrib=Odraz;break;
    case 0: return Ton;
    default: return ERR;
  }
  if( *S ) return ERR;
  return Ton;
}
static int NKrizDur( int Ton, int Attrib )
{
  const static int NBezKriz[7]={0,2,4,-1,1,3,5};
  const static int NSKriz[7]={7,-3,-1,6,-4,-2,0};
  const static int NSBe[7]={-7,-5,-3,4,-6,-4,-2};
  if( Attrib==Krizek ) return NSKriz[Ton-TC];
  else if( Attrib==Be ) return NSBe[Ton-TC];
  else return NBezKriz[Ton-TC];
}
static int NKrizMol( int Ton, int Attrib )
{
  const static int NBezKriz[7]={-3,-1,1,-4,-2,0,2};
  const static int NSKriz[7]={4,6,-4,3,5,7,-3};
  const static int NSBe[7]={2,4,-3,1,3,-7,-5};
  if( Attrib==Krizek ) return NSKriz[Ton-TC];
  else if( Attrib==Be ) return NSBe[Ton-TC];
  else return NBezKriz[Ton-TC];
}


typedef Err PovProc( SekvKursor *s, FILEEmul *f );

typedef struct
{
  const char *Text;
  PovProc *Proc;
} Povel;

static Err CtiBlok( SekvKursor *s, FILEEmul *f );

static Err KONProc( SekvKursor *s, FILEEmul *f )
{
  (void)s,(void)f;
  return KON;
}

int CtiCislo( FILEEmul *f )
{
  char *S=CtiSlovo(f);
  char *p;
  long l;
  if( !S ) return -0x8000;
  l=strtol(S,&p,10);
  if( p!=S+strlen(S) || l>0x7fff ) return -0x8000;
  return (int)l;
}

static Err PauProc( SekvKursor *s, FILEEmul *f )
{
  char *S=CtiSlovo(f);
  int Delka=0;
  int Attr=0;
  int c;
  if( !S ) return ERR;
  while ( (c=*S)!=0 && isdigit(c) )
  {
    Delka*=10;
    Delka+=c-'0';
    S++;
  }
  while ( (c=*S++)!=0 )
  {
    if( c=='.' ) Attr|=Tecka;
    else if( c=='|' ) Attr|=Soubezna;
    else return ERR;
  }
  s->Buf[0]=PPau;
  s->Buf[2]=c=KonvDelka(Delka,Attr);
  s->Buf[1]=Attr&Soubezna;
  if( c<=0 ) return ChybaC(S,s);
  s->Buf[3]=0;
  return VlozK(s);
}
static Err InsProc( SekvKursor *s, FILEEmul *f )
{
  char *W=CtiSlovo(f);
  int ps;
  if( !W || !PovolJmeno(W) ) return ERR;
  s->Buf[0]=PNastroj;
  ps=PridejRetez(s->S->Pis->Nastroje,W);
  if( ps<0 ) return ps;
  s->Buf[1]=ps;
  s->Buf[2]=s->Buf[3]=0;
  return VlozK(s);
}
static Err HlasProc( SekvKursor *s, FILEEmul *f )
{
  int C=CtiCislo(f);
  if( C<MAXERR ) return C;
  if( C<0 ) C=0;
  else if( C>LMaxVol ) C=LMaxVol;
  s->Buf[0]=PHlasit;
  s->Buf[1]=C;
  s->Buf[2]=s->Buf[3]=0;
  return VlozK(s);
}	
static Err HlasOProc( SekvKursor *s, FILEEmul *f )
{
  int C=CtiCislo(f);
  if( C<MAXERR ) return C;
  C++;
  C*=64*2/3;
  if( C<0 ) C=0;
  else if( C>LMaxVol ) C=LMaxVol;
  s->Buf[0]=PHlasit;
  s->Buf[1]=C;
  s->Buf[2]=s->Buf[3]=0;
  return VlozK(s);
}	
static Err OktavRelProc( SekvKursor *s, FILEEmul *f )
{
  int C=CtiCislo(f);
  if( C<MAXERR ) return C;
  if( C<-3 ) C=-3;
  if( C>3 ) C=3;
  s->Buf[0]=POktava;
  s->Buf[1]=C;
  s->Buf[2]=1;
  s->Buf[3]=0;
  return VlozK(s);
}	
static Err OktavAbsProc( SekvKursor *s, FILEEmul *f )
{
  int C=CtiCislo(f);
  if( C<MAXERR ) return C;
  if( C<-3 ) C=-3;
  if( C>3 ) C=3;
  s->Buf[0]=POktava;
  s->Buf[1]=C;
  s->Buf[2]=0;
  s->Buf[3]=0;
  return VlozK(s);
}	
static int OctInit; /* automaticka relativizace starych melodii */
static Err OktavProc( SekvKursor *s, FILEEmul *f )
{
  int C=CtiCislo(f);
  Err ret;
  if( C<MAXERR ) return C;
  if( C<-3 ) C=-3;
  if( C>3 ) C=3;
  if( OctInit>MAXERR )
  {
    int RP=C-OctInit;
    while( RP>+3 )
    {
      s->Buf[0]=POktava;
      s->Buf[1]=+3;
      s->Buf[2]=1;
      s->Buf[3]=0;
      ret=VlozK(s);
      if( ret<EOK ) return ret;
      RP-=+3;
    }
    while( RP<-3 )
    {
      s->Buf[0]=POktava;
      s->Buf[1]=-3;
      s->Buf[2]=1;
      s->Buf[3]=0;
      ret=VlozK(s);
      if( ret<EOK ) return ret;
      RP-=-3;
    }
    s->Buf[0]=POktava;
    s->Buf[1]=RP;
    s->Buf[2]=1;
    s->Buf[3]=0;
    OctInit=C;
    return VlozK(s);
  }
  else
  {
    s->Buf[0]=POktava;
    s->Buf[1]=C;
    s->Buf[2]=0;
    s->Buf[3]=0;
    OctInit=C;
    return VlozK(s);
  }
}	
static Err EffProc( SekvKursor *s, FILEEmul *f )
{
  int C;
  s->Buf[0]=PEfekt;
  C=CtiCislo(f);if( C<MAXERR ) return C;
  if( C<0 ) C=0;if( C>1000 ) C=1000;
  s->Buf[1]=C;
  C=CtiCislo(f);if( C<MAXERR ) return C;
  if( C<0 ) C=0;if( C>1000 ) C=1000;
  s->Buf[2]=C;
  s->Buf[3]=0;
  return VlozK(s);
}	
static Err SterProc( SekvKursor *s, FILEEmul *f )
{
  int C;
  s->Buf[0]=PStereo;
  C=CtiCislo(f);if( C<MAXERR ) return C;
  if( C<MinSt ) C=MinSt;if( C>MaxSt ) C=MaxSt;
  s->Buf[1]=C;
  C=CtiCislo(f);if( C<MAXERR ) return C;
  if( C<MinSt ) C=MinSt;if( C>MaxSt ) C=MaxSt;
  s->Buf[2]=C;
  s->Buf[3]=0;
  return VlozK(s);
}	
static Err DurProc( SekvKursor *s, FILEEmul *f )
{
  char *W=CtiSlovo(f);
  int Attrib,TonI;
  if( !W ) return ERR;
  TonI=ZpracujTon(W,&Attrib);
  if( TonI<0 ) return TonI;
  s->Buf[0]=PTonina;
  s->Buf[1]=NKrizDur(TonI,Attrib);
  s->Buf[2]=True;
  s->Buf[3]=0;
  return VlozK(s);
}
static Err MolProc( SekvKursor *s, FILEEmul *f )
{
  char *W=CtiSlovo(f);
  int Attrib,TonI;
  if( !W ) return ERR;
  TonI=ZpracujTon(W,&Attrib);
  if( TonI<0 ) return TonI;
  s->Buf[0]=PTonina;
  s->Buf[1]=NKrizMol(TonI,Attrib);
  s->Buf[2]=False;
  s->Buf[3]=0;
  return VlozK(s);
}
static Err DvojTProc( SekvKursor *s, FILEEmul *f )
{
  s->Buf[0]=PDvojTakt;
  s->Buf[1]=s->Buf[2]=s->Buf[3]=0;
  (void)f;
  return VlozK(s);
}

static Err TaktProc( SekvKursor *s, FILEEmul *f )
{
  int Hor,Dol;
  Hor=CtiCislo(f);
  if( Hor<MAXERR ) return Hor;
  Dol=CtiCislo(f);
  if( Dol<MAXERR ) return Dol;
  if( Hor<1 ) Hor=1;
  else if( Hor>9 ) Hor=9;
  if( Dol<1 ) Dol=1;
  else if( Dol>8 ) Dol=8;
  s->Buf[0]=PNTakt;
  s->Buf[1]=Hor;
  s->Buf[2]=Dol;
  s->Buf[3]=0;
  return VlozK(s);
}
static Err TempoProc( SekvKursor *s, FILEEmul *f )
{
  int Hor;
  Hor=CtiCislo(f);
  if( Hor<MAXERR ) return Hor;
  if( Hor<MinTemp ) Hor=MinTemp;
  if( Hor>MaxTemp ) Hor=MaxTemp;
  s->Buf[0]=PTempo;
  s->Buf[1]=Hor;
  s->Buf[2]=0;
  s->Buf[3]=0;
  return VlozK(s);
}

static Err SekvProc( SekvKursor *s, FILEEmul *f )
{
  char *W=CtiSlovo(f);
  Err err;
  if( !W ) return ERR;
  else
  {
    char *Ext=strrchr(W,'.');
    int ExtN;
    if( Ext ) *Ext++=0,ExtN=atoi(Ext);
    else ExtN=0;
    if( !SPovolJmeno(W) ) return ERR;
    else
    {
      Sekvence *S=s->S;
      Sekvence *Z;
      Z=NajdiSekv(S->Pis,W,ExtN);
      if( Z && ( Zavislost(Z,S) || PodHlas(Z,S) ) ) return ERR;
    }
    s->Buf[0]=PVyvolej;
    s->Buf[1]=PridejRetez(s->S->Pis->NazvySekv,W);
    s->Buf[2]=ExtN;
    s->Buf[3]=0;
    err=VlozK(s);
    if( err!=OK ) ZrusRetez(s->S->Pis->NazvySekv,s->Buf[1]);
    return err;
  }
}
static Err RepProc( SekvKursor *s, FILEEmul *f )
{
  int C=CtiCislo(f);
  if( C<MAXERR ) return C;
  if( C<1 ) C=1;
  if( C>99 ) C=99;
  s->Buf[0]=PRep;
  s->Buf[1]=C;
  s->Buf[2]=s->Buf[3]=0;
  return VlozK(s);
}	
static Err ERepProc( SekvKursor *s, FILEEmul *f )
{
  (void)f;
  s->Buf[0]=PERep;
  s->Buf[1]=s->Buf[2]=s->Buf[3]=0;
  return VlozK(s);
}

/* preklad !!! */
static Povel Pov[]=
{
  "{",CtiBlok,
  "}",KONProc,
  "P",PauProc,
  "Nastr",InsProc,"Instr",InsProc,
  "Hlasit",HlasOProc,
  "Hlas",HlasProc,"Vol",HlasProc,
  "Oktava",OktavProc,"Oct",OktavProc,
  "OctR",OktavRelProc,"OctA",OktavAbsProc,
  "Stereo",SterProc,
  "Effect",EffProc,
  "Dur",DurProc,"Major",DurProc,
  "Mol",MolProc,"Minor",MolProc,
  "||",DvojTProc,
  "Takt",TaktProc,"Measure",TaktProc,
  "Tempo",TempoProc,
  "Sekv",SekvProc,"Part",SekvProc,
  "Rep",RepProc,
  "Rep~",ERepProc,
  ""
};


static Err CtiJedn( SekvKursor *s, FILEEmul *f )
{
  char *W=CtiSlovo(f),*Slovo=W;
  int Ton;
  int Attr=0;
  int Delka=0,c;
  int Oktava;
  int NeumTest=False;
  if( !W ) return ERR;
  {
    Povel *P;
    for( P=Pov; P->Text[0]!=0; P++ )
    {
      if( !strcmpi(W,P->Text) ) return P->Proc(s,f);
    }
  }
  switch( *W++ )
  {
    case 'C': Ton=TC;Oktava=NormOkt-1;break;
    case 'D': Ton=TD;Oktava=NormOkt-1;break;
    case 'E': Ton=TE;Oktava=NormOkt-1;break;
    case 'F': Ton=TF;Oktava=NormOkt-1;break;
    case 'G': Ton=TG;Oktava=NormOkt-1;break;
    case 'A': Ton=TA;Oktava=NormOkt-1;break;
    case 'B': Ton=TH;Oktava=NormOkt-1;break;
    case 'H': Ton=TH;Oktava=NormOkt-1;break;
    case 'c': Ton=TC;Oktava=NormOkt;break;
    case 'd': Ton=TD;Oktava=NormOkt;break;
    case 'e': Ton=TE;Oktava=NormOkt;break;
    case 'f': Ton=TF;Oktava=NormOkt;break;
    case 'g': Ton=TG;Oktava=NormOkt;break;
    case 'a': Ton=TA;Oktava=NormOkt;break;
    case 'b': Ton=TH;Oktava=NormOkt;break;
    case 'h': Ton=TH;Oktava=NormOkt;break;
    case 0: return ERR;
    default: return ChybaC(Slovo,s);
  }
  while ( (c=*W++)!=0 ) switch( c )
  {
    case '.': Attr|=Tecka;break;
    case '-': Oktava--;break;
    case '+': Oktava++;break;
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': Delka=Delka*10+c-'0';break;
    case '#': Attr&=~KBOMaskA;Attr|=KrizekA;break;
    case '=': Attr&=~KBOMaskA;Attr|=OdrazA;break;
    case 'b': Attr&=~KBOMaskA;Attr|=BeckoA;break;
    case '|': Attr|=Soubezna;break;
    case '_': Attr|=Leg;break;
    default: if( !NeumTest )
      {
        Err ret=ChybaC(Slovo,s);
        if( ret!=OK ) return ret;
        NeumTest=True;
      }
      break;
  }
  if( Oktava<0 ) Oktava=0;
  if( Oktava>NOkt-1 ) Oktava=NOkt-1;
  if( Delka<=0 ) return ChybaC(Slovo,s);
  s->Buf[0]=Ton+Oktava*Okt;
  s->Buf[1]=Attr&(KBOMaskA|Soubezna|Leg);
  s->Buf[2]=c=KonvDelka(Delka,Attr);
  if( c<0 ) return ChybaC(Slovo,s);
  s->Buf[3]=0;
  return VlozK(s);
}

static Err CtiBlok( SekvKursor *s, FILEEmul *f )
{
  Err ret;
  while( (ret=CtiJedn(s,f))==OK );
  return ret==KON ? OK : ret;
}

void SetNasF( Pisen *P, int FNum, const char *Nam )
{
  if( FNum<0 || FNum>=NFNum ) return;
  strlncpy(P->NastrF[FNum],Nam,sizeof(P->NastrF[FNum]));
}
int CisloNasF( Pisen *P, const char *N )
{
  int i;
  for( i=0; i<NFNum; i++ )
  {
    if( !strcmp(N,P->NastrF[i]) ) return i;
  }
  return -1;
}
const char *NasF( Pisen *P, int N ) {return P->NastrF[N];}


static Err CtiInstrKey( Pisen *P, char *L )
{
  int NI=SCtiCislo(&L);
  char *N=SCtiSlovo(&L);
  if( !N ) return EOK;
  SetNasF(P,NI,N);
  return EOK;
}

static void *DPmyalloc( lword L ){return myalloc(L,True);}

static DynPole NLDef={sizeof(NastrDef),16,DPmyalloc,myfree};

static void ZacLocal( DynPole *ZL )
{
  *ZL=NLDef;
  ZacDyn(ZL);
}
void ZacLocaly( Pisen *P )
{
  ZacLocal(&P->LocDigi);
  ZacLocal(&P->LocMidi);
}
static void KonLocal( DynPole *ZL )
{
  KonDyn(ZL);
}
void KonLocaly( Pisen *P )
{
  KonLocal(&P->LocDigi);
  KonLocal(&P->LocMidi);
}
static Err CtiLocal( Pisen *P, char *C, DynPole *LD )
{
  int ni;
  long I=NewDyn(LD);
  NastrDef *AN=(NastrDef *)AccDyn(LD,I);
  char *S;
  char *PP;
  PathName SR;
  S=SCtiSlovo(&C);
  strlncpy(AN->Nazev,S,sizeof(AN->Nazev));
  for( ni=0; ni<NMSoub; ni++ )
  {
    *AN->Soubory[ni]=0;
  }
  for( ni=0; ni<FyzOkt*12; ni++ )
  {
    AN->IMSoubory[ni]=-1;
  }
  strcpy(SR,P->Cesta);
  PP=strrchr(SR,'\\');
  if( PP ) *PP=0;
  AbsCesta(AN->Soubory[0],C,SR);
  return EOK;
}

static Err CtiLocalKey( Pisen *P, char *L )
{
  char *N=SCtiSlovo(&L);
  if( !N ) return EOK;
  if( !strcmpi(N,"MIDI") ) return CtiLocal(P,L,&P->LocMidi);
  else return CtiLocal(P,L,&P->LocDigi);
}
static Err CtiComment( Pisen *P, char *L )
{
  if( !*P->Popis1 ) strlncpy(P->Popis1,L,sizeof(P->Popis1));
  else if( !*P->Popis2 ) strlncpy(P->Popis2,L,sizeof(P->Popis2));
  return EOK;
}

static int NEfektu;

static Err CtiRepeat( Pisen *P, char *L )
{
  long I=SCtiCislo(&L);
  if( I<0 ) return ERR;
  P->Repeat=( I!=0 );
  return EOK;
}
static Err CtiEfekt( Pisen *P, char *L )
{
  if( NEfektu<lenof(P->Efekt) )
  {
    long I;
    int i;
    char *N=SCtiSlovo(&L);
    if( !strcmpi(N,"ECHO") )
    {
      EfektT *EN=&P->Efekt[NEfektu++];
      for( i=0; i<4; i++ )
      {
        I=SCtiCislo(&L);
        if( I<0 ) return ERR;
        if( I>1000 ) I=1000;
        EN->Pars[i]=(int)I;
      }
    }
  }
  return EOK;
}

static Err UlozEfekty( FILE *f, Pisen *P )
{
  int E;
  for( E=0; E<lenof(P->Efekt); E++ )
  {
    EfektT *EN=&P->Efekt[E];
    if
    (
      EOF==fprintf
      (
        f,"#Effect Echo %d %d %d %d\n",
        EN->Pars[0],EN->Pars[1],EN->Pars[2],EN->Pars[3]
      )
    ) return EFIL;
  }
  if( EOF==fprintf(f,"#Repeat %d\n",P->Repeat) ) return EFIL;
  return EOK;
}

static Err UlozLocal( FILE *f, Pisen *P, DynPole *LD )
{
  long I;
  PathName SR;
  char *PP;
  strcpy(SR,P->Cesta);
  PP=strrchr(SR,'\\');
  if( PP ) *PP=0;
  for( I=0; I<NDyn(LD); I++ )
  {
    NastrDef *AN=(NastrDef *)AccDyn(LD,I);
    if( *AN->Nazev )
    {
      PathName r;
      RelCesta(r,AN->Soubory[0],SR);
      if( EOF==fprintf(f,"#Loc Intern %s %s\n",AN->Nazev,r) ) return EFIL;
    }
  }
  return EOK;
}

Err UlozLocaly( FILE *f, Pisen *P )
{
  Err ret;
  ret=UlozLocal(f,P,&P->LocDigi);
  if( ret>=EOK ) ret=UlozLocal(f,P,&P->LocMidi);
  ret=UlozEfekty(f,P);
  return ret;
}

static Err CtiIL( Pisen *P, const char *L )
{
  char BufL[80];
  char *S;
  char *R;
  strcpy(BufL,L);
  R=BufL;
  S=SCtiSlovo(&R);
  if( !S ) return EOK;
  if( !strcmpi(S,"Instr") ) return CtiInstrKey(P,R);
  if( !strcmpi(S,"Loc") ) return CtiLocalKey(P,R);
  if( !strcmpi(S,"Repeat") ) return CtiRepeat(P,R);
  if( !strcmpi(S,"Effect") ) return CtiEfekt(P,R);
  if( !strcmpi(S,"Comment") ) return CtiComment(P,R);
  return OK;
}

static Err CtiSekv( Pisen *P, FILEEmul *f )
{
  Sekvence *S;
  SekvKursor s;
  Err ret;
  char *Nazev=CtiRadek(f);
  if( !Nazev ) return KON;
  if( Nazev[0]==';' ) return OK;
  if( Nazev[0]=='#' ) return CtiIL(P,&Nazev[1]);
  if( !SPovolJmeno(Nazev) ) return ERR;
  if( !strcmp(Nazev,"$Melodie$") ) Nazev="$Compo$"; /* preklad !!! */
  S=NajdiSekv(P,Nazev,0);
  if( !S ) S=NovaSekv(P,Nazev);
  else
  {
    Sekvence *H;
    for( H=S; H; S=H,H=DalsiHlas(H) );
    S=NovyHlas(S);
  }
  if( !S ) return ERAM;
  ZacBlokZmen(S);
  OtevriK(&s,S,Zapis);
  if( (Mezera(f)!='{') ) {ZavriK(&s);return ERR;}
  OctInit=ERR;
  ret=CtiBlok(&s,f);
  ZavriK(&s);
  KonBlokZmen(S);
  return ret;
}

Pisen *CtiPisen(FILEEmul *f)
{
  Pisen *P;
  Err ret;
  P=NovaPisen();
  if( !P ) return NULL;
  NEfektu=0;
  if( !f ) {MazPisen(P);return NULL;}
  while( (ret=CtiSekv(P,f))==OK );
  P->Zmena=False;
  if( ret==KON )
  { /* neni zajisteno poradi sekvenci */
    if( P )
    {
      Sekvence *K;
      for( K=PrvniSekv(P); K; K=DalsiSekv(K) )
      {
        Sekvence *H;
        for( H=K; H; H=DalsiHlas(H) ) H->_CasPriTempu=-1;
      }
    }
    return P;
  }
  MazPisen(P);
  return NULL;
}

/* retezcove operace */

char *SCtiSlovo( char **_R )
{ /* zpracovava primo v bufferu */
  char *R=*_R;
  char *S=R;
  for(;;)
  {
    if( !*R ) break;
    if( *R==' ' ) {*R++=0;break;}
    R++;
  }
  *_R=R;
  return S;
}

long SCtiLCislo( char **_R )
{
  char *S=SCtiSlovo(_R);
  char *p;
  long l;
  if( !S || !*S ) return ERR;
  if( *S=='$' ) l=strtol(S+1,&p,16);
  else l=strtol(S,&p,10);
  if( p!=S+strlen(S) ) return ERR;
  return l;
}

word SCtiCislo( char **_R )
{
  long L=SCtiLCislo(_R);
  if( L!=(word)L && L!=(int)L ) return ERR;
  return (word)L;
}

#undef fopen

FILE *my_fopen(const char *path, const char *mode)
{
  return fopen(path,mode);
}