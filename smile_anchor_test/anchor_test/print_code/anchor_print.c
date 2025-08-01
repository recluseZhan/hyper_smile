#include <stdio.h>

int main() {
    // 按照你提供的汇编代码地址顺序写入机器码字节
    unsigned char code[] = {
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0x00, 0xb5, 0x39, 0x00, // lea    0x39b500(%rip),%rsi
        0x48, 0x8d, 0xbe, 0x00, 0x00, 0x40, 0x00, // lea    0x400000(%rsi),%rdi
        0xf3, 0xa5,                               // rep movsl %ds:(%rsi),%es:(%rdi)
        0x4c, 0x89, 0xd1,                         // mov    %r10,%rcx
        0x48, 0x8d, 0x35, 0xe3, 0xff, 0xff, 0xff, // lea    -0x1d(%rip),%rsi
        0xf3, 0xa5,                               // rep movsl %ds:(%rsi),%es:(%rdi)

        // loop:
        0xf0, 0x49, 0x0f, 0xc1, 0x09,             // lock xadd %rcx,(%r9)
        0x7b, 0x04,                               // jnp +4
        0xf3, 0xa5,                               // rep movsl
        0xeb, 0xf5                                // jmp -11 (to loop)
    };

    size_t code_len = sizeof(code);
    printf("Machine code (%zu bytes):\n", code_len);
    for (size_t i = 0; i < code_len; ++i) {
        printf("%02x", code[i]);
        if ((i + 1) % 7 == 0) printf("\n");
    }
    printf("\n");

    return 0;
}

