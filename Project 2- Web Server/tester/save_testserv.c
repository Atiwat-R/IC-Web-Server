// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <limits.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <assert.h>
// #include <unistd.h>
// #include <pthread.h>
// #include <malloc.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <sys/wait.h>
// #include <ctype.h>
// #include <sys/mman.h>
// #include <string.h>
// #include <errno.h>
// #include <dirent.h>
// #include <signal.h>
// #include "pcsa_net.h"
// #include "parse.h"

// /* Rather arbitrary. In real life, be careful with buffer overflow */
// #define MAXBUF 8192  

// void enqueue(int data);
// void* dequeue(void* ptr);

// typedef struct sockaddr SA;
// struct survival_bag{
// 	struct sockaddr_storage clientAddr;
// 	int connFd;
// 	char* path;
// };

// /*global vars*/
// pthread_mutex_t lock;
// pthread_cond_t emp;
// int sock =0;
// int max_threads =0;
// int global_count=0;
// int finished =1;
// int length=0; // number of queue

// /*Queue*/
// struct node{
//     int info;
//     struct node *ptr;
// }*head,*tail;
// struct sigaction ready, last;









// void respond_GET_HEAD(int connFd, char *filePath, char *uri, int isGet) {
//     char buf[MAXBUF];
// 	struct stat sb;
// 	printf("uri (%s)\n",uri);
//     char tmpUri[MAXBUF];
// 	strcpy(tmpUri,uri);

//     // TODO: SHOW MORE HEADER INFORMATION, EG DATE, MODIFIED ON, ETC.
//     char *extension = strchr(tmpUri, '.');
//     // check if ext is null
//     if (strcmp(extension,"")==0 || extension == NULL) {
//         printf("\n!!! File lacks Extension !!!\n");
//         exit(1);
//     }

// 	printf("extension = (%s)\n",extension);	
// 	char mimeType[MAXBUF];
// 	if(strcmp(extension,".html")==0 || strcmp(extension,".htm")==0){ // ori
// 		strcpy(	mimeType , "text/html");
// 	}
//     else if(strcmp(extension,".css")==0){
// 		strcpy(	mimeType , "text/css");
// 	}
// 	else if(strcmp(extension,".txt")==0){ // plain
// 		strcpy(	mimeType , "text/plain");
// 	}
//     else if(strcmp(extension,".js")==0){ // javascipt
// 		strcpy(	mimeType , "text/javascript");
// 	}
// 	else if(strcmp(extension,".jpg")==0 || strcmp(extension,".jpeg")==0){ // ori
// 		strcpy(mimeType,"image/jpeg");
// 	}
//     else if(strcmp(extension,".png")==0){
// 		strcpy(	mimeType , "image/png");
// 	}
//     else if(strcmp(extension,".gif")==0){
// 		strcpy(	mimeType , "image/gif");
// 	}
//     // support more ext

//     // Date
//     time_t t = time(NULL);
//     struct tm tm = *localtime(&t);
//     printf("Date: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

// 	printf("mimeType (%s)\n", mimeType);
// 	char newBuf[MAXBUF];
// 	sprintf(newBuf,"%s%s",filePath,uri);
// 	printf("path (%s)\n",newBuf);
//     int inputFd = open(newBuf,O_RDONLY);
//     fstat(inputFd, &sb);
//     size_t sizeOfFile = sb.st_size;
// 	printf("sizeOfFile: %lu\n",sizeOfFile);
//     printf("Last Modified: %s", ctime(&sb.st_mtime));
//     // printf("Last Modified: %lld\n", (long long) lastMod(NULL));



//     char dateBuf[MAXBUF];
//     time_t now = time(0);
//     struct tm tm2 = *gmtime(&now);
//     strftime(dateBuf, sizeof dateBuf, "%a, %d %b %Y %H:%M:%S %Z", &tm2);
//     printf("Time is: [%s]\n", dateBuf);

// // "Date: %s\r\n"


//     sprintf(buf, 
//             "HTTP/1.1 200 OK\r\n"
//             "Server: icws\r\n"
//             "Connection: close\r\n"
//             "Content-length: %lu\r\n"
//             "Content-type: %s\r\n\r\n", sizeOfFile,mimeType);

// 	printf("buf: \n%s",buf);
//     printf("--HEADER------------------------------------------------------------------\n\n");       
// 	char content[MAXBUF];
//     write_all(connFd, buf, strlen(buf));
// 	ssize_t numRead;

//     // while (read_line(inputFd, content, MAXBUF) > 0) {
//     //     write_all(connFd,content,MAXBUF);
//     //     if (strcmp(buf, "\r\n") == 0) break;    
//     // }


//     // for HEAD
//     if (isGet) {
//         while((numRead = read(inputFd,content,MAXBUF))>0){	
//             // printf("[] %s ]", content);
//             write_all(connFd,content,MAXBUF);
//         } 
//     }

// 	close(inputFd);
// }





// void serve_http(int connFd,char *path) {
//     char buf[MAXBUF];


//     // this read the header
//     //
// 	printf("path (%s)\n",path);	
//     if (!read_line(connFd, buf, MAXBUF)) 
//         return ;  /* Quit if we can't read the first line */

//     printf("LOG: %s\n", buf);



//     /* [METHOD] [URI] [HTTPVER] */
//     // This is default method for parsing requests
//     char method[MAXBUF], uri[MAXBUF], httpVer[MAXBUF];
//     sscanf(buf, "%s %s %s", method, uri, httpVer);

//     // puts("\n-------------------------------------------------------------\n");
//     // printf("(%s)\n", method);
//     // printf("(%s)\n", uri);
//     // printf("(%s)\n", httpVer);
//     // puts("\n-------------------------------------------------------------\n");

//     int readRet;
//     int totalReqSize = 0;
//     // Reads the reast of the header into buf
// 	while((readRet = read_line(connFd,buf,MAXBUF)) > 0) {

//         totalReqSize = totalReqSize + readRet;
//         if (totalReqSize > MAXBUF) { // Check if request size is too big
//             printf("\n!!! Request too big !!!\n");
//             exit(400);
//         }
//         printf("++LOG in loop: %s\n", buf);
//         if (strcmp(buf, "\r\n") == 0) break;

//     }

            










//     // //////////////////////////////////////////// aj parser 

//     // /* Pasing using starter code's parser */

//     // printf("\nGAGXYXGYAXGAXXGA %s XGYAXGYXGAGXYAXGYX\n", path);

//     // // int fd_in = open(path, O_RDONLY); // open request as specified by the given input path
//     // int index;

//     // // if (fd_in < 0) { // In case of failed file opening
//     // //     printf("Failed to open the file\n");
//     // //     return 0;
//     // // }

//     // int readRet;
//     // int totalReqSize = 0;

//     // // Accumulate size from read and compare it to MAXBUF, if over change size of buf as well
//     // readRet = read(connFd,buf,MAXBUF);


// 	// // while((readRet = read_line(connFd,buf,MAXBUF))>0){

//     // //     // Check of request size is too big
//     // //     totalReqSize = totalReqSize + readRet;
//     // //     if (totalReqSize > MAXBUF) {
//     // //         printf("\n!!! Request too big !!!\n");
//     // //         exit(1);
//     // //     }
//     // //     printf("++LOG in loop: %s\n", buf);
//     // //     if (strcmp(buf, "\r\n") == 0) break;
//     // // }



//     // // Check if Request size is too large. Still WIP
//     // // while ((readRet = read(connFd,buf,MAXBUF)) > 0) { // This is obtaining the header to buf
//     // //     //readRet = read(connFd,buf,MAXBUF);
//     // //     totalReqSize += readRet;
//     // //     if (totalReqSize > MAXBUF) {
//     // //         printf("!!! Request Size too Large !!!\n");
//     // //         exit(0);
//     // //     }
//     // //     printf("- %d\n", readRet);
//     // // }

//     // // printf("--BUFFER--------------------------------------------------------\n");
//     // // printf("\n%s\n", buf);
//     // // printf("----------------------------------------------------------------\n");


//     // Request *request = parse(buf,readRet,connFd);
//     // if (request == NULL) { // Check if what is parsed is NULL
//     //     printf("!!! NULL Request !!!\n");
//     //     exit(0);
//     // }
    

//     // //Parse the buffer to the parse function. You will need to pass the socket fd and the buffer would need to
//     // //be read from that fd
    
//     // // Store parsed data

//     // // if (request)


//     // char method[MAXBUF], uri[MAXBUF], httpVer[MAXBUF];
//     // strcpy(method, request->http_method);
//     // strcpy(uri, request->http_uri);
//     // strcpy(httpVer, request->http_version);

//     // /////////////////////////// Unchanged here on







//     // If its a GET request
//     if (strcasecmp(method, "GET") == 0 &&
//             uri[0] == '/') {
//         printf("--HEADER------------------------------------------------------------------\n\n");
//         printf("LOG: GET Request\n");
//         respond_GET_HEAD(connFd,path,uri,99);
//     }
//     else if (strcasecmp(method, "HEAD") == 0 &&
//             uri[0] == '/') {
//         printf("LOG: HEAD Request\n");
//         respond_GET_HEAD(connFd,path,uri,0);
//     }
//     else {
//         printf("LOG: Unknown request\n");
//         char bufx[MAXBUF];
//         sprintf(bufx, "LOG: Unknown request\r\n\r\n");
//         // printf("+++++++%s", bufx);
//         write(connFd,bufx,MAXBUF);
//         exit(501); // Error 501: Method Not Implemented
//     }

//     // free(buf);
//     // free(request->headers); // corrupted size vs. prev_size Aborted (core dumped)
//     // free(request);



    
// }

// ////// OLD CON HANDLER
// // void* conn_handler(void *args){
// // 	// struct survival_bag *context = (struct survival_bag*) args;
// // 	// pthread_detach(pthread_self());
// // 	// serve_http(context->connFd, context->path);
// // 	// close(context->connFd);
// // 	// free(context);
// // 	// return NULL;	

// //     int context = (int) args; 
// //     // printf("Thread-based server A\n");
// //     printf(">> %d\n", context);
// //     // printf("Thread-based server C\n");
// // }









// // /*enqueue*/
// // void enqueue(int data)
// // {
// //     int rc;
// //     struct node* temp;

// //     temp = (struct node *)malloc(1*sizeof(struct node));
// //     if(temp == NULL){
// //         printf("Error: couldn't allocate memory\n");
// //         exit(1);
// //     }
// //     temp->ptr = NULL;
// //     temp->info = data;

// //     if (pthread_mutex_lock(&lock) !=0){
// //         printf("Error: failed to lock\n");
// //         exit(1);
// //     } 



// //     if (length == 0) /*empty queue*/
// //     {
// //         head = temp;

// //     }else 
// //     {
// //         tail->ptr = temp; 
// //     }

// //     tail = temp;
// //     length++;
// //     rc = pthread_cond_broadcast(&emp);
// //     assert(rc==0);
// //     if (pthread_mutex_unlock(&lock)!=0){
// //         printf("Error: failed to unlock\n");
// //         pthread_exit(0);
// //     }


// //     return;
// // }












// // /*dequeue*/
// // void* dequeue(void* ptr)
// // {
// //     int rc;
// //     int i=0;
// //     int thread = *((int*)ptr);

// //     struct node* temp;

// //     while(finished){

// //         if (pthread_mutex_lock(&lock) !=0){
// //             printf("Error: lock has been failed\n");
// //             pthread_exit(0);
// //         }

// //         while (length == 0)
// //         {
// //             rc = pthread_cond_wait(&emp, &lock);
// //             assert(rc == 0);
// //         }

// //         int info = head->info;
// //         temp = head;
// //         head = head->ptr;
// //         free(temp);


// //         length--;
// //         rc = pthread_cond_broadcast(&emp);
// //         assert(rc==0);
// //         if (pthread_mutex_unlock(&lock)!=0){
// //             printf("Error: unlock has been failed\n");
// //             pthread_exit(0);
// //         }
// //     }
// //     pthread_exit(0);
// // }














// static char* path;

// void* conn_handler(void *args){

//     struct survival_bag *context = (struct survival_bag *) args;
//     pthread_detach(pthread_self());
    
//     serve_http(context->connFd, context->path);
//     close(context->connFd);
//     close(context);
// }

// static int counting = 0;


// // ./icws 222 src/sample-www/ 10
// int main(int argc, char* argv[]) {

//     int listenFd = open_listenfd(argv[1]); // Open and listen on port argv[1]. The web page on this port "request" for icws' sample-www 

//     printf("Start a\n");
//     int numThread = atoi(argv[3]);
//     max_threads = numThread;
//     pthread_t threadInfo[numThread];

//     // -------------------------------
    

//     struct sockaddr_storage clientAddr;
//     socklen_t clientLen = sizeof(struct sockaddr_storage);
    

//     // Create threads
//     pthread_t threadsArr[numThread];
//     for (int thrd=0; thrd<numThread;thrd++){
//         int rc = pthread_create(&threadsArr[thrd], NULL,  dequeue, (void*) &thrd);
//         if (rc){
//             printf("Error: failed to create thread\n");
//             return 1;   
//         }
//     }
//     // for (int i=0 ; i<numThread ; i++) {
//     //     pthread_create(&threadInfo[i], NULL, conn_handler, (void *) context);    
//     //     //pthread_detach(threadInfo[i]);   
//     // }   
//     for (;;) { //  for (int i=0 ; i<numThread ; i++)    // for(;;) repeats til all data is send. // Gotta reuse threads

//         // Establish connection
//         int connFd = accept(listenFd, (SA *) &clientAddr, &clientLen);
//         // printf("connFd (%d)\n",connFd);
//         if (connFd < 0) { 
//             fprintf(stderr, "Failed to accept\n"); return; 
//         }

//         // Making struct to be passed into threads
//         struct survival_bag *context = (struct survival_bag *) malloc(sizeof(struct survival_bag));    // Struct for storing relevant data, passed into each thread
//         context->connFd = connFd;
//         context->path = argv[2];
//         memcpy(&context->clientAddr, &clientAddr, sizeof(struct sockaddr_storage));

//         char hostBuf[MAXBUF], svcBuf[MAXBUF];
//         if (getnameinfo((SA *) &clientAddr, clientLen, 
//                         hostBuf, MAXBUF, svcBuf, MAXBUF, 0)==0) 
//             printf("Connection from %s:%s\n", hostBuf, svcBuf);
//         else
//             printf("Connection from ?UNKNOWN?\n");

//         pthread_create(&threadInfo, NULL, conn_handler, (void *) context); // pthread_create(&threadInfo[i], NULL, conn_handler, (void *) context);
//     }

//     printf("End b\n");

//     return 0;
// }
