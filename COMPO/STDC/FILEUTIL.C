#include <string.h>
#include <stdio.h>
#include "../macros.h"
#include "fileutil.h"

void lenstr( char *S, size_t l, char c )
{
  size_t i;
  for( i=strlen(S); i<l; i++ ) S[i]=c;
  S[l]=0;
}

void strlncpy( char *d, const char *z, size_t l )
{
  --l;
  strncpy(d,z,l);
  d[l]=0;
}

char *NajdiNazev( const char *W )
{
  char *Nam=strrchr(W,'\\');
  if( Nam ) return Nam+1;
  if( W[0]!=0 && W[1]==':' ) return (char *)W+2;
  return (char *)W;
}

char *NajdiExt( const char *N )
{
  char *Nam=strrchr(N,'.');
  if( Nam ) return Nam+1;
  return (char *)N+strlen(N);
}

char *NajdiPExt( const char *N )
{
  /* NajdiExt pusobilo najasnosti */
  /* je lepsi vracet tecku */
  char *Nam=strrchr(N,'.');
  if( Nam ) return Nam;
  return (char *)N+strlen(N);
}

char *estr( const char *S )
{
  return (char *)S+strlen(S);
}

void EndChar( char *S, int c )
{
  char *e=estr(S);
  if( e==S || e[-1]!=c ) *e++=c,*e=0;
}

const char *SizeNazev( const char *Nazev, size_t l )
{
  while( strlen(Nazev)>=l )
  {
    char *NN;
    NN=strchr(Nazev,':');
    if( NN ) Nazev=NN+1;
    else
    {
      NN=strchr(Nazev,'\\');
      if( NN ) Nazev=NN+1;
      else Nazev++;
    }
  }
  return Nazev;
}

void VyrobCesty( PathName P, FileName N, const char *W )
{
  char *Nam=NajdiNazev(W);
  size_t NPos=(size_t)(Nam-(char *)W);
  strncpy(P,W,NPos);
  P[NPos]=0;
  strcat(P,"*.");
  strcat(P,NajdiExt(Nam));
  strcpy(N,Nam);
  if( N[0]=='.' || N[0]=='*' ) N[0]=0;
}

Flag CestaNazev( const char *FPC, const char *FNC, char *FW )
{
  PathName FP;
  FileName FN;
  char *PosNm,*PosEx,*PosPo;
  strcpy(FP,FPC);
  strcpy(FN,FNC);
  PosNm = strrchr(FP,'\\'); if( PosNm==NULL ) return False;
  PosEx = strchr(PosNm,'.'); if( PosEx==NULL ) return False;
  PosPo = strrchr(FN,'.'); if( PosPo!=NULL ) *(PosPo++)= 0;
  *(PosNm++)= 0;
  *(PosEx++)= 0;
  strcpy(FW,FP);
  strcat(FW,"\\");
  if( *PosNm == '*' ) strcat(FW,FN);
  else return False;
  strcat(FW,".");
  strcat(FW, PosPo!=NULL ? PosPo : *PosEx != '*' ? PosEx : "" );
  return True;
}

Flag Cesta1Nazev( const char *FPC, const char *FNC, char *FW )
{
  if( !CestaNazev(FPC,FNC,FW) ) return False;
  return !strrchr(FW,'*') && !strrchr(FW,'?');
}

void RelCesta( char *Rel, const char *Abs, const char *Root )
{
  PathName RootP;
  strcpy(RootP,Root);
  EndChar(RootP,'\\');
  if( !strnicmp(RootP,Abs,strlen(RootP)) )
  { /* jsme v podadres ýi adres ýe root */
    strcpy(Rel,Abs+strlen(RootP));
  }
  else
  {
    PathName Prac;
    strcpy(Rel,Abs); /* v§dy m…§eme pou§¡t kompletn¡ cestu */
    if( Abs[1]==':' && Abs[2]=='\\' && !strnicmp(Abs,RootP,3) )
    { /* stejnì zdrojovì disk */
      strcpy(Prac,Abs+2);
      if( strlen(Prac)<strlen(Rel) ) strcpy(Rel,Prac);
    }
    {
      const char *RootS=RootP,*AbsS=Abs,*RP;
      while( *AbsS==*RootS ) AbsS++,RootS++;
      while( AbsS>Abs )
      {
        if( AbsS[-1]=='\\' ) break;
        AbsS--,RootS--;
      }
      /* Abs ... AbsS je spoleŸn  Ÿ st cesty */
      /* uva§ujeme rozd¡l mezi AbsS a RootS */
      /* po RootS mus¡me seçplhat pomoc¡ '..' , po AbsS mus¡me vyçplhat */
      RP=RootS;
      *Prac=0;
      while( *RP )
      {
        if( *RP=='\\' ) strcat(Prac,"..\\");
        RP++;
      }
      strcat(Prac,AbsS);
      if( strlen(Prac)<strlen(Rel) ) strcpy(Rel,Prac);
    }
  }
}

void AbsCesta( char *Abs, const char *Rel, const char *Root )
{
  strcpy(Abs,Root);
  EndChar(Abs,'\\');
  if( Rel[0]!=0 && Rel[1]==':' )
  {
    strcpy(Abs,Rel);
  }
  else if( Rel[0]=='\\' )
  {
    char *r;
    r=strchr(Abs,'\\');
    if( r ) *r=0;
    strcat(Abs,Rel);
  }
  else
  {
    while( Rel[0]=='.' )
    {
      if( Rel[1]=='.' && Rel[2]=='\\' )
      {
        char *r,*rr;
        r=strrchr(Abs,'\\');
        if( r )
        {
          *r=0,rr=strrchr(Abs,'\\');
          if( !rr ) rr=r;
          strcpy(rr,"\\");
        }
        Rel+=3;
      }
      else if( Rel[1]=='\\' )
      {
        Rel+=2;
      }
      else break;
    }
    strcat(Abs,Rel);
  }
}

char *fgetl( char *B, int L, FILE *f )
{
  char *r=fgets(B,L,f);
  if( !r ) return r;
  if( *B )
  {
    char *e=B+strlen(B)-1;
    while( e>=B )
    {
      if( *e=='\n' ) *e=0,e--;
      else if( *e=='\r' ) *e=0,e--;
      else break;
    }
  }
  return B;
}


long fgetw( FILE *f )
{
  int c,r;
  c=fgetc(f);
  if( c<0 ) return c;
  r=fgetc(f);
  if( r<0 ) return r;
  return ((long)c<<8)|r;
}
long fgetiw( FILE *f )
{
  int c,r;
  c=fgetc(f);
  if( c<0 ) return c;
  r=fgetc(f);
  if( r<0 ) return r;
  return ((long)r<<8)|c;
}
long fgeti24( FILE *f )
{
  long c,r;
  c=fgetc(f);if( c<0 ) return c;
  r=fgetc(f);if( r<0 ) return r;
  c|=r<<8;
  r=fgetc(f);if( r<0 ) return r;
  c|=r<<16;
  return c;
}
long fgetil( FILE *f )
{
  long c,r;
  c=fgetc(f);if( c<0 ) return c;
  r=fgetc(f);if( r<0 ) return r;
  c|=r<<8;
  r=fgetc(f);if( r<0 ) return r;
  c|=r<<16;
  r=fgetc(f);if( r<0 ) return r;
  c|=r<<24;
  return c;
}


long fgetbew( FILE *f )
{
  int c,r;
  c=fgetc(f);
  if( c<0 ) return c;
  r=fgetc(f);
  if( r<0 ) return r;
  c <<= 8;
  c|=r;
  return c;
}
long fgetbe24( FILE *f )
{
  long c,r;
  c=fgetc(f);if( c<0 ) return c;
  r=fgetc(f);if( r<0 ) return r;
  c <<= 8;
  c|=r;
  r=fgetc(f);if( r<0 ) return r;
  c <<= 8;
  c|=r;
  return c;
}
long fgetbel( FILE *f )
{
  long c,r;
  c=fgetc(f);if( c<0 ) return c;
  r=fgetc(f);if( r<0 ) return r;
  c <<= 8;
  c|=r;
  r=fgetc(f);if( r<0 ) return r;
  c <<= 8;
  c|=r;
  r=fgetc(f);if( r<0 ) return r;
  c <<= 8;
  c|=r;
  return c;
}
