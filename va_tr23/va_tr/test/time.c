#include <stdio.h>
unsigned long urdtsc()
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}
void main(){
    unsigned long t1,t2,ta,tb,tc;
    int sum=0;
    t1=urdtsc();
    for(int i=1;i<=100;i++){
        sum=sum+i;
    }
    t2=urdtsc();
    ta=(t2-t1)/3;
    
    t1=urdtsc();
    sum=1+1;
    t2=urdtsc();
    tb=(t2-t1)/3;
    
    t1=urdtsc();
    printf("hello");
    t2=urdtsc();
    tc=(t2-t1)/3;
    
    FILE *file1;
    file1 = fopen("report_test.txt","a");
    if(file1==NULL)
        return 1;
    fprintf(file1,"%ld\n",ta);
    fclose(file1);
    
    file1 = fopen("report_test1.txt","a");
    if(file1==NULL)
        return 1;
    fprintf(file1,"%ld\n",tb);
    fclose(file1);
    
    file1 = fopen("report_test2.txt","a");
    if(file1==NULL)
        return 1;
    fprintf(file1,"%ld\n",tc);
    fclose(file1);
}
