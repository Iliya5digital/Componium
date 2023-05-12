
include "macros.i"

record
	int s_h,s_w;
	img:
endrec

bss
global long,VRA
global int,BPL
global int,ScrH

export Draw

mc68020

macro andmi dr0,i
	if i=0
		and Mask1,dr0
	else
		if i=1
			and Mask2,dr0
		else
			if i=2
				and Mask3,dr0
			else
				and Mask4,dr0
			endif
	endif
endm

ScrW=640

data

	rept 16 /* maska okraj… obrazovky */
		dc.w 0
	endm
Maska:
	rept 40
		dc.w $FFFF
	endm
	rept 16
		dc.w 0
	endm
	
text
module Draw /* int x, int y, Sprite *Def */
	x=d0
	y=d1
	Def=a0
	TAdr=a1

	Tab=a2

	dr0=d2
	dr1=d3
	H=d4
	W=d5
	WCnt=d6
	MOff=d7
	
	PushReg x/y/H/dr0/dr1/W/WCnt/MOff/TAdr/Def/Tab,Proc
		move x,MOff
		and #$e,x
		asr #4,MOff
		add MOff,MOff
		
		move s_h(Def),H
		move s_w(Def),W
		add.l #img,Def
		tst y
		bge YPlus
			add y,H /* y<0 */
			neg y
			muls W,y
			add.l y,y
			add.l y,Def
			clr y
			bra YOkr
		YPlus:
		move ScrH,dr0
		sub y,dr0
		sub H,dr0
		bge YOkr
			add dr0,H /* y+h>ScrH */
		YOkr:
		muls BPL,y
		move.l VRA,TAdr
		add.l y,TAdr
		add MOff,TAdr
		sub #1,H
		sub #1,W
		neg x
		add #$10,x
		tst MOff
		bmi DrawZEMM
		move BPL,dr0
		subq #2,dr0
		sub W,dr0
		sub W,dr0
		cmp dr0,MOff
		bge DrawZEMM
			DrawLopY2:
				i=0
				move W,WCnt
				WLop2:
					clr.l dr0
					move (Def)+,dr0
					lsl.l x,dr0
					or.l dr0,(TAdr)
					add.l #2,TAdr
				dbf WCnt,WLop2
				sub W,TAdr
				sub W,TAdr
				add BPL,TAdr
				subq #2,TAdr
			dbf H,DrawLopY2
		bra EndDraw
		DrawZEMM:
			DrawLopZ:
				lea Maska,Tab
				add MOff,Tab
				move W,WCnt
				WLop:
					clr.l dr0
					move (Def)+,dr0
					lsl.l x,dr0
					and.l (Tab),dr0
					or.l dr0,(TAdr)
					add.l #2,TAdr
					add.l #2,Tab
				dbf WCnt,WLop
				sub W,TAdr
				sub W,TAdr
				add BPL,TAdr
				subq #2,TAdr
			dbf H,DrawLopZ
		EndDraw:
	PopReg
	rts
endmod

bss

export Font
Font:
	ds.b $1800+$80*3

export TiskS,TiskSG,TiskS8,TiskS8T

text
module TiskPisM /* ponechava a0,d0 */
	lea Font+$800,a1
	add d0,a1
	rept 16
		clr d2
		move.b (a1),d2
		lsl d1,d2
		or.b d2,1(a0)
		lsr #8,d2
		or.b d2,(a0)
		add #$100,a1
		add BPL,a0
	endm
	add #8,d4
	move BPL,d2
	asl #4,d2
	sub d2,a0
	rts
endmod

/* tisk svetleho pisma */

module TiskPisMG /* ponechava a0,d0 */
	lea Font+$800,a1
	add d0,a1
	rept 8
		move.b (a1),d2
		and #$55,d2
		lsl d1,d2
		or.b d2,1(a0)
		lsr #8,d2
		or.b d2,(a0)
		add #$100,a1
		add BPL,a0
		move.b (a1),d2
		and #$aa,d2
		lsl d1,d2
		or.b d2,1(a0)
		lsr #8,d2
		or.b d2,(a0)
		add #$100,a1
		add BPL,a0
	endm
	add #8,d4
	move BPL,d2
	asl #4,d2
	sub d2,a0
	rts
endmod

module TiskPisM8T /* ponechava a0,d0 */
	lea Font,a1
	add d0,a1
	move #$1ff,d2
	lsl.l d1,d2
	not.l d2 
	rept 8
		and.b d2,1(a0) /*  netransparentni */
		ror.l #8,d2
		and.b d2,(a0)
		rol.l #8,d2
		add BPL,a0
	endm
	move BPL,d2
	asl #3,d2
	sub d2,a0
	rept 8
		clr d2
		move.b (a1),d2
		asl #1,d2
		or.b (a1),d2
		lsl d1,d2
		or.b d2,1(a0)
		lsr #8,d2
		or.b d2,(a0)
		add #$100,a1
		add BPL,a0
	endm
	move BPL,d2
	asl #3,d2
	sub d2,a0
	add #8,d4
	rts
endmod

module TiskPisM8 /* ponechava a0,d0 */
	lea Font,a1
	add d0,a1
	move #$ff,d2
	lsl.l d1,d2
	not.l d2 
	rept 8
		and.b d2,1(a0) /*  netransparentni */
		ror.l #8,d2
		and.b d2,(a0)
		rol.l #8,d2
		add BPL,a0
	endm
	move BPL,d2
	asl #3,d2
	sub d2,a0
	rept 8
		clr d2
		move.b (a1),d2
		lsl d1,d2
		or.b d2,1(a0)
		lsr #8,d2
		or.b d2,(a0)
		add BPL,a0
		add #$100,a1
	endm
	add #8,d4
	move BPL,d2
	asl #3,d2
	sub d2,a0
	rts
endmod

module TiskSF /* v a3 adr. pro tisk 1 pismene */
	PushReg d2/d3/d4/d5/a2
		move d0,d4
		move d0,d5
		move d0,d1
		and #7,d1
		neg d1
		add #8,d1 /* v d1 in-byte offset */
		asr #3,d0 /* v a0 zac. adresa */
		add d0,a0
		move.l a1,a2
		bra TSWhile
		TSLop:
			jsr (a3)
			add.l #1,a0
		TSWhile:
			move BPL,d0
			subq #1,d0
			asl #3,d0
			cmp d0,d4
			bge TSEnd
			clr d0
			move.b (a2)+,d0
		bne TSLop
		TSEnd:
		sub d5,d4
		move d4,d0 /* vraci sirku, ne souradnici */
	PopReg
	rts
endmod

module TiskS
	move.l a3,-(a7)
	lea TiskPisM(PC),a3
	bsr TiskSF
	move.l (a7)+,a3
	rts
endmod
module TiskSG
	move.l a3,-(a7)
	lea TiskPisMG(PC),a3
	bsr TiskSF
	move.l (a7)+,a3
	rts
endmod
module TiskS8
	move.l a3,-(a7)
	lea TiskPisM8(PC),a3
	bsr TiskSF
	move.l (a7)+,a3
	rts
endmod
module TiskS8T
	move.l a3,-(a7)
	lea TiskPisM8T(PC),a3
	bsr TiskSF
	move.l (a7)+,a3
	rts
endmod


/* proporc. tisk */

bss

export PFont,Prop
PFont:   ds.b $1000
Prop:  ds.b $100

text

export PropS

module PropPis16M /* ponechava a1,d2 */
	lea Prop,a2
	clr d3
	move.b (a2,d0),d3
	lea PFont,a2
	add d0,a2
	move d2,d1
	move d2,d0
	add d3,d2
	asr #3,d1
	add d1,a0
	and #7,d0
	neg d0
	add #8,d0
	rept 16
		clr d1
		move.b (a2),d1
		lsl d0,d1
		or.b d1,1(a0)
		asr #8,d1
		or.b d1,(a0)
		add BPL,a0
		add #$100,a2
	endm
	move BPL,d1
	asl #4,d1
	sub d1,a0
	Povel:
	rts
endmod

;int PropS( void *LAdr, char *Str, int x );
module PropS
	PushReg a2/a3/d3/d4
		move d0,d4
		move d0,d2
		move.l a0,a3
		bra TSWhile
		TSLop:
			move.l a3,a0
			bsr PropPis16M
		TSWhile:
			move BPL,d0
			subq #1,d0
			asl #3,d0
			cmp d0,d2
			bge EndTS
			clr d0
			move.b (a1)+,d0
		bne TSLop
		EndTS:
		sub d4,d2
		move d2,d0 /* vratit sirku */
	PopReg
	rts
endmod

/* cary a obdelniky */

macro ZMasky.size Pocet
	__i=Pocet
	rept Pocet
		dc.size (1<<__i)-1
		__i=__i-1
	endm
endm
macro KMasky.size Pocet
	__i=Pocet
	rept Pocet
		dc.size -(1<<__i)&0xffff
		__i=__i-1
	endm
endm

data
CaraZacM: ZMasky.w 16
CaraKonM: KMasky.w 16

text

export HCara
module HCara
	VAZ=a0
	VAK=a1
	MA=a2
	xz=d0
	xk=d1
	d=d2
	PushReg VAZ/VAK/MA/xz/xk/d,Proc
	lea CaraZacM,MA
	move.l VAZ,VAK
	move xz,d
	asr #4,d
	add d,VAZ
	add d,VAZ
	and #$f,xz
	add xz,xz
	move xk,d
	asr #4,d
	add d,VAK
	add d,VAK
	and #$f,xk
	add xk,xk
	move VAK,d
	sub VAZ,d
	bgt Delsi
		move (MA,xz),xz
		and (CaraKonM-CaraZacM,MA,xk),xz
		or xz,(VAZ)
	bra KonKre
	Delsi:
		move (MA,xz),xz
		or xz,(VAZ)+
		bra WhileKr
		KrLop:
			or #$ffff,(VAZ)+
			WhileKr:
			cmp.l VAZ,VAK
		bhi KrLop
		move (CaraKonM-CaraZacM,MA,xk),xz
		or xz,(VAZ)
		KonecD:
	KonKre:
	PopReg
	rts
endmod

BlokR=$5555

export HBlok
module HBlok
	VAZ=a0
	VAK=a1
	VAP=a2
	MA=a3
	xz=d0
	xk=d1
	h=d2
	d=d3
	PushReg xz/xk/h/d/VAZ/VAK/VAP/MA,Proc
	asr #1,h
	sub #1,h
	lea CaraZacM,MA
	move.l VAZ,VAK
	move xz,d
	asr #4,d
	add d,VAZ
	add d,VAZ
	and #$f,xz
	add xz,xz
	move xk,d
	asr #4,d
	add d,VAK
	add d,VAK
	and #$f,xk
	add xk,xk
	move VAK,d
	sub VAZ,d
	bgt Delsi
		move (MA,xz),xz
		and (CaraKonM-CaraZacM,MA,xk),xz
		and #BlokR,xz
		move h,d
		move.l VAZ,VAP
		KVertLop:
			or xz,(VAP)
			add BPL,VAP
			add BPL,VAP
		dbf d,KVertLop
	bra KonKre
	Delsi:
		move (MA,xz),xz
		and #BlokR,xz
		move h,d
		move.l VAZ,VAP
		add.l #2,VAZ
		DVertLop1:
			or xz,(VAP)
			add BPL,VAP
			add BPL,VAP
		dbf d,DVertLop1
		bra WhileKr
		KrLop:
			move h,d
			move.l VAZ,VAP
			add.l #2,VAZ
			DVertLop2:
				or #BlokR,(VAP)
				add BPL,VAP
				add BPL,VAP
			dbf d,DVertLop2
			WhileKr:
			cmp.l VAZ,VAK
		bhi KrLop
		move (CaraKonM-CaraZacM,MA,xk),xz
		and #BlokR,xz
		move h,d
		move.l VAZ,VAP
		DVertLop3:
			or xz,(VAP)
			add BPL,VAP
			add BPL,VAP
		dbf d,DVertLop3
		KonecD:
	KonKre:
	PopReg
	rts
endmod

