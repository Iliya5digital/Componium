/* DYNPOLE.C - kompaktni pole s promennou velikosti */
/* SUMA 3/1993-3/1993 */

#include "../macros.h"
#include <string.h>

#include "dynpole.h"

void ZacDyn( DynPole *D )
{
	D->Obs=NULL;
	D->UzitDel=D->AlokDel=0;
}

void KonDyn( DynPole *D )
{
	if( D->Obs ) D->DPfree(D->Obs);
	D->Obs=NULL;
	D->UzitDel=D->AlokDel=0;
}

long AlokDyn( DynPole *D, long NPrvku )
{
	void *OObs=D->Obs;
	long NDel,ODel;
	if( !OObs ) D->AlokDel=D->UzitDel=0;
	ODel=D->UzitDel;
	NDel=D->UzitDel+NPrvku;
	if( NDel>D->AlokDel )
	{
		void *NObs;
		long ADel=D->AlokDel;
		while( ADel<NDel ) ADel+=D->AlokKrok;
		NObs=D->DPmalloc(ADel*D->VelPrvku);
		if( !NObs ) return -1;
		if( OObs ) memcpy(NObs,OObs,D->UzitDel*D->VelPrvku),D->DPfree(OObs);
		D->Obs=NObs;
		D->AlokDel=ADel;
	}
	return ODel;
}
long InsDyn( DynPole *D, const void *Prvek )
{
	if( AlokDyn(D,1)<0 ) return -1;
	memcpy((char *)D->Obs+D->UzitDel*D->VelPrvku,Prvek,D->VelPrvku);
	return D->UzitDel++;
}
long NewDyn( DynPole *D )
{
	if( AlokDyn(D,1)<0 ) return -1;
	return D->UzitDel++;
}
int DelDyn( DynPole *D, long I )
{
	char *O=(char *)D->Obs+I*D->VelPrvku;
	memcpy(O,O+D->VelPrvku,(D->UzitDel-I-1)*D->VelPrvku);
	D->UzitDel--;
	return 0;
}

void *PrvniDyn( const DynPole *D, long *I )
{
	*I=0;
	if( *I>=D->UzitDel ) return NULL;
	return D->Obs;
}
void *DalsiDyn( const DynPole *D, void *A, long *I )
{
	(*I)++;
	if( *I>=D->UzitDel ) return NULL;
	return (byte *)A+D->VelPrvku;
}

