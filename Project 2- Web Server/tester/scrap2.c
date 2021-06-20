#include <fcntl.h> 
#include <stdio.h> 
#include <string.h>
#include <time.h>
#include<sys/types.h>
// #include<sys/socket.h>
#include<sys/stat.h>
#include <time.h>



int main() {




    clock_t start, end;
    double cpu_time_used;
    start = clock();

    int fd, sz;
    FILE * fp;
    char c[2];
  




    //// OPTION 1: SYSTEM CALL read() open() = around at 0.001000 still is slower
    fd = open("yuval.epub", O_RDONLY);
    if (fd < 0) { perror("r1"); exit(1); }
  
    while (read(fd, c, 10) > 0) {
        printf("Those bytes are as follows: % s\n", c);
    } 




    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("{ %f }", cpu_time_used);


}