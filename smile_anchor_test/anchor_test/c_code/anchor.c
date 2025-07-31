#include<string.h>
#include<stdint.h>
uint8_t p0[4096],p1[4096],p2[4096];
uint8_t p[4096];
int main(){
    memset(p0,0,4096);
    memset(p1,0,4096);
    memset(p2,0,4096);
    p[0]=1024;
    unsigned long enclave_size = 0x400000;
    unsigned long ssa_offset = 0x39B510;

    asm volatile(
        "mov $1024,%%r10\n\t"
        "mov $0,%%rsp\n\t"
        "mov $0,%%rdi\n\t"
        "mov $0,%%rsi\n\t"
        "mov %0,%%r9\n\t"
        ::"r"(p):
    );

    asm volatile(
        "mov %%r10,%%rcx\n\t"
        "lea 0x39B500(%%rip),%%rsi\n\t"
        "lea 0x400000(%%rsi),%%rdi\n\t"
        "rep movsd\n\t"

        "mov %%r10,%%rcx\n\t"
        "lea -0x1d(%%rip),%%rsi\n\t"
        "rep movsd\n\t"

      "loop:\n\t"
        "lock xadd %%rcx,(%%r9)\n\t"
        "jnp worker\n\t"
        "rep movsd\n\t"
        "jmp loop\n\t"
      "worker:\n\t"
        :::
    );
}
