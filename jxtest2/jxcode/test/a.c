#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define CPU_SETSIZE 1024

typedef struct {
    uint64_t cpu_bits[CPU_SETSIZE/64];
}cpu_set_tt;

void CPU_SET(int cpu,cpu_set_tt *cpuset){
    cpuset->cpu_bits[cpu/64]|=(1ULL<<(cpu%64));
}

void CPU_ZERO(cpu_set_tt *cpuset){
    for(int i=0;i<CPU_SETSIZE/64;++i){
        cpuset->cpu_bits[i]=0;
    }
}

int main(){
    cpu_set_tt cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0,&cpuset);
    
    printf("hellpo\n");
    getchar();
    return 0;
}
