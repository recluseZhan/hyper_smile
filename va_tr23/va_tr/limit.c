#define KVM_HYPERCALL   ".byte 0x0f,0x01,0xc1"
#define KVM_HC_VAPIC_POLL_IRQ  1 
#include<linux/init.h>
#include<linux/module.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/sched.h>
#include<linux/export.h>
#include<linux/init_task.h>
#include<linux/delay.h>


MODULE_LICENSE("GPL");
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

unsigned long app_baseaddr;
unsigned long app_size;
unsigned long secs_addr;
unsigned long erlange;

unsigned long t_pid;
struct mm_struct *c_mm;
int   index;

struct PageInfo{
   unsigned long vir_addr;
   unsigned long phy_addr;
};
__attribute__((aligned(4096))) int random_data_page[1024]={0xaa};

void clear_ifg(void *info)
{
	__asm("cli \n"
		:
		:
		:
		);
     unsigned int cpu;
     cpu = task_cpu(current);
  // xinxin   printk("Trampoline: %x",cpu);
}

void start_ifg(void *info)
{
	__asm("sti \n"
		:
		:
		:
		);
}

void get_appbase(unsigned long t_pid)
{
      struct task_struct *task,*p;
      struct list_head *pos;
      int count=0;
// xinxin      printk(KERN_ALERT"getpgd:\n");
      task=&init_task;
      list_for_each(pos,&task->tasks)
      {
               p=list_entry(pos, struct task_struct, tasks);
               count++;
		if (p->pid == t_pid )
		{
		    app_baseaddr = p->mm->mmap_base;
		    app_size  = p->mm->task_size;
		    break;
		}
       }
       //unsigned long iii = app_size-app_baseaddr;
// xinxin      printk(",p->pid=0x%lx,t_pid = 0x%lx\n",p->pid,t_pid);
 // xinxin     printk("app_baseaddr = 0x%lx\n",app_baseaddr);
 // xinxin     printk("app_size = 0x%lx\n",app_size);
      //printk("app_size = 0x%lx\n",iii);
      return ;
}


void non_v2p(unsigned long vaddr,unsigned long t_pid,struct PageInfo pinfo[],int i){
    
    unsigned long paddr=0;
    unsigned long page_addr=0;
    unsigned long P_OFFSET=0;
    unsigned long *pgd, *pud, *pmd, *pte;
    unsigned long *pte_c;
    unsigned long pte_mask;
    unsigned long *pte_rp;
    unsigned long pte_r;
     
    struct task_struct *task,*p;
    struct list_head *pos;
    int count = 0;
 
    task = &init_task;
    list_for_each(pos,&task->tasks)
    {
        p=list_entry(pos, struct task_struct, tasks);
        count++;
	    if (p->pid == t_pid)
	    {
		    pgd = (unsigned long)p->mm->pgd;
		    pgd = pgd + ((vaddr>>39) & 0x1FF);
		    break;
	    }
    }
    
 // xinxin   printk("pgd = 0x%lx *pgd = 0x%lx  \n", pgd, *pgd);


    pud = (unsigned long *)(((unsigned long)*pgd & PTE_PFN_MASK) + PAGE_OFFSET);
    pud = pud + ((vaddr>>30) & 0x1FF);
 // xinxin   printk("pud = 0x%lx *pud = 0x%lx  \n", pud, *pud);
    
    pmd = (unsigned long *)(((unsigned long)*pud & PTE_PFN_MASK) + PAGE_OFFSET);
    pmd = pmd + ((vaddr>>21) & 0x1FF);
 // xinxin   printk("pmd = 0x%lx *pmd = 0x%lx  \n", pmd, *pmd);

    pte = (unsigned long *)(((unsigned long)*pmd & PTE_PFN_MASK) + PAGE_OFFSET);
    pte = pte + ((vaddr>>12) & 0x1FF);
    
    //
    //pte_rp = (unsigned long *)pte;  
    pinfo[i].vir_addr = vaddr;
    pinfo[i].phy_addr = *pte;
    
    *pte = *pte &0xfffffffffffffffe;
    
    __flush_tlb_local();
    __flush_tlb_global();


    page_addr= (*pte) & PAGE_MASK;
    P_OFFSET=vaddr&~PAGE_MASK;
    paddr=page_addr|P_OFFSET;
    
   // __flush _tlb_all();
   // __flush_tlb_local();
   // __flush_tlb_global();
    
 // xinxin   printk("pte = 0x%lx *pte = 0x%lx \n", pte, *pte);
 // xinxin   printk("page_addr=0x%lx,page_offset=0x%lx\n",page_addr,P_OFFSET);    
 // xinxin   printk("vaddr=0x%lx,paddr=0x%lx\n",vaddr,paddr);

   // *pte = pte_r;
 //   __asm__ volatile("rsm":::);
    return;
}

void en_v2p(unsigned long vaddr,unsigned long t_pid,unsigned long phy_addr){
    
    unsigned long paddr=0;
    unsigned long page_addr=0;
    unsigned long P_OFFSET=0;
    unsigned long *pgd, *pud, *pmd, *pte;
    unsigned long *pte_c;
    unsigned long pte_mask;
    unsigned long *pte_rp;
    unsigned long pte_r;
    
    struct task_struct *task,*p;
    struct list_head *pos;
    int count = 0;
    
    task = &init_task;
    list_for_each(pos,&task->tasks)
    {
        p=list_entry(pos, struct task_struct, tasks);
        count++;
	    if (p->pid == t_pid)
	    {
		    pgd = (unsigned long)p->mm->pgd;
		    pgd = pgd + ((vaddr>>39) & 0x1FF);
		    break;
	    }
    }
    
 // xinxin   printk("pgd = 0x%lx *pgd = 0x%lx  \n", pgd, *pgd);


    pud = (unsigned long *)(((unsigned long)*pgd & PTE_PFN_MASK) + PAGE_OFFSET);
    pud = pud + ((vaddr>>30) & 0x1FF);
 // xinxin   printk("pud = 0x%lx *pud = 0x%lx  \n", pud, *pud);
    
    pmd = (unsigned long *)(((unsigned long)*pud & PTE_PFN_MASK) + PAGE_OFFSET);
    pmd = pmd + ((vaddr>>21) & 0x1FF);
   // xinxin printk("pmd = 0x%lx *pmd = 0x%lx  \n", pmd, *pmd);

    pte = (unsigned long *)(((unsigned long)*pmd & PTE_PFN_MASK) + PAGE_OFFSET);
    pte = pte + ((vaddr>>12) & 0x1FF);
    
    //
   // pte_rp = (unsigned long *)pte;  
    *pte = phy_addr;
    
    //__flush_tlb_local();
    //__flush_tlb_global();


    page_addr= (*pte) & PAGE_MASK;
    P_OFFSET=vaddr&~PAGE_MASK;
    paddr=page_addr|P_OFFSET;
    
   // __flush _tlb_all();
   // __flush_tlb_local();
   // __flush_tlb_global();
    
 // xinxin   printk("pte = 0x%lx *pte = 0x%lx \n", pte, *pte);
 // xinxin   printk("page_addr=0x%lx,page_offset=0x%lx\n",page_addr,P_OFFSET);    
 // xinxin   printk("vaddr=0x%lx,paddr=0x%lx\n",vaddr,paddr);

   // *pte = pte_r;
 //   __asm__ volatile("rsm":::);
    return;
}

struct PageInfo pinfo[1000];

unsigned long laucher_non_v2p(unsigned long app_baseaddr,unsigned long t_pid,unsigned long app_size){

   unsigned long page_size = 0x1000;
   //
   unsigned long current_vaddr;
   int i = 0;
   //app_size = 0x90200;
   for(page_size=0; page_size<app_size; page_size= page_size+0x1000)
   {
        __flush_tlb_local();
        __flush_tlb_global();
        
        current_vaddr = app_baseaddr + page_size;

        if(current_vaddr == &random_data_page){
             index = i;
             break;
        }

        non_v2p(current_vaddr,t_pid,pinfo,i);
  // xinxin      printk("pinfo[%d]_vaddr=%lx,paddr=%lx\n",i,pinfo[i].vir_addr,pinfo[i].phy_addr);
        i++;
  // xinxin      printk("\n");
   }
/*
   for(page_size=0x3000; page_size<erlange; page_size= page_size+0x1000)
   {

        current_vaddr = secs_addr + page_size;
        non_v2p(current_vaddr,t_pid,pinfo[i]);
        i++;
   }
*/
   index = i;
  // xinxin printk("index = %d",index);
   
   return 0;
}

unsigned long launch_en_v2p(struct PageInfo pinfo[index],unsigned long t_pid){

    int i;
    for(i=0; i<index; i++)
    {
            __flush_tlb_local();
            __flush_tlb_global();
        
            en_v2p(pinfo[i].vir_addr,t_pid,pinfo[i].phy_addr);
  // xinxin          printk("\n");
    }
    return 0;

}

unsigned long v2p(unsigned long vaddr,unsigned long t_pid){
    
    unsigned long paddr=0;
    unsigned long page_addr=0;
    unsigned long P_OFFSET=0;
    unsigned long *pgd, *pud, *pmd, *pte;
    unsigned long *pte_c;
    unsigned long pte_mask;
    unsigned long *pte_rp;
    unsigned long pte_r;
    
    struct task_struct *task,*p;
    struct list_head *pos;
    int count = 0;
    
    task = &init_task;
    list_for_each(pos,&task->tasks)
    {
        p=list_entry(pos, struct task_struct, tasks);
        count++;
	    if (p->pid == t_pid)
	    {
		    pgd = (unsigned long)p->mm->pgd;
		    pgd = pgd + ((vaddr>>39) & 0x1FF);
		    break;
	    }
    }
    
 // xinxin  printk("pgd = 0x%lx *pgd = 0x%lx  \n", pgd, *pgd);


    pud = (unsigned long *)(((unsigned long)*pgd & PTE_PFN_MASK) + PAGE_OFFSET);
    pud = pud + ((vaddr>>30) & 0x1FF);
  // xinxin  printk("pud = 0x%lx *pud = 0x%lx  \n", pud, *pud);
    
    pmd = (unsigned long *)(((unsigned long)*pud & PTE_PFN_MASK) + PAGE_OFFSET);
    pmd = pmd + ((vaddr>>21) & 0x1FF);
  // xinxin  printk("pmd = 0x%lx *pmd = 0x%lx  \n", pmd, *pmd);

    pte = (unsigned long *)(((unsigned long)*pmd & PTE_PFN_MASK) + PAGE_OFFSET);
    pte = pte + ((vaddr>>12) & 0x1FF);
    
    pte_r = *pte;

    page_addr= (*pte) & PAGE_MASK;
    P_OFFSET=vaddr&~PAGE_MASK;
    paddr=page_addr|P_OFFSET;
// xinxin    printk("pte = 0x%lx *pte = 0x%lx \n", pte, *pte);

 // xinxin   printk("page_addr=0x%lx,page_offset=0x%lx\n",page_addr,P_OFFSET);
 // xinxin   printk("vaddr=0x%lx,paddr=0x%lx\n",vaddr,paddr);


 //   __asm__ volatile("rsm":::);
    return paddr;
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
unsigned long trampoline(unsigned long pi,unsigned long app_baseaddr,unsigned long app_size){
    //unsigned long  va = 0x7ffe6a2f0f08;
    //unsigned long pi = 9349;
    //get_appbase(pi);
    //c_mm= (struct mm_struct*)kmalloc(sizeof(struct mm_struct),GFP_KERNEL);
    //getmem_space(c_mm);
    asm volatile(
        "mov %%cr3,%%rax\n\t"
        "mov %%rax,%%cr3\n\t"
        :::
    );
    unsigned long t1,t2,t;
    t1=urdtsc();
    laucher_non_v2p(app_baseaddr,pi,app_size);
    t2=urdtsc();
    t=(t2-t1);
    //printk("times of trampoline time : %ld \n ", (t2-t1)/3*4);
    
    launch_en_v2p(pinfo,pi);
   // msleep(100000);
   /*
   struct file *file;
   ssize_t ret;        
   file = filp_open("tt.txt",O_WRONLY|O_CREAT,0644);
   if(IS_ERR(file)){
       printk("file error\n");
       return -1;
   }
   const int *data=t;
   const char *dd="\n";
   size_t len = strlen(data);
   ret=kernel_write(file,data,len,&file->f_pos);
   filp_close(file,NULL);
   
   file = filp_open("tt.txt",O_WRONLY|O_CREAT,0644);
   len = strlen(dd);
   ret=kernel_write(file,dd,len,&file->f_pos);
   filp_close(file,NULL);*/
   //
  // xinxin  printk("tram_end");
    return t;
   // non_v2p(va,pi);
}

unsigned long get_trampoline_gpa(void){
    unsigned long vaddr1;
    unsigned long paddr1;
    vaddr1 = &trampoline;

    paddr1 = v2p(vaddr1,(unsigned long)current->pid);
   // xinxin printk("trampoline_pid=%d",current->pid);
   // xinxin printk("trampoline_gva=0x%lx",vaddr1);
  // xinxin  printk("trampoline_gpa=0x%lx",paddr1);
    return paddr1;
}
unsigned long get_trampoline_pid(void){
  // xinxin  printk("trampoline_pid=%d",current->pid);
    return (unsigned long)current->pid;
}

unsigned long count_time(void){
    unsigned long start = clear_ifg;
    unsigned long end = get_trampoline_pid;
// xinxin    printk("start=%lx,end=%lx,end-start=%ld\n",start,end,end-start);
    return start;
    
}
unsigned long t_start,t_end;
static int __init limit_init(void)
{   
    __flush_tlb_local();
    __flush_tlb_global();
    t_start=urdtsc();
    unsigned long t1,t2;
    
   // printk(KERN_ALERT"handle_limit module is entering..\n");
    
    //unsigned long pi = 9349
  
    
   
    
    //preempt_disable();
    //local_irq_disable();
    
    //va = 0x7ffc93231be8;
   // pi=3543;
    //printk("p=0x%lx",p); 
    //native_write_cr3(__native_read_cr3());
   // __flush_tlb_local();
   // __flush_tlb_global();
    //__flush_tlb_one_user(va);
    //handler0(va,pi);
    //__flush_tlb_one_user(va);
  // __flush_tlb_local();
   //__flush_tlb_global();
    //vtp(va,pi);
    //handler0((unsigned long)p,(unsigned long)current->pid);
  //  printk("p=0x%lx",p);
   
  /*
    vaddr1 = &trampoline;

    paddr1 = v2p(vaddr1,(unsigned long)current->pid);
    printk("trampoline_pid=%d",current->pid);
    printk("trampoline_gva=0x%lx",vaddr1);
    printk("trampoline_gpa=0x%lx",paddr1);
    
 */
    
    //kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ,(unsigned long)paddr1,(unsigned long)len1,(unsigned long)paddr1,(unsigned long)paddr1);
   // __flush_tlb_local();
   // __flush_tlb_global();
   // local_irq_enable();
   // preempt_enable();
   count_time();
   
  // xinxin printk(KERN_ALERT"handle_limit module is entering..\n");

    return 0;
}

static void __exit limit_exit(void)
{
 //   printk(KERN_ALERT"handle_limit module is leaving..\n");

    
}
EXPORT_SYMBOL(trampoline);
EXPORT_SYMBOL(get_trampoline_gpa);
EXPORT_SYMBOL(get_trampoline_pid);
EXPORT_SYMBOL(count_time);


module_init(limit_init);
module_exit(limit_exit);
