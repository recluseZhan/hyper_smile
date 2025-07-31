/*
 * Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
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

#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
int printf(const char* fmt, ...)
{
    char buf[BUFSIZ] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return (int)strnlen(buf, BUFSIZ - 1) + 1;
}
static unsigned char key[16] = {                                                    0x2b,0x7e,0x15,0x16, 0x28,0xae,0xd2,0xa6,
    0xab,0xf7,0x32,0x29, 0x1a,0xc1,0x30,0x08
};
static unsigned char input[4096] = {0x1,0x2,0x4,0x6,0x8,0x9};

extern "C" unsigned long long aes_encrypt(unsigned char *input,
                                          unsigned char *output,
                                          unsigned char *key);
void init_input(){
    memset(input,0xac,4096);
}
void attack(uint8_t *buf){
    memset(buf,0xdead,4096);
}
void worker(uint8_t *buf){
    //attack
    //memset(input,0xdead,4096);
    //
    for(int i=0;i<256;i++){
        aes_encrypt(input+16, buf+16, key);
    }
}
extern void worker_end(void);
__asm__(".global worker_end\n"
        "worker_end:\n");

asm volatile (
R"ASM(
    .text
    .global aes_encrypt
    .type   aes_encrypt,@function
aes_encrypt:
    movdqu  (%rdi), %xmm0
    movdqu  (%rdx), %xmm5
    pxor    %xmm2, %xmm2

    aeskeygenassist $1,  %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm6

    aeskeygenassist $2,  %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm7

    aeskeygenassist $4,  %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm8

    aeskeygenassist $8,  %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm9

    aeskeygenassist $16, %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm10

    aeskeygenassist $32, %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm11

    aeskeygenassist $64, %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm12

    aeskeygenassist $128, %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm13

    aeskeygenassist $27, %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm14

    aeskeygenassist $54, %xmm0, %xmm1
    call    key_combine
    movaps  %xmm0, %xmm15

encrypt:
    pxor       %xmm5,  %xmm0
    aesenc     %xmm6,  %xmm0
    aesenc     %xmm7,  %xmm0
    aesenc     %xmm8,  %xmm0
    aesenc     %xmm9,  %xmm0
    aesenc     %xmm10, %xmm0
    aesenc     %xmm11, %xmm0
    aesenc     %xmm12, %xmm0
    aesenc     %xmm13, %xmm0
    aesenc     %xmm14, %xmm0
    aesenclast %xmm15, %xmm0
    movdqu %xmm0, (%rsi)
    ret

    .global key_combine
    .type   key_combine,@function
key_combine:
    pshufd  $0xff, %xmm1, %xmm1
    shufps  $0x10, %xmm0, %xmm2
    pxor    %xmm2, %xmm0
    shufps  $0x8c, %xmm0, %xmm2
    pxor    %xmm2, %xmm0
    pxor    %xmm1, %xmm0
    ret
)ASM"
);

