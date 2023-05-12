/* nacitani XIMG souboru - direct color */

#include <stdio.h>
#include <string.h>

#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>

#include "macros.h"
#include "memspc.h"
#include "imgload.h"
#include "FILEUTIL.H"

#define BUF_OPT ( 64L*1024 )
#define BUF_MIN ( 1024 )

void *IMGLoad( const char *N, int *W, int *H, int *NPlanu )
{
	int ret=-1;
	byte *Buf=NULL;
	FILE *f=fopen(N,"rb");
	if( f )
	{
		byte *B;
		long w;
		int c;
		int NJP;
		long lenh;
		long wid,hei;
		long ZLine;
		int rept=0;
		byte *RepBuf=NULL;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		w=fgetw(f);if( w!=1 ) goto Error;
		lenh=fgetw(f);if( lenh!=11 ) goto Error;
		w=fgetw(f);if( w<0 ) goto Error;
		*NPlanu=(int)w; /* obvykle 16 */
		NJP=(*NPlanu+7)/8;
		w=fgetw(f);if( w!=3 ) goto Error; /* cosi */
		w=fgetw(f);if( w<0 ) goto Error; /* rozl. x */
		w=fgetw(f);if( w<0 ) goto Error; /* rozl. y */
		wid=fgetw(f);if( wid<=0 ) goto Error; /* w */
		*W=(int)wid;
		hei=fgetw(f);if( hei<=0 ) goto Error; /* h */
		*H=(int)hei;
		B=Buf=mallocSpc(wid*hei*NJP,'IMGL');
		if( !Buf ) goto Error;
		w=fgetw(f);if( w!=0x5849 ) goto Error; /* mag XI */
		w=fgetw(f);if( w!=0x4d47 ) goto Error; /* mag MG */
		w=fgetw(f);if( w!=0 ) goto Error; /* resvd */
		c=fgetc(f);
		for( ZLine=hei*wid*NJP; ZLine>0; )
		{
			if( c==0 ) /* spec. povel */
			{
				if( fgetc(f)!=0 ) goto Error;
				if( fgetc(f)!=0xff ) goto Error;
				/* je to opak. radku */
				rept=fgetc(f);if( rept<0 ) goto Error;
				RepBuf=B;
				c=fgetc(f);
			}
			if( c<0 ) goto Error;
			if( c==0x80 ) /* blok */
			{
				int b=fgetc(f);if( b<=0 || b>wid ) goto Error;
				ZLine-=b;
				while( --b>=0 )
				{
					c=fgetc(f);if( c<0 ) goto Error;
					*B++=c;
				}
				c=fgetc(f);
			}
			ef( c&0x80 ) /* byte FF */
			{
				int b=c&0x7f;
				ZLine-=b;
				while( --b>=0 ) *B++=0xff;
				c=fgetc(f);
			}
			else /* byte 0 */
			{
				if( c<=0 ) goto Error;
				ZLine-=c;
				while( --c>=0 ) *B++=0;
				c=fgetc(f);
			}
			if( RepBuf )
			{
				if( RepBuf<=B-wid*NJP )
				{
					while( --rept>0 )
					{
						byte *RB=RepBuf;
						long b=wid*NJP;
						ZLine-=b;
						while( --b>=0 ) *B++=*RB++;
					}
					RepBuf=NULL;
				}
			}
		}
		ret=0;
		Error:
		fclose(f);
		if( ret && Buf ) freeSpc(Buf),Buf=NULL;
	}
	return Buf;
}

#pragma pack(push,1)
struct tpiheader
{
	long Magic; /*'PNT'<<8;*/
	short x100;  /* 0x100 */
	short NCol;
	short W,H;
	short NPlan;
	short Resvd;
	long LenDump;
	byte resvd[128-16-4];
};
#pragma pack(pop)

static int fputiw( int W, FILE *f )
{
	if( fputc((byte)W,f)<0 ) return EOF;
	return fputc(W>>8,f);
}
static int fputi24( long W, FILE *f )
{
	if( fputc((byte)W,f)<0 ) return EOF;
	if( fputc((byte)(W>>8),f)<0 ) return EOF;
	if( fputc((byte)(W>>16),f)<0 ) return EOF;
	return 0;
}
static int fputil( long W, FILE *f )
{
	if( fputc((byte)W,f)<0 ) return EOF;
	if( fputc((byte)(W>>8),f)<0 ) return EOF;
	if( fputc((byte)(W>>16),f)<0 ) return EOF;
	if( fputc((byte)(W>>24),f)<0 ) return EOF;
	return 0;
}

enum {MaxRep=128};

static int UlozBlok( word *Blok, int *LBlok, FILE *f )
{
	if( *LBlok>0 )
	{
		int L=*LBlok;
		if( fputc(L-1,f)<0 ) return EOF;
		while( --L>=0 )
		{
			if( fputiw(*Blok++,f)<0 ) return EOF;
		}
	}
	*LBlok=0;
	return 0;
}

static int PridejBlok( word *Blok, int *LBlok, FILE *f, word W )
{
	Blok[*LBlok]=W;
	(*LBlok)++;
	if( *LBlok>=MaxRep )
	{
		return UlozBlok(Blok,LBlok,f);
	}
	return 0;
}

static int PridejRep( word *Blok, int *LBlok, FILE *f, word LW, int rep )
{
	if( rep>0 )
	{
		if( rep<3 )
		{
			while( --rep>=0 ) if( PridejBlok(Blok,LBlok,f,LW)<0 ) return EOF;
		}
		else
		{
			if( *LBlok>0 ) if( UlozBlok(Blok,LBlok,f)<0 ) return EOF;
			if( fputc(rep-1+0x80,f)<0 ) return EOF;
			if( fputiw(LW,f)<0 ) return EOF;
		}
	}
	return 0;
}

int SavePACW( FILE *f, word *Buf, long L)
{
	int LBloku=0;
	word LW=0;
	int rep=0;
	word *Blok=mallocSpc(MaxRep*sizeof(*Blok),'PCCS');
	if( !Blok ) return -1;
	while( L>0 )
	{
		word A=*Buf++;
		L--;
		if( rep<MaxRep && A==LW ) rep++;
		else
		{
			if( PridejRep(Blok,&LBloku,f,LW,rep)<0 ) goto Error;
			LW=A,rep=1;
		}
	}
	if( PridejRep(Blok,&LBloku,f,LW,rep)<0 ) goto Error;
	if( UlozBlok(Blok,&LBloku,f)<0 ) goto Error;
	freeSpc(Blok);
	return 0;
	Error:
	freeSpc(Blok);
	return -1;
}

int SaveRAWPCC( const char *N, word *Buf, int W, int H )
{
	/* run-length compress. */
	FILE *f;
	int r;
	f=fopen(N,"wb");
	r=-1;
	if( f )
	{
		long L=(long)W*H;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		fputiw(W,f);
		fputiw(H,f);
		if( SavePACW(f,Buf,L)<0 ) goto Error;
		r=0;
		Error:
		fclose(f);
	}
	return r;
}

int SavePCC( const char *N, word *Buf, int W, int H )
{ /* run-length compress. */
	/* muzeme pridat i TGA hlavicku */
	FILE *f;
	int r;
	f=fopen(N,"wb");
	r=-1;
	if( f )
	{
		long L=(long)W*H;
		//long l;
		//word *buf;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		fputiw(W,f);
		fputiw(H,f);
		/*
		for( buf=Buf,l=L; l>0; l--,buf++ )
		{ // overlay pryc - v celem obrazku
			word A=*buf;
			A=(A&0x1f)|((A&0xffc0)>>1);
			A&=0x7fff;
			*buf=A;
		}
		*/
		if( SavePACW(f,Buf,L)<0 ) goto Error;
		r=0;
		Error:
		fclose(f);
	}
	return r;
}

static unsigned GetPixel888(FILE *f, int *error)
{
	int r,g,b,a;
	b=fgetc(f);if( b<0 ) goto Error;
	g=fgetc(f);if( g<0 ) goto Error;
	r=fgetc(f);if( r<0 ) goto Error;
	a=0xff;
	return (a<<24)|(r<<16)|(g<<8)|b;
  Error:
  *error = 1;
  return 0;
}

static unsigned GetPixel8888(FILE *f, int *error)
{
	int r,g,b,a;
	b=fgetc(f);if( b<0 ) goto Error;
	g=fgetc(f);if( g<0 ) goto Error;
	r=fgetc(f);if( r<0 ) goto Error;
	a=fgetc(f);if( a<0 ) goto Error;
	return (a<<24)|(r<<16)|(g<<8)|b;
  Error:
  *error = 1;
  return 0;
}

void *TGALoad( const char *N, int *W, int *H, int *NPlanu )
{ /* uncompressed */
	FILE *f;
	byte *Buf=NULL,*B;
	int r;
	f=fopen(N,"rb");
	r=-1;
	if( f )
	{
		int nPlanes;
		long w,h,L,N,a;
		int n;
    int type;
		Flag Reverse=False;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		n=fgetc(f);
		if( n<0 ) goto Error; /*  Number of Characters in Identification Field. */
		while( --n>=0 )
		{
			if( fgetc(f)<0 ) goto Error;
		}
		if( fgetc(f)!=0 ) goto Error; /*  Color Map Type. */
    type = fgetc(f);
    if (type!=2 && type!=10) goto Error; /*  Image Type Code. */ /* RGB uncompress. */
		/*   3   : Color Map Specification. */
		if( fgetiw(f)!=0 ) goto Error;
		if( fgetiw(f)!=0 ) goto Error;
		if( fgetc(f)!=0 ) goto Error;
		/*   8   : Image Specification. */
		if( fgetiw(f)!=0 ) goto Error; /* X */
		if( fgetiw(f)!=0 ) goto Error; /* Y */
		w=fgetiw(f); if( w<0 ) goto Error;
		h=fgetiw(f); if( h<0 ) goto Error;
		N=fgetc(f);if( N<0 && N!=16 && N!=24 && N!=32 ) goto Error;
		a=fgetc(f);
		if( !(a&0x20 ) ) Reverse=True;
		a|=0x20;
		// a should be 0x20 + alpha bits
		if( N==32 && a!=0x28 ) goto Error; /*  Image Descriptor Byte. */
		if( N==24 && a!=0x20 ) goto Error; /*  Image Descriptor Byte. */
		if( N==16 && a!=0x21 ) goto Error; /*  Image Descriptor Byte. */
		/* 18 */
		L=w*h;
		nPlanes=N;
		if( nPlanes==24 ) nPlanes=32;
    if (type==10 && N<24) goto Error;
		B=Buf=mallocSpc(L*nPlanes/8,'TGAL');if( !Buf ) goto Error;
    if (type==2)
    {
      int error = 0;
		  while( L>0 )
		  {
			  if( N==32 )
			  {
				  *(long *)B = GetPixel8888(f,&error);
				  if(error) goto Error;
				  
				  B+=sizeof(long);
			  }
			  else if( N==24 )
			  {
				  *(long *)B = GetPixel888(f,&error);
				  if(error) goto Error;
				  B+=sizeof(long);
			  }
			  else
			  {
				  long v=fgetiw(f);
				  if( v<0 ) goto Error;
				  // is 555 format
				  *(word *)B=(word)v;
				  B+=sizeof(word);
			  }
			  L--;
		  }
    }
    else if (type==10)
    {
      // RLE
      int error = 0;
      while(L>0)
      {
        int c = fgetc(f);
        if(c & 0x80) // Run length chunk (High bit = 1)
        {
          int bLength=c-127; // Get run length
          unsigned int pixel = N==32 ? GetPixel8888(f,&error) : GetPixel888(f,&error);
          if (error) goto Error;

          L -= bLength;
          while (--bLength>=0)
          {
            *(long *)B = pixel;
  				  B+=sizeof(long);
          }

        }
        else // Raw chunk
        {
          int bLength=c+1; // Get run length

          L -= bLength;
          while (--bLength>=0)
          {
            *(long *)B = N==32 ? GetPixel8888(f,&error) : GetPixel888(f,&error);
  				  B+=sizeof(long);
          }
          if (error) goto Error;
        }
      }


    }
		r=0;
		if( Buf && Reverse )
		{
			int i;
			long nbl=((nPlanes+7)>>3)*w;
			void *DBuf=mallocSpc(nbl*h,'TGAL');
			if( !DBuf ) goto Error;
			for( i=0; i<h; i++ )
			{
				void *S=(byte *)Buf+i*nbl;
				void *D=(byte *)DBuf+(h-1-i)*nbl;
				memcpy(D,S,nbl);
			}
			freeSpc(Buf);
			Buf=DBuf;
		}
		Error:
		fclose(f);
		if( r && Buf ) free(Buf),Buf=NULL;
		*NPlanu=(int)nPlanes;
		*W=(int)w;
		*H=(int)h;
	}
	return Buf;
}

static int UlozBBlok( word *Blok, int *LBlok, FILE *f )
{
	if( *LBlok>0 )
	{
		int L=*LBlok;
		if( fputc(L-1,f)<0 ) return EOF;
		while( --L>=0 )
		{
			if( fputc(*Blok++,f)<0 ) return EOF;
		}
	}
	*LBlok=0;
	return 0;
}

static int PridejBBlok( word *Blok, int *LBlok, FILE *f, word W )
{
	Blok[*LBlok]=W;
	(*LBlok)++;
	if( *LBlok>=MaxRep )
	{
		return UlozBBlok(Blok,LBlok,f);
	}
	return 0;
}

static int PridejBRep( word *Blok, int *LBlok, FILE *f, word LW, int rep )
{
	if( rep>0 )
	{
		if( rep<3 )
		{
			while( --rep>=0 ) if( PridejBBlok(Blok,LBlok,f,LW)<0 ) return EOF;
		}
		else
		{
			if( *LBlok>0 ) if( UlozBBlok(Blok,LBlok,f)<0 ) return EOF;
			if( fputc(rep-1+0x80,f)<0 ) return EOF;
			if( fputc(LW,f)<0 ) return EOF;
		}
	}
	return 0;
}

static int SavePACB( FILE *f, byte *Buf, long L )
{
	int LBloku=0;
	word LW=0;
	int rep=0;
	word *Blok=mallocSpc(MaxRep*sizeof(*Blok),'PCCS');
	if( !Blok ) return -1;
	
	while( L>0 )
	{
		word A=*Buf++;
		L--;
		if( rep<MaxRep && A==LW ) rep++;
		else
		{
			if( PridejBRep(Blok,&LBloku,f,LW,rep)<0 ) goto Error;
			LW=A,rep=1;
		}
	}
	if( PridejBRep(Blok,&LBloku,f,LW,rep)<0 ) goto Error;
	if( UlozBBlok(Blok,&LBloku,f)<0 ) goto Error;
	
	freeSpc(Blok);
	return 0;
	Error:
	freeSpc(Blok);
	return -1;
}

int SavePAC256( const char *N, byte *_Buf, int W, int H, long *RGB, Flag TGA )
{ /* run-length compress. */
	byte *Buf=(byte *)_Buf;
	//word *Blok=mallocSpc(MaxRep*sizeof(*Blok),'PACS');
	FILE *f;
	int r;
	//if( !Blok ) return -1;
	f=fopen(N,"wb");
	r=-1;
	if( f )
	{
		long L=(long)W*H;
		int I;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		if( !TGA )
		{
			fputiw(W,f);
			fputiw(H,f);
			fputiw(256,f); /* poc. barev */
			for( I=0; I<256; I++ ) fputi24(RGB[I],f);
			if( SavePACB(f,Buf,L)<0 ) goto Error;
		}
		else
		{
			fputc(0,f); /*  Number of Characters in Identification Field. */
			fputc(1,f); /*  Color Map Type. */
			fputc(9,f); /*  Image Type Code. 1 (index) + 8 (compressed) */
			/*   3   : Color Map Specification. */
			fputiw(0,f); /* beg */
			fputiw(256,f); /* count */
			fputc(24,f); /* bit RGBA */
			/*   8   : Image Specification. */
			fputiw(0,f); /*  X Origin of Image. */
			fputiw(0,f); /*  Y Origin of Image. */
			fputiw(W,f); /*  Width Image. */
			fputiw(H,f); /*  Height Image. */
			fputc(8,f); /*  Image Pixel Size. */
			fputc(0x20,f); /*  Image Descriptor Byte. */
			/* color map */
			for( I=0; I<256; I++ ) fputi24(RGB[I],f);
			/* 18 */
			if( SavePACB(f,Buf,L)<0 ) goto Error;
		}
		
		
		r=0;
		Error:
		fclose(f);
	}
	//freeSpc(Blok);
	return r;
}

int TGALSave( const char *N, int W, int H, unsigned long *Buf ) /* TC s hloubkou Resol */
{ /* uncompressed */
	FILE *f;
	int r;
	f=fopen(N,"wb");
	r=-1;
	if( f )
	{
		long L=(long)W*H;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		fputc(0,f); /*  Number of Characters in Identification Field. */
		fputc(0,f); /*  Color Map Type. */
		fputc(2,f); /*  Image Type Code. */ /* RGB uncompress. */
		/*   3   : Color Map Specification. */
		fputiw(0,f);
		fputiw(0,f);
		fputc(0,f);
		/*   8   : Image Specification. */
		fputiw(0,f); /*  X Origin of Image. */
		fputiw(0,f); /*  Y Origin of Image. */
		fputiw(W,f); /*  Width Image. */
		fputiw(H,f); /*  Height Image. */
		//fputc(24,f); /*  Image Pixel Size. */
		//fputc(0x20,f); /*  Image Descriptor Byte. */
		fputc(32,f); /*  Image Pixel Size. */
		fputc(0x28,f); /*  Image Descriptor Byte. */
		// insert alpha bytes
		/* 18 */
		// pouze pro Resol==8
		while( L>0 )
		{
			long argb=*Buf++;
			int a=(argb>>24)&0xff;
			int r=(argb>>16)&0xff;
			int g=(argb>>8)&0xff;
			int b=(argb>>0)&0xff;
			if( fputc(b,f)<0 ) goto Error;
			if( fputc(g,f)<0 ) goto Error;
			if( fputc(r,f)<0 ) goto Error;
			if( fputc(a,f)<0 ) goto Error;
			L--;
		}
		r=0;
		Error:
		fclose(f);
	}
	return r;
}

int TGASave( const char *N, int W, int H, word *Buf ) /* jen DC */
{ /* uncompressed */
	FILE *f;
	int r;
	f=fopen(N,"wb");
	r=-1;
	if( f )
	{
		long L=(long)W*H;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		fputc(0,f); /*  Number of Characters in Identification Field. */
		fputc(0,f); /*  Color Map Type. */
		fputc(2,f); /*  Image Type Code. */ /* RGB uncompress. */
		/*   3   : Color Map Specification. */
		fputiw(0,f);
		fputiw(0,f);
		fputc(0,f);
		/*   8   : Image Specification. */
		fputiw(0,f); /*  X Origin of Image. */
		fputiw(0,f); /*  Y Origin of Image. */
		fputiw(W,f); /*  Width Image. */
		fputiw(H,f); /*  Height Image. */
		fputc(16,f); /*  Image Pixel Size. */
		fputc(0x21,f); /*  Image Descriptor Byte. */
		/* 18 */
		while( L>0 )
		{
			word A=*Buf++;
			L--;
			A=(A&0x1f)|((A&0xffc0)>>1); /* overlay pryc */
			A&=0x7fff;
			if( fputiw(A,f)<0 ) goto Error;
		}
		r=0;
		Error:
		fclose(f);
	}
	return r;
}

void *KonvNBP2PP( word *Buf, int W, int H, int N )
{ /* N, PixelPaket<-BitPlanes */
	byte OBuf[16];
	byte *OB;
	long L=(long)W*H/16;
	long LD=(long)W*H;
	byte *DBuf=mallocSpc(LD,'BPPP');
	if( !DBuf ) return DBuf;
	OB=DBuf;
	while( --L>=0 )
	{
		int OI=0;
		word Mask;
		for( Mask=0x8000; Mask>0; Mask>>=1 )
		{
			byte PP=0;
			int i;
			for( i=0; i<N; i++ )
			{
				if( Buf[i]&Mask ) PP|=1<<i;
			}
			OBuf[OI++]=PP;
		}
		for( OI=0; OI<16; OI++ ) *OB++=OBuf[OI];
		Buf+=N;
	}
	return DBuf;
}

void KonvBP2PP( word *Buf, int W, int H )
{ /* 256, PixelPaket<-BitPlanes */
	byte OBuf[16];
	byte *OB=(byte *)Buf;
	long L=(long)W*H/16;
	while( --L>=0 )
	{
		int OI=0;
		word Mask;
		for( Mask=0x8000; Mask>0; Mask>>=1 )
		{
			byte PP=0;
			if( Buf[0]&Mask ) PP|=1;
			if( Buf[1]&Mask ) PP|=2;
			if( Buf[2]&Mask ) PP|=4;
			if( Buf[3]&Mask ) PP|=8;
			if( Buf[4]&Mask ) PP|=16;
			if( Buf[5]&Mask ) PP|=32;
			if( Buf[6]&Mask ) PP|=64;
			if( Buf[7]&Mask ) PP|=128;
			OBuf[OI++]=PP;
		}
		for( OI=0; OI<8; OI++ ) OB[OI+8]=OBuf[OI];
		for( OI=0; OI<8; OI++ ) OB[OI]=OBuf[OI+8];
		OB+=16;
		//for( OI=0; OI<16; OI++ ) *OB++=OBuf[OI]; // Big Endian version
		Buf+=8;
	}
}

void KonvBP2PPEndian( word *Buf, int W, int H )
{ /* 256, PixelPaket (Motorola)<-BitPlanes(Intel) */
	byte OBuf[16];
	byte *OB=(byte *)Buf;
	long L=(long)W*H/16;
	while( --L>=0 )
	{
		int OI=0;
		word Mask;
		for( Mask=0x8000; Mask>0; Mask>>=1 )
		{
			byte PP=0;
			if( Buf[0]&Mask ) PP|=1;
			if( Buf[1]&Mask ) PP|=2;
			if( Buf[2]&Mask ) PP|=4;
			if( Buf[3]&Mask ) PP|=8;
			if( Buf[4]&Mask ) PP|=16;
			if( Buf[5]&Mask ) PP|=32;
			if( Buf[6]&Mask ) PP|=64;
			if( Buf[7]&Mask ) PP|=128;
      //PP = ((PP>>4)&0xf)|((PP<<4)&0xf0);
			OBuf[OI++]=PP;
		}
		for( OI=0; OI<8; OI++ ) OB[OI+8]=OBuf[OI];
		for( OI=0; OI<8; OI++ ) OB[OI]=OBuf[OI+8];
		OB+=16;
		Buf+=8;
	}
}

void KonvPP2BP( word *Buf, int W, int H )
{
  // Pixel Packet is fine for OpenGL, no need to convert
  #if 0
	byte SBuf[16];
	byte *SB=(byte *)Buf;
	/* 256, PixelPaket->BitPlanes */
	/* musis z planu udelat pixel pakety */
	long L=((long)W*H)>>4;
	while( --L>=0 )
	{
		int OI;
		word Mask;
		byte *SBP;
		word *BP;
		for( SBP=SBuf,OI=16; --OI>=0; ) *SBP++=*SB++;
		for( BP=Buf,OI=8; --OI>=0; ) *BP++=0;
		SBP=SBuf;
		for( Mask=0x8000; Mask>0; Mask>>=1 )
		{
			byte PP=*SBP++;
			if( PP ) /* nula je hodne caste cislo */
			{
				if( PP&1 ) Buf[0]|=Mask;
				if( PP&2 ) Buf[1]|=Mask;
				if( PP&4 ) Buf[2]|=Mask;
				if( PP&8 ) Buf[3]|=Mask;
				if( PP&16 ) Buf[4]|=Mask;
				if( PP&32 ) Buf[5]|=Mask;
				if( PP&64 ) Buf[6]|=Mask;
				if( PP&128 ) Buf[7]|=Mask;
			}
		}
		Buf+=8;
	}
  #endif
}

int TGANSave( const char *N, int W, int H, void *_Buf, long *RGB, int NC ) /* paleta - NC barev */
{ /* color map uncompressed */
	FILE *f;
	byte *Buf=_Buf;
	int r;
	f=fopen(N,"wb");
	r=-1;
	if( f )
	{
		int I;
		long L=(long)W*H;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		fputc(0,f); /*  Number of Characters in Identification Field. */
		fputc(1,f); /*  Color Map Type. */
		fputc(1,f); /*  Image Type Code. */ /* CM uncompress. */
		/*   3   : Color Map Specification. */
		fputiw(0,f); /* beg */
		fputiw(NC,f); /* count */
		fputc(24,f); /* bit RGBA */
		/*   8   : Image Specification. */
		fputiw(0,f); /*  X Origin of Image. */
		fputiw(0,f); /*  Y Origin of Image. */
		fputiw(W,f); /*  Width Image. */
		fputiw(H,f); /*  Height Image. */
		fputc(8,f); /*  Image Pixel Size. */
		fputc(0x20,f); /*  Image Descriptor Byte. */
		/* 18 */
		for( I=0; I<NC; I++ ) fputi24(RGB[I],f);
		fwrite(Buf,sizeof(char),L,f);
		r=0;
		Error:
		fclose(f);
	}
	return r;
}

int TGA256Save( const char *N, int W, int H, void *_Buf, long *RGB )
{
	return TGANSave(N,W,H,_Buf,RGB,256);
}

static int LoadPACW( FILE *f, word *B, long w)
{
	while( w>0 )
	{
		int c=fgetc(f);
		if( c<0 ) goto Error;
		if( c&0x80 )
		{
			long v=fgetiw(f);if( v<0 ) goto Error;
			c&=0x7f;
			c++;
			w-=c;
			while( --c>=0 ) *B++=(word)v;
		}
		else
		{
			c++;
			w-=c;
			while( --c>=0 )
			{
				long v=fgetiw(f);if( v<0 ) goto Error;
				*B++=(word)v;
			}
		}
	}
	return 0;
	Error:
	return -1;
}

void *RAWPCCLoad( const char *N, int *W, int *H )
{
	word *Buf=NULL;
	int r=-1;
	FILE *f=fopen(N,"rb");
	if( f )
	{
		long w,h;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		w=fgetiw(f);if( w<=0 ) goto Error;
		h=fgetiw(f);if( h<=0 ) goto Error;
		*W=(int)w,*H=(int)h;
		w*=h;
		Buf=mallocSpc(w*2,'PCCL');
		if( !Buf ) goto Error;
		LoadPACW(f,Buf,w);
		r=0;
		Error:
		if( Buf && r<0 ) {freeSpc(Buf);Buf=NULL;}
		fclose(f);
	}
	return Buf;
}

VRAMPixel *PCCLoad( const char *N, int *W, int *H )
{
	word *B,*Buf=NULL;
  VRAMPixel *BV, *BufV = NULL;
	int r=-1;
	FILE *f=fopen(N,"rb");
	if( f )
	{
		long w,h;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		w=fgetiw(f);if( w<=0 ) goto Error;
		h=fgetiw(f);if( h<=0 ) goto Error;
		*W=(int)w,*H=(int)h;
		w*=h;
		Buf=mallocSpc(w*2,'PCCL');
		if( !Buf ) goto Error;
		LoadPACW(f,Buf,w);
		for( B=Buf; w>0; w--,B++ )
		{
			word v=*B;
			v=(v&0x1f)|((v&0xffe0)<<1); /* vloz overlay bit */
			*B=v;
		}
    // convert 565 to 8888
    w = *W * *H;
		BufV=mallocSpc(w*sizeof(VRAMPixel),'PCCL');
		for( B=Buf,BV=BufV; w>0; w--,B++,BV++ )
		{
			word v=*B;
      int r = COL_565_R(v), g = COL_565_G(v), b = COL_565_B(v);
      if (v==0xffdf)
      {
        *BV = 0;
      }
      else
      {
  			*BV= (r<<0)|(g<<8)|(b<<16)|(0xff<<24);
      }
		}
		r=0;
		Error:
		if (Buf) {freeSpc(Buf);Buf=NULL;}
		if( BufV && r<0 ) {freeSpc(BufV);BufV=NULL;}
		fclose(f);
	}
	return BufV;
}

void *PAC256Load( const char *N, int *W, int *H, long *RGB )
{
	byte *B,*Buf=NULL;
	int r=-1;
	FILE *f=fopen(N,"rb");
	if( f )
	{
		long w,h,C;
		int I;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		w=fgetiw(f);if( w<=0 ) goto Error;
		h=fgetiw(f);if( h<=0 ) goto Error;
		C=fgetiw(f);if( C!=256 ) goto Error;
		*W=(int)w,*H=(int)h;
		for( I=0; I<256; I++ )
		{
			long b=fgeti24(f);if( w<0 ) goto Error;
			RGB[I]=b;
		}
		w*=h;
		B=Buf=mallocSpc(w,'PCCL');
		if( !Buf ) goto Error;
		while( w>0 )
		{
			int c=fgetc(f);
			if( c<0 ) goto Error;
			if( c&0x80 )
			{
				int v=fgetc(f);if( v<0 ) goto Error;
				c&=0x7f;
				c++;
				w-=c;
				while( --c>=0 ) *B++=v;
			}
			else
			{
				c++;
				w-=c;
				if( fread(B,1,c,f)!=(size_t)c ) goto Error;
				B+=c;
			}
		}
		r=0;
		Error:
		if( Buf && r<0 ) {freeSpc(Buf);Buf=NULL;}
		fclose(f);
	}
	return Buf;
}

#define rgb565(r,g,b) ( ((word)(r)<<11)|((word)(g)<<5)|((word)(b)) )

void KonvTransparent( word *Buf, int W, int H )
{
	long P=(long)W*H;
	while( P>0 )
	{
		word AB=(word)(*Buf&rgb565(31,62,31));
		if( AB==rgb565(31,62,31) ) *Buf=0xffff;
		Buf++;
		P--;
	}
}

long KonvTitl( VRAMPixel *To, VRAMPixel *Buf, int W, int H )
{
	long P=(long)W*H;
	int rep=0;
	VRAMPixel val=0;
	int w=W;
	VRAMPixel *OPtr=To;
	while( P>0 )
	{
		VRAMPixel v=*Buf++;
		P--,w--;
		if( ((v>>24)&0xff)==0 ) v=0;
		if( v!=val || w<0 )
		{
			if( rep>0 )
			{
				*To++=rep;
				*To++=val;
			}
			rep=1;
			val=v;
			if( w<0 ) w+=W;
		}
		else rep++;
	}
	if( rep>0 )
	{
		*To++=rep;
		*To++=val;
	}
	*To++=0;
	*To++=0;
	return diff(To,OPtr);
}

VRAMPixel *TransRadek( VRAMPixel *TBuf, int W, int H )
{
	long Cnt=(long)W*H;
	while( Cnt>0 )
	{
		VRAMPixel N=*TBuf++;
		TBuf++; /* barva nas nezajima */
		Cnt-=N;
	}
	return TBuf;
}

/* nabizi jednoduchy format - pouzivame v utilitach */


void *MTPILoad( const char *N, int *W, int *H, int *NP, int *NC, long *RGB, long Mag )
{ /* monochrome IMG */
	int ret=-1;
	void *Buf=NULL;
	int f=open(N,O_RDONLY|O_BINARY);
	if( f>=0 )
	{
		struct tpiheader h;
		int c;
		Flag TPSpec=Mag==(MAG('PNT')<<8);
		if( read(f,&h,sizeof(h))!=sizeof(h) ) goto Error;

    SwapEndianL(&h.Magic);
    SwapEndianW(&h.x100);  /* 0x100 */
    SwapEndianW(&h.NCol);
    SwapEndianW(&h.W);
    SwapEndianW(&h.H);
    SwapEndianW(&h.NPlan);
    SwapEndianW(&h.Resvd);
	  SwapEndianL(&h.LenDump);

		if( Mag && h.Magic!=Mag ) goto Error;
		if( h.x100!=0x100 ) goto Error;
		if( h.NCol>256 ) goto Error; /* moc velka paleta */
		Buf=mallocSpc(h.LenDump,'LTPI');if( !Buf ) goto Error;
		{
			word RGB24[256][3]; /* Motorola - words */
			long Sz=h.NCol*sizeof(*RGB24);
			if( read(f,RGB24,Sz)!=Sz ) goto Error;
			for( c=0; c<h.NCol; c++ )
			{
				long r,g,b;
        word w;
				w=RGB24[c][0];SwapEndianW(&w);r=w*255/1000;
				w=RGB24[c][1];SwapEndianW(&w);g=w*255/1000;
				w=RGB24[c][2];SwapEndianW(&w);b=w*255/1000;
				*RGB++=(r<<16)|(g<<8)|(b);
			}
		}
		RGB-=h.NCol;
		if( h.NCol>1 )
		{
			long p;
			p=RGB[1],RGB[1]=RGB[h.NCol-1],RGB[h.NCol-1]=p;
		}
		if( TPSpec )
		{
			*W=(h.W+15)/16*16; /* True Paint zaokrouhluje */
		}
		else *W=h.W;
		*H=h.H;
		*NP=h.NPlan;
		*NC=h.NCol;
		if( read(f,Buf,h.LenDump)!=h.LenDump ) goto Error;
    KonvBP2PPEndian(Buf,*W,*H);
		if( h.W!=*W && h.NCol==0 ) /* TruePaint nechava okraj */
		{
			int l,c;
			word *S=Buf,*D=Buf;
			for( l=0; l<h.H; l++ )
			{
				for( c=0; c<h.W; c++ ) *D++=*S++;
				S+=*W-h.W;
			}
			*W=h.W;
		}
		ret=0;
		Error:
		close(f);
		if( ret && Buf ) freeSpc(Buf),Buf=NULL;
	}
	return Buf;
}

int MTPISave( const char *N, int W, int H, int NP, int NC, long *RGB, void *Buf, long Len, long Mag )
{ /* monochrome IMG */
	int ret=-1;
	int f=creat(N,O_RDONLY);
	if( f>=0 )
	{
		struct tpiheader h;
		int c;
		memset(&h,0,sizeof(h));
		h.Magic=Mag;
		h.x100=0x100;
		h.NCol=NC;
		h.NPlan=NP;
		h.W=W;
		h.H=H;
		h.LenDump=Len;
		if( write(f,&h,sizeof(h))!=sizeof(h) ) goto Error;
		if( NC>1 )
		{
			long p;
			p=RGB[1],RGB[1]=RGB[NC-1],RGB[NC-1]=p;
		}
		{
			word RGB24[256][3]; /* Motorola - words */
			long Sz=h.NCol*sizeof(*RGB24);
			for( c=0; c<NC; c++ )
			{
				long r,g,b,w;
				w=*RGB++;
				r=(byte)(w>>16);
				g=(byte)(w>>8);
				b=(byte)(w>>0);
				RGB24[c][0]=(word)(r*1000/255);
				RGB24[c][1]=(word)(g*1000/255);
				RGB24[c][2]=(word)(b*1000/255);
			}
			if( write(f,RGB24,Sz)!=Sz ) goto Error;
		}
		RGB-=NC;
		if( NC>1 )
		{
			long p;
			p=RGB[1],RGB[1]=RGB[NC-1],RGB[NC-1]=p;
		}
		if( write(f,Buf,Len)!=Len ) goto Error;
		ret=0;
		Error:
		close(f);
	}
	return ret;
}

int TPISave( const char *N, int W, int H, int NP, int NC, long *RGB, void *Buf, long Len )
{
	return MTPISave(N,W,H,NP,NC,RGB,Buf,Len,'PNT'<<8);
}
void *TPILoad( const char *N, int *W, int *H, int *NP, int *NC, long *RGB )
{
	return MTPILoad(N,W,H,NP,NC,RGB,'PNT'<<8);
}

int PANSave( const char *N, int W, int H, int NP, int NC, long *RGB, void *Buf, long Len )
{ /* obecny obrazek - paleta + memory dump */
	/* obvykle je po wordech planove nebo direct color */
	int ret=-1;
	FILE *f=fopen(N,"wb");
	if( f )
	{
		int c;
		if( fputiw(W,f)<0 ) goto Error;
		if( fputiw(H,f)<0 ) goto Error;
		if( fputiw(NC,f)<0 ) goto Error;
		if( fputiw(NP,f)<0 ) goto Error;
		if( fputil(Len,f)<0 ) goto Error;
		for( c=0; c<NC; c++ )
		{
			if( fputi24(*RGB++,f)<0 ) goto Error;
		}
		if( SavePACW(f,Buf,Len/2)<0 ) goto Error;
		ret=0;
		Error:
		fclose(f);
	}
	return ret;
}
void *PANLoad( const char *N, int *W, int *H, int *NP, int *NC, long *RGB )
{ /* obecny obrazek - paleta + memory dump */
	/* obvykle je po wordech planove nebo direct color */
	int ret=-1;
	word *Buf=NULL;
	FILE *f=fopen(N,"rb");
	if( f )
	{
		int c;
		long w,h,nc,np,len;
		if( setvbuf(f,NULL,_IOFBF,BUF_OPT)<0 )
		{
			if( setvbuf(f,NULL,_IOFBF,BUF_MIN)<0 ) goto Error;
		}
		w=fgetiw(f);if( w<=0 ) goto Error;
		h=fgetiw(f);if( h<=0 ) goto Error;
		nc=fgetiw(f);if( nc<0 ) goto Error;
		np=fgetiw(f);if( np<0 ) goto Error;
		len=fgetil(f);if( len<0 ) goto Error;
		*W=(int)w,*H=(int)h;
		*NP=(int)np,*NC=(int)nc;
		Buf=mallocSpc(len,'IPAN');
		if( !Buf ) goto Error;
		for( c=0; c<nc; c++ )
		{
			long l=fgeti24(f);
			if( l<0 ) goto Error;
			*RGB++=l;
		}
		if( LoadPACW(f,Buf,len/2)<0 ) goto Error;
		ret=0;
		Error:
		if( ret && Buf ) freeSpc(Buf),Buf=NULL;
		fclose(f);
	}
	return Buf;
}

