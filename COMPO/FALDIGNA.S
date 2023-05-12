/* DIGIMUS.S - digit ln¡ zvuk (Componium) - F030 */
/* SUMA 12/1992-3/1993 */

include "macros.i"

super
MC68030

VMaxVol=$100
XBIOS=1 ; zda podporovat xbiosu v preruseni

text

import FalcKans    ; - parametry hrani - nastroje ...
export Falc2PlayDig ; - vyroba bufferu (samplu)
export Falc8PlayDig ; - vyroba bufferu (samplu)

/* na F030 DMA prehravani */
/* pouziti X aritmetiky - 32.16 bitu */

record
	long RPos,RKrk,RRep; /* 32 casti */
	long RPosF,RKrkF,RRepF; /* .16 casti */
	long RNas;
	word resvd; /* pro L/P Vol */
	byte LVol,PVol;
	word Fade;
	/* dal uz to .S nepouziva */
	int StereoMin,StereoMax;
	long NasDef;
	fn_resvd: ds.b 6
	FNastroj:
endrec

COPTIM=1 ; zda vyuzivat prazdnych kanalu na zrychleni

; nekdo si bere z registru, nekdo ma registru malo,
; takze potrebuje konstantu

text

macro PlayFalc Label,NKan

DelkaVzorku equ NKan*4
StepKonst equ DelkaVzorku-4

bss
	SaveLen: ds.w 1
text
	Len=d0 /* parametry */
	Base=a0
	
	/* tento blok se movem.l presouva - dulezite poradi */

	Pos=d1
	Krk=d2
	Rep=d3

	PosF=d4 /* word desetin */
	KrkF=d5
	RepF=a1

	Nas=a2
	Step=a3

	SaveBuf=a4
	
	Snd=d6 /* nutne dat. reg */
	Stereo=a5

	FadeL=d7
	FadeP=a6

	macro DoKanal I,instr
		FalcInit&I:
		movem.l (FNastroj*I)+FalcKans,Pos/Krk/Rep/PosF/KrkF/RepF/Nas/Stereo
		if COPTIM
			tst KrkF
			bne Play&I
			tst.l Krk
			beq NoPlay&I
		endif
		Play&I:
		move Stereo,Snd
		swap Snd
		FalcLop&I:
			move.w (Nas,Pos.l*2),Snd /* intove nastroje */
			add.w KrkF,PosF
			addx.l Krk,Pos
			bcc NoRep&I /* prenos - dosli jsme na nulu */
				sub.w RepF,PosF
				subx.l Rep,Pos
				bcs NoRep&I /* je-li Rep v poradku, musi byt C */
				sub.w KrkF,PosF /* kanal je uz prazdny */
				subx.l Krk,Pos /* kanal je uz prazdny */
				if I!=0
					if COPTIM
						bra NoPlay&I
					endif
				endif
			NoRep&I:
			move.l Snd,(Base)+ /* v def nastroje musi byt patric. offsety */
			add.l Step,Base
		dbf Len,FalcLop&I
		if COPTIM
		bra KonPlay&I
		NoPlay&I:
		clr Snd
		NFalcLop&I:
			move.l Snd,(Base)+
			add.l Step,Base
		dbf Len,NFalcLop&I
		KonPlay&I:
		endif
		move.l Pos,FalcKans+I*FNastroj+RPos
		move.w PosF,FalcKans+I*FNastroj+RPosF+2 /* jen word */
	endm

	macro FadeKanal I,instr /* jako DoKanal, jenze s fadovanim */
		FalcInitFad&I:
		movem.l (FNastroj*I)+FalcKans,Pos/Krk/Rep/PosF/KrkF/RepF/Nas/Stereo
		if COPTIM
			tst KrkF
			bne PlayFad&I
			tst.l Krk
			beq NoPlayFad&I
		endif
		PlayFad&I:

	FadeLog=7
	; vyfaduj behem 2^FadeLog dob
	; pro FadeLog=8 je to 0.01 sec, FadeLog=6 je to 0.002 sec
		StereoL=Step
		move.w Stereo,Snd
		beq NoPlayFad&I
		
		clr.b Snd
		move.w Snd,StereoL
		lsr.w #FadeLog,Snd
		move.w Snd,FadeL

		move.w Stereo,Snd
		and.w #$ff,Snd
		StereoP=Stereo
		lsl.w #8,Snd
		move Snd,StereoP
		lsr.w #FadeLog,Snd
		move.w Snd,FadeP
		
		FalcLopFad&I:
			move.w (Nas,Pos.l*2),Snd /* intove nastroje */
			add.w KrkF,PosF
			addx.l Krk,Pos
			bcc NoRepFad&I /* prenos - dosli jsme na nulu */
				sub.w RepF,PosF
				subx.l Rep,Pos
				bcs NoRepFad&I /* je-li Rep v poradku, musi byt C */
				sub.w KrkF,PosF ; kanal je uz prazdny
				subx.l Krk,Pos ; kanal je uz prazdny
				if I!=0
					if COPTIM
						bra NoPlayFad&I
					endif
				endif
			NoRepFad&I:
			move Snd,2(Base); v def nastroje musi byt patric. offsety

			; vypocitej hlasitost po fadu

			cmp FadeL,StereoL
			sub FadeL,StereoL
			bcc LOKFad&I
				move #0,StereoL
			LOKFad&I:
			cmp FadeP,StereoP
			sub FadeP,StereoP
			bcc POKFad&I
				move #0,StereoP
			POKFad&I:
			
			move StereoL,Snd
			lsr #8,Snd
			move.b Snd,(Base)+

			move StereoP,Snd
			lsr #8,Snd
			move.b Snd,(Base)+
			
			add.w #DelkaVzorku-2,Base
		dbf Len,FalcLopFad&I
		if COPTIM
		bra KonPlayFad&I
		NoPlayFad&I:
		clr Snd
		NFalcLopFad&I:
			move.l Snd,(Base)+
			add.w #StepKonst,Base
		dbf Len,NFalcLopFad&I
		KonPlayFad&I:
		endif
		move.l Pos,FalcKans+I*FNastroj+RPos
		move.w PosF,FalcKans+I*FNastroj+RPosF+2 /* jen word */
		move StereoL,Snd
		move StereoP,Pos
		lsl #8,Pos
		move.b Pos,Snd
		move.w Snd,FalcKans+I*FNastroj+LVol
		move #StepKonst,Step
	endm

	macro DoFadeKanal I ; rohodni, zda fadovat
		tst.w FalcKans+I*FNastroj+Fade
		bne.b YesFade&I
			DoKanal I
		bra EndFade&I
		YesFade&I:
			FadeKanal I
		EndFade&I:
	endm

	Label:
	PushReg d3-d7/a2-a6
		sub #1,Len
		blo NoFalcLop
		move #StepKonst,Step
		move.l Base,SaveBuf
		addq #4,SaveBuf
		move Len,SaveLen
		DoFadeKanal 0
		move.l SaveBuf,Base
		addq #4,SaveBuf
		move SaveLen,Len
		DoFadeKanal 1
		if NKan>2
			move.l SaveBuf,Base
			addq #4,SaveBuf
			move SaveLen,Len
			DoFadeKanal 2
			move.l SaveBuf,Base
			addq #4,SaveBuf
			move SaveLen,Len
			DoFadeKanal 3
			move.l SaveBuf,Base
			addq #4,SaveBuf
			move SaveLen,Len
			DoFadeKanal 4
			move.l SaveBuf,Base
			addq #4,SaveBuf
			move SaveLen,Len
			DoFadeKanal 5
			move.l SaveBuf,Base
			addq #4,SaveBuf
			move SaveLen,Len
			DoFadeKanal 6
			move.l SaveBuf,Base
			move SaveLen,Len
			DoFadeKanal 7
		endif
		NoFalcLop:
	PopReg
	rts
endm

module PlayFal2
	; PlayFalc Falc2PlayDig,2
	Falc2PlayDig:
	rts
endmod

module PlayFal8
	PlayFalc Falc8PlayDig,8
endmod

text

if !XBIOS
export FalcGetPlayBuf
;void *FalcGetPlayBuf( void )
module FalcGetPlayBuf
	bclr #7,$ff8901

	moveq #0,d2
	move.b $ff890D,d0
	move.b $ff890B,d1
	move.b $ff8909,d2
	lsl.l #8,d2
	move.b d1,d2
	lsl.l #8,d2
	move.b d0,d2
	move.l d2,a0
	
	rts
endmod

; jednotlive byty se ctou od nejnizzsiho do nejvyssiho
; dojde-li k nesrovnalosti, precte se z vyssiho nova (vysoka hodnota)
; ale z nissiho jeste stara (vysoka) hodnota
; (mezi dvema vzorky 50 kHz uplyne pri 16 MHz 320 cyklu)

endif

import FalcTimerPlay
export FalcIntrPlay

text
super

_TDA=1 /* pouzivej Timer A */
_SOFTSEM=1

savptr equ $4a2

module FalcIntrPlay
	tst PlaySem
	bmi NoPlay
		sub #1,PlaySem
		if _TDA
			bclr #5,$fffa0f /* povol Timer A */
		else
			bclr #4,$fffa11 /* povol Timer D */
		endif
		movem.l D0-D2/A0-A1,-(a7)
		move sr,d0
		and #$f8ff,d0 /* povol preruseni s nizsi prior. */
		or #$500,d0
		move d0,sr
		if XBIOS
			move.l savptr,-(a7)
			move.l #SaveRegs,savptr
		endif
		jsr FalcTimerPlay
		if XBIOS
			move.l (a7)+,savptr
		endif
		movem.l (a7)+,D0-D2/A0-A1
		add #1,PlaySem
		rte
	NoPlay:
	if _TDA
		bclr #5,$fffa0f /* povol Timer A */
	else
		bclr #4,$fffa11 /* Timer D */
	endif
	rte
	data
	PlaySem: dc.w 0 /* semafor - jen jeden vstup do proc. */
	if XBIOS
		bss
			ds.l 64 ; dost mista i s rezervou
		SaveRegs:
			ds.l 0
	endif
endmod
