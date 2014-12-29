.section .data
fb0:
    .ascii "/dev/fb0\0"
fbFile:
    .int 0
.global fbAddr
fbAddr:
    .word 0

openMsg:
    .ascii "The framebuffer opened.\n"

.global frameBufferInfo
frameBufferInfo:
    .int 448 @ #0  xres
    .int 256 @ #4  yres
    .int 16  @ #8  bits_per_pixel
    .int 0   @ #12 line_length
    .int 0   @ #16 screensize
.set xres, 0
.set yres, 4
.set bits_per_pixel, 8
.set line_length, 12
.set screensize, 16

.section .text
.global GetFBAddr
GetFBAddr:
    ldr     r3, =fbAddr
    ldr     r0, [r3]
    mov     pc, lr

.global GetFBInfo
GetFBInfo:
    ldr     r0, =frameBufferInfo
    mov     pc, lr

.global InitFramebuffer
InitFramebuffer:
    @ Check the validity of the inputs
    cmp     r0, #4096
    cmple   r1, #4096
    cmple   r2, #32
    movhi   pc, lr

    @ Store the inputs in the struct
    ldr     r3, =frameBufferInfo
    str     r0, [r3, #xres]
    str     r1, [r3, #yres]
    str     r2, [r3, #bits_per_pixel]

    @ Call the function with the struct
    push    {lr}
    ldr     r0, =fbAddr
    ldr     r1, =frameBufferInfo
    bl      setup_fb
    @ Store the returned file handle
    ldr     r3, =fbFile
    str     r0, [r3]

    @ Return the address of the mmap'd framebuffer
    bl      GetFBAddr
    pop     {pc}

.global CleanupFramebuffer
CleanupFramebuffer:
    ldr     r3, =fbFile
    ldr     r0, [r3]
    ldr     r1, =fbAddr
    ldr     r3, =frameBufferInfo
    ldr     r2, [r3, #screensize]
    push {lr}
    bl      close_fb
    pop {pc}
