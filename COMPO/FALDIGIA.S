/* DIGIMUS.S - digit ln¡ zvuk (Componium) - F030 */
/* SUMA 12/1992-3/1993 */

include "macros.i"

super
MC68030

text

import FalcKans    ; - parametry hrani - nastroje ...
export FalcPlayDig ; - vyroba bufferu (samplu)

import F_Divis
import F_Povel

/* na F030 DMA prehravani */
/* pouziti X aritmetiky - 32.16 bitu */

record
	long RPos,RKrk,RRep; /* 32 casti */
	long RPosF,RKrkF,RRepF; /* .32 casti */
	long RNas;
	byte resvd; /* pro L/P Vol */
	byte Fade; /* fadeouty konc… t¢n… */
	byte LVol,PVol;
	byte EffC,EffD,EffA,EffB; /* efekty - zat¡m se pou§¡v  jen A,B */
	/* dal uz to .S nepouziva */
	int StereoMin,StereoMax;
	long NasDef;
	FNastroj:
endrec

text

Dsp_HSR =$ffffa202
Dsp_HRX =$ffffa204
Dsp_HTX =$ffffa204

Dsp_RBF=0 /* Data Ready */
Dsp_CTS=1 /* Clear To Send */

FBRamPomer equ 128

macro DspWaitCTS
	local Wait
	Wait:
		btst.b #Dsp_CTS,(Port)
	beq.b Wait
endm

macro QDspWaitCTS reg
	local Wait
	Wait:
		btst.b reg,(Port)
	beq.b Wait
endm

module FalcPlayDig

	NKan=d0
	Len=d1 /* parametry */

	/* tento blok se movem.l presouva - dulezite poradi */

	Pos=d2
	Krk=d3
	Rep=d4

	PosF=d5 /* word desetin */
	KrkF=d6
	RepF=d7
	
	Nas=a0
	Stereo=a1
	
	Snd=a2 /* nutnØ adresovì registr */
	Kanal=a3

	Base=a4
	Port=a5
	
	PushReg d3-d7/a2-a6

	lea Dsp_HTX,Base
	lea Dsp_HSR,Port
	
	move F_Povel,Len
	beq.b NoPovel
		sub #1,Len
		bne.b NoFade
			DspWaitCTS
			move.l #8,(Base)
			; chceme Len=$7fffff/freq = $7fffff/(25175000L/(256*(Divis+1)))
			move F_Divis,Len
			addq #1,Len
			mulu #85,Len
			DspWaitCTS
			move.l Len,(Base)
		bra.b NoPovel
		NoFade:
		clr F_Povel
	NoPovel:

	DspWaitCTS
	move.l #4,(Base)
	
	DspWaitCTS
	move.l NKan,(Base)
	
	subq #1,NKan
	
	lea FalcKans,Kanal
	KanLop:
		movem.l (Kanal),Pos/Krk/Rep/PosF/KrkF/RepF/Nas/Stereo
		
		tst.l Krk
		bne.b Zni
		tst.l KrkF
		bne.b Zni
			sub.l Stereo,Stereo
		Zni:
		
		DspWaitCTS
		move.l Stereo,(Base)
		
		cmpa.w #0,Stereo ; je adres.
		beq Nezni
		
		; poçleme parametry efekt… A a B
		clr.l Len
		move.w EffA(Kanal),Len
		DspWaitCTS
		move.l Len,(Base)
		
		; zkus¡me zjistit, zda doch z¡ bØhem bloku k loopingu
		
		move.l Krk,Len ; celoŸ¡s. Ÿ st je urŸitØ max. 16b
		addq.l #1,Len
		asl.l #7,Len
		add.l Pos,Len
		bcs.b JeLoop
			move #FBRamPomer/4-1,Len
			moveq #Dsp_CTS,Rep
			FalcLop:
				rept 4
					local Wait
					move.w (Nas,Pos.l*2),Snd /* intove nastroje */
					add.l KrkF,PosF
					addx.l Krk,Pos
					QDspWaitCTS Rep
					move.l Snd,(Base) /* v def nastroje musi byt patric. offsety */
				endm
			dbf Len,FalcLop
		bra.b LoopDone
		JeLoop:
			move #FBRamPomer-1,Len
			FalcLopL:
				move.w (Nas,Pos.l*2),Snd /* intove nastroje */
				add.l KrkF,PosF
				addx.l Krk,Pos
				bcc.b NoRep /* prenos - dosli jsme na nulu */
					sub.l RepF,PosF
					subx.l Rep,Pos
					bcs.b NoRep /* je-li Rep v poradku, musi byt C */
					sub.l KrkF,PosF /* kanal je uz prazdny */
					subx.l Krk,Pos /* kanal je uz prazdny */
				NoRep:
				DspWaitCTS
				move.l Snd,(Base) /* v def nastroje musi byt patric. offsety */
			dbf Len,FalcLopL
		LoopDone:
		move.l Pos,RPos(Kanal)
		move.l PosF,RPosF(Kanal)
		move.l Stereo,Pos
		asr.l #8,Pos
		asr.l #8,Pos ; fade slo§ka
		beq.b NeFade
			clr.l RKrk(Kanal)
			clr.l RKrkF(Kanal)
		NeFade:
		Nezni:
		add #FNastroj,Kanal
	dbf NKan,KanLop
	
	PopReg
	rts
endmod

import FalcTimerPlay
export FalcIntrPlay

text
super

_TDA=1 /* pouzivej Timer A */

module FalcIntrPlay
	tst PlaySem
	bmi NoPlay
		sub #1,PlaySem
		if _TDA
			bclr #5,$fffffa0f /* povol Timer A */
		else
			bclr #4,$fffffa11 /* povol Timer D */
		endif
		
		movem.l D0-D2/A0-A1,SaveRegs

		; dne 4.9.95 jsme se rozhodli povolit i VB interrupt
		move sr,d0
		and #$f8ff,d0 /* povol preruseni s nizsi prior. */
		;or #$500,d0
		or #$300,d0
		move d0,sr
		
		jsr FalcTimerPlay
		
		movem.l SaveRegs,D0-D2/A0-A1
		
		add #1,PlaySem
		rte
	NoPlay:
	if _TDA
		bclr #5,$fffffa0f /* povol Timer A */
	else
		bclr #4,$fffffa11 /* Timer D */
	endif
	rte
data
	PlaySem: dc.w 0 /* semafor - jen jeden vstup do proc. */
bss
	SaveRegs: ds.l 16
text
endmod
