
/*
 * func.s
 *
 * 
 *  Author: Valek
 */ 

 ;Экспортируем asm_func
.global asm_add
.extern summa
asm_add:
;аргумент   r24 AH r22 AL
ldi r30,lo8(summa) ;загружаем в Z адресс переменной
ldi r31,hi8(summa)

ld r16,Z+ ; извлекаем переменную
ld r17,Z+
ld r18,Z+
ld r19,Z
;прибавляю
clr r20
add r16,r22 ;+AL
adc r17,r24 ;+AH;
adc r18,r20 ;+0
adc r19,r20 ;+0
ldi r30,lo8(summa) ;загружаем в Z адресс переменной
ldi r31,hi8(summa)

;сохраняем переменную
st Z+,r16
st Z+,r17
st Z+,r18
st Z ,r19

ret


;деление на 64
.global asm_div
.extern summa
asm_div:
;аргумент uint32  r25:r24:r23:r22
;r18 r27
ldi r30,lo8(summa)
ldi r31,hi8(summa)

ld r22,Z+
ld r23,Z+
ld r24,Z+
ld r25,Z

ldi r18,6 ;6 сдвигов  2^6=64
 div:			
	clc ;cбрасываю флаг переноса
    ror r25  ;сдвигаем значение вправо сначала старший \n\t"
	ror r24  ;потом младший через флаг переноса \n\t"
	ror r23
	ror r22
	dec r18;-1 cчетчик 
	brne div  ;проверка условия
	movw r24,r22;копирую слово		 

ret

//возвращает значение напряжения
.global asm_ui
asm_ui:

; входные данные r25:r24  r23:r22
;       C=          A    *   B 
;  r11:r10:r9  

clr r11    ;очищаем на всякий случай
clr r10    ;
mov r8,r22 ; копируем младший BL, в Rr
mul  r24,r8 ;умножаю младшие регистры AL*BL
mov r9,r0  ;из r0 младший результат операции mul в CL0
mov r10,r1 ;из r1 старший результата операции mul в СH0

mul r25,r8 ; AH*BL  старший А на младший B
add r10,r0 ;CH0+ L  
adc r11,r1 ;CL1+H 

;mul dataL,KoeffH ;умножаем младший на старший
mov r8,r23 ; BH to Rr
mul r24,r8 ; AL*BH
add r10,r0 ;CH0+L
adc r11,r1 ;CH1+H

mul r25,r8 ;AH*BH
add r11,r0 ;CL1+L
;add r12,r1 ;4-й разряд нам тут не требуется

;деление на 128
;перемещаем в регистры
mov r26,r11
mov r25,r10
mov r24,r9

;деление сдвигом на 128
ldi r18,7 ;7 сдвигов  2^7=128
 div128:			
	clc ;cбрасываю флаг переноса
    ror r26  ;сдвигаем значение вправо сначала старший
	ror r25  ;потом младший через флаг переноса 
	ror r24
	dec r18;-1 cчетчик 
	brne div128  ;проверка условия
	
;значение возвращается в регистрах
;r25:r24
ret

;прибавляю к С60(uint32_t) число uint16_t
.global asm_addA
.extern C60
asm_addA:
; C  =  C(uint32_t)         +  B(uint16_t) 
;                             r25:r24
;адрес С60
ldi r30,lo8(C60) ;загружаем в Z адресс переменной
ldi r31,hi8(C60)

ld r18,Z+ ; извлекаем переменную
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

;прибавляю
clr r1
add r18,r24 ;+AL
adc r19,r25 ;+AH;
adc r20,r1 ;+0
adc r21,r1 ;+0

ldi r30,lo8(C60) ;загружаем в Z адресс переменной
ldi r31,hi8(C60)

//2BF20

;ldi r18,0x20
;ldi r19,0xBF
;ldi r20,0x02
;ldi r21,0x00


;сохраняем переменную
st Z+,r18
st Z+,r19
st Z+,r20
st Z ,r21

ret
//***********************************************
//деление uint32_t на uint16_t 
//взято из дезасеблированного хекса
.global asm_divB
.extern C60
asm_divB:
   ;адрес С60
     ldi r30,lo8(C60) ;загружаем в Z адресс переменной
     ldi r31,hi8(C60)

     ld r22,Z+ ;извлекаем делимое из памяти
     ld r23,Z+ ;сохраняем в регистры
     ld r24,Z+
     ld r25,Z
	           ;делитель у нас константа,    
	
	           ;делитель
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
	movw	r18,r22 ;r18 младший бит
	;movw	r20,r24
	;movw	r22,r26
	;movw	r24,r30
	;sts	D0148,r18
	
	mov r24,r18
ret

