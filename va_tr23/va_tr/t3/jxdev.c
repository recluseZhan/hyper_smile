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

static int data = 0;
#define DEV_ID 231
#define DEVNAME "jxdev" 

__attribute__((aligned(4096))) unsigned char pp0[4096];

//test

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

//打开设备 
static int jxdev_open(struct inode *inode, struct file *filp) 
{   
    int ret;
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
    
        
    return 0;
} 
//写设备 
static ssize_t jxdev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset) 
{
    copy_from_user(pp0,buf,size);
    printk("jxdev read data %d\n",pp0[1]);
    kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)pp0,(unsigned long)0x77,(unsigned long)pp0,(unsigned long)pp0);
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
    int ret;
    ret = register_chrdev(DEV_ID,DEVNAME,&fops);
    if (ret < 0) {
 // xinxin     printk(KERN_EMERG DEVNAME " can't register major number.\n");
      return ret;
    }
     
    return 0;
}

static void __exit jxdev_exit(void)
{
    unregister_chrdev(DEV_ID,DEVNAME);
}

module_init(jxdev_init);
module_exit(jxdev_exit);
