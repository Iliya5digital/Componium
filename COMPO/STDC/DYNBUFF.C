/* rychle dynamicke buffery - word, long */
/* SUMA 5/1993 */

#include <macros.h>
#include "dynbuff.h"

void InitBuf( Buffer *Buf )
{
	Buf->Buf=NULL;
	Buf->Len=Buf->Ptr=0;
}
void PustBuf( Buffer *Buf )
{
	if( Buf->Buf ) myfree(Buf->Buf);
}

Flag FVyhradBuf( Buffer *Buf, long Del )
{
	long NLen=Buf->Len+BufferStep;
	long NPtr=Buf->Ptr+Del;
	void *B;
	NLen=max(NLen,NPtr);
	B=myalloc(NLen,True);
	if( !B ) return False;
	if( Buf->Buf )
	{
		memcpy(B,Buf->Buf,Buf->Len);
		myfree(Buf->Buf);
	}
	Buf->Buf=B;
	Buf->Len=NLen;
	Plati( NPtr<=Buf->Len );
	return True;
}

Flag VyhradBuf( Buffer *Buf, long Del )
{
	if( Buf->Ptr+Del>Buf->Len ) return FVyhradBuf(Buf,Del);
	return True;
}


Flag ZapisC( Buffer *Buf, char Obs )
{
	if( Buf->Ptr+sizeof(Obs)>Buf->Len ) if( !FVyhradBuf(Buf,sizeof(Obs)) ) return False;
	*(char *)((byte *)Buf->Buf+Buf->Ptr)=Obs;
	Buf->Ptr+=sizeof(Obs);
	return True;
}
Flag ZapisW( Buffer *Buf, word Obs )
{
	if( Buf->Ptr+sizeof(Obs)>Buf->Len ) if( !FVyhradBuf(Buf,sizeof(Obs)) ) return False;
	*(word *)((byte *)Buf->Buf+Buf->Ptr)=Obs;
	Buf->Ptr+=sizeof(Obs);
	return True;
}
Flag ZapisL( Buffer *Buf, lword Obs )
{
	if( Buf->Ptr+sizeof(Obs)>Buf->Len ) if( !FVyhradBuf(Buf,sizeof(Obs)) ) return False;
	*(lword *)((byte *)Buf->Buf+Buf->Ptr)=Obs;
	Buf->Ptr+=sizeof(Obs);
	return True;
}
Flag ZapisB( Buffer *Buf, const Buffer *Obs )
{
	if( Buf->Ptr+sizeof(Obs)>Buf->Len ) if( !FVyhradBuf(Buf,sizeof(Obs)) ) return False;
	memcpy(((byte *)Buf->Buf+Buf->Ptr),Obs->Buf,Obs->Ptr);
	Buf->Ptr+=Obs->Ptr;
	return True;
}
