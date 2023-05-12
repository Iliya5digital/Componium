#ifndef __FILEUTIL
#define __FILEUTIL

#ifdef __cplusplus
extern "C" 
{
  #endif
  
  void lenstr( char *S, size_t l, char c );
  void strlncpy( char *d, const char *z, size_t l );
  #if !defined __MSDOS__
  long Fsize( const char *P );
  unsigned long Ftime( const char *P );
  #endif
  
  char *estr( const char *S );
  void EndChar( char *S, int c ); /* pokud nekonci znakem, prida ho */
  
  //void GetPath( PathName P );
  
  char *NajdiNazev( const char *W );
  char *NajdiExt( const char *N );
  char *NajdiPExt( const char *N );
  
  const char *SizeNazev( const char *Nazev, size_t l );
  
  void VyrobCesty( PathName P, FileName N, const char *W );
  Flag CestaNazev( const char *FPC, const char *FNC, char *FW );
  Flag Cesta1Nazev( const char *FPC, const char *FNC, char *FW );
  void RelCesta( char *Rel, const char *Abs, const char *Root );
  void AbsCesta( char *Abs, const char *Rel, const char *Root );
  
  #if defined __MSDOS__ || !defined __STDIO
  #include <stdio.h>
  #endif
  
  char *fgetl( char *B, int L, FILE *f );
  
  long fgetw( FILE *f );
  long fgetiw( FILE *f );
  long fgeti24( FILE *f );
  long fgetil( FILE *f );

  long fgetbew( FILE *f );
  long fgetbe24( FILE *f );
  long fgetbel( FILE *f );

  FILE *my_fopen(const char *filename, const char *mode);
  int my_open(const char *filename, int mode);
  #define fopen my_fopen
  #define open my_open

  #ifdef __cplusplus
};

#endif


#endif
