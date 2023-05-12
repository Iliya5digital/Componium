#ifndef __INFOSOUB
#define __INFOSOUB

enum {DelInfoNaz=31,DelInfoObs=127};

typedef struct
{
  char Nazev[DelInfoNaz+1];
  char Buf[DelInfoObs+1];
} InfoPol;

typedef struct
{
  InfoPol *Buf;
  size_t Uzito,Vyhrazeno;
  PathName Soub;
  Flag Zmena;
} Info;

#ifdef __cplusplus
  extern "C" {
#endif

Flag CtiInfo( const char *Soub, Info *I );
Flag NovInfo( const char *Soub, Info *I );
void FreeInfo( Info *D );
Flag PisInfo( const char *Soub, Info *I );
Flag KonInfo( Info *I ); /* je-li treba, ulozi */

/* polozky cislovany od 0 */

InfoPol *NajdiIPol( const Info *D, const char *N, int Index );
Flag VlozIPol( Info *D, const InfoPol *P, int Index );
char *GetText( const Info *I, const char *Naz, int Index );
Flag SetText( Info *I, const char *Naz, const char *Obs, int Index ); /* pokusi se polozku najit Index-tou polozku Nazev, neni-li, vlozi, a nastavi */
void MazIPolOd( Info *D, const char *Naz, int Index );

#ifdef __cplusplus
  };
#endif

#endif
