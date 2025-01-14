#include<linux/init.h>
#include<linux/module.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/sched.h>
#include<linux/export.h>
#include<linux/init_task.h>
#include <linux/types>


MODULE_LICENSE("GPL");
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
EXPORT_SYMBOL(crcr);

static uint32_t crch(const void *data,size_t len){
    uint32_t crc = ~0;
    crc = crc32c(crc,data,len);
    return ~crc;
    /*
    uint32_t crc = 0xffffffff;
    asm volatile(
        "movl %1,%%ecx\n\t"
        "movl %2,%%esi\n\t"
        "xorl %%eax,%%eax\n\t"
        "loop_start:\n\t"
        "crc32b (%%esi),%%eax\n\t"
        "incl %%esi\n\t"
        "loop loop_start\n\t"
        "movl %%eax,%0\n\t"
        :"=r"(crc)
        :"r"(len),"r"(data)
      
    );
    return ~crc;
    */
}
EXPORT_SYMBOL(crch);

static int __init crc_init(void)
{
    uint8_t data[4096];
    uint32_t len=4096;
    uint32_t crc1,crc2;
    unsigned long t1,t2;
    memset(data,1,4096);
    t1=urdtsc();
    crc1=crcr(data,4096);
    t2=urdtsc();
    printk("crc1=%lx,t=%ld",crc1,(t2-t1));
   
    t1=urdtsc();
    crc2=crch(data,4096);
    t2=urdtsc();
    printk("crc1=%lx,t=%ld",crc2,(t2-t1));
  // xinxin   printk(KERN_ALERT"crc module is entering..\n");
    
    //printk("0x%lx\n", (unsigned long)crc32(a,len));

    
  // xinxin   printk(KERN_ALERT"crc module is entering..\n");


    return 0;
}

static void __exit crc_exit(void)
{
   // xinxin  printk(KERN_ALERT"crc module is leaving..\n");
    
}


module_init(crc_init);
module_exit(crc_exit);
