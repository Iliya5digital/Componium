typedef struct budik
{
	long ZaDobu;
	struct budik *Dalsi;
} Budik;

typedef struct
{
	Budik *Prvni,*Volne;
} FrontaB;

void ZaradBCas( FrontaB *F, Budik *K, long Cas );	/* zavedeni do fronty */
void PridejBCas( FrontaB *F, Budik *K, long Cas ); /* zmena pozice ve fronte */
void VypustBCas( FrontaB *F, Budik *K ); /* vypusteni z fronty */

long CasBudiku( FrontaB *F, Budik *K );

#define CasPrvniho(F) ( (F)->Prvni.ZaDobu )
#define PrvniBud(F) ( (F)->Prvni )
#define DalsiBud(B) ( (B)->Dalsi )
#define VolnyBud(F) ( (F)->Volne )

#define UplynulCas(F,t) ( (F)->Prvni.ZaDobu-=(t) )
#define UplynPrvniho(F) ( (F)->Prvni.ZaDobu=0 )

void ZrusBud( FrontaB *F );
