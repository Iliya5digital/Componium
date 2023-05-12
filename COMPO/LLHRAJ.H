/* LLPCOMPO.H - nejnizzsi uroven hrani - tohle je pro vsechny vystupy stejne */
/* SUMA 3/1993 - 3/1993 */

#ifndef __LLHRAJ
#define __LLHRAJ

#ifndef __COMPOUTL
	#include "utlcompo.h"
#endif

typedef long Freq;
enum {FreqDes=100};
enum {MaxFyzKan=32};

typedef struct
{
	PathName Naz;
	int KOd,KDo;
	Flag Uzivat;
} SpcInfo;

typedef struct llhraj
{
	long TempStep; /* udava, jak kolik intern. jednotek jeden tik pomaleho playeru */
	int NFyzKan;
	const char *(*Nazev)( void );
	void (*SetNastr)( int Nastr, int FKan, DynPole *P ); /* cislovani dane DefHNastr */
	void (*SetStereo)( int VMin, int VMax, int FKan ); /* V = -128 .. +128 */
	void (*SetEfekt)( int EffA, int EffB, int FKan ); /* V = -128 .. +128 */
	/* je-li Vol==0, znamene to Ticho! - NoteOff, Vol<0 znamena pripravu na ukonceni */
	/* vrati-li po NoteOn True, sam rovnou ukoncil ton - kanal je volny (MIDI bici) */
	Flag (*HrajTon)( int Ton, int Vol, int FKan );
	Err (*ZacHraj)( CasT Cas, CasT Od ); /* Cas je v internich jednotkach - viz TempStep */
	Flag (*CasHraj)( void ); /* jakmile vr t¡ True, konŸ¡ */
	Err (*KonHraj)( void );
	Err (*ZacSNastr)( void ); /* inic. nastr. sys. - pri zac. programu - musi nastavit AktHraj */
	Err (*KonSNastr)( void ); /* dein. nastr. sys. - pri kon. programu - musi nastavit AktHraj */
	void (*MazSNastr)( void ); /* dein. nastr. sys - musi byt uspesna - musi nastavit AktHraj */
	Err (*JinSNastr)( void ); /* prep. nastr. sys. - musi nastavit AktHraj - v AktHraj je, z koho prepiname */
	Err (*CtiOrchestr)( void ); /* vyber orchestru - s FSel - vse zustalo v pameti (neni proveden Kon) */
	Err (*PisOrchestr)( void ); /* ukladani orchestru - s FSel - ponecha v pameti */
	const char *(*EditSNastr)( const char *N, const WindRect *Orig, Pisen *P ); /* zalozeni (N==NULL), vysl.: ERRP nedostatek pameti, NULL cancel  */
	const char **(*SoupisSNastr)( Pisen *P );
	Err (*ZacHNastr)( Pisen *P ); /* zacina preklad - inic. nastr */
	/* prirad nastroji cislo a definuj ho, spocti si k nemu, co je treba */
	/* v KLo, KHi nam vrat kanaly, na kterych umis nastroj zahrat */
	int (*DefHNastr)( const char *Naz, int Ton, int Tip, Pisen *P );
	void (*AktHNastr)( void ); /* po prekladu - pred hranim - pustit cache */
	#ifdef __STDIO
		Err (*SaveHNastr)( FILE *f, Pisen *P, const char *AVRPath ); /* ulozi tabulku nastroju - pro prehravani z COC souboru */
	#else
		Err (*SaveHNastr)();
	#endif
	void (*KonHNastr)( void ); /* dohrano - dein. nastr */
	Err (*VymazNastr)( char *N, Pisen *P ); /* vymazat nastroj z orchestru */
	const char *(*NastrInfo)( const char *NNam, Pisen *P );
	#ifdef __INFOSOUB
		void (*CtiCFG)( const Info *Inf ); /* jen si pripravi, co zapne pri ZacSNastr */
		Flag (*UlozCFG)( Info *Inf );
	#else
		void (*CtiCFG)();
		Flag (*UlozCFG)();
	#endif
	Flag (*SpecInfo)( SpcInfo *SI, int i ); /* ovladani multinastroju - hlavne MIDI */
	Flag (*SpecZap)( SpcInfo *SI, int i ); /* SI=NULL - vsecny vypnout, jinak do i-slotu dat SI */
	int (*GetVol)( int GP[MaxFyzKan] ); /* jak maji byt ukazatele - 0..1000 */
	CasT (*GetCas)( void ); /* jake ma zpozdeni */
	void (*Pause)( void ); /* mgf. funkce */
	void (*Continue)( void );
	void (*ResetPau)( void ); /* po previnuti */
	CasT (*DelkaZneni)( Pisen *P, int Nas, int Ton ); /* jak dlouho bude zn¡t sample */
} LLHraj;

/*
	vsechny Kony - pokud vrati chybu, musi byt to, co zaviraji, jeste
	nezavrene, a navic to musi jit zkusit zavrit znovu!
	(Take by mely obsahovat moznost ignorovani chyby.)
*/

/* COMCOMPO.C */

Err OJinSNastr( LLHraj *PrepNa, LLHraj *PrepZ ); /* obecne prepinani Kon, Zac */
const char *JmenoOrc( const char *Nam );

#ifdef __AES__
void SetMezStrTed( TEDINFO *T, const char *Naz );
const char *MezStrTed( TEDINFO *T );
void SetMIntTed( TEDINFO *T, long i );
long MIntTed( TEDINFO *T );
#endif

/* PLYCOMPO.C */

extern Flag Falcon;

long TimerAD( void ); /* vrati o kolik popojel */
long TimerAPauza( void ); /* preruseni behem stani - pozor na rychlost! */
CasT TimerADF( CasT TStep ); /* dostane o kolik jedn. ma pokrocit, vrati kolik jednotek lze preskocit - vse interne */

Flag TestHraje( void ); /* True = konec */
Flag TestAbort( void ); /* True = abort */
void InitKonec( void );
Flag HrajCinnost( void ); /* obsluha udalosti a dialogu - True = Abort */

/* format COC souboru: - LoadHNastr, SaveHNastr
	long N; // pocet nastroju
	struct
	{
		long Ind;
		char Path[128];
	} Nastroje[N]; // prirazeni nastroju - lokal/global - vse dohromady
	long Del; // delka hudebnich dat
	char Data[Del]; // hudebni data
*/

#endif
