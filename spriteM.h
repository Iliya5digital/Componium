
typedef struct
{
	int s_h,s_w; /* vyska, sirka ve wordech */
	int Img[0];
} Sprite; /* musi nasledovat definice obrazku - pozor na alignement */

/* SPRITE.S */
extern word *VRA;

extern int ScrH;
extern int BPL;

extern byte Font[];

void __regargs Draw( int x, int y, const Sprite *Def );
int __regargs TiskS( void *VAdr, const char *Str, int x );
int __regargs TiskSG( void *VAdr, const char *Str, int x ); /* sedy */
int __regargs TiskS8( void *VAdr, const char *Str, int x );
int __regargs TiskS8T( void *VAdr, const char *Str, int x ); /* tucne */
void __regargs HCara( void *VAdr, int xz, int xk );
void __asm HBlok
(
	register __a0 void *VAdr, register __d0 int xz,
	register __d1 int xk, register __d2 int hb
);

extern byte PFont[0x1000]; /* proporcio. varianta fontu */
extern byte Prop[0x100];

int PropS( void *LAdr, const char *Str, int x );

int ZacGraf( void *NormFont );
void KonGraf( void );
