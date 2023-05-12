; COMPO.ASM
; spojovani kanalu + stereofonie + efekty pro Componium
; SUMA 9/1995-9/1995
; komunikuje pýes host-port

	nolist
compo ident	1,0
	include "intequ.asm"
	include "ioequ.asm"
	include "macros.asm"
	list

; externe musi byt definovan NKan bud jako 16, 8, 4, nebo jako 2
; ex. r…zn‚ varianty LODu

; CRA bits------------------------------------------------
; 15 = PS        = 0      : 0=/1, 1=/8, prescalar
; 14-13 = WL1-0  = 10     : 00=8, 01=12, 10=16, 11=24 bits (set to 16-bits now)
; 12-8 = DC4-DC0 = 0 0001 : divider control (normally word transfer rate)
; 7-0 = PM7-0    = $00    : divide by 1-256, 0=fastest (divide by 1)

CRA_BITS	EQU	$4100 ; ve skut. 4 kanaly (data+stereo)

; SSI CRB bits --------------------------------------------------
; CRB determines the serial modes
; 15 = RIE	= 1 : 0=disable, 1=enable receive interrupt
; 14 = TIE	= 1 : 0=disable, 1=enable transmit interrupt
; 13 = RE	= 1 : 0=disable, 1=enable receive
; 12 = TE	= 1 : 0=disable, 1=enable transmit

; 11 = MODE	= 1 : 0=normal, 1=network
; 10 = GCK	= 0 : 0=continuous, 1=gated
;  9 = SYN	= 0 : 0=asynchronous, 1=synchronous, transmit and receive
;  8 = FSL0	= 0 : 0=word, 1=bit length, Frame Sync clock

;  7 = FSL1	= 0 : 0=TX and RX same length, 1=different
;  6 = SHFD	= 0 : 0=MSB first, 1=LSB first
;  5 = SCKD	= 0 : 0=external, 1=internal bit clock
;  4 = SCD2	= 0 : 0=input, 1=output on SC2, transmit frame sync

;  3 = SCD1	= 0 : 0=input, 1=output on SC1, receive frame sync
;  2 = SCD0	= 0 : 0=input, 1=output on SC0, receive clock
;  1 = OF1	= 0 :
;  0 = OF0	= 0 : OF0 not to SC0 ; must match SCKD

CRB_BITS	EQU	$5800

; TOS 4.04 nekdy spatne inicializuje L: promenne umistene v interni pameti

	org l:0

SavePR: ds 1
Volume: ds 1

KanVol: ds 1
FadeSpd: ds 1

; sily efektu
	swspc x
EffASila: ds 1
EffBSila: ds 1
EffBMap: ds 1 ; indikuje dlouhodob‚ pou§it¡ efekt…

; parametry pro echo
	swspc l

dsp_init_zac equ *

eff_a_par: ds 8
eff_b_par: ds 8

dsp_init_kon equ *

	swspc x
EffParTab: dc eff_a_par,eff_b_par

FBRamPomer equ 128

; buffery, do kterìch sŸ¡t me data
eff_a_sum: dsm FBRamPomer
eff_b_sum: dsm FBRamPomer

; pracovn¡ buffery pro echo
EffLenBuf equ $1000
eff_a_buf: dsm EffLenBuf
eff_b_buf: dsm EffLenBuf


DSPLenBuf equ $1000-$200

out_buffer: dsm DSPLenBuf
	
	org p:$0
	jmp start

	org p:SSITD
	jsr dataout
	org p:SSITDE
	jsr dataout
 
	org p:$40

; R1(M1) je ukazatel pro z pis do bufferu out_bufferu
; R2(M2) je ukazatel pro Ÿten¡

dataout:
	jset #TFS,X:<<SSR,_dout_snc1
	movep X:(R2),X:<<TX
	rti
_dout_snc1:
	movep Y:(R2)+,X:<<TX
	rti

start:
	movep #$1F8,X:<<PCC
	movep #0,X:<<BCR
	movep #1,X:PBC
	move SR,Y:SavePR
	andi  #<$F0,MR ; scaling mode na 0, interrupt na 0
	movep #CRA_BITS,X:<<CRA
; priprav datove oblasti
; cele prijimane bloky - vc. sterea
	
	move #0.8,Y0 ; nech malou rezervu
	move Y0,Y1
	move #0,X0
	move X0,X1
	move Y0,X:Volume
	move X0,Y:Volume
	
	move #out_buffer,R1
	move #DSPLenBuf-1,M1
	move R1,R2
	move M1,M2
	move #>DSPLenBuf,Y0
	rep Y0 ; vymaz a inicializuj vyrovnavaci buffer
	move X,L:(R1)+
	
	move #dsp_init_zac,R0
	rep #dsp_init_kon-dsp_init_zac
	move X,L:(R0)+
	
	move #-1,M5
	move #>EffLenBuf,Y0
	move #eff_a_buf,R5
	rep Y0
	move X,L:(R5)+
	move #eff_b_buf,R5
	rep Y0
	move X,L:(R5)+
	
	; inicialzuj efekty
	; default hodnoty A - long delay
	move #>EffLenBuf-1,Y1
	move #>eff_a_buf,Y0
	;move Y,L:eff_a_par
	move Y1,X:eff_a_par
	move Y0,Y:eff_a_par
	move #0.7,Y0
	move Y0,X:eff_a_par+1 ; feedback
	move #0.9,Y0
	move Y0,X:eff_a_par+2 ; gain
		
	; default hodnoty B - short delay
	move #>EffLenBuf/3-1,Y1
	move #>eff_b_buf,Y0
	move Y1,X:eff_b_par
	move Y0,Y:eff_b_par
	move #0.6,Y0
	move Y0,X:eff_b_par+1 ; feedback
	move #0.9,Y0
	move Y0,X:eff_b_par+2 ; gain
	
	move #0,X0
	move X0,X:EffBMap
	
;read SSI SR to clear TUE bit
	btst  #4,X:<<SSR
	movep #CRB_BITS,X:<<CRB
; interrupt priority
	movep X:<<IPR,X:SavePR
	movep #$3000,X:<<IPR

main:
; tady muzes posilat maxima (na pozadani)
;cekame na dva bloky dat
	_rcvcom: jclr #0,X:<<HSR,_rcvcom
	movep X:<<HRX,X0
	jset #0,X0,ComAbort
	jset #1,X0,ComGetPos
	jset #2,X0,ComPutData
	jset #3,X0,ComFadeOut
	jset #4,X0,ComVolume
	jset #5,X0,ComEffPar
	
ComAbort:
	movep #0,X:<<CRB ; disable interr.
	movep X:SavePR,X:<<IPR ; vrat IPR
	move Y:SavePR,SR
DummyLop:
	_Wait: jclr #0,X:<<HSR,_Wait
	movep X:<<HRX,X0 ; prevezmi vyp.
	jmp DummyLop

ComGetPos:
	; zjisti n skok Ÿtec¡ hlavy pýed zapisovac¡ (voln‚ m¡sto)
	move R2,A
	move R1,X0
	sub X0,A #>DSPLenBuf,X0
	jge <_plus
		add X0,A
	_plus:
	_sndpos: jclr #1,X:<<HSR,_sndpos
	movep A,X:<<HTX
	jmp <main

ComPutData:
	; pýevezme data pro jeden blok zvuku
	_rcvnkan: jclr #0,X:<<HSR,_rcvnkan
	movep X:<<HRX,N7
	
	move R1,R0
	move M1,M0
	
	move #eff_a_sum,R3
	move #eff_b_sum,R4
	move #FBRamPomer-1,M3
	move M3,M4
	
	clr B
	do #FBRamPomer,_ClrLop
		move B,L:(R0)+
		move B,L:(R3)+
		move B,L:(R4)+
	_ClrLop:
	
	do N7,_KanLop
		_rcvbal: jclr #0,X:<<HSR,_rcvbal
		movep X:<<HRX,Y0
		
		move #0,N0
		
		; neprve fade (zhora)
		tfr Y0,B #>$ff0000,X1
		tst B
		jeq <_Nezni
		
		and X1,B1 #>1<<(23-6),X1
		
		; je to 7b, ale o 6b posunuto nahoru
		jeq <_NoFade
			move B1,X0
			mpy X1,X0,B ; m me rychlost fadeoutu rel. k celk. hlasitosti
			bset #0,N0
			move B1,X:FadeSpd
		_NoFade:
		
		tfr Y0,B #>$ff,X0
		and X0,B1 #>1<<(23-8),X1
		mpy X1,Y0,A B1,Y0
		and X0,A1 #>1<<(23-9),X1
		; Y0 a A1 obsahuj¡ 8b balance
		; posuneme-li je o 9b dol…, dostaneme v A0,B0 23 balance
		mpy Y0,X1,B A1,Y0
		mpy Y0,X1,A
		
		move A0,X1
		move B0,X0
		move X,L:KanVol
		
		jclr #0,N0,_nocfade
			move X:FadeSpd,Y0
			;move #0.01,Y0
			mpy Y0,X1,A
			mpy Y0,X0,B
			move AB,L:FadeSpd
		_nocfade:
		
		_rcveff: jclr #0,X:<<HSR,_rcveff
		movep X:<<HRX,Y0
		
		move #0,N1 ; zaznamen v me pou§it¡ effekt…
		
		tfr Y0,B #>$ff,X0
		and X0,B1 #>1<<(23-8),X1
		jeq <_NoEffB
			bset #1,N1
			bset #1,X:EffBMap
		_NoEffB:
		mpy X1,Y0,A B1,Y0
		and X0,A1 #>1<<(23-9),X1
		jeq <_NoEffA
			bset #0,N1
			bset #0,X:EffBMap
		_NoEffA:
		; Y0 a A1 obsahuj¡ 8b balance
		; posuneme-li je o 9b dol…, dostaneme v A0,B0 23b s¡lu efektu
		mpy Y0,X1,B A1,Y0
		mpy Y0,X1,A
		
		move A0,X:EffASila
		move B0,X:EffBSila
		
		move R1,R0
		do #FBRamPomer,_AddLop
			_rcvdata: jclr #0,X:<<HSR,_rcvdata
			movep X:<<HRX,X0
			
			;move #>(1<<12),X0
			
			move L:KanVol,Y ; balance (frac)
			mpy X0,Y1,A ; X0 je 16 bit… se znam‚nkem
			mpy X0,Y0,B
			
			move A,X1
			move B,X0
			
			; v X m me data pro zpracov n¡ pýes efekty
			
			; pýiŸ¡t me k hlavn¡mu bufferu
			move L:(R0),AB
			add X1,A
			add X0,B
			move AB,L:(R0)+
			
			jclr #0,N1,_NoSumA
				move X:EffASila,Y0
				move L:(R3),AB
				mac Y0,X1,A
				mac Y0,X0,B
				move AB,L:(R3)+
			_NoSumA:
			jclr #1,N1,_NoSumB
				move X:EffBSila,Y0
				move L:(R4),AB
				mac Y0,X1,A
				mac Y0,X0,B
				move AB,L:(R4)+
			_NoSumB:
			
			jclr #0,N0,_NoIFade
				move L:KanVol,AB
				move L:FadeSpd,X
				sub X1,A #0,Y1
				tlt Y1,A
				sub X0,B
				tlt Y1,B
				move AB,L:KanVol
			_NoIFade:
			nop
			
		_AddLop:
		_Nezni:
		nop
		
	_KanLop:
	
	; spoŸ¡tej efekty
	
	jclr #0,X:EffBMap,_NoATot
		move #eff_a_sum,R4
		move #eff_a_par,R5
		move R1,R0
		jsr <EffEcho
	_NoATot:
	
	jclr #1,X:EffBMap,_NoBTot
		move #eff_b_sum,R4
		move #eff_b_par,R5
		move R1,R0
		jsr <EffEcho
	_NoBTot:
	
	move R1,R0
	do #FBRamPomer,_SatLop
		move L:Volume,Y ; celkovì fade-out
		clr B Y1,A
		sub Y0,A L:(R0),X
		tlt B,A
		mpy X0,Y1,B A,X:Volume
		mpy X1,Y1,A
		rep #8
		asl A
		rep #8
		asl B
		move AB,L:(R0)+
	_SatLop:
	
	move R0,R1 ; pýejdi na dalç¡ £sek
	
	jmp <main
	
ComFadeOut:
	_WaitL: jclr #0,X:<<HSR,_WaitL
	movep X:<<HRX,Y:Volume
	jmp <main

ComVolume:
	_WaitL: jclr #0,X:<<HSR,_WaitL
	movep X:<<HRX,X:Volume
	jmp <main

ComEffPar:
	move #-1,M0
	move #EffParTab,R0
	_rcvnum: jclr #0,X:<<HSR,_rcvnum
	movep X:<<HRX,N0
	nop
	move X:(R0+N0),R0
	do #4,_parlop
		_rcvpar: jclr #0,X:<<HSR,_rcvpar
		movep X:<<HRX,X:(R0)+
	_parlop:
	; par. 0 nen¡ frac
	move (R0)-
	move (R0)-
	move (R0)-
	move (R0)-
	move X:(R0),X0
	move #>EffLenBuf-1,X1
	mpy X0,X1,A
	cmp X1,A
	tgt X1,A
	move A,X:(R0)
	jmp <main

; efekty:
; v R4 dost v  vstup
; v R5 dost v  parametry
; do R0 m  pýiŸ¡st vìstup

; X:(R5) je delay-1 (tj. pýipraven pro M5)
; Y:(R5) je aktu ln¡ posice
; X:(R5)+1 je feedback
; X:(R5)+2 je gain

EffEcho:
	; vezmi si parametry
	move X:(R5),M3
	move Y:(R5)+,R3
	move X:(R5)+,Y0 ; feedback
	move X:(R5)-,Y1 ; gain
	move (R5)-
	
	; proveÔ efekt z R4 do R0
	do #FBRamPomer,_echo_loop
	
		; vezmi si, co tebe zbylo z minula
		move L:(R3),X
		
		; pýidej k tomu feedbackem novì vzorek
		move L:(R4)+,AB
		mac X1,Y0,A
		mac X0,Y0,B
		move AB,L:(R3)+
		
		; minulì vzorek pýidej podle gain do vìstupn¡ho bufferu
		move L:(R0),AB
		mac X1,Y1,A
		mac X0,Y1,B
		move AB,L:(R0)+
		
	_echo_loop:
	
	move R3,Y:(R5)
	rts

EndProg equ *
	if @CVS(N,EndProg)>$200
		error "Program not in internal RAM"
	endif
