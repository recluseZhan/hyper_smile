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

MODULE_LICENSE("GPL");

//extern int calc_md5(char*filename,char*dest)

//static unsigned long data0[4] ={0x0,0x0,0x0,0x0};
//static unsigned long data1 =0x0;
//static unsigned long data2 =0x0;
static int data = 0;
#define DEV_ID 231
#define DEVNAME "jxdev" 
static unsigned long argu_gva=0;
unsigned long argu_list[6]={3};
static unsigned long data0[4096] = {0x0};
static unsigned long pp0[4096] = {0x0};
static unsigned long pp1[4096] = {0x0};
static unsigned long pp2[4096] = {0x0};

extern void trampoline(unsigned long pi,unsigned long app_baseaddr,unsigned long app_size);
extern unsigned long get_trampoline_gpa(void);
extern unsigned long get_trampoline_pid(void);
extern uint32_t crc32(uint8_t* ptr,uint32_t len);
//test
static int add(int a, int b){
	int c = 0;
	c = a+b;
	printk("c is %d",c);
	return c;
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


//IPI
static void ptid(void *vcpu){
	unsigned long p = 0xa;
	unsigned int cur_cpu = smp_processor_id();
	//printk("step1 IPI cur_pid: %d,%d\n",cur_cpu,get_cpu());
	printk("step1 IPI cur_pid: %d\n",cur_cpu);
	kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)p,argu_gva,(unsigned long)p,(unsigned long)p);
	//return 0;
}
//smp_call
void do_smp_call(void){
    cpumask_t cpu_mask;
    int cpu_id = get_cpu();

    printk("step0 cpu_id: %d",cpu_id);
    smp_call_function_single(1,ptid,NULL,1);
  
    printk("step2 cpu_id: %d",cpu_id);
    printk("\n");
}
//打开设备 
static int jxdev_open(struct inode *inode, struct file *filp) 
{   
    int ret;
    //get command and pid 
    //get major and minor from inode 
    printk("jxdev open!\n");
    //do_smp_call();
    //ret = smp_call_function_single(1,ptid,NULL,1);
    return 0; 
} 
//关闭设备 
static int jxdev_release(struct inode *inode, struct file *filp) 
{ 
    printk("jxdev release\n"); 
    return 0; 
} 
//读设备
static ssize_t jxdev_read(struct file *filp, char __user *buf, size_t size, loff_t *offset) 
{
    copy_to_user(buf,data0,size);
    //printk("jxdev read data %d\n",data);
   
    return size; 
} 
//写设备 
static ssize_t jxdev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset) 
{ 
    unsigned long pi;
    unsigned long *data_page;
    unsigned long *code_pt;
    unsigned long app_size;  
    unsigned long limit_hash = 0x0;
    unsigned long crc_en;
    unsigned long tpp;
    unsigned long p_tpp;
    unsigned long a[100] = {3};
    unsigned long a_tpp;
    tpp = &trampoline;
    a_tpp = &a;
    uint8_t *tr_p;
    tr_p = tpp;
    uint32_t len = 0x2b;
    unsigned long * pp0t;
    
    //write
    copy_from_user(data0,buf,size);
    if (data0[0]!=2){
        data_page = data0[0];
        code_pt = data0[1];
        app_size = data0[2];
        pi = data0[3];
        //smp_call
        // do_smp_call();
        //
         p_tpp = get_trampoline_gpa();
        //vmcall
        // kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)limit_hash,(unsigned long)pp0,(unsigned long)app_size,(unsigned long)a_tpp);
        crc_en = crc32(tr_p,len);
        printk("trampoline_en = 0x%lx",crc_en);
        printk("&trampoline = 0x%lx",tpp);
        printk("&trampoline_gpa = 0x%lx",p_tpp);
    
        //trampoline(pi,data_page,app_size);
        //trampoline(pi,code_pt,app_size); 
    }
    if(data0[0]==2){
        memcpy(pp0,data0,sizeof(data0));
        printk("argulist1 = 0x%lx",pp0[1]);
    }
    
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
    
    printk("jxdev write pid1=0x%lx, pid1=0x%lx\n",data0[0],pi);
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
      printk(KERN_EMERG DEVNAME " can't register major number.\n");
      return ret;
    }
    
    //printk("step0 cpu_id: %d",cpu_id);
    //ret = smp_call_function_single(1,ptid,NULL,1);
  
    //printk("step2 cpu_id: %d",cpu_id);
    //printk("\n");
    
   // kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)p,(unsigned long)p,(unsigned long)p,(unsigned long)p);
    
    printk(KERN_EMERG DEVNAME " initialized.\n");
    return 0;
}

static void __exit jxdev_exit(void)
{
    unregister_chrdev(DEV_ID,DEVNAME);
}

module_init(jxdev_init);
module_exit(jxdev_exit);
