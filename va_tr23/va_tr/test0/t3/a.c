#include<stdio.h>
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}
void main(){
    unsigned long t1,t2;
    t1 = urdtsc();
    int a = 1;
    int b = 2;
    int c;
    int i = 0;
    for(i=0;i<100;i++)
        c = a+b;
    printf("%d\n",c);
    t2=urdtsc();
    FILE *file1;
     file1 = fopen("dd.txt","a");
     
     fprintf(file1,"%ld\n",(t2-t1)*5/17);
     fclose(file1);
    
}
