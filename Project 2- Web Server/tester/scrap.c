#include <fcntl.h> 
#include <stdio.h> 
#include <string.h>
#include <time.h>
#include<sys/types.h>
// #include<sys/socket.h>
#include<sys/stat.h>
#include <time.h>

#define MAXBUF 2048 // <- Don't change me; this is my lucky number
​
// Implemented & reviewed - assume it works correctly
void send_error(int connFd, Request *request, int code) { ... } 
void send_headers(int connFd, Request *request) { ... }
int is_valid_request(Request *request) { ... }
int file_exists(Request *request) { ... }
int open_filefd(Request *request) { ... }
​
// Only support GET and HEAD
void serve_http(int connFd) {
    char buf[MAXBUF];
    
    // Read 8K. The request is supposed to be at most 8K
    int bytesRead = read(connFd, buf, 8192);
    
    Request *request = parse(buf, MAXBUF, connFd);
    
    // Reject invalid requests
    if (request == NULL || !is_valid_request(request)) {
        send_error(connFd, request, 400);
        return ; // caller will close this connFd
    }
    
    if (strcmp(request->http_method, "GET") == 0 ||
        strcmp(request->http_method, "HEAD") == 0) {
        if (!file_exists(request)) { 
            send_error(connFd, request, 404);
            return;
        }
        send_headers(connFd, request); 
        if (strcmp(request->http_method, "GET") == 0) {
            int inputFd = open_filefd(request);
            while ((bytesRead = read(inputFd, buf, MAXBUF)) > 0) {
                write(connFd, buf, bytesRead);
            }
            close(inputFd);
        }
    }   
    else { 
        // reject non-HEAD/GET requests
        send_error(connFd, request, 501);
    }
    // caller will close this connFd
}





// int main()
// {


//     // typedef struct sockaddr SA;
//     // struct survival_bag{
//     //     struct sockaddr_storage clientAddr;
//     //     int connFd;
//     //     char* path;
//     // };

//     // printf("{ %d }", sizeof(sockaddr));







//     clock_t start, end;
//     double cpu_time_used;
//     start = clock();

//     int fd, sz;
//     FILE * fp;
//     char *c = (char *) calloc(100, sizeof(char));
  
  
//     //// OPTION 1: SYSTEM CALL read() open() = around 0.002000 this is slower
//     // fd = open("yuval.epub", O_RDONLY);
//     // if (fd < 0) { perror("r1"); exit(1); }
  
//     // while (read(fd, c, 1) > 0) {
//     //     printf("Those bytes are as follows: % s\n", c);
//     // } 


//     // OPTION 2: BUFFERED fread() fopen() = around 0.001000
//     // fp = fopen("yuval.epub", "r");
//     // if (fp < 0) { perror("r1"); exit(1); }

//     // while (fread(c, 1, 1, fp) > 0) {
//     //     printf("Those bytes are as follows: % s\n", c);
//     // } 





// // 2 bytes at a time

//     //// OPTION 1: SYSTEM CALL read() open() = around at 0.001000 still is slower
//     fd = open("yuval.epub", O_RDONLY);
//     if (fd < 0) { perror("r1"); exit(1); }
  
//     while (read(fd, c, 2) > 0) {
//         printf("Those bytes are as follows: % s\n", c);
//     } 


//     // OPTION 2: BUFFERED fread() fopen() = making 0.000000 frequently
//     // fp = fopen("yuval.epub", "r");
//     // if (fp < 0) { perror("r1"); exit(1); }

//     // while (fread(c, 1, 2, fp) > 0) {
//     //     printf("Those bytes are as follows: % s\n", c);
//     // } 



//     end = clock();
//     cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
//     printf("{ %f }", cpu_time_used);
  
// }