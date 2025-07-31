.text
.global anchor_func
.global worker
#.section .anchor_page, "ax"        # executable, allocatable
#.p2align 12, 0x90                 # align to 2^12 = 4096 bytes, pad with 0x90 (NOP)
#.global anchor
#.type   anchor, @function
anchor_func:
        .fill 0x1000-42,1,0x90
     anchor:
        mov   %r10, %rcx
        lea   0x39B500(%rip), %rsi
        lea   0x400000(%rsi), %rdi
        rep   movsd

        mov   %r10, %rcx
        lea   -0x1d(%rip), %rsi
        rep   movsd
    loop:
        lock  xadd  %rcx, (%r9)
        jnp   worker
        rep   movsd
        jmp   loop
    #add_random_code:
    #    .fill 0x1000-46,1,0x90
#.section .worker_page, "ax"
worker:
