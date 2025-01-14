
#include <stdio.h>
#include<stdint.h>
#include <nmmintrin.h>
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}
uint32_t crcr(uint8_t* ptr,uint32_t len){

    uint32_t crc = 0xffffffff;
    while (len != 0)
    {
        uint8_t byte = *ptr;
        uint8_t j = 8;
        crc = crc ^ byte;
        for(j; j > 0; --j)
        {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
        ptr++;
        len--;
    }
    crc = crc ^ 0xffffffff;
  // xinxin  printk("trampoline_en = 0x%lx",crc);
    return crc;
}
uint32_t crch(const void *data,size_t size){
    double t1,t2,tu,tt,tx;
    const uint8_t *buffer = (const uint8_t *)data;
    uint32_t crc = 0xffffffff;
    t1=urdtsc();
    t2=urdtsc();
    tu = t2-t1;
    
    t1=urdtsc();
    for(size_t i=0;i<size;i++){
        
    }
    t2=urdtsc();
    tx=t2-t1;
    
    t1=urdtsc();

   _mm_crc32_u8(crc,buffer[0]);
   t2=urdtsc();
   tt= (t2-t1)/3.4;
   //if(tt>0&&tt<50){
       FILE *file1;
     file1 = fopen("ttt.txt","a");
     fprintf(file1,"%lf\n",tt);
     fclose(file1);
   //}
    
   //printf("%ld\n",(t2-t1-tu));
   
    for(size_t i=0;i<size;i++){
        crc=_mm_crc32_u8(crc,buffer[i]);
    }
   // t2=urdtsc();
   /// printf("%ld\n",(t2-t1));
    return crc^0xffffffff;
}
int main(){
    uint8_t buffer[4096];
    unsigned long t1,t2;
    for(int i=0;i<4096;i++){
        buffer[i]=1;
    }
    //t1=urdtsc();
    uint32_t crc=crch(buffer,4096);
   // t2=urdtsc();
    //printf("0x%08x,%ld\n",crc,(t2-t1));
    
   // t1=urdtsc();
    crc=crcr(buffer,4096);
    //t2=urdtsc();
    //printf("0x%08x,%ld\n",crc,(t2-t1));
    return 0;
}
