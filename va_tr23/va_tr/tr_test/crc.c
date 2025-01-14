#include<linux/init.h>
#include<linux/module.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/sched.h>
#include<linux/export.h>
#include<linux/init_task.h>


MODULE_LICENSE("GPL");

uint32_t crc32(uint8_t* ptr,uint32_t len){

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
    printk("trampoline_en = 0x%lx",crc);
    return crc;
}

static int __init crc_init(void)
{
    uint8_t *a;
    a = 0xffffffffc074c000;
    uint32_t len = 0x5d;
    
    printk(KERN_ALERT"crc module is entering..\n");
    printk("crc address=%lx\n",(unsigned long)crc32);
    
    //printk("0x%lx\n", (unsigned long)crc32(a,len));

    
    //printk(KERN_ALERT"crc module is entering..\n");


    return 0;
}

static void __exit crc_exit(void)
{
    printk(KERN_ALERT"crc module is leaving..\n");
    
}
EXPORT_SYMBOL(crc32);

module_init(crc_init);
module_exit(crc_exit);
