/* buzeni kanalu - relativni budiky */
/* SUMA 10/1993 */

#include <macros.h>
#include <stdc\budiky.h>

static void FZaradBCas( FrontaB *F, Budik *K, long Cas )
{
	Budik *BS,**BL;
	for( BL=&F->Prvni; (BS=*BL)!=NULL && BS->ZaDobu<=Cas; BL=&BS->Dalsi )
	{
		Cas-=BS->ZaDobu;
	}
	/* v BL ten, za koho mame pridat */
	K->Dalsi=BS;
	*BL=K;
	K->ZaDobu=Cas;
	if( BS ) BS->ZaDobu-=Cas;
}

static Budik **NajdiBud( Budik **F, Budik *K )
{
	Budik *BS,**BL;
	for( BL=F,BS=*BL; BS!=K; BL=&BS->Dalsi,BS=*BL )
	{
		Plati( BS );
	}
	return BL;
}

void ZaradBCas( FrontaB *F, Budik *K, long Cas )
{
	*NajdiBud(&F->Volne,K)=K->Dalsi; /* vyradit z volnych */
	FZaradBCas(F,K,Cas);
}
void PridejBCas( FrontaB *F, Budik *K, long Cas )
{
	*NajdiBud(&F->Prvni,K)=K->Dalsi; /* vyradit z fronty */
	FZaradBCas(F,K,Cas); /* nemeni se zatrideni ve Volne */
}
void VypustBCas( FrontaB *F, Budik *K )
{
	*NajdiBud(&F->Prvni,K)=K->Dalsi; /* vyradit z fronty */
	K->Dalsi=F->Volne; /* zaradit do volnych */
	F->Volne=K;
}

long CasBudiku( FrontaB *F, Budik *K )
{
	Budik *B;
	long ret=0;
	for( B=F->Prvni; B!=K; B=B->Dalsi )
	{
		Plati( B );
		ret+=B->ZaDobu;
	}
	return ret+K->ZaDobu;
}

void ZrusBud( FrontaB *F )
{
	while( PrvniBud(F) ) VypustBCas(F,PrvniBud(F));
}
