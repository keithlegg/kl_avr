	.file	"blink.c"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__RAMPZ__ = 0x3b
__tmp_reg__ = 0
__zero_reg__ = 1
.global	__mulsf3
.global	__ltsf2
.global	__gtsf2
.global	__fixunssfsi
	.text
.global	main
	.type	main, @function
main:
	push r28
	push r29
	in r28,__SP_L__
	in r29,__SP_H__
	sbiw r28,14
	in __tmp_reg__,__SREG__
	cli
	out __SP_H__,r29
	out __SREG__,__tmp_reg__
	out __SP_L__,r28
/* prologue: function */
/* frame size = 14 */
/* stack size = 16 */
.L__stack_usage = 16
	ldi r24,lo8(1)
	ldi r25,lo8(1)
	ldi r18,lo8(-1)
	movw r30,r24
	st Z,r18
.L10:
	ldi r24,lo8(2)
	ldi r25,lo8(1)
	ldi r18,lo8(2)
	ldi r19,lo8(1)
	movw r30,r18
	ld r19,Z
	ldi r18,lo8(1)
	eor r18,r19
	movw r30,r24
	st Z,r18
	ldi r24,0
	ldi r25,lo8(64)
	ldi r26,lo8(28)
	ldi r27,lo8(70)
	std Y+1,r24
	std Y+2,r25
	std Y+3,r26
	std Y+4,r27
	ldi r18,0
	ldi r19,0
	ldi r20,lo8(122)
	ldi r21,lo8(69)
	ldd r22,Y+1
	ldd r23,Y+2
	ldd r24,Y+3
	ldd r25,Y+4
	call __mulsf3
	movw r26,r24
	movw r24,r22
	std Y+5,r24
	std Y+6,r25
	std Y+7,r26
	std Y+8,r27
	ldi r18,0
	ldi r19,0
	ldi r20,lo8(-128)
	ldi r21,lo8(63)
	ldd r22,Y+5
	ldd r23,Y+6
	ldd r24,Y+7
	ldd r25,Y+8
	call __ltsf2
	tst r24
	brge .L13
	ldi r24,lo8(1)
	ldi r25,0
	std Y+10,r25
	std Y+9,r24
	rjmp .L4
.L13:
	ldi r18,0
	ldi r19,lo8(-1)
	ldi r20,lo8(127)
	ldi r21,lo8(71)
	ldd r22,Y+5
	ldd r23,Y+6
	ldd r24,Y+7
	ldd r25,Y+8
	call __gtsf2
	cp __zero_reg__,r24
	brge .L14
	ldi r18,0
	ldi r19,0
	ldi r20,lo8(32)
	ldi r21,lo8(65)
	ldd r22,Y+1
	ldd r23,Y+2
	ldd r24,Y+3
	ldd r25,Y+4
	call __mulsf3
	movw r26,r24
	movw r24,r22
	movw r22,r24
	movw r24,r26
	call __fixunssfsi
	movw r26,r24
	movw r24,r22
	std Y+10,r25
	std Y+9,r24
	rjmp .L7
.L8:
	ldi r24,lo8(-112)
	ldi r25,lo8(1)
	std Y+12,r25
	std Y+11,r24
	ldd r24,Y+11
	ldd r25,Y+12
/* #APP */
 ;  110 "/usr/lib/avr/include/util/delay_basic.h" 1
	1: sbiw r24,1
	brne 1b
 ;  0 "" 2
/* #NOAPP */
	std Y+12,r25
	std Y+11,r24
	ldd r24,Y+9
	ldd r25,Y+10
	sbiw r24,1
	std Y+10,r25
	std Y+9,r24
.L7:
	ldd r24,Y+9
	ldd r25,Y+10
	sbiw r24,0
	brne .L8
	rjmp .L10
.L14:
	ldd r22,Y+5
	ldd r23,Y+6
	ldd r24,Y+7
	ldd r25,Y+8
	call __fixunssfsi
	movw r26,r24
	movw r24,r22
	std Y+10,r25
	std Y+9,r24
.L4:
	ldd r24,Y+9
	ldd r25,Y+10
	std Y+14,r25
	std Y+13,r24
	ldd r24,Y+13
	ldd r25,Y+14
/* #APP */
 ;  110 "/usr/lib/avr/include/util/delay_basic.h" 1
	1: sbiw r24,1
	brne 1b
 ;  0 "" 2
/* #NOAPP */
	std Y+14,r25
	std Y+13,r24
	rjmp .L10
	.size	main, .-main
	.ident	"GCC: (GNU) 4.8.2"
