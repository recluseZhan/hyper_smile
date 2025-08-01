.text
.global anchor_func
.global worker
.global attacker
anchor_func:
        .fill 0x1000-42,1,0x90
     anchor:
        mov   %r10, %rcx
        lea   0x39B500(%rip), %rsi
        lea   0x400000(%rsi), %rdi
        ## attack 1
        #lea   -0x17B500(%rsi), %rdi # data_offset 0x220000
        rep   movsd

        mov   %r10, %rcx
        lea   -0x1d(%rip), %rsi
        rep   movsd
    loop:
        lock  xadd  %rcx, (%r9)
        #jnp   worker
        ## attack 2
        jnp   attacker
        rep   movsd
        jmp   loop
worker:
        .fill 0x4a,1,0x90
attacker:
        mov %rax,%rax
