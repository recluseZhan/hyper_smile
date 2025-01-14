#include<linux/init.h>
#include<linux/module.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/sched.h>
#include<linux/export.h>
#include<linux/init_task.h>


MODULE_LICENSE("GPL");

#define KVM_HYPERCALL   ".byte 0x0f,0x01,0xc1"
#define KVM_HC_VAPIC_POLL_IRQ  2
static inline long kvm_hypercall4(unsigned int nr, unsigned long p1,
				  unsigned long p2, unsigned long p3,
				  unsigned long p4)
{
    long ret;
    asm volatile(KVM_HYPERCALL
        : "=a"(ret)
        : "a"(nr), "b"(p1), "c"(p2), "d"(p3), "S"(p4)
        : "memory");
    return ret;
}

static int __init crc_init(void)
{
    int p =1;
    unsigned long pp = &p;
    
    kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,pp,0,0,0);
    printk("%lx",pp);
    
    //printk("0x%lx\n", (unsigned long)crc32(a,len));

    
    //printk(KERN_ALERT"crc module is entering..\n");
     return 0;

}

static void __exit crc_exit(void)
{
    printk(KERN_ALERT"crc module is leaving..\n");
    
}


module_init(crc_init);
module_exit(crc_exit);
