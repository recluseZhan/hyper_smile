#include <stdio.h>

int main() {
    unsigned char code[] = {
        0xf3, 0x0f, 0x1e, 0xfa,                   // endbr64
        0x55,                                     // push   %rbp
        0x48, 0x89, 0xe5,                         // mov    %rsp,%rbp
        0x48, 0x83, 0xec, 0x20,                   // sub    $0x20,%rsp
        0x48, 0x89, 0x7d, 0xe8,                   // mov    %rdi,-0x18(%rbp)
        0xc7, 0x45, 0xfc, 0x00, 0x00, 0x00, 0x00, // movl   $0x0,-0x4(%rbp)
        0xeb, 0x25,                               // jmp    0x3e
        0x48, 0x8b, 0x45, 0xe8,                   // mov    -0x18(%rbp),%rax
        0x48, 0x8d, 0x48, 0x10,                   // lea    0x10(%rax),%rcx
        0x48, 0x8d, 0x05, 0x00, 0x00, 0x00, 0x00, // lea    0x0(%rip),%rax
        0x48, 0x8d, 0x15, 0x00, 0x00, 0x00, 0x00, // lea    0x0(%rip),%rdx
        0x48, 0x89, 0xce,                         // mov    %rcx,%rsi
        0x48, 0x89, 0xc7,                         // mov    %rax,%rdi
        0xe8, 0x00, 0x00, 0x00, 0x00,             // call   <relative offset>
        0x83, 0x45, 0xfc, 0x01,                   // addl   $0x1,-0x4(%rbp)
        0x81, 0x7d, 0xfc, 0xff, 0x00, 0x00, 0x00, // cmpl   $0xff,-0x4(%rbp)
        0x7e, 0xd2,                               // jle    <back to loop>
        0x90,                                     // nop
        0x90,                                     // nop
        0xc9,                                     // leave
        0xc3                                      // ret
    };

    size_t code_len = sizeof(code);
    printf("Machine code (%zu bytes):\n", code_len);
    for (size_t i = 0; i < code_len; ++i) {
        printf("%02x", code[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
    printf("\n");

    return 0;
}

