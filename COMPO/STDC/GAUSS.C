/* vzorove reseni - Gaussova eliminace */

#include <stdio.h>

#define RNG 3

typedef struct
{
	double L[RNG][RNG]; /* postupne po radcich */
	double P[RNG];
} Soustava;

Soustava Zad=
{
	{
		{-5,6,0,},
		{-5,8,0,},
		{-1,4,7,},
	},
	{0,4,8},
};

void TiskZad( Soustava *S )
{
	int i,j;
	for( i=0; i<RNG; i++ )
	{
		for( j=0; j<RNG; j++ ) printf("%6.3lf ",S->L[i][j]);
		printf("| %6.3lf\n",S->P[i]);
	}
	printf("\n");
}

static int Vyres( Soustava *S, double *res )
{
	int i,j,k,l;
	char BylPivot[RNG];
	for( i=0; i<RNG; i++ ) BylPivot[i]=0;
	for( i=0; i<RNG; i++ )
	/*i=0;*/
	{ /* postupne nuluj sloupce */
		int JePivot=0;
		for( j=0; j<RNG; j++ )
		{ /* najdi pivota */
			if( S->L[j][i] && !BylPivot[j] )
			{
				BylPivot[j]=1;
				JePivot=1;
				for( k=0; k<RNG; k++ ) if( k!=j )
				{
					double K=S->L[k][i]/S->L[j][i];
					for( l=0; l<RNG; l++ )
					{
						S->L[k][l]-=K*S->L[j][l];
					}
					S->P[k]-=K*S->P[j];
				}
				break;
			}
		}
		if( !JePivot ) return -1; /* degradovana */
	}
	for( i=0; i<RNG; i++ )
	{
		for( j=0; j<RNG; j++ ) if( S->L[i][j] ) res[j]=S->P[i]/S->L[i][j];
	}
	return 0;
}

int main()
{
	double res[RNG];
	Soustava Sou=Zad;
	TiskZad(&Sou);
	if( Vyres(&Sou,res)>=0 )
	{
		int i,j;
		for( j=0; j<RNG; j++ ) printf("%6.3lf ",res[j]);
		printf("\n\n");
		for( i=0; i<RNG; i++ )
		{
			double s=0;
			for( j=0; j<RNG; j++ ) s+=Zad.L[i][j]*res[j];
			printf("%6.3lf ",s);
		}
		printf("\n\n");
	}
	TiskZad(&Sou);
	return 0;
}
