#ifndef spritem_h_included
#define spritem_h_included

typedef struct
{
  int s_h,s_w; /* vyska, sirka ve wordech */
  #pragma warning (suppress:4200)
  int Img[0];
} Sprite; /* musi nasledovat definice obrazku - pozor na alignement */

/* SPRITE.S */

extern byte Font[];

void  Draw( int x, int y, const Sprite *Def );
int TiskS( int y, const char *Str, int x );
int TiskSG( int y, const char *Str, int x ); /* sedy */
int TiskS8( int y, const char *Str, int x );
int TiskS8T( int y, const char *Str, int x ); /* tucne */
void HCara( int y, int xz, int xk );
void HBlok( int y, int xz, int xk, int hb );

extern byte PFont[0x1000]; /* proporcio. varianta fontu */
extern byte Prop[0x100];

int PropS( int ye, const char *Str, int x );

int ZacGraf( void *NormFont );
void KonGraf( void );

#endif
