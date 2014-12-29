.set xres, 0
.set yres, 4
.set bits_per_pixel, 8
.set line_length, 12
.set screensize, 16

.text
.globl main
main:
    mov     r0, #448
    mov     r1, #256
    mov     r2, #16
    bl      InitFramebuffer
    fbAddr .req r8
    mov     fbAddr, r0

    bl GetFBInfo
    fbInfo .req r4
    mov     fbInfo, r0

    r_xres .req r5
    r_yres .req r6
    ldr     r_xres, [fbInfo, #xres]
    ldr     r_yres, [fbInfo, #yres]

    color .req r0
    x .req r1
    y .req r2
    mov     y, #0
    mov     color, #0xff
LdrawRows:
    @ Reset x for the inner loop
    mov     x, #0

    @ Set the base index to the beginning of the line
    @ using (y * line_length) + fbAddr
    index .req r7
    ldr     index, [fbInfo, #line_length]
    .unreq fbInfo
    mul     index, index, y
    add     index, index, fbAddr
    .unreq fbAddr

    LdrawPixels:
        @ Increment the index by 2 bytes (next pixel)
        strh    color, [index], #2
        add     x, x, #1
        teq     x, r_xres
        bne     LdrawPixels
        .unreq x
        .unreq r_xres
        .unreq index
        .unreq color

    add   y, y, #1
    teq     y, r_yres
    bne     LdrawRows
    .unreq y
    .unreq r_yres

    mov     r0, #5
    bl      sleep

    bl      CleanupFramebuffer

    mov     r0, #0
    mov     r7, #1
    svc     #0
