#include<linux/init.h>
#include<linux/module.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/sched.h>
#include<linux/export.h>
#include<linux/init_task.h>


MODULE_LICENSE("GPL");
extern void handler0(unsigned long vaddr,unsigned long t_pid);

void vtp(unsigned long vaddr,unsigned long t_pid){
    
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
    
   printk("pgd = 0x%lx *pgd = 0x%lx  \n", pgd, *pgd);


    pud = (unsigned long *)(((unsigned long)*pgd & PTE_PFN_MASK) + PAGE_OFFSET);
    pud = pud + ((vaddr>>30) & 0x1FF);
    printk("pud = 0x%lx *pud = 0x%lx  \n", pud, *pud);
    
    pmd = (unsigned long *)(((unsigned long)*pud & PTE_PFN_MASK) + PAGE_OFFSET);
    pmd = pmd + ((vaddr>>21) & 0x1FF);
    printk("pmd = 0x%lx *pmd = 0x%lx  \n", pmd, *pmd);

    pte = (unsigned long *)(((unsigned long)*pmd & PTE_PFN_MASK) + PAGE_OFFSET);
    pte = pte + ((vaddr>>12) & 0x1FF);
    
    pte_r = *pte;

    page_addr= (*pte) & PAGE_MASK;
    P_OFFSET=vaddr&~PAGE_MASK;
    paddr=page_addr|P_OFFSET;
    printk("pte = 0x%lx *pte = 0x%lx \n", pte, *pte);

    printk("page_addr=0x%lx,page_offset=0x%lx\n",page_addr,P_OFFSET);
    printk("vaddr=0x%lx,paddr=0x%lx\n",vaddr,paddr);

 //   __asm__ volatile("rsm":::);
    return;
}

static int __init v2p_init(void)
{
    unsigned long vaddr1 = &handler0;
   
    printk(KERN_ALERT"vaddr to paddr module is entering..\n");
    vtp(vaddr1,(unsigned long)current->pid);
    printk("pid=%d",current->pid);
    printk("add_base=0x%lx",vaddr1);

    return 0;
}

static void __exit v2p_exit(void)
{
    printk(KERN_ALERT"vaddr to paddr module is leaving..\n");
    
}
EXPORT_SYMBOL(vtp);

module_init(v2p_init);
module_exit(v2p_exit);
