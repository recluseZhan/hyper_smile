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

uint8_t key[16] = {
    0x2b, 0x7e, 0x15, 0x16, 
    0x28, 0xae, 0xd2, 0xa6, 
    0xab, 0xf7, 0x32, 0x29, 
    0x1a, 0xc1, 0x30, 0x08
};
void aes_encrypt(uint8_t *input, uint8_t *output, uint8_t *key);
__attribute__((aligned(4096))) uint8_t data_page[4096];

void ecall_worker(void){
    int size = 0x1000;
    uint8_t *data = (uint8_t *)malloc(size);
    for(int j=0;j<11462;j++){
    for(int i=0;i<256;i++){
        aes_encrypt(data+i*16, data_page+i*16, key);
    }
    }
    printf("dump worker\n"); 
}

__asm__ (
    ".section .data\n"
    ".comm buf, 16, 16\n"
    
    ".section .text\n"
    
    ".global aes_encrypt\n"
    "aes_encrypt:\n"
    "    movdqu (%rdi), %xmm0\n"  // Load input into %xmm0
    "    movdqu (%rdx), %xmm5\n"  // Load key into %xmm5
    "    pxor   %xmm2, %xmm2\n"   // Clear %xmm2

    "    aeskeygenassist $1, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm6\n"
    "    aeskeygenassist $2, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm7\n"
    "    aeskeygenassist $4, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm8\n"
    "    aeskeygenassist $8, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm9\n"
    "    aeskeygenassist $16, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm10\n"
    "    aeskeygenassist $32, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm11\n"
    "    aeskeygenassist $64, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm12\n"
    "    aeskeygenassist $128, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm13\n"
    "    aeskeygenassist $27, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm14\n"
    "    aeskeygenassist $54, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm15\n"

    "encrypt:\n"
    "    pxor       %xmm5,  %xmm0\n"
    "    aesenc     %xmm6,  %xmm0\n"
    "    aesenc     %xmm7,  %xmm0\n"
    "    aesenc     %xmm8,  %xmm0\n"
    "    aesenc     %xmm9,  %xmm0\n"
    "    aesenc     %xmm10, %xmm0\n"
    "    aesenc     %xmm11, %xmm0\n"
    "    aesenc     %xmm12, %xmm0\n"
    "    aesenc     %xmm13, %xmm0\n"
    "    aesenc     %xmm14, %xmm0\n"
    "    aesenclast %xmm15, %xmm0\n"
    
    "    movdqu %xmm0, (%rsi)\n"  // Store output from %xmm0
    "    ret\n"

    "key_combine:\n"
    "    pshufd $0b11111111, %xmm1, %xmm1\n"
    "    shufps $0b00010000, %xmm0, %xmm2\n"
    "    pxor   %xmm2, %xmm0\n"
    "    shufps $0b10001100, %xmm0, %xmm2\n"
    "    pxor   %xmm2, %xmm0\n"
    "    pxor   %xmm1, %xmm0\n"
    "    ret\n"
);
