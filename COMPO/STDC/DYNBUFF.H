typedef struct
{
	void *Buf;
	long Len;
	long Ptr;
} Buffer;
#define BufferStep (4*1024)

void InitBuf( Buffer *Buf );
void PustBuf( Buffer *Buf );
Flag VyhradBuf( Buffer *Buf, long Del );
Flag ZapisW( Buffer *Buf, word Obs );
Flag ZapisL( Buffer *Buf, lword Obs );
Flag ZapisC( Buffer *Buf, char Obs );
Flag ZapisB( Buffer *Buf, const Buffer *Obs );

/* import */

void *myalloc( long am, Flag trvale );
void myfree( void *mem );

