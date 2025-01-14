#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>
#include <openssl/aes.h>
unsigned long urdtsc()
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}

unsigned long tt1,tt2,tta;
void aes_en0(const unsigned char *data,unsigned char *data_crypto,const unsigned char *key,int len){
    AES_KEY aes_key;
    AES_set_encrypt_key(key,128,&aes_key);
    int num_blocks=len/16;
    int i;
    for(i=0;i<num_blocks;i++){
        tt1=urdtsc();
        AES_encrypt(data+(i*16),data_crypto+(i*16),&aes_key);
        tt2=urdtsc();
    }
}
void aes_en(const unsigned char *data,unsigned char *data_crypto,const unsigned char *key){
    AES_KEY aes_key;
    AES_set_encrypt_key(key,128,&aes_key);
    tt1=urdtsc();
    AES_encrypt(data,data_crypto,&aes_key);
    tt2=urdtsc();
}
void main(){
    int size = 4096;
    uint8_t *code = (uint8_t *) malloc(size);
    uint8_t *code_crypto = (uint8_t *) malloc(size);
    unsigned long t1,t2;
    int i;
    
    unsigned char key[16] = "0123456789abcdef";   
    unsigned char *data = (unsigned char *) malloc(size);
    unsigned char *data_crypto = (unsigned char *) malloc(size);
    
    
    memset(code,0x1f,size);
    memset(code_crypto,0,size);
    
    memset(data,0x1f,size);    
    memset(data_crypto,0,size);
    
    
    
    for(i=0;i<size;i++){
        _mm_clflushopt(&data_crypto[i]);
        _mm_clflushopt(&data[i]);
    }
    t1=urdtsc();
    aes_en(data,data_crypto,key);
    //printf("%.2x\n",data_crypto[16]);
   // memcpy(code_crypto,code,size);
 
            
      //  _mm_clflushopt(&data[0]);
  
    //for(i=0;i<20000;i++)
        //aesEncrypt(key, 16, data, data_crypto, 4096);
    t2=urdtsc();
    
    printf("SMILE time(ns):%ld\n",(t2-t1)*5/17*(19043));
    
}

