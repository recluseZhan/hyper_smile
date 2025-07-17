#include <linux/linkage.h>
#include <asm/ibt.h>
    .text
    .align 16
SYM_FUNC_START(aes_gcm_encrypt)
    movdqu  j1(%rip), %xmm0        
    pxor    key0(%rip), %xmm0      
    aesenc  key1(%rip), %xmm0      
    aesenc  key2(%rip), %xmm0      
    aesenc  key3(%rip), %xmm0      
    aesenc  key4(%rip), %xmm0      
    aesenc  key5(%rip), %xmm0      
    aesenc  key6(%rip), %xmm0      
    aesenc  key7(%rip), %xmm0      
    aesenc  key8(%rip), %xmm0      
    aesenc  key9(%rip), %xmm0      
    aesenclast key10(%rip), %xmm0  
    # XMM0 ^ plaintext -> ciphertext
    movdqu  (%rsi), %xmm1         
    pxor    %xmm0, %xmm1         
    movdqa  %xmm1, %xmm10        
    # H = E(K, 0^128)
    movdqu  zero(%rip), %xmm2    
    pxor    key0(%rip), %xmm2    
    aesenc  key1(%rip), %xmm2
    aesenc  key2(%rip), %xmm2
    aesenc  key3(%rip), %xmm2
    aesenc  key4(%rip), %xmm2
    aesenc  key5(%rip), %xmm2
    aesenc  key6(%rip), %xmm2
    aesenc  key7(%rip), %xmm2
    aesenc  key8(%rip), %xmm2
    aesenc  key9(%rip), %xmm2
    aesenclast key10(%rip), %xmm2 # XMM2 = H
    # EJ0 = E(K, J0)
    movdqu  j0(%rip), %xmm3     # XMM3 = J0 (IV||0x00000001)
    pxor    key0(%rip), %xmm3   
    aesenc  key1(%rip), %xmm3
    aesenc  key2(%rip), %xmm3
    aesenc  key3(%rip), %xmm3
    aesenc  key4(%rip), %xmm3
    aesenc  key5(%rip), %xmm3
    aesenc  key6(%rip), %xmm3
    aesenc  key7(%rip), %xmm3
    aesenc  key8(%rip), %xmm3
    aesenc  key9(%rip), %xmm3
    aesenclast key10(%rip), %xmm3 # XMM3 = EJ0
    # X1 = GHASH(H, C)
    movdqa  %xmm2, %xmm0        # XMM0 = H
    movdqa  %xmm10, %xmm1       
    
    movdqa %xmm0, %xmm5
    pclmulqdq $0x00, %xmm1, %xmm5   # a0*b0
    movdqa %xmm0, %xmm6
    pclmulqdq $0x11, %xmm1, %xmm6   # a1*b1
    movdqa %xmm0, %xmm7
    pclmulqdq $0x10, %xmm1, %xmm7   # a0*b1
    movdqa %xmm0, %xmm8
    pclmulqdq $0x01, %xmm1, %xmm8   # a1*b0
    pxor %xmm8, %xmm7             
    movdqa %xmm7, %xmm8
    psrldq $8, %xmm7
    pslldq $8, %xmm8
    pxor %xmm8, %xmm5
    pxor %xmm7, %xmm6             # <XMM6:XMM5>
    
    movdqa %xmm5, %xmm7
    movdqa %xmm6, %xmm8
    pslld $1, %xmm5
    pslld $1, %xmm6
    psrld $31, %xmm7
    psrld $31, %xmm8
    movdqa %xmm7, %xmm9
    pslldq $4, %xmm8
    pslldq $4, %xmm7
    psrldq $12, %xmm9
    por %xmm7, %xmm5
    por %xmm8, %xmm6
    por %xmm9, %xmm6
    
    movdqa %xmm5, %xmm7
    movdqa %xmm5, %xmm8
    movdqa %xmm5, %xmm9
    pslld $31, %xmm7
    pslld $30, %xmm8
    pslld $25, %xmm9
    pxor %xmm8, %xmm7
    pxor %xmm9, %xmm7
    movdqa %xmm7, %xmm8
    psrldq $4, %xmm8
    pslldq $12, %xmm7
    pxor %xmm7, %xmm5
    
    movdqa %xmm5, %xmm2
    movdqa %xmm5, %xmm4
    movdqa %xmm5, %xmm1
    psrld $1, %xmm2
    psrld $2, %xmm4
    psrld $7, %xmm1
    pxor %xmm4, %xmm2
    pxor %xmm1, %xmm2
    pxor %xmm8, %xmm2
    pxor %xmm2, %xmm5
    pxor %xmm5, %xmm6           # XMM6 = X1
    # X2 = (X1 ^ L) * H
    movdqu  len(%rip), %xmm2   
    pxor    %xmm6, %xmm2       
    movdqa  %xmm0, %xmm0       # XMM0 = H
    movdqa  %xmm2, %xmm1       # XMM1 = X1^L
    
    movdqa %xmm0, %xmm5
    pclmulqdq $0x00, %xmm1, %xmm5
    movdqa %xmm0, %xmm6
    pclmulqdq $0x11, %xmm1, %xmm6
    movdqa %xmm0, %xmm7
    pclmulqdq $0x10, %xmm1, %xmm7
    movdqa %xmm0, %xmm8
    pclmulqdq $0x01, %xmm1, %xmm8
    pxor %xmm8, %xmm7
    movdqa %xmm7, %xmm8
    psrldq $8, %xmm7
    pslldq $8, %xmm8
    pxor %xmm8, %xmm5
    pxor %xmm7, %xmm6
    
    movdqa %xmm5, %xmm7
    movdqa %xmm6, %xmm8
    pslld $1, %xmm5
    pslld $1, %xmm6
    psrld $31, %xmm7
    psrld $31, %xmm8
    movdqa %xmm7, %xmm9
    pslldq $4, %xmm8
    pslldq $4, %xmm7
    psrldq $12, %xmm9
    por %xmm7, %xmm5
    por %xmm8, %xmm6
    por %xmm9, %xmm6
    movdqa %xmm5, %xmm7
    movdqa %xmm5, %xmm8
    movdqa %xmm5, %xmm9
    pslld $31, %xmm7
    pslld $30, %xmm8
    pslld $25, %xmm9
    pxor %xmm8, %xmm7
    pxor %xmm9, %xmm7
    movdqa %xmm7, %xmm8
    psrldq $4, %xmm8
    pslldq $12, %xmm7
    pxor %xmm7, %xmm5
    movdqa %xmm5, %xmm1
    movdqa %xmm5, %xmm2
    movdqa %xmm5, %xmm4
    psrld $1, %xmm1
    psrld $2, %xmm2
    psrld $7, %xmm4
    pxor %xmm2, %xmm1
    pxor %xmm4, %xmm1
    pxor %xmm6, %xmm1
    pxor %xmm1, %xmm5
    pxor %xmm5, %xmm6   # XMM6 = X2
    # Tag = EJ0 ^ X2
    pxor    %xmm6, %xmm3 # XMM3 = Tag
    
    movdqu  %xmm10, (%rdi)       
    movdqu  %xmm3, 16(%rdi)     
    RET
SYM_FUNC_END(aes_gcm_encrypt)

.section .rodata
    .align 16
key0:
    .byte 0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff
key1:
    .byte 0xc0,0x39,0x34,0x78,0x84,0x6c,0x52,0x0f,0x0c,0xf5,0xf8,0xb4,0xc0,0x28,0x16,0x4b
key2:
    .byte 0xf6,0x7e,0x87,0xc2,0x72,0x12,0xd5,0xcd,0x7e,0xe7,0x2d,0x79,0xbe,0xcf,0x3b,0x32
key3:
    .byte 0x78,0x9c,0xa4,0x6c,0x0a,0x8e,0x71,0xa1,0x74,0x69,0x5c,0xd8,0xca,0xa6,0x67,0xea
key4:
    .byte 0x54,0x19,0x23,0x18,0x5e,0x97,0x52,0xb9,0x2a,0xfe,0x0e,0x61,0xe0,0x58,0x69,0x8b
key5:
    .byte 0x2e,0xe0,0x1e,0xf9,0x70,0x77,0x4c,0x40,0x5a,0x89,0x42,0x21,0xba,0xd1,0x2b,0xaa
key6:
    .byte 0x30,0x11,0xb2,0x0d,0x40,0x66,0xfe,0x4d,0x1a,0xef,0xbc,0x6c,0xa0,0x3e,0x97,0xc6
key7:
    .byte 0xc2,0x99,0x06,0xed,0x82,0xff,0xf8,0xa0,0x98,0x10,0x44,0xcc,0x38,0x2e,0xd3,0x0a
key8:
    .byte 0x73,0xff,0x61,0xea,0xf1,0x00,0x99,0x4a,0x69,0x10,0xdd,0x86,0x51,0x3e,0x0e,0x8c
key9:
    .byte 0xda,0x54,0x05,0x3b,0x2b,0x54,0x9c,0x71,0x42,0x44,0x41,0xf7,0x13,0x7a,0x4f,0x7b
key10:
    .byte 0x36,0xd0,0x24,0x46,0x1d,0x84,0xb8,0x37,0x5f,0xc0,0xf9,0xc0,0x4c,0xba,0xb6,0xbb
    .align 16
j0:
    .byte 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x00,0x00,0x00,0x01
    .align 16
j1:
    .byte 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x00,0x00,0x00,0x02
    .align 16
zero:
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .align 16
len:
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80

