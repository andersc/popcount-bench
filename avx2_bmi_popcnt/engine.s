//
//  avx2_bmi2_popcnt bench
//

.globl _popcnt_bmi2 , _popcnt_avx2, _popcnt_popcnt
.align 2

//64-bit popcnt using the built-in popcnt instruction
_popcnt_popcnt:
        popcntq     (%rdi), %r11
        mov         %r11,(%rdx)
        add         $8h,%rdi
        add         $8h,%rdx
        dec         %rsi
        jnz         _popcnt_popcnt
        ret

//64-bit popcnt using BMI2
_popcnt_bmi2:
        mov         (%rdi),%r11
        pextq       %r11,%r11,%r11
        not         %r11
        tzcnt       %r11,%r11
        mov         %r11,(%rdx)
        add         $8h,%rdi
        add         $8h,%rdx
        dec         %rsi
        jnz         _popcnt_bmi2
        ret

//64-bit popcnt using AVX2
_popcnt_avx2:
        vmovdqa     (%rcx),%ymm2
        add         $20h,%rcx
        vmovdqa     (%rcx),%ymm3
        add         $20h,%rcx
        vmovdqa     (%rcx),%ymm4
popcnt_avx2_loop:
        vmovdqa     (%rdi),%ymm0
        vpand       %ymm0, %ymm2, %ymm1
        vpandn      %ymm0, %ymm2, %ymm0
        vpsrld      $4h,%ymm0, %ymm0
        vpshufb     %ymm1, %ymm3, %ymm1
        vpshufb     %ymm0, %ymm3, %ymm0
        vpaddb      %ymm1,%ymm0,%ymm0       //popcnt (8-bits)
        vpsadbw     %ymm0,%ymm4,%ymm0       //popcnt (64-bits)
        vmovdqa     %ymm0,(%rdx)
        add         $20h,%rdi
        add         $20h,%rdx
        dec         %rsi
        jnz         popcnt_avx2_loop
        ret


