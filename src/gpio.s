ldr r0,=0x20200000

mov r1,#1
lsl r1,#12
str r1,[r0]
mov r1,#1
lsl r1,#4

mov r3,#1
lsl r3,#21
str r3,[r0,#4]
mov r3,#1
lsl r3,#17

mov r4,#1
lsl r4,#21
str r4,[r0,#8]
mov r4,#1
lsl r4,#27

mov r5,#0xa00000
loop:
str r1,[r0,#40]
str r3,[r0,#40]
str r4,[r0,#40]
mov r2,r5
wait1:
sub r2,r2,#1
cmp r2,#0
bne wait1

str r1,[r0,#28]
mov r2,r5
wait2:
sub r2,r2,#1
cmp r2,#0
bne wait2

str r1,[r0,#40]
str r3,[r0,#40]
str r4,[r0,#40]
mov r2,r5
wait3:
sub r2,r2,#1
cmp r2,#0
bne wait3

str r3,[r0,#28]
mov r2,r5
wait4:
sub r2,r2,#1
cmp r2,#0
bne wait4

str r1,[r0,#40]
str r3,[r0,#40]
str r4,[r0,#40]
mov r2,r5
wait5:
sub r2,r2,#1
cmp r2,#0
bne wait5

str r1,[r0,#28]
str r3,[r0,#28]
mov r2,r5
wait6:
sub r2,r2,#1
cmp r2,#0
bne wait6

str r1,[r0,#40]
str r3,[r0,#40]
str r4,[r0,#40]
mov r2,r5
wait7:
sub r2,r2,#1
cmp r2,#0
bne wait7

str r4,[r0,#28]
mov r2,r5
wait8:
sub r2,r2,#1
cmp r2,#0
bne wait8

str r1,[r0,#40]
str r3,[r0,#40]
str r4,[r0,#40]
mov r2,r5
wait9:
sub r2,r2,#1
cmp r2,#0
bne wait9

str r1,[r0,#28]
str r4,[r0,#28]
mov r2,r5
wait10:
sub r2,r2,#1
cmp r2,#0
bne wait10

str r1,[r0,#40]
str r3,[r0,#40]
str r4,[r0,#40]
mov r2,r5
wait11:
sub r2,r2,#1
cmp r2,#0
bne wait11

str r3,[r0,#28]
str r4,[r0,#28]
mov r2,r5
wait12:
sub r2,r2,#1
cmp r2,#0
bne wait12

str r1,[r0,#40]
str r3,[r0,#40]
str r4,[r0,#40]
mov r2,r5
wait13:
sub r2,r2,#1
cmp r2,#0
bne wait13

str r1,[r0,#28]
str r3,[r0,#28]
str r4,[r0,#28]
mov r2,r5
wait14:
sub r2,r2,#1
cmp r2,#0
bne wait14

cmp r5,#0x100000
be loop
sub r5,r5,#0x100000

b loop
