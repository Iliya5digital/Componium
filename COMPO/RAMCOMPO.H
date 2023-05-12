/* Spolecne deklarace pro Componium */
/* SUMA 10/1992-4/1993 */

#ifndef __COMPOUTL
	#include "utlcompo.h"
#endif

enum
{
	ystep=6, /* vzdalenost linek osnovy */
	yosnova=ystep*16/2, /* vzdalenost horniho okraje od tonu C */
	xzac=4, /* pozice klicu */
	yzac=54, /* pozice 1. osnovy - (asi tonu C ) */
	ydif=84, /* vzdalenost osnov */
	yprv=yzac+40, /* velikost 1. osnovy */
	zacxpos=xzac+28, /* pozice za klici */
};

enum /* Typy oken */
{
	SDNoty=0x100,
};

typedef struct
{
	int LHan; /* uplne krizove spojeni */
	SekvSoubor Soub[NMaxHlasu]; /* nastaven na zacatek okna */
	/* ze Soub[0] lze cist hlavni sekvenci! */
	int NSoub; /* pocet zobraz. hlasu */
	int KKan; /* hlas, ve kterem je kursor */
	SekvSoubor Kurs; /* pozice kursoru */
	WindRect KR; /* pozice kurzoru (rel. v okne ) */
	CasT ZacCas,KonCas; /* historie - kazdy vznika jinak */
	CasT CasZac,CasKon;
	int IStopa,NStop; /* prvni zobr. hlas, pocet zobr. hlasu */
	int ZBStopa,KBStopa; /* prvni hlas v bloku, posl hl. v bl. (<0 = odp. soub. uzavren) */
	SekvSoubor ZBlok,KBlok; /* zac. bloku v 1. hlase, konec v posl. hlase */
	Flag ZmenaObrazu; /* nucene prekreslovani celeho pri lib. casti */
	Flag ZobrazKursor; /* vynucene zobrazeni kursoru */
	Flag VNazvuZmena;
	Flag Rozvoj; /* indikace - tento rezim by mel byt ve vsech souborech */
} OknoInfo;

/* MDF */

Pisen *MdfImport( const char *Nam );
Flag MdfExport( const char *Nam, Pisen *P );

/* LOG */

extern PathName PisenP;
extern Flag Demo; /* pro ABOUT */

void NastavExtPisne( const char *E ); /* pýi Ÿten¡ bude CPS, MID ... */

Pisen *CtiMelF( const char *Nam );
Flag PisMelF( const char *Nam, Pisen *P );
Pisen *OtevriPisen( Flag DefNazev );
void VymazSekvenci( Sekvence *S ); /* vymaze sekvenci, zavre okna, urcite nezavre pisen */
void ZavriPisen( void ); /* zepta se a tu pisen zavre */
Flag ZavriFPisen( Pisen *P ); /* zavre okna pisne a uzavre pisen */
Flag ZavriFFPisen( Pisen *P ); /* jen uzavre pisen */
Flag UlozFPisen( Pisen *P, Flag PtejSe );
void UlozPisen( void );
void UlozJako( void );
Err CtiCFG( void );
void UlozCFG( void );
void KonCFG( void );

/* GRF */

#define kanypos(Kan) (yzac+(Kan)*ydif)
#ifdef __MULTI
	void KresliNoty( int LHan, WindRect *Kres );
#endif

const char *ZkratNastr( char *NK, const char *N, int Na );
int PovelPoradi( enum CPovel P );
int PovelPred( enum CPovel P, enum CPovel p );
int UrciEProp( JednItem *B );
void KresliENotu( int LHan, JednItem *B, int Prop ); /* kresleni pro ediatci mysi */
void KresliKursor( int LHan );
enum {InsNic,InsNota,InsSoubezna,InsNas}; /* v *Ins */
enum {HNic,HEdit,HEditDvoj,HKursor,HKursPred}; /* v HMode */
Flag HledejKursor( int LHan, int x, int y, int HMode, int *Ins ); /* pokud je vysl=False, v ins vraci vyse uv. enum Ins... */
void BlikKursor( int LHan );

/* SPC */

void ZacBlok( int LHan ); /* vyznaci zac. resp. konec na posici kursoru. */
void KonBlok( int LHan );
void ProzatimZacBlok( int LHan );
void ProzatimKonBlok( int LHan ); /* zmena konce behem vyznacovani */
void ZrusBlok( int LHan );

void MazPBlok( void );
Flag JeVBloku( OknoInfo &OI, int Kan, const StrPosT *pos, CasT cas );

Flag TaktBlok( void );
Flag JednotkaBlok( void );
Flag VsechnoBlok( void );
Flag ZkopirujBlok( void );
Flag VlozBlok( void );
Flag SmazBlok( void );

Flag PosunBlok( int Pos );

typedef enum {TonN,TonO,TonK,TonB} PI;
typedef PI PosuvT[7];

void NormalizaceF( Pisen *P, Sekvence *M, Kontext *MK );
void Normalizace( void ); /* normalizace zapisu - odstraneni prebyt. znacek ... */

/* EDT */

int InitEdit( SekvKursor *s, JednItem Povel, int Kan, CasT CasPos, Flag Hledat ); /* priprava editace povelu */
int SInitEdit( SekvSoubor *s, JednItem Povel, int Kan, CasT CasPos, Flag Hledat );
/* <0 - needitovat */
/* >0 - povel uz existuje */

void KonecEdit( SekvKursor *s, OknoInfo *OI ); /* ukonceni tehoz */

void NormujK( SekvKursor *s ); /* normalizace poradi povelu */
void NormujS( SekvSoubor *s ); /* normalizace poradi povelu */
void TNormujK( SekvKursor *s, SekvKursor *Kurs ); /* normalizace poradi soub. not */

void MazTonF( SekvKursor *s );
Err VlozTon( OknoInfo *OI, Flag Soub );
void MazTon( OknoInfo *OI );
int Editace( int Asc, int Scan, int Shf ); /* editace not */

void MEditace( int LHan, int mx, int my, Flag Dlouhe );
void NasVloz( int LHan );
void TonVloz( int LHan, int mx, int my, Flag Dvoj ); /* vkladani nove noty */
void TonSVloz( int LHan, int mx, int my, Flag Dvoj ); /* vkladani soubezne noty */

/* editace mysi - souradnice KX,KY,MX,MY, pozice kursoru */

/* editace povelu */
void ZmenaToniny( int NKriz, Flag Dur, int Kan, CasT CasPos, Flag Hledat );
void ZmenaTaktu( int H, int D, int Kan, CasT CasPos, Flag Hledat );
void ZmenaTempa( int H, int Kan, CasT CasPos, Flag Hledat );
void ZmenaHlasit( int H, int Kan, CasT CasPos );
void ZmenaOktavy( int O, Flag Rel, int Kan, CasT CasPos );
void ZmenaNastroje( const char *S, int Kan, CasT CasPos );
void ZmenaSterea( int Min, int Max, int Kan, CasT CasPos );
void ZmenaEfektu( int EffA, int EffB, int Kan, CasT CasPos );
void VyvolaniS( Sekvence *S );
void ZmenaVyvolaniS( Sekvence *S );
void ZacRepet( int Opak, int Kan, CasT CasPos ); /* zalozi */
void KonRepet( int Kan, CasT CasPos );
void EdZacRepet( int Opak, int Kan, CasT CasPos ); /* edituje jiz existujici */
void Predtakti( int Kan, CasT CasPos );
void BlokRepetice( int LHan, int Opak );

/* ---------- */

int HlavniTempo( int LHan ); /* pozor, LHan muze byt zaporny */
void HlavniTakt( int LHan, int *HH, int *DD ); /* pozor, LHan muze byt zaporny */

/* ---------- */

void InitTonina( Sekvence *D, SekvSoubor *s ); /* inicializace sekvence D pomoci Z - tonina apod. */

void ZalozHlas( void );
void ProhodHlas( void );
void ZrusHlas( void );

void NastavZmeny( Sekvence *S, Flag Krit ); /* nastavi priznak zmeny vsem zasazenym oknum krome LHan */

/* MNU */

#ifdef __INTERF
	Flag SekvOtevri( Pisen *P, Sekvence *S, WindRect *Obr, CasT KCas, int KKan, WindRect *Orig, const Kontext *K );
	void KursorRect( WindRect *Orig, int LHan );
#endif
void FLogZavri( int LHan );
Flag LogZavri( int LHan );

void OtevriAbout( void );

int JeOtevrena( Sekvence *S, const Kontext *K ); 
/* pokud uz sekvence s danym kontextem existuje, vrati jeji LHan */

Flag ZavriVse( void );

int PrvniKEdit( OknoInfo *OI ); /* podle polozky Ve vsech hlasech poskutnou z okna Vrchni vsechny edit. kanaly */
int DalsiKEdit( OknoInfo *OI, int Kan );

const char *TempoText( int Tempo );
const char *HlasitText( int Hlas );

#ifdef __LLHRAJ
	#define PovolHraj(LH) (&AYBufFalc) /* zajisti dostupny prehr. mod */
#endif

void ZmenNasF( Pisen *P, int FNum, const WindRect *S, const char *FText ); /* F1..F10 */
void UzijNasF( int FNum, OknoInfo *OI );

/* priznak male znamena editaci druhym triggerem - PopUp */

void ZmenVyvolejS( void );
void ZmenNas( Flag Male ); /* autom. pocatek - i edituje */
void HlasitD( Flag Male ); /* autom. pocatek - i edituje */
void StereoD( Flag Male ); /* autom. pocatek - i edituje */
void EfektD( Flag Male ); /* autom. pocatek - i edituje */
void OktavaD( Flag Male ); /* autom. pocatek - i edituje */
void TempoD( Flag Hledat, const WindRect *Orig, Flag Male );
void ToninaD( Flag Hledat, const WindRect *Orig, Flag Male );
void TaktD( Flag Hledat, const WindRect *Orig, Flag Male );
int AskRepetD( int Opak, const WindRect *Orig ); /* jen vrati pocet opak. */
void EdRepetD( void );
void AttribD( Flag Ton, int *VI, int *DI, Flag *Lg, const WindRect *Orig ); /* editace atributu noty */

void ZobrazRekurzi( OknoInfo *OI );

/* COM */
/* zapouzdreni konkr. COMPOCOM, COMPOPLY */

typedef struct
{
	Sekvence *S;
	int Kan;
	CasT CPos;
	StrPosT PPos;
	Flag Ukazat;
	const char *Alert;
} CompoErr;

extern PathName CocPath;

Err CompoCom( Flag Hraj, Sekvence *S, CasT Cas, Kontext *K, CompoErr *CE ); /* centralni spojovaci bod - prehravani/preklad */
Err OZacSNastr( void ); /* inic. a deinic. nastr. systemu */
Err OKonSNastr( void );

const char *OEditNastr( const char *N, Pisen *P ); /* zalozeni (N==NULL), vysl.: ERRP nedostaek pameti, NULL cancel  */
const char **OSoupisNastroju( Banka B, Pisen *P ); /* je-li banka, udelej k ni doplnek */
Err OVymazNastroj( char *N, Pisen *P );

void MazPosuv( PosuvT Pos, SekvKursor *s );
Flag HrajeSeVOkne( void );
int HrajOkno( void );

void AzDohraje( void (*Proc)( void ) ); /* co se m  udØlat po dohr n¡ */

/* TOL */

void ToolDraw( int I );
void ToolSelect( int I, Flag S );

void DskMode( void );
void OtevriTool( void );
void ZavriTool( void );

int ToolOkno( void );

/* TON */

void OtevriToN( void );
void ZavriToN( void );
int ToNOkno( void );
void SetToNastroj( int I, const char *Naz, Flag DrawS );
void ZobrazToN( void ); /* podle vrch. sekv. */

/* VYB */

int PocetPisni( void );
Pisen *VrchniPisen( void );

Sekvence *VyberSekv( Pisen *P, const char *Tit, Flag _JedenHlas, const WindRect *Orig, Sekvence *Def, Flag PovolMale );
Pisen *VyberPisen( const char *Text );
int VyberOkno( void );

/*const char *FZalozNastr( Pisen *P, Banka B, Flag Strucne, const WindRect *Orig );*/
const char *VyberNastroj( Pisen *P, Banka B, BankaIndex Def, Flag Strucne, const WindRect *Orig, Flag PovolMale, const char *Tit );

void InitVyber( void );

/* RAM */

#ifdef __NAROD
	extern CmpPar CesPar;
#endif

extern Flag Falcon;

extern Flag Rezerva;
extern Flag IhnedKresli;

extern int OknoMaxStop,OknoMaxStopA;
extern const char *SterLS,*SterPS;
extern PathName CompoRoot;

enum edmode {DEditace,DMazani,};
extern enum edmode EdMode,EdModeO;

extern int argc;
extern const char **argv; /* predav. param. */

void Chyba( Err err );

int Locallocksnd( void );
void Localunlocksnd( void );

const char *TvorNazev( const Sekvence *S, int LHan, Flag Zmena ); /* tvorba nazvu okna */

Flag OknoJeSekv( int LHan );
int VrchniSekv( void ); /* je-li vrchni sekvence, vrati LHan */

#define SekvenceOI(OI) ( SekvenceS(&OI->Soub[0]) )

//void Kresli( int LHan );
void MoznaKresli( int LHan ); /* kresli, doslo-li ke zmene */
void IgnKresli( int LHan ); /* prekresli, a ignoruje zpravy v bufferu na prekresleni tehoz */

#define UkazKursor(OI) ((OI)->ZobrazKursor=True)
#define NeukazujKursor(OI) ((OI)->ZobrazKursor=False)

int XMax( int LHan );

void TaktVlevo( OknoInfo *OI );
void TaktVpravo( OknoInfo *OI );
void PosunCasPos( OknoInfo *OI, CasT Pos );
void PosunCas( OknoInfo *OI ); /* v ostatnich kanalech posune na pozici kanalu s kursorem */

CasT CasKursoru( OknoInfo *OI ); /* zjisti cas kursoru */
void IStopaZKKan( OknoInfo *OI ); /* nastavi index prvni stopy */
void NajdiKursor( OknoInfo *OI, CasT CasPos ); /* nastavi kursor na cas */
void NaPoziciKurs( OknoInfo *OI ); /* nastavi okno podle pozice kursoru */
void JdiNaPozici( OknoInfo *OI, CasT CPos, int KKan ); /* nastavi kursor na danou pozici a hne oknem */
void SekvPriot( int LHan, int Kan, Sekvence *H ); /* prida kanal do okna */
void SekvPrizav( int LHan ); /* ubere kanal z okna */

void LAutoRozmer( WindRect *D, int LHan, int AsHan );
void ObrPrekresli( void );
void NastavNazvy( void );
void CNastavDesktop( void );

/* Prevazne v RAM */
/* kompat. se starym pojetim MULTIITF */
/* sluzby tykajici se okna s notami */

void DoSipky( int Sip, int LHan );
Flag OknoStisk( int LHan, Flag Dvojite, Flag Dlouhe, int MX, int MY ); /* zpracovani stisku 1. tlacitka mysi */
Flag OknoStisk2( int LHan, Flag Dvojite, Flag Dlouhe, int MX, int MY ); /* druhy button */
int OknoZnak( int LHan, int Asc, int Scan, int Shf, Flag Buf ); /* zprac. klaves */
void OknoMidi( int LHan, int *MBuf ); /* zprac. MIDI */
Flag OknoPust( int LHan, int MX, int MY ); /* pusteni tlacitka mysi */
Flag OknoPust2( int LHan, int MX, int MY ); /* pusteni tlacitka mysi */
void Okrouhli( WindRect *R, int LHan ); /* zaokrouhleni vnitrku okna */
void PlneOkno( WindRect *R, int LHan );
void Zmenseno( int LHan ); /* zmenseni pri WM_SIZED */
Flag LzeTopnout( int LHan, int MX, int MY ); /* test pred topnutim */
void DoHSlide( int Pos, int LHan );
void DoVSlide( int Pos, int LHan );
Flag OZavri( int LHan );
void AutoRozmer( WindRect *D, int LHan );
int NotyKursor( int LHan, Flag Uvnitr, Flag Obrys );

#define Panika() (AlertRF(PANIKA,1,__FILE__,__LINE),Konec=True)

