/* oprava chyb v PCEXTLIB.LIB */
/* SUMA 7/1993 */

#include <ext.h>
#include <tos.h>

int getdisk( void ) {return Dgetdrv();}
int setdisk( int drive ) {return Dsetdrv(drive);}

int chdir( char *f )
{
	if( f[1]==':' )
	{
		Dsetdrv((f[0]-'A')&0xf);
	}
	return Dsetpath(f);
}
