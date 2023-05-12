/*
	spolecne deklarace pro AY-digitalni prekladac a player
	SUMA 1993
*/


enum
{
	/* povely hraciho strojku */
	MNotaZac=0,MSync, /* NotaKon je Zac s volume 0 */
	MNastr,MStereo, /* kontrolery */
	MKonec,
	/* dalç¡ povely pýid v me na konec - chceme z…stat kompatibiln¡ */
	MEfekt,
};
enum
{
	MaxLogKan=32,
	Okt0Ind=3, /* vzdalenost oktavy 0 od tonu C2 */
	OktMaxInd=8, /* vzdalenost uz nezvl. oktavy od tonu C2 */
};

typedef signed char MPovel;

/* PLYCOMPO.C */

#ifdef __COMPOUTL
	Err Zahraj( const char *Naz, const MPovel *Pis, CasT Del, CasT Od, int MaxKan, Pisen *P );
#endif

/* import */

void RecalcFreq( double Freq ); /* pýepoŸ¡tej vnitýn¡ buffery */
