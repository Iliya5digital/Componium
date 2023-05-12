/* Componium - sluzby nizke urovne */
/* SUMA 9/1992-1/1994 */

#ifndef __COMPOUTL
#define __COMPOUTL

#include "MACROS.H"

enum CPovel
{
  TC=0,TD,TE,TF,TG,TA,TH,
  Okt,
  NOkt=3,
  NormOkt=2,
  PPau=NOkt*Okt, /* pauza */
  PTakt,
  PDvojTakt,PrepMin=PDvojTakt,
  PDvojTaktUm,
  /* taktove cary */
  PERep, /* kon. repetice */
  PTonina,PTempo,PNTakt, /* prep. taktu, toniny, tempa */
  PRep, /* zac. repetice */
  PHlasit, /* zmena hlasitosti */
  PNastroj, /* zmena nastroje */
  POktava, /* oktavovy posuv */
  PEfekt, /* nastaven¡ efekt… */
  PStereo, /* volba polohy */
  PrepMax,
  PVyvolej=PrepMax, /* vyvolani sekvence */
  PZacSekv,PKonSekv, /* zac. a konec sekvence - v rekurz. modu */
  PUmPauza, /* pri kresleni umela pauza za koncem repetice */
};
  /* PrepMin, PrepMax vymezuje interval prepinacich povelu */
  /* jejich poradi je zavazne pri normalizaci */
  /* PrepMax uz neni prepinaci */


/*
  nektere noty maji priznak Soubezna -
  pak se nazapocitavaji do delky, jen se kresli, hraji a edituji
*/

enum
{
  Jednotka=4, /* delka jednotky melodie (povel, nota ) */
  Takt=3*64, /* doba 4/4 v jednotkach */
  Tempo=96, /* impl. tempo */
  MaxRepVnor=4, /* max. pocet vnorenych repetic v jedne sekvenci */
  MaxSekVnor=4, /* max. pocet sekvenci */
  LMaxVol=255, /* hlasit. skala */
  MinSt=-0x7f,MaxSt=0x7f, /* rozsah stereofonie */
  MinTemp=20,MaxTemp=300, /* rozsah tempa */
};

enum
{
  NMaxHlasu=32, /* max. kanalu v jedne sekvenci */
  NMaxStop=8, /* max. kanalu v jednom okne - dulezite pro velke obrazovky */
  NMaxStopA=NMaxStop, /* max. kanalu v okne pri automat. otevirani */
  NMaxPisni=4,
};

#define TempoMinFyz ( 60*sec ) /* delka jedne minuty v casovych jednotkach */
#define TempoMin (TempoMinFyz/(Takt/4)) /* konstanta pro vypocet fyz. delky z logicke */

#define MAXPOS 0x7FFFFFFFL
#define MAXCAS 0x7FFFFFFFL

enum
{
  Leg=1,Tecka=2,Triola=4,
  KrizekA=8,BeckoA=16,OdrazA=KrizekA|BeckoA,KBOMaskA=OdrazA,
  Soubezna=32,
};

typedef long PosT;

typedef long CasT;

enum {sec=10000}; /* konverze z casu->long */

typedef struct
{
  PosT p[MaxSekVnor];
} StrPosT; /* presna identifikace jednotky ve vnor. soub. */

enum
{
  ERAM=-256, /* nedostatek pameti */
  EFIL, /* chyba pri cteni/zapisu/otevirani ... souboru */
  ECAN, /* zrusena operace */
  ERR, /* obecna chyba */
  MAXERR, /* vyssi nez vsechny chyby */
  EOK=0, /* zvykl jsem si na EOK z DEPOZITU */
  OK=0, /* drive zavedene - kompatibilita */
  KON=ERAM-2, /* konec souboru */
  ZAC, /* zac. souboru */
};

typedef int Err;

#if MAXERR>OK
  #error "MAXERR>OK"
#endif

enum {DelkaBloku=512};
enum {DelkaBanky=256,MaxPouziti=0x4000};

typedef struct
{
  char *Retez;
  int Pouziti;
} BankaItem;
typedef BankaItem Banka[DelkaBanky];
typedef int BankaIndex;

typedef int JednItem;

#define JednVel sizeof(JednItem)

typedef JednItem JednotkaBuf[Jednotka];

typedef struct blok
{
  int Delka; /* staci int */
  struct blok *prev,*next;
  JednItem Zac[DelkaBloku];
} Blok; /* pozor - Blok a Blok0 jsou velmi podobne ! */
typedef struct
{
  int Delka;
  struct blok *prev,*next;
  //JednItem Zac[0];
} Blok0;

struct Sekvence
{
  BankaIndex INazev;
  int Psani,Cteni; /* pocty otevreni pro dany rezim */
  int _CasPriTempu; /* spocitany cas plati pri tempu ( <0 - neplati ) */
  PosT _GlobDel; /* delka v pametovych jednotkach */
  CasT _CasDelka; /* delka v jednotkach casu */
  Flag Zmeny; /* doslo k nejake editaci uvnitr sekv. */
  Flag KritZmeny; /* doslo k nejake z va§n‚ editaci uvnitr sekv. */
  int CntBlokZmen; /* pocitani vnoreni */
  Sekvence *next,*prev; /* ostatni sekvence v pisni */
  struct Pisen *Pis; /* abychom vedeli, komu patri */
  Sekvence *dalsi,*predch; /* dalsi hlasy */
  Blok0 Zac,Kon; /* v Zac.next zacatek, v Kon.prev konec */
  struct SekvKursor *Soubory; /* seznam kursoru pracujicich se sekvenci */
  Flag Uzito; /* pom. promenna */
};

#ifndef __DYNPOLE
#include "stdc\dynpole.h"
#endif

enum {NFNum=10};
enum {NMSoub=4};


enum
{
  FyzOkt=9, /* oktavovy rozsah vyst. zarizeni */
  CisMinOkt=0, /* vzdal. prvni oktavy od C2 */
};

typedef struct
{
  char Nazev[32];
  /* podporujeme multisamply */
  PathName Soubory[NMSoub];
  /* ka§dì t¢n m  ud no, jakìm samplem se hraje */
  long IMSoubory[FyzOkt*12];
  /* kvuli ukladani do COC */
  Flag Uzit;
} NastrDef;

typedef struct
{
  int Typ;
  int Pars[4]; /* max. 4 parametry */
} EfektT;

struct Pisen
{
  char Popis1[40];
  char Popis2[40];
  PathName NazevS; /* nazev bez .EXT */
  PathName Cesta; /* kompletni nazev souboru */
  Sekvence Zac;
  int Otevreno; /* kolikrat otevreno okno */
  Banka NazvySekv;
  Banka Nastroje;
  Sekvence *SekPtr[DelkaBanky];
  Flag Zmena;
  struct SekvSoubor *SSoubory; /* seznam souboru - prochazeji celou strukt. */
  DynPole LocDigi; /* lokalni orchestry */
  DynPole LocMidi; /* volne uzitelne prehravacem */
  char NastrF[NFNum][32];
  EfektT Efekt[2];
  Flag Repeat;
};

struct Kontext
{
  int VTaktu; /* pozice v ramci taktu */
  int KteryTakt; /* kumuluje se pocet taktovych car */
  int Tonina; /* rovnou pocet krizku! */
  int Takt; /* delka taktu v logickych jednotkach (tj. 4/4= konstanta Takt) */
  int Tempo; /* ve ctvrtich za minutu */
  int Oktava; /* oktavovy posuv */
  int Nastroj; /* index nastroje */
  int Hlasit; /* hlasitost */
  int StereoMin,StereoMax;
  int EffA,EffB;
};

/* pozdeji v kontextu i nastroj, oktava, hlasitost atd. */
/* mozne tez automaticke udrzovani korektnich kontextu */

struct SekvKursor
{
  struct SekvKursor *next;
  Sekvence *S;
  Blok *Bl; /* ukazatel do sekv. - vyber bloku */
  int Pos; /* pozice v bloku - staci int */
  JednotkaBuf Buf; /* pristupova promenna */
  PosT _GlobPos; /* pozice v sekvenci */
  CasT _CasPos; /* pozice v casovych jednotkach */
  int Status; /* chybove hlaseni */
  int Opakuj; /* opakovani vnoremymi repticemi - def. 1 */
  int Mode; /* cteni - zapis */
  int Uroven; /* pocet vnoreni v repeticich */
  byte RepOpak[MaxRepVnor]; /* pro vnorene repetice jejich pocet opak. */
  CasT RepPos[MaxRepVnor]; /* pro vnorene repetice jejich caspos (je pred hlavou, nemuze se menit) */
  Kontext A; /* kontext na akt. pozici GlobPos */
  Kontext K; /* pocatecni kontext kursoru */
};

struct SekvSoubor
{
  struct SekvSoubor *next;
  SekvKursor ss[MaxSekVnor];
  int SUroven;
  Flag SHierarch;
};

struct SekvText
{
  SekvKursor SS;
  byte SSBuf[64];
  int SSPtr,SSCte;
  Flag NovyRad;
  int Indent;
};

enum {Cteni=2,Zapis=1};

/* STRCOMPO.C */

char *Duplicate( const char *S ); /* alokuje pamet a zkopiruje do ni retezec */

void NovaBanka( Banka B );
void ZrusBanku( Banka B ); /* nesmi obsahovat zadne retezce */
int SpoctiBanku( const BankaItem *B );
Flag JeVBance( const BankaItem *B, BankaIndex i );
BankaIndex PridejRetez( BankaItem *B, const char *S );
Flag ZrusRetez( BankaItem *B, BankaIndex i ); /* vraci, zda ho skutecne zrusil */
BankaIndex NajdiRetez( const BankaItem *B, const char *S );
char *ZBanky( const BankaItem *B, BankaIndex i );
Err ZmenRetez( Banka B, BankaIndex i, const char *D );
BankaIndex PrvniVBance( const BankaItem *B );
BankaIndex DalsiVBance( const BankaItem *B, BankaIndex i );
BankaIndex MaxVBance( const BankaItem *B );

/* obsluha nedostatku pameti */

#define RezVel (24*1024L)
#define KritRez (4*1024L)

void *myalloc( long size, Flag Trvale );
void myfree( void *b );

/* alternativni pouziti */
/* pri pouziti s radicem MEMPOOL umoznuje odentifikovat visici bloky */
/* vyhodne pro programy, ktere vyuzivaji PREHRAJ.PRJ */
/* zavedeno pri tvorbe Gravona 9/1994 */

void *myallocSpc( long size, Flag Trvale, long Idtf );
void myfreeSpc( void *b );

/* INTCOMPO.C */

extern void (*ZmenaSekv)( Sekvence *S, Flag Krit ); /* reakce na zmeny v sekvenci */

void ZacBlokZmen( Sekvence *S ); /* hodne zmen za sebou, netreba stale zaznamenavat */
void KonBlokZmen( Sekvence *S );

#define ZacBlokZmenK(s) ZacBlokZmen(SekvenceK(s))
#define KonBlokZmenK(s) KonBlokZmen(SekvenceK(s))

Blok *NovyBlok( Blok *za );
void MazBlok( Blok *B );

Sekvence *NovaSekv( Pisen *P, const char *Nazev );
void MazSekv( Sekvence *S );

Sekvence *NovyHlas( Sekvence *S );
void MazHlas( Sekvence *S );
Sekvence *DalsiHlas( Sekvence *S );
void ProhHlas( Sekvence *H, Sekvence *D );

void NazevPisne( Pisen *P, const char *Nazev );
Pisen *NovaPisen();
void MazPisen( Pisen *P );
const char *NazevSekv( const Sekvence *S );
Err ZmenNazevSekv( const Sekvence *S, const char *Nazev );
const char *NazevSekvPis( const Sekvence *S ); /* vytvori kompl. nazev */
const char *NazevSoubPis( const Sekvence *S ); /* vytvori nazev vc. cesty */

Sekvence *NajdiSekv( const Pisen *P, const char *Nazev, int Hlas );
Sekvence *NajdiSekvBI( const Pisen *P, BankaIndex I, int Hlas );
Sekvence *PrvniSekv( const Pisen *P );
Sekvence *DalsiSekv( const Sekvence *S );

void SetKontext( SekvKursor *r, SekvKursor *Kont ); /* prenaseni kontextu mezi urovnemi */
void SetKKontext( SekvKursor *r, const Kontext *Kont ); /* prenaseni kontextu mezi urovnemi */
int CmpKontext( const Kontext *K, const Kontext *k );

CasT DelkaSekv( Pisen *P, BankaIndex I, int Hlas, SekvKursor *Kont ); /* delka sekv. podle nazvu - zavisi na kontextu */
CasT CasDelka( Sekvence *S, SekvKursor *Kont ); /* dotaz na delku v cas. jedn - zavisi na kontextu */
/* Kont==NULL - default - povoleno jen pro main - jindy nemusi souhlasit */

PosT PosDelka( Sekvence *S ); /* dotaz na delku  poctu povelu */

Flag Zavislost( Sekvence *Kdo, Sekvence *NaKom ); /* zjisteni, zda Kdo nejak vyvolava NaKom - jednohl. oprava */
Flag PodHlas( Sekvence *Kdo, Sekvence *Koho ); /* zjisteni, zda Kdo je najaky hlas NaKom */

void EquJed( JednItem *d, const JednItem *z );
Flag CmpJed( const JednItem *d, const JednItem *z );

void OtevriK( SekvKursor *s, Sekvence *S, int Mode );
void RozdvojK( SekvKursor *s, const SekvKursor *o );
void RZapisK( SekvKursor *s, const SekvKursor *o );
void ZavriK( SekvKursor *s );

Flag OdstranK( SekvKursor *s );

Flag PrejdiK( SekvKursor *s, Flag Takty ); /* prechod na dalsi jednotku */
/* musi to jit - nehlida chyby */
/* Takty - vrati-li True - nepohne se, generujte umely takt */
Err PrejdiZK( SekvKursor *s );

Err CtiK( SekvKursor *s ); /* cteni jednotky, posun na dalsi, poskytuje i takty */
Err JCtiK( SekvKursor *s ); /* cteni jednotky, nevklada takty */
Err KopyK( SekvKursor *d, SekvKursor *s ); /* kopiruje jednotku ze s do d - zajistuje pridavani nastroju apod. */
Err TestK( SekvKursor *s ); /* jen cteni jednotky */
Err ZpatkyK( SekvKursor *s ); /* posun na predchozi - ovlivnuje buffer */

CasT DelkaK( SekvKursor *s ); /* urci delku jednotky na akt. pozici */
CasT DelkaKF( SekvKursor *s ); /* urci delku jednotky v bufferu, pozor - v Buf musi byt to, co na akt. pozici (pro insert plati primerene) */
Err MazK( SekvKursor *s ); /* vypusteni jednotky - stara se i o Banky - nici buffer (vypustena jednotka) */
Err MazNOK( SekvKursor *s ); /* vypusteni jednotky - nestara se o Banky */
Err VlozK( SekvKursor *s ); /* vlozeni jednotky, posun za ni - nestara se o Banky */
Err PisK( SekvKursor *s ); /* prepsani jednotky, posun za ni - nestara se o Banky */

PosT KdeJeK( SekvKursor *s ); /* dotaz na pozici */
CasT CasJeK( SekvKursor *s ); /* dotaz na cas */

Err NajdiK( SekvKursor *s, PosT Pos ); /* nastaveni pozice */
Err NajdiCasK( SekvKursor *s, CasT Pos ); /* nastaveni casu */
Err NajdiSCasK( SekvKursor *s, CasT Pos ); /* nastaveni casu */
Err NajdiTaktK( SekvKursor *s, int Num ); /* nastaveni casu */
CasT ZjistiCasK( SekvKursor *p, PosT Pos ); /* konverze pozice -> cas */
Flag BezChybyK( SekvKursor *s );

int NajdiZacRepK( SekvKursor *k ); /* vraci opcet opak., s ukazuje na PERep - najdeme PRep (jen sekvkursor) */

#define KKontextK(s) (&(s)->K) /* poc. kontext */
#define AKontextK(s) (&(s)->A) /* akt. kontext */
#define BufK(s) ( (s)->Buf )
#define StatusK(s) ( (s)->Status )
#define SekvenceK(s) ( (s)->S )

/* REKCOMPO.C */

void OtevriS( SekvSoubor *s, Sekvence *S, int Mode );
void RozdvojS( SekvSoubor *s, const SekvSoubor *o );
void RZapisS( SekvSoubor *s, const SekvSoubor *o );
void ZavriS( SekvSoubor *s );
void RezimS( SekvSoubor *s, Flag Hierarch ); /* def. po otevreni je False, rozdvojeni to zachovava */

#define ZacBlokZmenS(s) ZacBlokZmen(SekvenceS(s))
#define KonBlokZmenS(s) KonBlokZmen(SekvenceS(s))

Err CtiS( SekvSoubor *s ); /* cteni jednotky, posun na dalsi, poskytuje i takty */
Err JCtiS( SekvSoubor *s ); /* cteni jednotky, nevklada takty */
Err KopyS( SekvSoubor *d, SekvSoubor *s ); /* kopiruje jednotku ze s do d - zajistuje pridavani nastroju apod. */
Err TestS( SekvSoubor *s ); /* jen cteni jednotky */
Err ZpatkyS( SekvSoubor *s ); /* posun na predchozi - ovlivnuje buffer */

CasT DelkaS( SekvSoubor *s ); /* urci delku jednotky na akt. pozici */
CasT DelkaSF( SekvSoubor *s ); /* urci delku jednotky v bufferu, pozor - v Buf musi byt to, co na akt. pozici (pro insert plati primerene) */

Kontext *KKontextS( SekvSoubor *s ); /* poc. kontext */
Kontext *AKontextS( SekvSoubor *s ); /* akt. kontext */

/* dotazy na akt. kontext */

#define ToninaS(s)  ( AKontextS(s)->Tonina )
#define TempoS(s)   ( AKontextS(s)->Tempo )
#define TaktS(s)    ( AKontextS(s)->Takt )
#define OktavaS(s)  ( AKontextS(s)->Oktava )
#define NastrojS(s) ( AKontextS(s)->Nastroj )
#define HlasitS(s)  ( AKontextS(s)->Hlasit )
#define VTaktuS(s)  ( AKontextS(s)->VTaktu )

void ZavriVolajici( Sekvence *S ); /* srazi ty, kdo se chteji vracet pres S */
void ZavriVolajiciZ( SekvKursor *s ); /* srazi ty, kdo se chteji vracet na s */

Err MazS( SekvSoubor *s ); /* vypusteni jednotky - nestara se o Banky */
Err VlozS( SekvSoubor *s ); /* vlozeni jednotky, posun za ni - nestara se o Banky */
Err PisS( SekvSoubor *s ); /* prepsani jednotky, posun za ni - nestara se o Banky */

Err KdeJeS( SekvSoubor *s, StrPosT *Pos ); /* dotaz na pozici */
CasT CasJeS( SekvSoubor *s ); /* dotaz na cas */

Err NajdiS( SekvSoubor *s, const StrPosT *Pos ); /* nastaveni pozice */
Err NajdiCasS( SekvSoubor *s, CasT Pos ); /* nastaveni casu */
Err NajdiSCasS( SekvSoubor *s, CasT Pos ); /* nastaveni casu */
Err NajdiTaktS( SekvSoubor *s, int Num ); /* nastaveni casu */
CasT ZjistiCasS( SekvSoubor *p, PosT Pos ); /* konverze pozice -> cas */
Flag BezChybyS( SekvSoubor *s );

int NajdiZacRepS( SekvSoubor *s ); /* vraci pocet opakovani - s ukazuje na PERep - najdeme PRep (jen sekvkursor) */
int CmpPos( const StrPosT *P, const StrPosT *p );
void NajdiPosS( SekvSoubor *s, const SekvSoubor *o ); /* musi byt ve stejne sekvenci a oba stejne Hierarch */

SekvKursor *AktK( SekvSoubor *s );

Flag Neumele( JednItem Druh );

/* pristupy k promennym */

#define SekvenceS(s) ((s)->ss[0].S) /* nejnadrazenejsi sekvence */
JednItem *BufS( SekvSoubor *s );
Err StatusS( SekvSoubor *s );

/* TXTCOMPO.C */

extern const char MainMel[];

#if defined __STDIO || defined _STDIO_H
  #define CtiCislo(f) ( (int)CtiLCislo(f) )
  long CtiLCislo( FILE *f );
  char *CtiSlovo( FILE *f ); /* vnitrni staticky buffer */
  char *CtiRadek( FILE *f ); /* vnitrni staticky buffer */
#endif

void AttrDelka( int Delka, int *DelVys, int *Attr );
Flag PovolJmeno( const char *N );

SekvText *OtevriT( Sekvence *S );
void ZavriT( SekvText *t );
int CtiT( SekvText *t );

extern int (*ChybaC)( const char *Slovo, SekvKursor *s ); /* mozne spec. procedury */

void SetNasF( Pisen *P, int FNum, const char *Nam );

int CisloNasF( Pisen *P, const char *N );
const char *NasF( Pisen *P, int N );

void ZacLocaly( Pisen *P );
void KonLocaly( Pisen *P );
#ifdef __STDIO
Err UlozLocaly( FILE *f, Pisen *P );
#endif

class FILEEmul
{
  public:
  virtual size_t fread(void *buffer, size_t size, size_t count) = 0;
  virtual int fgetc() = 0;
};

inline int fgetc(FILEEmul *f)
{
  return f->fgetc();
}

inline size_t fread(void *buffer, size_t size, size_t count, FILEEmul *f)
{
  return f->fread(buffer,size,count);
}

Pisen *CtiPisen(FILEEmul *file);

/* ruzne texty */
/* retezcove operace */

char *SCtiSlovo( char **_R );
long SCtiLCislo( char **_R );
word SCtiCislo( char **_R );

#endif