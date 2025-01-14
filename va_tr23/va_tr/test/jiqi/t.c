#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
typedef void (*FuncType)();
void main(){
    unsigned char MC[]={
        0x55,
        0x48,0x89,0xe5,
        0xb8,0x01,0x00,0x00,0x00,
        0x5d,
        0xc3
    };
    //unsigned char* exe = (unsigned char*)malloc(sizeof(MC));
    //size_t codeSize = sizeof(MC);
    //unsigned char* exe = mmap(NULL,codeSize,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    //memcpy(exe,MC,sizeof(MC));
    //mprotect(exe,codeSize,PROT_READ|PROT_EXEC);
    
    
    FuncType func = (FuncType)MC;
    func();
    
    //printf("%d\n",result);
    //free(exe);
}
