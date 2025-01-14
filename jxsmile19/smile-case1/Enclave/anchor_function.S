/*
 * Copyright (C) 2011-2019 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#define xax  rax
#define xbx  rbx
#define xcx  rcx
#define xdi  rdi
#define xsi  rsi

.text
.global anchor_function
//.global formal_reporter
.global add_random_code

/*
interrogation core (icore) run the attested reporter in enclave mode
monitor core (mcore) run the verify reporter and worker in smm

step:
1> icore run ssa copy
1> mcore run ssa check

2> icore run reporter page copy
2> mcore run reporter page check

3> icore run worker copy
3> mcore run worker check

4> icore run data copy?
4> mcore run data check?


use the sharememory to config the:
         1  flag; set the no-epc fisrt address for space
	     2  destination address %rdi
	     3  source  address   %rsi
	     4  counter   %rcx


mcore set and icore read
*/

anchor_function:   //for test


    lea 0x30ef48(%rip), %rsi  //for ssa
    //mov %rsi, 0x10(%rdi)      //for test
    mov %r10, %rcx
    rep movsd
    lea -0x12(%rip),%rsi       //for code
    mov %r10, %rcx
    rep movsd
    //mov %rsi, 0x18(%rdi)    //for test

//loop:
//    lock xadd %rcx, (%r9)       //for worker
//    jp worker_shell        //SMM set 512*8, SMM set 0, SMM set 1 for pf = 0,
//    rep movsd
//    jmp loop

add_random_code:                        //fill 4kb page, following instruction can added to random code,
mov %rax, %rbx
mov %rdx, %rcx
mov %rcx, %rdx
mov %rdx, %rax
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
retq

    
worker_shell:
    push   %rbp
    mov    %rsp,%rbp
    sub    $0x10,%rsp
    mov    %r11, -0x8(%rbp)
//    mov    %r9, -0x10(%rbp)  
    lea 0x8(%r9),%rdi
    callq worker
//    call do_ereport_define1
    mov   -0x8(%rbp), %r11
    leaveq
eexit:
  //  mov $0x1, (%r9)
    mov %r11, %rbx
    mov %r11, %rcx
    mov $0x4, %rax
    enclu
    retq


do_ereport_define1:
	push   %rbx
	push   %rcx
	push   %rdx
	mov    $0, %rbx
	mov    $0, %rcx
        lea 0x39b510(%rip), %r8
	mov    %r8, %rdx
	mov    $0, %rax
	enclu
	pop    %rdx
	pop    %rcx
	pop    %rbx
	retq


/*
	in this version, we goes to set a fixed address for input

	reporter: rdi = P_0, rcx=4096, %r9, are passed from the outside, reporter and wowrker are adjacent

	//the ssa_offset(%rip) is the base address of ssa frame
	//the reporter_offset(%rip) is the base address of reporter page
*/
/*
formal_reporter
        lea ssa_offset(%rip), %rsi
	rep movsd
	mov $0x1000, %rcx
	lea reporter_offset(%rip), %rsi
        rep movsd
loop:
	xadd %rcx, (%r9)
	jnp worker        //SMM set 512*8, SMM set 0, SMM set 1 for pf = 0,
	rep movsd
	jmp loop

*/






