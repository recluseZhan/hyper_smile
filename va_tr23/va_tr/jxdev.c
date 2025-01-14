#define KVM_HYPERCALL   ".byte 0x0f,0x01,0xc1"
#define KVM_HC_VAPIC_POLL_IRQ  2

#include <asm/processor.h>
#include <asm/alternative.h>
#include <linux/interrupt.h>
#include <uapi/asm/kvm_para.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/current.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <linux/ctype.h>
#include <linux/smp.h>

#include <asm/segment.h>
#include <linux/buffer_head.h>
#include <asm/processor.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");

//extern int calc_md5(char*filename,char*dest)

//static unsigned long data0[4] ={0x0,0x0,0x0,0x0};
//static unsigned long data1 =0x0;
//static unsigned long data2 =0x0;
static int data = 0;
#define DEV_ID 231
#define DEVNAME "jxdev" 
static unsigned long crc_ssa=0;
static unsigned long crc_anchor=0;
static unsigned long crc_worker=0;
__attribute__((aligned(4096))) uint8_t pp0[4096] = {0x0};
__attribute__((aligned(4096))) uint8_t pp1[4096] = {0x0};
__attribute__((aligned(4096))) uint8_t pp[4096] = {0x0};
__attribute__((aligned(4096))) uint8_t data_tr[4096] = {0};
__attribute__((aligned(4096))) uint8_t end[1000] = {0};
__attribute__((aligned(4096))) uint8_t test[4096] = {0};
unsigned long t_launch,t_building,t_entry;


extern unsigned long trampoline(unsigned long pi,unsigned long app_baseaddr,unsigned long app_size);
extern unsigned long get_trampoline_gpa(void);
extern unsigned long get_trampoline_pid(void);
extern uint32_t crc32(uint8_t* ptr,uint32_t len);
unsigned long count_time(void);
//test
static int add(int a, int b){
	int c = 0;
	c = a+b;
	// xinxin printk("c is %d",c);
	return c;
}
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}

//vmcall
static inline long kvm_hypercall0(unsigned int nr)
{
	long ret;
	asm volatile(KVM_HYPERCALL
		     : "=a"(ret)
		     : "a"(nr)
		     : "memory");
	return ret;
}

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

//
static int launch_time(void){
    
     return 0;
}
//IPI
static void ptid2(void *vcpu){
    
    //kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)test,(unsigned long)0,(unsigned long)0,(unsigned long)0);
    //printk("hello%x",test[0]);
    int i;        
    for(i=0;i<30;i++)
        printk("0x%x ",test[i]);
    
}
static void ptid(void *vcpu){
	unsigned long t1,t2;
	unsigned long p = 0xa;
	unsigned int cur_cpu = smp_processor_id();
	kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)p,(unsigned long)pp1,(unsigned long)data_tr,(unsigned long)0xbb);
	printk("test p=%lx",p);
	//printk("step1 IPI cur_pid: %d,%d\n",cur_cpu,get_cpu());
	// xinxin printk("step1 IPI cur_pid: %d\n",cur_cpu);
	//t1=urdtsc();
	/*
	if(pp[700]==0xaa){
	    kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,crc_ssa,(unsigned long)pp0,(unsigned long)0xaa,(unsigned long)0xaa);
	}
	if(pp[700]==0xbb){
	    kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,crc_anchor,(unsigned long)pp1,(unsigned long)0xbb,(unsigned long)0xbb);
	}
	if(pp[700]==0xdd){
	     kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,crc_worker,(unsigned long)data_tr,(unsigned long)0xdd,(unsigned long)0xdd);
	}
	t2=urdtsc();
	*/
        //printk("hypercall enter time(ns) : %ld \n ", (t2-t1));
	//return 0;
}
//smp_call
void do_smp_call(void){
    cpumask_t cpu_mask;
    int cpu_id = get_cpu();

  // xinxin  printk("step0 cpu_id: %d",cpu_id);
    smp_call_function_single(1,ptid,NULL,1);
    //smp_call_function_single(2,ptid2,NULL,1);
    
  // xinxin  printk("step2 cpu_id: %d",cpu_id);
  // xinxin  printk("\n");
}
//打开设备 
static int jxdev_open(struct inode *inode, struct file *filp) 
{   
    int ret;
    //get command and pid 
    //get major and minor from inode 
   // xinxin printk("jxdev open!\n");
    //do_smp_call();
    //ret = smp_call_function_single(1,ptid,NULL,1);
    return 0; 
} 
//关闭设备 
static int jxdev_release(struct inode *inode, struct file *filp) 
{ 
 // xinxin   printk("jxdev release\n"); 
    return 0; 
} 
//读设备
static ssize_t jxdev_read(struct file *filp, char __user *buf, size_t size, loff_t *offset) 
{
      //copy_to_user(buf,pp0,size);
    //printk("jxdev read data %d\n",data);
        asm volatile(
        "mov %%cr3,%%rax\n\t"
        "mov %%rax,%%cr3\n\t"
        :::
        );
    
    return 0;
    //return size; 
} 
//写设备 
static ssize_t jxdev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset) 
{ 
    unsigned long pi;
    unsigned long *data_page;
    unsigned long *code_pt;
    unsigned long app_size;  
    
    unsigned long crc_en;
    unsigned long p_tpp;

    uint8_t *tr_p;
    uint32_t len = 0x2b;
    unsigned long crc_code;
    int i;
    unsigned long t1,t2,t;    
    copy_from_user(pp,buf,size);
  
    //write
   
    if(pp[700]==0xcc){
        data_page=pp[0];
        code_pt=pp[1];
        app_size=pp[2];
        pi=pp[3];
        t1=urdtsc();
        t = trampoline(pi,data_page,app_size);
        t2=urdtsc();
        t_building=t2-t1;
       
    }
    else{
    
        if(pp[700]==0xaa){
            copy_from_user(pp0,buf,size);    
            crc_ssa = crc32(pp0,0x100);//0x100
     // xinxin       printk("ssa_hash=%lx\n",crc_ssa);
            //do_smp_call();
        }
        if(pp[700]==0xbb){
            copy_from_user(pp1,buf,size);    
            crc_anchor = crc32(pp1,40);//40
      // xinxin      printk("anchor_hash=%lx\n",crc_anchor);
            //do_smp_call();
        }
        if(pp[700]==0xdd){
            //copy_from_user(data_tr,buf,size);
            //crc_worker = crc32(data_tr,0x100);
      // xinxin      printk("anchor_hash=%lx\n",crc_worker);    
            do_smp_call();
            
        }
       // t2=urdtsc();
       // printk("t1=%ld,t2=%ld ,two times of urdtsc time : %ld \n ", t1,t2,(t2-t1)/3);
        //
        //vmcall
        // kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)limit_hash,(unsigned long)pp0,(unsigned long)app_size,(unsigned long)a_tpp);
       
    // xinxin    printk("&trampoline = 0x%lx",&trampoline);
   // xinxin     printk("&trampoline_gpa = 0x%lx",p_tpp);
    }
    
        //trampoline(pi,data_page,app_size);
        //trampoline(pi,code_pt,app_size); 
   
    
    //argu_gva = data0[1];
    //pp0t = data0[1];
    //memcpy(pp0,argu_gva,sizeof(pp0));
   // printk("argu_gva[0] = 0x%lx",data0[1]);
    //code_pt = data0[1];
   // app_size = data0[2];
  //  pi = data0[3];
  //  memcpy(pp0,pp0t,100);
  //  printk("argulist1 = 0x%lx",pp0[1]);
   // argu_gva = argu_list;
    
    
    
  //  int a = 1;
  //  int *p = &a;  
 //   pid1 = 2740;
  //  printk(KERN_ALERT"(enclave)\n");
  //  vaddr1 = 0x7ffe6aee280c;
  //  handler0(vaddr1,pid1);
   // int a = 1;
   // int b = 2;
   // int c =0;
   // unsigned long p = 0x0;
   // c=add(a,b);
  //  printk("c=%d",c);
	
    //copy_from_user(data0,buf,size);
    //vaddr1 = data0[0];
    //pid1 = data0[1];
    //handler0(vaddr1,pid1);    
  //  printk("dddd=%d",c);
  //  kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)p,(unsigned long)p,(unsigned long)p,(unsigned long)p);
    
   // printk("jxdev write pid1=0x%lx, pid1=0x%lx\n",data0[0],pi);
    return size; 
} 
//操作方法集 
static struct file_operations fops = {
    .owner = THIS_MODULE, 
    .open = jxdev_open, 
    .release= jxdev_release, 
    .read = jxdev_read, 
    .write = jxdev_write, 
}; 

//cdev设备模块初始化 
static int __init jxdev_init(void) 
{  
   // unsigned long t1,t2;
    //t1=urdtsc();
    //int a = 1;
    unsigned long p = 0xa;
/*
    unsigned long vaddr1=0;
    unsigned long pid1=0;    
    pid1 = 2740;
    printk(KERN_ALERT"(enclave)\n");
    vaddr1 = 0x7ffe6aee280c;
    handler0(vaddr1,pid1);
*/    
    int ret;
    
    cpumask_t cpu_mask;
    int cpu_id = get_cpu();
    
    //printk("p=%lx",(unsigned long)p);
    ret = register_chrdev(DEV_ID,DEVNAME,&fops);
 
    
    if (ret < 0) {
 // xinxin     printk(KERN_EMERG DEVNAME " can't register major number.\n");
      return ret;
    }
    
    unsigned long t1,t2;
   // printk("end877=%lx\n",end[877]);
    //printk("step0 cpu_id: %d",cpu_id);
    //ret = smp_call_function_single(1,ptid,NULL,1);
  
    //printk("step2 cpu_id: %d",cpu_id);
    //printk("\n");
    
   // kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)p,(unsigned long)p,(unsigned long)p,(unsigned long)p);
    t1=urdtsc();
   // kvm_hypercall0(KVM_HC_VAPIC_POLL_IRQ);
    t2=urdtsc();
   // printk("vmtime=%ld\n",(t2-t1)/3);
   // xinxin printk(KERN_EMERG DEVNAME " initialized.\n");
   // t2=urdtsc();
   // printk("launcher time:%ld\n",(t2-t1));
   //launch_time();
    return 0;
}

static void __exit jxdev_exit(void)
{
    unregister_chrdev(DEV_ID,DEVNAME);
}

module_init(jxdev_init);
module_exit(jxdev_exit);
