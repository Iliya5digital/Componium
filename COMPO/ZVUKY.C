/* zvuk na F030 */
/* SUMA 4/1994 */

#include <stdlib.h>
#include <tos.h>
#include <sndbind.h>

#define C_ESC 0x1b

int main()
{
	if( locksnd()>=0 )
	{
		long len=(long)Malloc(-1)-128L*1024;
		if( len>0 )
		{
			void *Buf=malloc(len);
			if( Buf )
			{
				int old_psg=(int)soundcmd(ADDERIN,INQUIRE);
				int old_add=(int)soundcmd(ADCINPUT,INQUIRE);
				soundcmd(ADCINPUT,old_psg&~3);
				soundcmd(ADDERIN,2);
				devconnect(DMAPLAY,DAC,CLK_25M,CLK25K,NO_SHAKE);
				devconnect(ADC,DMAREC,CLK_25M,CLK25K,NO_SHAKE);
				setmode(STEREO16);
				settrack(0,0);
				setmontrack(0);
				for(;;)
				{
					void *EBuf=(char *)Buf+len;
					struct
					{
						void *ply,*rec;
						void *res0,*res1;
					} buffers;
					Cconws("Esc -> Konec, Jin  kl vesa -> START\r\n");
					if( (char)Cnecin()==C_ESC ) break;
					buffoper(0);
					setbuffer(RECORD,Buf,EBuf);
					buffoper(RECORD_ENABLE);
					Cconws("Kl vesa -> STOP\r\n");
					Cnecin();
					buffptr(&buffers);
					if( buffers.rec>Buf ) EBuf=buffers.rec;
					setbuffer(PLAY,Buf,EBuf);
					buffoper(PLAY_ENABLE);
				}
				buffoper(0);
				devconnect(DMAPLAY,DAC,CLK_25M,0,NO_SHAKE);
				devconnect(ADC,0,CLK_25M,0,NO_SHAKE);
				soundcmd(ADCINPUT,old_psg);
				soundcmd(ADDERIN,old_add);
			}
		}
		unlocksnd();
	}
	return 0;
}
