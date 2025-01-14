#include <stdio.h>
#include <fcntl.h>
#define DEVNAME "/dev/jxdev"
__attribute__((aligned(4096))) unsigned char worker_tr[4096];
void main(){
    int fd;
    int i;
    int a = 1;
    for(i=0;i<4096;i++){
        worker_tr[i]=3;
    }
    //worker_tr[3]=&a;
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    write(fd,worker_tr,sizeof(worker_tr));
    close(fd);
   
}

