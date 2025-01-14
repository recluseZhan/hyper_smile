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
#include "aes.h"
#include "rsa.h"
#define DEVNAME "/dev/jxdev"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DBG_ENCL        1
#define ONE_TIME_CONTEXT_SIZE 0x1000
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX
#include <sys/mman.h>
#include <signal.h>
#include <sched.h>
#include <string.h>
#include "libsgxstep/enclave.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/pt.h"
#include "sgx_urts.h"
#include "App.h"
#include "Enclave/encl_u.h"
#include <sys/mman.h>
#include <sgx_report.h>
/* Global EID shared by multiple threads */
void *data_pt = NULL, *data_page = NULL, *code_pt = NULL;
int fault_fired = 0, aep_fired = 0;
sgx_enclave_id_t eid = 0;
sgx_enclave_id_t global_eid = 0;

sgx_launch_token_t token = {0};
int retval = 0, updated = 0;
char old = 0x00, new = 0xbb;
int fd;
unsigned long ma1,ma2,ma;
void *a_pt;

void test_attestation(int flag);

int ocall_limit= 100;

static unsigned long t_anchor,t_worker,t_id,t_trap;
static unsigned long t_aes,t_rsa;

typedef struct _sgx_errlist_t
{
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

unsigned long urdtsc()
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}


extern void sgx_reporter_enter(void *req, void *entry);

extern void sgx_reporter_enter2(void *tcs, const long fn, void *ocall_table, void *output, unsigned long size, void *addr);




/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] =
{
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

void aep_cb_func(void)
{
    gprsgx_region_t gprsgx = {0};
    uint64_t erip = edbgrd_erip() - (uint64_t) get_enclave_base();
    info("Hello world from AEP callback with erip=%#llx! Resuming enclave..", erip);
     test_attestation(1);
    edbgrd(get_enclave_ssa_gprsgx_adrs(), &gprsgx, sizeof(gprsgx_region_t));
    dump_gprsgx_region(&gprsgx);

    aep_fired++;
}

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++)
    {
        if(ret == sgx_errlist[idx].err)
        {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl)
        printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave( "./Enclave/encl.so", /*debug=*/DBG_ENCL,
                                &token, &updated, &eid, NULL );
//  ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS)
    {
        print_error_message(ret);
        return -1;
    }
    register_aep_cb(aep_cb_func);
    print_enclave_info();
    return 0;
}


__attribute__((aligned(4096))) unsigned long output[0x10000] = {0};
__attribute__((aligned(4096))) unsigned long cpyopt[1024] = {0};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void fault_handler(int signo, siginfo_t * si, void  *ctx)
{
    ASSERT( fault_fired < 5);

    switch ( signo )
    {
      case SIGSEGV:
        info("Caught page fault (base address=%p)", si->si_addr);
        break;

      default:
        info("Caught unknown signal '%d'", signo);
        abort();
    }

    if (si->si_addr == data_page)
    {
        info("Restoring data access rights..");
        ASSERT(!mprotect(data_page, 4096, PROT_READ | PROT_WRITE));
        print_pte_adrs(data_pt);
    }
    else if (si->si_addr == code_pt)
    {
        info("Restoring code access rights..");
        ASSERT(!mprotect(code_pt, 4096, PROT_READ | PROT_EXEC));
        print_pte_adrs(code_pt);
    }
    else
    {
        info("Unknown #PF address!");
    }

    fault_fired++;
}

void attacker_config_page_table(void)
{
    struct sigaction act, old_act;

    /* NOTE: finer-grained permissions can be revoked using
     * `remap_page_table_level` and directly editing PTEs (e.g., app/memcmp),
     * but care needs to be taken as the Linux kernel expects PTE inversion
     * when unmapping PTEs (only relevant for MARK_NOT_PRESENT). We simply use
     * mprotect here as we don't need fine-grained permissions or performance
     * for this example, and mprotect transparently takes care of PTE
     * inversion.
     */
    info("revoking data page access rights..");
    SGX_ASSERT( get_a_addr(eid, &data_pt) );
    data_page = (void*) ((uintptr_t) data_pt & ~PFN_MASK);
    info("data at %p with PTE:", data_pt);
    print_pte_adrs(data_pt);
    ASSERT(!mprotect(data_page, 4096, PROT_NONE));
    print_pte_adrs(data_pt);

    info("revoking code page access rights..");
    SGX_ASSERT( get_code_addr(eid, &code_pt) );
    info("code at %p with PTE:", code_pt);
    print_pte_adrs(code_pt);
    ASSERT(!mprotect(code_pt, 4096, PROT_NONE));
    print_pte_adrs(code_pt);

    /* Specify #PF handler with signinfo arguments */
    memset(&act, sizeof(sigaction), 0);
    act.sa_sigaction = fault_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;

    /* Block all signals while the signal is being handled */
    sigfillset(&act.sa_mask);
    ASSERT(!sigaction( SIGSEGV, &act, &old_act ));
}

/* _xin 
inline int set_cpu(int i)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(i, &mask);
    printf("thread %u, i = %d\n", pthread_self(), i);
    if(-1 == pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask))
    {
        return -1;
    }
    return 0;
}
*/
int flag = 1;

static unsigned long pp0[4096] = {0x2};
__attribute__((aligned(4096))) unsigned long pp1[4096] = {0x0};
__attribute__((aligned(4096))) unsigned long pp2[4096] = {0x0};
/*
* argu_list:
*   [0] = flag;
*   [1] = output address
*   [2] = base address of first dumping page
*   [3] = counter of first dumping page
*   [4] = base address of second dumping page
*   [5] = counter of second dumping page
*/
unsigned long argu_list[6] = {0};

void test_attestation(int flag)
{
    
    unsigned long *base;
    unsigned long *limit;
    base = (unsigned long *)get_enclave_base();
    int size = (int)get_enclave_size();
    limit = base+size/0x8;
    unsigned long *ssa;
    ssa = (unsigned long *)get_enclave_ssa_gprsgx_adrs();

    unsigned long *tcs;
    
    
    tcs = (unsigned long *)sgx_get_tcs();
    printf("tcs. frame ==  %lx, %lx, %lx\n", (unsigned long)tcs[0], (unsigned long)tcs[1],(unsigned long)tcs[2]);
//   tcs += 513;
 //   tcs = tcs-0x8800;     //different layout make diff  tcs , carefully
 //   tcs = tcs-0x206800;

 printf("tcs=%lx\n",tcs);
  //  tcs = tcs-0x206800;
    printf("new_tcs=%lx\n",tcs);
 //xinxin    getchar();
//	tcs = tcs-0x4400;

    memset(output, 0, sizeof(output));
    memset(argu_list, 0, sizeof(argu_list));
    
    argu_list[0] = flag;
    argu_list[1] = (unsigned long)&(output);
    argu_list[2] = (unsigned long )&ssa;
    argu_list[3] = 30;
    argu_list[4] = (unsigned long )&ssa;
    argu_list[5] = 40;

    printf("MAIN: t_attestation %lx, %lx, %lx\n", tcs, output,argu_list);

    printf("MAIN: begin ENCLAVE ECALL: ------\n\n");
    memset(output, 0, sizeof(output));
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = SGX_SUCCESS;
    unsigned long a = 12;
    unsigned long b = 13;
    int sum;



    unsigned long ms[2]= {0};
    ms[0] = (unsigned long)&output;
//    printf("\nMAIN:  t_attestation sim ms parameter: %lx, %lx \n", ms[0], ms[1]);
   // ret= attested_reporter1(eid, output,argu_list, &a, &b);

    unsigned long sharebuff[100] = {0};
    sharebuff[0]=0x200;
//tcs---rdi   1 ----rsi   NULL----rdx    output---rcx    0x1000---r8 -->r10, (%r9)
    int p, mid1= 0, mid2 =0 , s=0;
   for(p=0;p<1;p++){
    unsigned long t1, t2;
          int i;
unsigned long t =0;
    t1 = urdtsc();
    t2 = urdtsc();
//    printf("Host: two times of urdtsc time : %f \n ", (t2-t1)/3.4);
   
    t1=0;
    t2=0;
    t_id=0;
    t1 = urdtsc();
    sgx_reporter_enter2 (tcs, 1, NULL,  output, sharebuff[0], sharebuff );
    for(i=0;i<0x200;i++)
        clflush(&output[i]);
   t2=urdtsc();
   t_id=(t2-t1)/3.4;
     // t= t2-t1; 
    mid1 = (mid1+t)/2;
    t1 = urdtsc();
    for(i=0; i<0x40; i++)
    {
        if (cpyopt[i]== output[i])
		s++;
    }
    t2 = urdtsc();
   
    //      t= t2-t1; 
    mid2 = (mid2+t)/2;
    t1=urdtsc();
    sgx_reporter_enter2 (tcs, 1, NULL,  output, sharebuff[0], sharebuff );
    for(i=0;i<0x200;i++)
        clflush(&output[i]);
    t2 = urdtsc();
    t_id=t_id + (t2-t1)/3.4;
    //printf("identity report time:%ld\n",(t2-t1)/3.4);
//xinxin  getchar();
  

 //   memset(output, 0, sizeof(output));
 //   sleep(1);
    }
    

    printf("Host: current report time : %f \n  verfied time : %f \n and test loop: %d \n", mid1/3.4, mid2/3.4, s);

 printf(" mr_enclave: ");
   int i;
    for(i=1; i<6; i++){
    	printf(" 0x%lx ", sharebuff[i]);

        }
 printf(" mr_enclave: end \n\n");
//ssa[0] = 100;
// memcpy(output,&ssa[0],100);
    printf("\nT_attest: get target thread SSA: \n");
    for(i=0; i<0x40; i++)
    {
        printf("%016lx ", output[i]);
        if(i%4 ==0)
            printf("\n");

    }

//    printf("RAX:    %lx\n", output[12]);
    printf("RCX:    %lx\n", output[12]);
    printf("RDX:    %lx\n", output[13]);
    printf("RBX:    %lx\n", output[14]);
    printf("RSP:    %lx\n", output[15]);
    printf("RBP:    %lx\n", output[16]);
    printf("RSI:    %lx\n", output[17]);
    printf("RDI:    %lx\n", output[18]);
    
    //memcpy(pp0,output,sizeof(output));

    if (ret != SGX_SUCCESS)
        abort();

}

void *thread_attestation(void *arg)
{
    unsigned long *base;
    unsigned long *limit;
    base = (unsigned long *)get_enclave_base();
    int size = (int)get_enclave_size();
    limit = base+size/0x8;
    unsigned long *ssa;
    ssa = (unsigned long *)get_enclave_ssa_gprsgx_adrs();

    unsigned long *tcs;
    tcs = (unsigned long *)sgx_get_tcs();
    //tcs += 513;

    memset(output, 0, sizeof(output));
    memset(argu_list, 0, sizeof(argu_list));

    argu_list[0] = flag;
    argu_list[1] = (unsigned long)&(output);
    argu_list[2] = (unsigned long )&ssa;
    argu_list[3] = 30;
    argu_list[4] = (unsigned long )&ssa;
    argu_list[5] = 40;


    printf("t_attestation %lx, %lx, %lx\n", tcs, output,argu_list);
    printf("begin print\n\n");
    memset(output, 0, sizeof(output));
    sgx_status_t ret = SGX_SUCCESS;

//   ret= attested_reporter1(eid, output, argu_list, 0, 0);
    unsigned long ms[2]= {0};
    ms[0] = (unsigned long)&output;
    //sgx_reporter_enter2 (tcs, 1, NULL,  ms, 0, NULL );
    int i;

    for(i=0; i<0x200; i++)
    {
        printf(" 0x%lx ", output[i]);

    }

    printf("\n");
    if (ret != SGX_SUCCESS)
        abort();
}

void *thread_general(void *arg)
{
    int sum;
    sgx_status_t ret = SGX_SUCCESS;

    unsigned long read_flags= 0;
    unsigned long a = 12;
    unsigned long b = 13;

    printf("\nMAIN: t_general1: start general thread\n ");


    memset(output, 0, sizeof(output));
    unsigned long t1, t2, t=0;

    int p;
    unsigned long mid1 = 0, mid2=0, s=0;
    unsigned long mr_encl[4] = {0xf586b9df737b6b35,0x1064f84839ddab91,0x438e33d4e2792e0d,0xc577d3a582be92d4};
    for(p=0; p<1; p++){
  
    int i;  
    t1=0;
    t2=0;

    t1 = urdtsc();
    printf("\nggg = %d\n",ret);
    
    a = data_page;
    b = code_pt;
    ret = addint(eid,&sum, a,b, output);
    printf("\nkkk=%d\n",ret);
    t2 = urdtsc();
    t= t2-t1;   
    if(t>0)
       mid1 = (mid1+t)/2;
    t1 = urdtsc();
    for(i=0; i<0x8; i++)
    {
        if (mr_encl[i]== output[i])
		s++;
    }
    t2 = urdtsc();
     t= t2-t1; 
     if(t>0)
        mid2 = (mid2+t)/2;

    }
    printf("====Host - general: current report time : count: %d, time: %f\n and result = %d\n", mid1, mid1/3.4, s);
    printf("====Host - general: current verify time : count: %d, time: %f\n and result = %d\n", mid2, mid2/3.4, s);


 //   printf("\nMAIN: t_general1: the mr_enclave data:\n");
printf("\nTarget Thread SSA:  \n");
   for(p=0; p<0x40; p++)
    {
        printf("%016lx ", output[p]);
             if(p%4 ==0)
            printf("\n");

    }

//    printf("RAX:    %lx\n", output[12]);
    printf("RCX:    %lx\n", output[12]);
    printf("RDX:    %lx\n", output[13]);
    printf("RBX:    %lx\n", output[14]);
    printf("RSP:    %lx\n", output[15]);
    printf("RBP:    %lx\n", output[16]);
    printf("RSI:    %lx\n", output[17]);
    printf("RDI:    %lx\n", output[18]);


//    memset(output, 0, sizeof(output));
    printf("\n");
    printf("MAIN: ******t_general: end execution with result:%d\n", sum);
    if (ret != SGX_SUCCESS)
        abort();

}

//unsigned long *outp = (unsigned long *) malloc(4096);
void thread_general1(void)
{
    //unsigned long *outp = (unsigned long *) malloc(4096);
    int sum;
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
//  ret= addint(global_eid, &sum, 11,12);

    unsigned long read_flags= 0;
    unsigned long a = 12;
    unsigned long b = 13;


    printf("\nMAIN: t_general1: start general thread\n ");
   /* _xin if(set_cpu(1))
    {
        printf("set cpu error\n");
    }*/

    memset(output, 0, sizeof(output));
    unsigned long t1, t2, t=0;

    int p;
    unsigned long mid1 = 0, mid2=0, s=0;
    unsigned long mr_encl[4] = {0xf586b9df737b6b35,0x1064f84839ddab91,0x438e33d4e2792e0d,0xc577d3a582be92d4};
    for(p=0; p<1; p++){
  
    int i;

    //t1 = urdtsc();
   // t2 = urdtsc();
//    printf("----Host- general: two times of urdtsc time : count: %d,  time: %f\n ", (t2-t1), (t2-t1)/3.4);
   
    a = data_page;
    b = code_pt;
    
    t1=urdtsc();
    ret = addint(eid,&sum, a,b, output);
    t2=urdtsc();
    //printf("worker report time:%ld\n",(t2-t1));
 /*   for(i=0; i<0x50; i++)
    {
        if (mr_encl[i]== output[i])
		s++;
    }
  */ 
    t= t2-t1;
 //xinxin    getchar();   
    if(t>0)
       mid1 = (mid1+t)/2;
    t1 = urdtsc();
    for(i=0; i<0x8; i++)
    {
        if (mr_encl[i]== output[i])
		s++;
    }
    t2 = urdtsc();
     t= t2-t1; 
     if(t>0)
        mid2 = (mid2+t)/2;

 //   sleep(1);
  //  memset(output, 0, sizeof(output));
    }
   // printf("====Host - general: current report time : count: %d, time: %f\n and result = %d\n", mid1, mid1/3.4, s);
   // printf("====Host - general: current verify time : count: %d, time: %f\n and result = %d\n", mid2, mid2/3.4, s);


 //   printf("\nMAIN: t_general1: the mr_enclave data:\n");
printf("\nTarget Thread SSA:  \n");
   for(p=0; p<0x40; p++)
    {
        printf("%016lx ", output[p]);
             if(p%4 ==0)
            printf("\n");

    }

//    printf("RAX:    %lx\n", output[12]);
    printf("RCX:    %lx\n", output[12]);
    printf("RDX:    %lx\n", output[13]);
    printf("RBX:    %lx\n", output[14]);
    printf("RSP:    %lx\n", output[15]);
    printf("RBP:    %lx\n", output[16]);
    printf("RSI:    %lx\n", output[17]);
    printf("RDI:    %lx\n", output[18]);


//    memset(output, 0, sizeof(output));
    printf("\n");
    printf("MAIN: ******t_general: end execution with result:%d\n", sum);
    if (ret != SGX_SUCCESS)
        abort();

}


//test 
void thread_general2(void)
{
    int sum;
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
//  ret= addint(global_eid, &sum, 11,12);

    unsigned long read_flags= 0;
    unsigned long a = 12;
    unsigned long b = 13;


    printf("\nMAIN: t_general1: start general thread\n ");
   /* _xin if(set_cpu(1))
    {
        printf("set cpu error\n");
    }*/

    memset(output, 0, sizeof(output));
    unsigned long t1, t2, t=0;

    int p;
    unsigned long mid1 = 0, mid2=0, s=0;
    unsigned long mr_encl[4] = {0xf586b9df737b6b35,0x1064f84839ddab91,0x438e33d4e2792e0d,0xc577d3a582be92d4};
    for(p=0; p<1; p++){
  
    int i;

    t1 = urdtsc();
    t2 = urdtsc();
//    printf("----Host- general: two times of urdtsc time : count: %d,  time: %f\n ", (t2-t1), (t2-t1)/3.4);
   
    t1=0;
    t2=0;
 //   memset(output, 0, sizeof(output));
    t1 = urdtsc();
    ret = addint(global_eid,&sum, a,b, output);
 /*   for(i=0; i<0x50; i++)
    {
        if (mr_encl[i]== output[i])
		s++;
    }
  */ 
    t2 = urdtsc();
    t= t2-t1;   
    if(t>0)
       mid1 = (mid1+t)/2;
    t1 = urdtsc();
    for(i=0; i<0x4; i++)
    {
        if (mr_encl[i]== output[i])
		s++;
    }
    
    t2 = urdtsc();
     t= t2-t1; 
     if(t>0)
        mid2 = (mid2+t)/2;

  //  cacheflush(output, int, 0x10000);

   // clflush(output+1);
    __asm__ __volatile__("clflush (%0)\n"
			::"r"(output)
			:);
 //  sleep(1);
//    memset(output, 0, sizeof(output));
    }
    printf("====Host - general: current report time : count: %d, time: %f\n and result = %d\n", mid1, mid1/3.4, s);
    printf("====Host - general: current verify time : count: %d, time: %f\n and result = %d\n", mid2, mid2/3.4, s);


//    printf("\nMAIN: t_general1: the mr_enclave data:\n");


   printf("\nTarget threads SSA:\n");
   for(p=0; p<80; p++)
    {
        printf(" 0x%016lx \n", output[p]);
        output[p] = 0;

    }

    memset(output, 0, sizeof(output));
    printf("\n");
    printf("MAIN: ******t_general: end execution with result:%d\n", sum);
    if (ret != SGX_SUCCESS)
        abort();

}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */

    printf("Ocall: %lx ========>  %s", urdtsc(), str);
        ocall_limit --;
   if(ocall_limit <2)
    {
	sleep(10);
        test_attestation(1);
	ocall_limit= 100;
    }
}
/*
unsigned long t000,t111;

char stack_for_attack[0x2000];
void attacked(){
    __asm__ __volatile__(
        "mov %0,%%rsp\n\t"
        "add $0x1000, %%rsp\n\t"
        "call print_attack\n\t"
        ::"r"(&stack_for_attack)
    );
}


void ocall_get_ptr(unsigned long *ptr)
{
    *ptr = (unsigned long)attacked;
}

unsigned long base;
void ocall_build_payload(unsigned long *payload, size_t len)
{
    printf("base = %lx\n", base);
    payload[0] = "\x58";             // gadget 1
    payload[1] = "\x48\x83\xc0\x05";
    payload[2] = "\x48\x89\xc0";             // gadget 2
    payload[3] = "\xc3";   // EEXIT target
    payload[4] = "\x48\x89\x05\x00\x00\x00\x00";             // gadget 3
    payload[5] = "\xc3";
}

#define INFO "\033[92m[INFO ]\033[39m "
#define ERROR "\033[91m[ERROR]\033[39m "
char *data;
unsigned long getrop=0;
unsigned long putrop=3;
void gadget1(){
    __asm__ __volatile__(
        "pop %%rax\n\t"
        "add $0x5, %%rax\n\t"
        "mov %%rax,%0\n\t"
        "ret\n\t"
        :"=r"(getrop)
        ::
    );
}*/
/*
void ack(){
    static char shellcode[] = {0xf3,0x0f,0x1e,0xfa,
                               0x58,
                               0x48,0x83,0xc0,0x05,
                               0x48,0x89,0xc0,
                               0xc3};
    void *mem = mmap(NULL,sizeof(shellcode),PROT_WRITE | PROT_EXEC,MAP_ANON | MAP_PRIVATE,-1,0);
    memcpy(mem,shellcode,sizeof(shellcode));
    void (*fn)()=mem;
    fn();
}
unsigned long secret[4] = {1,2,3,4};
unsigned long payload[6]={3,3,3,3,3,3};
void bugfun(){
    
    memcpy((void *)(secret + 4),payload, sizeof(payload));
    printf("secret stack add=%lx",secret);
    __asm__ __volatile__(
        "pop %%rax\n\t"
        //"add $0x5, %%rax\n\t"
        "ret\n\t"
        :::
    );
   // printf("getrop=%lx",getrop);
}
*/
void* thread_func(void *args){

    info_event("reading/writing debug enclave memory..");
    edbgrd(data_pt, &old, 1);
    edbgwr(data_pt, &new, 1);
    edbgrd(data_pt, &new, 1);
    info("data at %p (page %p): old=0x%x; new=0x%x", data_pt, data_page, old & 0xff, new & 0xff);
    
    return NULL;
}
void test1_code(int flag){
    int output=0;
    int opt1=2;
    int opt2=1;
    if(flag==0)
        output=opt1+opt2;
    else
        output=opt1-opt2;
    printf("output=%d",output);
}
//

void print_attack(){}
char stack_for_attack[0x2000];
void attacked(){
    __asm__ __volatile__(
        "mov %0,%%rsp\n\t"
        "add $0x1000, %%rsp\n\t"
        "call print_attack\n\t"
        ::"r"(&stack_for_attack)
    );
}
void ocall_get_ptr(unsigned long *ptr)
{
    *ptr = (unsigned long)attacked;
}
//unsigned long payload[5];
unsigned long base;
void ocall_build_payload(unsigned long *payload, size_t len)
{
   // unsigned long base=gadget1;
   // uint8_t *a;
   // int i;
    //a= gadget1;
   // printf("base = %lx\n", base);
   // for(i=0;i<=8;i++){printf("%02x",*(a+i));}
    printf("base = %lx\n", base);
    payload[0] = base+0x4084;             // gadget 1
    payload[1] = 0xFFFFFFFFFFFFFFFF;
    payload[2] = base+0x408f;             // gadget 2
    payload[3] = (unsigned long)attacked;   // EEXIT target
    payload[4] = base+0x4099;             // gadget 3
    printf("&gadget= %lx\n",payload[0]);
}
void test2_code(){
    unsigned long payload[5];
    unsigned long shadow[8]={0,0,0,0,0,0,0,0};
    payload[0] = base+0x4084;             // gadget 1
    payload[1] = 0xFFFFFFFFFFFFFFFF;
    payload[2] = base+0x408f;             // gadget 2
    payload[3] = (unsigned long)attacked;   // EEXIT target
    payload[4] = base+0x4099;             // gadget 3
    memcpy(shadow,payload, sizeof(payload));
    printf(
    "\n0x%lx: 0x%016lx    0x%016lx\n0x%lx: 0x%016lx    0x%016lx\n0x%lx: 0x%016lx    0x%016lx\n0x%lx: 0x%016lx    0x%016lx",&shadow,shadow[0],shadow[1],&shadow[2],shadow[2],shadow[3],&shadow[4],shadow[4],shadow[5],&shadow[6],shadow[6],shadow[7]
    );
    //printf("%lx\n%lx\n%lx    %lx\n%lx    %lx\n",shadow,shadow[0],shadow[1],shadow[2],shadow[3],shadow[4]);
    printf("\n");
//xinxin     getchar();
//xinxin     getchar();
}

const uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

void create_secret(uint8_t *pt,uint8_t *ct)
{
    int len = 16;
    unsigned long t1,t2;
    t1=urdtsc();
    aesEncrypt(key, 16, pt, ct, 1);
    t2=urdtsc();
    t_aes=(t2-t1)/3.4;
    printf("\n\naes time %lf",(t2-t1)/3.4);
    //getchar();
    //rsa
    printf("\n");
    t1=urdtsc();
    int p, q, n, phi, e, d, bytes;
    int *encoded, *decoded;
    //int *buffer;
    int *pubkey = (int *)malloc(len);
    memset(pubkey,0,len);
    FILE *f;
    srand(time(NULL));
    while(1) {
        p = randPrime(SINGLE_MAX);
//xinxin         getchar();

        q = randPrime(SINGLE_MAX);
//xinxin         getchar();

        n = p * q;
        if(n < 128) {
            printf("Modulus is less than 128, cannot encode single bytes. Trying again ... ");
 //xinxin            getchar();
        }
        else break;
    }
    if(n >> 21) bytes = 3;
    else if(n >> 14) bytes = 2;
    else bytes = 1;
    
    printf("bytes=%d",bytes);
    phi = (p - 1) * (q - 1);
    e = randExponent(phi, EXPONENT_MAX);
    d = inverse(e, phi);
   // buffer = key;
    
    encoded = encodeMessage(len,bytes,key,e,n);
    t2=urdtsc();
    printf("\n\n\nras time %lf\n",(t2-t1)/3.4);
   // getchar();
    memcpy(pubkey,encoded,len);
    //printf("\nrsa,encoded_0=%lx",encoded[0]);
//xinxin     getchar();
    decoded = decodeMessage(len/bytes, bytes, encoded, d, n);
    for(int i = 0; i< 16;i++)
        printf("%x ",decoded[i]);

   // for (int i =0;i<size;i++){
   //     printf("%lx ",pt[i]);
   // }
}
__attribute__((aligned(4096))) uint8_t data_tr[4096] = {0};
__attribute__((aligned(4096))) uint8_t code_tr[4096] = {0};
__attribute__((aligned(4096))) uint8_t worker_tr[4096] = {0};
void worker(void){
    int size = 4096;
    uint8_t *data = (uint8_t *) malloc(size);
    uint8_t *code = (uint8_t *) malloc(size);
    uint8_t *data_crypto = (uint8_t *) malloc(size);
    uint8_t *code_crypto = (uint8_t *) malloc(size);
    unsigned long t1,t2;
    int i;
    
    
    memset(worker_tr,0,size);
    memset(data_tr,0,size);
    memset(code_tr,0,size);
    memset(data,0,size);
    memset(code,0,size);
    memset(data_crypto,0,size);
    memset(code_crypto,0,size);
    
    printf("data at 0x%lx (page %p),code at %p\n",data_pt,data_page,code_pt);
    memset(data_page,0,size);
    //memcpy(worker_tr,worker,4096);
    
    //memcpy(data_tr,data_page,size);
    //memcpy(code_tr,data_page,size);
    //memcpy(worker_tr,worker,4096);
    //for(i=0;i<4096;i++)
      //  clflush(&worker_tr[i]);
    
   // fd = open(DEVNAME,O_RDWR);
   // printf("fd:%d\n",fd);
   // read(fd,worker_tr,sizeof(worker));
    //close(fd);
    ma1=urdtsc();
    t1=urdtsc(); 
    //memcpy(worker_tr,worker,4);
    /*
    asm volatile(
        "clflush (%0)\n\t"
        "clflush (%1)\n\t"
        "clflush (%2)\n\t"
        "clflush (%3)\n\t"
        ::"r"(worker_tr),"r"(worker_tr),"r"(worker_tr),"r"(worker_tr):"memory"
    );
*/
    for(i=0;i<749;i++)
        clflush(&worker_tr[i]);
    //memcpy(data, data_page, size);
    //memcpy(data, data_page, size);
    //clflush(data_tr);
    //memcpy(code, code_pt, size);
    t2=urdtsc();
    ma2=urdtsc();
    ma=ma+(ma2-ma1);
    t_worker=(t2-t1)/3.4;
    printf("\nworker report time:%ld\n",(t2-t1)/3.4);
    memcpy(data, data_page, size);
    memcpy(code, code_pt, size);
 //   memcpy(code, code_pt, size);
    //printf("\ndata_copy=0x%lx\n",*(data+8*0x119)-2);
    //printf("\ncode_copy=0x%lx\n",*code);
    //aes,esa
 
    create_secret(data,data_crypto);

    
  //  create_secret(code,code_crypto);
  //  t2=urdtsc();
  //  printf("\nworker report time:%ld\n",(t2-t1)/3.4);
   // memcpy(code, code_pt, size);
    //create_secret(code,code_crypto);
     
    printf("\ndata_crypto=0x%lx\n",*(data_crypto+8*0x119)-2);
   // printf("\ncode_crypto=0x%lx\n",*code_crypto);
    free(data);
    free(code);
       
}
//extern int anchor_function();
/* Application entry */

int SGX_CDECL main(int argc, char *argv[])
{
    
    (void)(argc);
    (void)(argv);
    
    
    //initialize_enclave();
    SGX_ASSERT( sgx_create_enclave( "./Enclave/encl.so", /*debug=*/DBG_ENCL,
                                &token, &updated, &eid, NULL ) ); 
                         
    info("Dry run to allocate pages");
    SGX_ASSERT( enclave_dummy_call(eid, &retval) );
    SGX_ASSERT( page_aligned_func(eid) );
    
    attacker_config_page_table();
    register_aep_cb(aep_cb_func);
    print_enclave_info();
    
    //unsigned long base;
    //printf("Give the base address of enclave in hex number: ");
   // scanf("%lx", &base);
    
    //t000 = urdtsc();
   // enclave_main(eid,base);
   // bugfun();
    //ggg();
    //t1 = urdtsc();
    //printf("ROP demo attack time : %ld \n", (t1-t0)/3400);
//xinxin     getchar();

    /* Initialize the enclave */
    /* _xin1
    
    if(initialize_enclave() < 0)
    {
        printf("Enter a character before exit ...\n");
        return -1;
    }*/

   /* _xin if(set_cpu(3))
    {
        printf("set cpu error\n");
    }*/
    /*2*/

//xinxin     getchar();
    
    unsigned long enc_base_addr = get_enclave_base();
    unsigned long ssa_addr = get_enclave_ssa_gprsgx_adrs();
    unsigned long *ssa_p;
    unsigned long size = 2088960;
    unsigned long p0;
  //  unsigned long ssa_offset = ssa_addr - enc_base_addr;
    unsigned long aa=0x0;
    unsigned long ret;
    unsigned long ssa_offset;
    unsigned long main_addr;
    unsigned long pi;
    unsigned long psize=0x1000;
    int i;
    unsigned long t1,t2;
    uint8_t *test_code;

    test_code = &test1_code;
    //unsigned long *p0_p; 
    main_addr = &main;
    p0 = enc_base_addr + size;
    ssa_p = ssa_addr;
   __attribute__((aligned(4096))) uint8_t pp0[4096] = {0x0};
   __attribute__((aligned(4096))) uint8_t pp1[4096] = {0x0};
   __attribute__((aligned(4096))) uint8_t pp[4096] = {0x0};
   __attribute__((aligned(4096))) uint8_t encl_id[4096] = {0x0};
   //uint8_t encl_id[32] = {0x0};
   memset(encl_id,0,4096);
   
   // unsigned long *pp0 = (unsigned long *)malloc(4096);
    p0 = pp0;
    ssa_offset = ssa_addr-(0x82a-0x567+main_addr);
    
    printf("\nenclave_base=%lx\n",enc_base_addr);
    //printf("\n\n\n");
    printf("ssa=%lx\n",ssa_addr);
    //printf("ssa_offset=%lx\n",ssa_offset);
    //printf("p0=%lx\n",p0);
    //printf("ssa_p=%lx\n",ssa_p[1]);
    //printf("p0_p=%lx\n",pp0[1]);
    
    t1=urdtsc();
    
    asm(
    "mov $0x100,%%r10\n\t"
    "mov %0,%%rdi\n\t"
    
    //"lea (%%rip,%2,1),%%rsi\n\t"
    //"lea $0x635(%%rip),%%rsi\n\t"
    //ssa_copy
    "mov %1,%%rsi\n\t"
    "mov %%r10,%%rcx\n\t"
    "rep movsq\n\t"
    //anchor_copy
    "mov %2,%%rdi\n\t"
    "lea -0x1a(%%rip),%%rsi\n\t"  
   // "mov $0x28,%%r10\n\t"     
    "mov %%r10, %%rcx\n\t"
    "rep movsq\n\t"
    // 
    ::"r"(pp0),"g"(ssa_addr),"r"(pp1)
    
    );
    unsigned long cha;
    cha = main+0x3b7;
   // cha = cha - ssa_addr;
    cha = ssa_addr - cha;
    printf("cha=%lx,ssa=%lx",cha,ssa_addr);
//xinxin     getchar();
    uint8_t tt[4096];
    memset(tt,3,4096);
    t2=urdtsc();
  //  t_anchor=(t2-t1)/3.4;
   // printf("anchor time:%ld\n",(t2-t1)/3.4);
    
    asm("mov $0x100,%%r10\n\t"
        "mov %0,%%rdi\n\t"
        ::"r"(tt)
        );
    ma1=urdtsc();
    t1=urdtsc();
    asm(
        "mov %0,%%rsi\n\t"  //for ssa
        "mov %%r10, %%rcx\n\t"
        "rep movsq\n\t"
        "lea -0x12(%%rip),%%rsi\n\t"       //for code
        "mov %%r10, %%rcx\n\t"
        "rep movsq\n\t"
        ::"r"(ssa_addr)
    );
    for(i=0;i<700;i++)
        clflush(&tt[i]);
    
   // clflush(tt);
    t2=urdtsc();
    ma2=urdtsc();
    ma=ma2-ma1;
    t_anchor=(t2-t1)/3.4;
    printf("anchor time:%ld\n",(t2-t1)/3.4);
    worker();
    //sgx_enclave_id_t *identity = &eid;
    //sgx_measurement_t *id0 = (sgx_measurement_t *)&eid;
    
    t1=urdtsc();
    sgx_measurement_t *id0 = (sgx_measurement_t *)&eid;
    memcpy(encl_id,id0->m,4096);
    for(i=0;i<32;i++)
        clflush(&encl_id[i]);
    t2=urdtsc();
    //t_id=(t2-t1)/3.4;
    for(i=0;i<sizeof(id0->m);i++){
        printf("%02x ",id0->m[i]);
    }
    printf("%ld\n",sizeof(id0->m));
    int rett;
    unsigned long c=1,b=1;
    int sum;
    unsigned long dd[110];
    t1=urdtsc();
    rett = addint(eid,&sum,t1,t2,output);
    t2=urdtsc();
    sum = (t2-t1)/3.4;
    //flagxin
    printf("%ld",sum);
  //  getchar();
    //getchar();
    //anchor_f(ssa_addr,p0);
  /*
    asm volatile(
        "mov %0,%%r9\n\t"
        "mov %1,%%r15\n\t"
        ::"r"(ssa_addr),"r"(p0)
    );*/
    //anchor_function();
    //SGX_ASSERT( anchor(eid,ssa_addr,p0) );
    printf("ssa_addr=%lx\n",pp0);
    printf("anchor_addr=%lx\n",pp1);
    printf("ssax=%lx\n",pp0[700]);
    printf("anchorx=%lx\n",pp1[700]);
    pp0[700]=0xaa;
    pp1[700]=0xbb;
    worker_tr[700]=0xdd;
   // printf("aa=0x%lx\n",aa);
    //smp_call vm_calls
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    write(fd,pp0,sizeof(pp0));
    close(fd);   
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    write(fd,pp1,sizeof(pp1));
    close(fd);
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    write(fd,worker_tr,sizeof(worker_tr));
    close(fd);
    
   
   // printf(get_enclave_ssa_gprsgx_adrs());
    //
    

    info("all is well; exiting..");
    
    /*2*/

//    fd = open(argv[1],O_RDWR);
    
//xinxin       getchar();
      
    //  asm volatile("mov %%cr3, %%rax\n\t"
    //		  "mov %%rax, %%cr3\n\t"
    //		  :::);    
   
   // native_write_cr3(native_read_cr3());
    //asm volatile("mov %0,%%cr3"::"r"(&main),"m"(__force_order));  
   
   //
   
   //
    pthread_t th;
    pthread_create(&th,NULL,thread_func,NULL);
    pthread_join(th,NULL);
    
//xinxin     getchar();
    //printf("please enter the pi:");
    //scanf("%ld",&pi);
    pid_t pid=getpid();
    printf("pi=%d\n",pid);
    pi = (unsigned long)pid;
//xinxin     getchar();
    
    
    //thread_general1();
     test_attestation(1);
//xinxin      getchar();
    
    pp[700]=0xcc;
    pp[0]=data_page;
   //xinxin pp[1]=code_pt;
    pp[1]=(unsigned long)test1_code;
    pp[2]=psize;
    pp[3]=pi;
    t1=urdtsc();
    fd = open(DEVNAME,O_RDWR);
   t1=urdtsc();
    write(fd,pp,sizeof(pp));
   t2=urdtsc();
    //printf("trampoline time:%ld\n",(((t2-t1)*5)/17)*4);
    t_trap=(t2-t1)/3.4*4*4;
    //getchar();
    close(fd);
    
     
     //id_report();
     FILE *file1;
     file1 = fopen("report_time.txt","a");
    /*
     if(file1==NULL)
         return 1;
     if(t_trap>80000||t_trap<20000){
         t_trap=0;
     } 
     if(t_anchor>35000||t_anchor<10000){
         t_anchor=0;
     }
     if(t_worker>30000||t_worker<5000){
         t_worker=0; 
     }   
     if(t_id>50000||t_id<20000){
         t_id=0;
     }*/
     fprintf(file1,"%ld\t%ld\t%ld\t%ld\n",t_trap/4*2,t_anchor,t_worker,t_id);
     fclose(file1);
     
     printf("tra_building:%ld,anchor/SSA report time:%ld, worker report time:%ld, identity report time:%ld\n",t_trap,t_anchor,t_worker,t_id);
     
    //int sum;
   // unsigned long a = data_page;
   // unsigned long b = code_pt; 
   // t1=urdtsc();
   // ret = addint(eid,&sum, a,b, output);
   // t2=urdtsc();
   // printf("worker report time:%ld\n",(t2-t1));
    for(i=0;i<=40;i++)
        printf("%02x",*(test_code+i));
    printf("\n\n");
    
   // unsigned long shadow[5];
   // build_payload();
   // memcpy(shadow,payload,sizeof(payload));
   // for(i=0;i<=4;i++)
    //    printf("%lx ",shadow[i]);
    //sgx_get_tcs();
//xinxin    getchar();
  //  unsigned long t0,t1;
  //  t0 = urdtsc();
  // printf("Give the base address of enclave in hex number: ");
   //scanf("%lx", &base);
   base = get_enclave_base();
    //enclave_main(eid);
   test2_code();
  //  t1 = urdtsc();
  //  printf("ROP demo attack time : %ld \n", (t1-t0)/3400);
   
//xinxin     getchar();
    //
     getchar();
    SGX_ASSERT( sgx_destroy_enclave( eid ) );
    return 0;
}


/*

Once the error:
[sig_handler sig_handler.cpp:93] signal handler is triggered
[sig_handler sig_handler.cpp:149] NOT enclave signal
Makefile:240: recipe for target 'run' failed
make: *** [run] Segmentation fault


please check two reasons: 1 anchor_funciton in layout of enclave.so is changed 
                          2 tcs address layout is changed 

*/
