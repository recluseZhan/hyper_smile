#include<linux/init.h>
#include<linux/module.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/sched.h>
#include<linux/export.h>
#include<linux/init_task.h>


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
static int count;
void check(unsigned long *vm_ssa,unsigned long *ssa,unsigned long *vm_anchor,unsigned long *anchor){
    
    int i;
    unsigned long t1,t2;
    t1=urdtsc();
    for(i=0;i<4096;i++){
        if(vm_ssa[i]!=ssa[i]){
            count++;
        }
        if(vm_anchor[i]!=anchor[i]){
            count++;
        }
        
    }
    t2=urdtsc();
    printk("anchor/ssa time: %ld\n",(t2-t1)*5/17);


}
void check2(unsigned long *vm_worker,unsigned long *worker){
    int i;
    unsigned long t1,t2;
    t1=urdtsc();
    for(i=0;i<4096;i++){
        if(vm_worker[i]!=worker[i]){
            count++;
        }
    }
    t2=urdtsc();
    printk("worker time: %ld\n",(t2-t1)*5/17);
}
static int __init check_init(void)
{
    unsigned long vm_ssa[4096];
    unsigned long vm_anchor[4096]; 
    unsigned long vm_worker[4096];
    
    unsigned long ssa[4096];
    unsigned long anchor[4096];
    unsigned long worker[4096];

    int i;
    unsigned long t1,t2;
    for(i=0;i<4096;i++){
        vm_ssa[i]=1;
        vm_anchor[i]=1;
        vm_worker[i]=2;
    }
    for(i=0;i<4096;i++){
        ssa[i]=vm_ssa[i];
        anchor[i]=vm_anchor[i];
        worker[i]=vm_worker[i];
    }
    check(vm_ssa,ssa,vm_anchor,anchor);
    
    printk(KERN_ALERT"check module is entering..\n");


    return 0;
}

static void __exit check_exit(void)
{
    printk(KERN_ALERT"check module is leaving..\n");
    
}
EXPORT_SYMBOL(check);

module_init(check_init);
module_exit(check_exit);
