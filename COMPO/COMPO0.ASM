; COMPO.ASM
; spojovani kanalu + stereofonie pro Componium
; SUMA 1/1994-2/1994

	nolist
	include "intequ.asm"
	include "ioequ.asm"
	list
	
; CRA bits------------------------------------------------
; 15 = PS        = 0      : 0=/1, 1=/8, prescalar
; 14-13 = WL1-0  = 10     : 00=8, 01=12, 10=16, 11=24 bits (set to 16-bits now)
; 12-8 = DC4-DC0 = 0 0001 : divider control (normally word transfer rate)
; 7-0 = PM7-0    = $00    : divide by 1-256, 0=fastest (divide by 1)

CRA_BITS	EQU	$4700
; ve skut. 16 kanalu (data+stereo)

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

CRB_BITS	EQU	$F800

intracks equ 8
ECHO equ 0
STEREO_TIME equ 0
INTERPOL equ 0

	org l:$0

in_data: ds intracks*2
; strida se kanal/stereohlasitost
vol_lr: ds intracks*2

out_1: dc 0
	if INTERPOL
out_2: dc 0
ipol: dc 0
	endif
	
	if STEREO_TIME
buf_st_p: dc 0
	endif
	
sync: dc 0
SavePR: ds 1

; zpozdeni je umerne hlasitosti
; pro min. hlasitos je zpozdeni max.
; zpozdeni = (1.0 - hlasitost) * len_st

	if ECHO
len_eb equ 25000/20 ; delka ozveny - jakoby hall
echo_buf: dsm len_eb
	endif
	if STEREO_TIME
len_st equ 25000/2000 ; delka zpozdeni pro stereo - odpovida rozteci 60 cm
stereo_bufs: dsm len_st
sb_f:	dsm len_st
	dsm len_st
	dsm len_st
	dsm len_st
	dsm len_st
	dsm len_st
	dsm len_st
off_st: equ sb_f-stereo_bufs
	if intracks!=8
		error "# of tracks"
	endif
	endif
	
	org p:$0
	jmp start

	org p:SSIRD
	jsr datain
	org p:SSIRDE
	jsr datain
	
	org p:SSITD
	jsr dataout
	org p:SSITDE
	jsr dataout
 
	org p:$40

datain:
	jset #RFS,X:<<SSR,_din_sync
	movep X:<<RX,X:(r7)+
	rti
_din_sync:
	movep X:<<RX,X:(r7)+
	bset #0,X:<sync
	rti
	if INTERPOL

dataout:
	jset #0,X:<ipol,_druhy_vz
	jset #TFS,X:<<SSR,_dout_snc1
	movep X:out_1,X:<<TX
	rti
_dout_snc1:
	bset #0,X:<ipol
	movep Y:out_1,X:<<TX
	rti
_druhy_vz:
	jset #TFS,X:<<SSR,_dout_snc2
	movep X:out_2,X:<<TX
	rti
_dout_snc2:
	bclr #0,X:<ipol
	movep Y:out_2,X:<<TX
	rti

	else

dataout:
	jset #TFS,X:<<SSR,_dout_sync
	movep X:out_1,X:<<TX
	rti
_dout_sync:
	movep Y:out_1,X:<<TX
	rti

	endif
	
start:
;set port c to SSI + SCI
	movep #$1F8,X:<<PCC
	movep #CRA_BITS,X:<<CRA
; priprav datove oblasti
; cele prijimane bloky - vc. sterea
	move #0,X0
	move #0,X1
	if ECHO
		move #echo_buf,R6
		move #len_eb-1,M6
		rep #len_eb
		move X,L:(R6)+
	endif
	if STEREO_TIME
		move #stereo_bufs,R5
		move #$ffff,M5
		rep #off_st*intracks
		move X0,X:(R5)+
		move #stereo_bufs,R5
		move #len_st-1,M5
	endif
	move #in_data,R7
	move R7,R4
	move #intracks*2-1,M7
	move M7,M4
; priprav registry hlasitosti;
; napln pole
	move #vol_lr,R3
	move M4,M3
;read SSI SR to clear TUE bit
	btst  #4,X:<<SSR
	movep #CRB_BITS,X:<<CRB
	movep #>0,X:<<BCR
; interrupt priority
	movep X:<<IPR,X:SavePR
	movep #$3000,X:<<IPR
	movep #1,X:PBC
; scaling mode na 0, interrupt na 0
	andi  #<$F0,MR
main:
; tady muzes posilat maxima (na pozadani)
;cekame na dva bloky dat
	jset #0,X:<<HSR,Abort
	_rcvstereo: jclr #0,X:<sync,_rcvstereo
	bclr #0,X:<sync
	_rcvdata: jclr #0,X:<sync,_rcvdata
	bclr #0,X:<sync
; rozdel stereo na dve casti
	do #intracks,_rlloop
		clr A #$ff0000,Y0
		move X:(R4)+,A1 ; nacti stereo
		tfr A,B (R4)+ ; preskoc data
		and Y0,A1
		asr A #$00ff00,Y0
		and Y0,B1 A,X:(R3)+
		rep #7
		asl B
		move B,X:(R3)+
	_rlloop:

	if !STEREO_TIME
		clr A #0,B
		do #intracks,AddLoop
			move (R4)+
			move X:(R4)+,X0 ; nacti data
			move X:(R3)+,X1 ; X1 = hlasitost
			mac X0,X1,A X:(R3)+,X1
			mac X0,X1,B
		AddLoop:
	else
	;;	move M5,Y0 ; Y0 = (len_st-1)>>23,
		move #0,Y0
		move #>off_st,Y1
		move R5,X:<buf_st_p
		do #intracks,InLoop
	;		move #$ffff,M5
	;		move R5,B
	;		add Y1,B
	;		move B,R5
	;;	move #>off_st,N5
	;;	nop
	;;	move (R5)+N5
	;		move #len_st-1,M5
			
			move (R4)+ ; preskoc stereo
			move X:(R4)+,X0 ; nacti data
	;		move X0,X:(R5) ; uloz aktual. vzorek do prisl. kanalu
	
			move X:(R3),X1 ; X1 = hlasitost

	;		move #>1,B
	;		mac Y0,X1,B ; B=(len_st-1*hlasitost)>>23+1 - vypocti zpozdeni
	;		move B,N5
	;		nop
	;		move (R5)+N5
	;		move X:(R5),X0 ; bere se kruhem - skut. zpozdeni je len_st-B
	;		move (R5)-N5
			mpy X0,X1,B
			move B,Y:(R3)+ ; uloz vysledek L-kanalu
	
			move X:(R3),X1 ; X1 = hlasitost
	;		move #>1,B
	;		mac Y0,X1,B ; B je zpozdeni
	;		move B,N5
	;		nop
	;		move (R5)+N5
	;		move X:(R5),X0 ; bere se kruhem - skut. zpozdeni je len_st-B
	;		move (R5)-N5
			mpy X0,X1,B
			move B,Y:(R3)+ ; uloz vysledek P-kanalu
			
		InLoop:
	
		move X:<buf_st_p,R5 ; vrat R5
		nop
		move (R5)+
		
		clr A #0,B
		do #intracks,AddLoop
			move Y:(R3)+,X0
			add X0,A Y:(R3)+,X0
			add X0,B
		AddLoop:
		
	endif
	
	asr a ; pro zmenseni zkresleni radeji snizime hlasitost
	asr b
	if ECHO
		move AB,L:(R6)+ ; automaticky limituje
		move L:(R6),X ; nejvzdalenejsi
		move #len_eb/2,N6 ; priprav adresu dalsiho vzorku
		move #0.10,Y0
		mac X1,Y0,A
		mac X0,Y0,B
		move L:(R6+N6),X ; v polovine
		move #len_eb/4,N6 ; priprav adresu dalsiho vzorku
		move #0.35,Y0
		mac X1,Y0,A
		mac X0,Y0,B
		move L:(R6+N6),X ; ve ctvrtine
		move #0.45,Y0
		mac X1,Y0,A
		mac X0,Y0,B
	endif
	if INTERPOL
		move L:<out_2,X
		; priprav interpolaci 25 kHz na 50 kHz
		move AB,L:<out_2
		add X1,A
		asr A ; A = (A+X1)/2 - levy kanal
		add X0,B
		asr B ; B = (B+X0)/2 - pravy kanal
		move AB,L:<out_1
		bclr #0,X:<ipol ; urci poradi vysilani vzorku
	else
		move AB,L:<out_1
	endif
	jmp   <main
Abort:
	movep X:<<HRX,X0 ; prevezmi vyp.
	movep X:SavePR,X:<<IPR ; vrat IPR
	movep #0,X:<<CRB ; disable interr.
Lop: jmp Lop
EndProg equ *
	if @CVS(N,EndProg)>$200
		error "Program not in internal RAM"
	endif
