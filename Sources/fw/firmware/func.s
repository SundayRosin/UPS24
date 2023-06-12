
/*
 * func.s
 *
 * 
 *  Author: Valek
 */ 

 ;������������ asm_func
.global asm_add
.extern summa
asm_add:
;��������   r24 AH r22 AL
ldi r30,lo8(summa) ;��������� � Z ������ ����������
ldi r31,hi8(summa)

ld r16,Z+ ; ��������� ����������
ld r17,Z+
ld r18,Z+
ld r19,Z
;���������
clr r20
add r16,r22 ;+AL
adc r17,r24 ;+AH;
adc r18,r20 ;+0
adc r19,r20 ;+0
ldi r30,lo8(summa) ;��������� � Z ������ ����������
ldi r31,hi8(summa)

;��������� ����������
st Z+,r16
st Z+,r17
st Z+,r18
st Z ,r19

ret


;������� �� 64
.global asm_div
.extern summa
asm_div:
;�������� uint32  r25:r24:r23:r22
;r18 r27
ldi r30,lo8(summa)
ldi r31,hi8(summa)

ld r22,Z+
ld r23,Z+
ld r24,Z+
ld r25,Z

ldi r18,6 ;6 �������  2^6=64
 div:			
	clc ;c�������� ���� ��������
    ror r25  ;�������� �������� ������ ������� ������� \n\t"
	ror r24  ;����� ������� ����� ���� �������� \n\t"
	ror r23
	ror r22
	dec r18;-1 c������ 
	brne div  ;�������� �������
	movw r24,r22;������� �����		 

ret

//���������� �������� ����������
.global asm_ui
asm_ui:

; ������� ������ r25:r24  r23:r22
;       C=          A    *   B 
;  r11:r10:r9  

clr r11    ;������� �� ������ ������
clr r10    ;
mov r8,r22 ; �������� ������� BL, � Rr
mul  r24,r8 ;������� ������� �������� AL*BL
mov r9,r0  ;�� r0 ������� ��������� �������� mul � CL0
mov r10,r1 ;�� r1 ������� ���������� �������� mul � �H0

mul r25,r8 ; AH*BL  ������� � �� ������� B
add r10,r0 ;CH0+ L  
adc r11,r1 ;CL1+H 

;mul dataL,KoeffH ;�������� ������� �� �������
mov r8,r23 ; BH to Rr
mul r24,r8 ; AL*BH
add r10,r0 ;CH0+L
adc r11,r1 ;CH1+H

mul r25,r8 ;AH*BH
add r11,r0 ;CL1+L
;add r12,r1 ;4-� ������ ��� ��� �� ���������

;������� �� 128
;���������� � ��������
mov r26,r11
mov r25,r10
mov r24,r9

;������� ������� �� 128
ldi r18,7 ;7 �������  2^7=128
 div128:			
	clc ;c�������� ���� ��������
    ror r26  ;�������� �������� ������ ������� �������
	ror r25  ;����� ������� ����� ���� �������� 
	ror r24
	dec r18;-1 c������ 
	brne div128  ;�������� �������
	
;�������� ������������ � ���������
;r25:r24
ret

;��������� � �60(uint32_t) ����� uint16_t
.global asm_addA
.extern C60
asm_addA:
; C  =  C(uint32_t)         +  B(uint16_t) 
;                             r25:r24
;����� �60
ldi r30,lo8(C60) ;��������� � Z ������ ����������
ldi r31,hi8(C60)

ld r18,Z+ ; ��������� ����������
ld r19,Z+
ld r20,Z+
ld r21,Z

//bb8
;ldi r24,0xda
;ldi r25,0x0c

//2B368
;ldi r18,0x68
;ldi r19,0xB3
;ldi r20,0x02
;ldi r21,0x00

;���������
clr r1
add r18,r24 ;+AL
adc r19,r25 ;+AH;
adc r20,r1 ;+0
adc r21,r1 ;+0

ldi r30,lo8(C60) ;��������� � Z ������ ����������
ldi r31,hi8(C60)

//2BF20

;ldi r18,0x20
;ldi r19,0xBF
;ldi r20,0x02
;ldi r21,0x00


;��������� ����������
st Z+,r18
st Z+,r19
st Z+,r20
st Z ,r21

ret
//***********************************************
//������� uint32_t �� uint16_t 
//����� �� ������������������ �����
.global asm_divB
.extern C60
asm_divB:
   ;����� �60
     ldi r30,lo8(C60) ;��������� � Z ������ ����������
     ldi r31,hi8(C60)

     ld r22,Z+ ;��������� ������� �� ������
     ld r23,Z+ ;��������� � ��������
     ld r24,Z+
     ld r25,Z
	           ;�������� � ��� ���������,    
	
	           ;��������
	ldi	r18,0x70  ; 0x70   6000
	ldi	r19,0x17  ; 0x17	
	ldi	r20,0
	ldi	r21,0
	; rcall	L058E
	;----------------------*
    ; pc=0x58E(0xB1C)
    ;
    ;  L058E:
	ldi	r26,0x21  ;k21 ; .equ	k21	= 0x21; 33d
	mov	r1,r26
	sub	r26,r26
	sub	r27,r27
	movw	r30,r26
	rjmp	L05A1
    ;	-----------	jump on last line
    L0594:
	rol	r26
	rol	r27
	rol	r30
	rol	r31
	cp	r26,r18
	cpc	r27,r19
	cpc	r30,r20
	cpc	r31,r21
    brcs	L05A1
;	-----		branch on last line
	sub	r26,r18
	sbc	r27,r19
	sbc	r30,r20
	sbc	r31,r21
L05A1:
	rol	r22
	rol	r23
	rol	r24
	rol	r25
	dec	r1
	brne	L0594
;	-----		branch on last line
	com	r22
	com	r23
	com	r24
	com	r25
	movw	r18,r22 ;r18 ������� ���
	;movw	r20,r24
	;movw	r22,r26
	;movw	r24,r30
	;sts	D0148,r18
	
	mov r24,r18
ret

