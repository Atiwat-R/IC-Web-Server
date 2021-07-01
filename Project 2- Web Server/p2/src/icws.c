
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include "pcsa_net.h"
#include "parse.h"


// ----------------------------------------------------------------------------------------------
// Thread Pool stuff


// Thread Pool, modified from https://nachtimwald.com/2019/04/12/thread-pool-in-c/
#ifndef __TPOOL_H__
#define __TPOOL_H__

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

struct tpool;
typedef struct tpool tpool_t;

typedef void (*thread_func_t)(void *arg);

tpool_t *tpool_create(size_t num);
void tpool_destroy(tpool_t *tm);

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg);
void tpool_wait(tpool_t *tm);

#endif /* __TPOOL_H__ */

// Work queue
struct tpool_work {
    thread_func_t      func;
    void              *arg;
    struct tpool_work *next;
};
typedef struct tpool_work tpool_work_t;

// thread pool
struct tpool {
    tpool_work_t    *work_first;
    tpool_work_t    *work_last;
    pthread_mutex_t  work_mutex;
    pthread_cond_t   work_cond;
    pthread_cond_t   working_cond;
    size_t           working_cnt;
    size_t           thread_cnt;
    bool             stop;
};


// --------------------------------------------
// Simple helpers for creating and destroying work objects. 

static tpool_work_t *tpool_work_create(thread_func_t func, void *arg)
{
    tpool_work_t *work;

    if (func == NULL)
        return NULL;

    work       = malloc(sizeof(*work));
    work->func = func;
    work->arg  = arg;
    work->next = NULL;
    return work;
}

static void tpool_work_destroy(tpool_work_t *work)
{
    if (work == NULL)
        return;
    free(work);
}

/**
 * Work will need to be pulled from the queue at some point to be processed. 
 * Since the queue is a linked list this handles not only pulling an object 
 * from the list but also maintaining the list work_first and work_last references for us.
 */
static tpool_work_t *tpool_work_get(tpool_t *tm)
{
    tpool_work_t *work;

    if (tm == NULL)
        return NULL;

    work = tm->work_first;
    if (work == NULL)
        return NULL;

    if (work->next == NULL) {
        tm->work_first = NULL;
        tm->work_last  = NULL;
    } else {
        tm->work_first = work->next;
    }

    return work;
}

// -----------------------------------------






// ---------------------------------------------------------------
// This is the heart and soul of the pool and is where work is handled.
// At a high level this waits for work and processes it.

static void *tpool_worker(void *arg)
{
    tpool_t      *tm = arg;
    tpool_work_t *work;

    while (1) {
        pthread_mutex_lock(&(tm->work_mutex));

        while (tm->work_first == NULL && !tm->stop)
            pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex));

        if (tm->stop)
            break;

        work = tpool_work_get(tm);
        tm->working_cnt++;
        pthread_mutex_unlock(&(tm->work_mutex));

        if (work != NULL) {
            work->func(work->arg);
            tpool_work_destroy(work);
        }

        pthread_mutex_lock(&(tm->work_mutex));
        tm->working_cnt--;
        if (!tm->stop && tm->working_cnt == 0 && tm->work_first == NULL)
            pthread_cond_signal(&(tm->working_cond));
        pthread_mutex_unlock(&(tm->work_mutex));
    }

    tm->thread_cnt--;
    pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->work_mutex));
    return NULL;
}

// ---------------------------------------------------------------









tpool_t *tpool_create(size_t num)
{
    tpool_t   *tm;
    pthread_t  thread;
    size_t     i;

    if (num == 0)
        num = 2;

    tm             = calloc(1, sizeof(*tm));
    tm->thread_cnt = num;

    pthread_mutex_init(&(tm->work_mutex), NULL);
    pthread_cond_init(&(tm->work_cond), NULL);
    pthread_cond_init(&(tm->working_cond), NULL);

    tm->work_first = NULL;
    tm->work_last  = NULL;

    for (i=0; i<num; i++) {
        pthread_create(&thread, NULL, tpool_worker, tm);
        pthread_detach(thread);
    }

    return tm;
}





void tpool_destroy(tpool_t *tm)
{
    tpool_work_t *work;
    tpool_work_t *work2;

    if (tm == NULL)
        return;

    pthread_mutex_lock(&(tm->work_mutex));
    work = tm->work_first;
    while (work != NULL) {
        work2 = work->next;
        tpool_work_destroy(work);
        work = work2;
    }
    tm->stop = true;
    pthread_cond_broadcast(&(tm->work_cond));
    pthread_mutex_unlock(&(tm->work_mutex));

    tpool_wait(tm);

    pthread_mutex_destroy(&(tm->work_mutex));
    pthread_cond_destroy(&(tm->work_cond));
    pthread_cond_destroy(&(tm->working_cond));

    free(tm);
}




bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg)
{
    tpool_work_t *work;

    if (tm == NULL)
        return false;

    work = tpool_work_create(func, arg);
    if (work == NULL)
        return false;

    pthread_mutex_lock(&(tm->work_mutex));
    if (tm->work_first == NULL) {
        tm->work_first = work;
        tm->work_last  = tm->work_first;
    } else {
        tm->work_last->next = work;
        tm->work_last       = work;
    }

    pthread_cond_broadcast(&(tm->work_cond));
    pthread_mutex_unlock(&(tm->work_mutex));

    return true;
}



void tpool_wait(tpool_t *tm)
{
    if (tm == NULL)
        return;

    pthread_mutex_lock(&(tm->work_mutex));
    while (1) {
        if ((!tm->stop && tm->working_cnt != 0) || (tm->stop && tm->thread_cnt != 0)) {
            pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tm->work_mutex));
}




// ----------------------------------------------------------------------------------------------
// Poll to hadle connection timeout

#include <sys/poll.h>

struct pollfd fds[2]; 
int timeout_sec; // to use as an argument, *1000 so it'd become milisec
int ret;






// ----------------------------------------------------------------------------------------------
// Web Server stuff


/* Rather arbitrary. In real life, be careful with buffer overflow */
#define MAXBUF 8192  

typedef struct sockaddr SA;
struct survival_bag{
	struct sockaddr_storage clientAddr;
	int connFd;
	char* path;
};



void respond_GET_HEAD(int connFd, char *filePath, char *uri, int isGet) {
    char buf[MAXBUF];
	struct stat sb;
	printf("uri (%s)\n",uri);
    char tmpUri[MAXBUF];
	strcpy(tmpUri,uri);

    // TODO: SHOW MORE HEADER INFORMATION, EG DATE, MODIFIED ON, ETC.
    char *extension = strchr(tmpUri, '.');
    // check if ext is null
    if (strcmp(extension,"")==0 || extension == NULL) {
        printf("\n!!! File lacks Extension !!!\n");
        exit(1);
    }

	printf("extension = (%s)\n",extension);	
	char mimeType[MAXBUF];
	if(strcmp(extension,".html")==0 || strcmp(extension,".htm")==0){ // ori
		strcpy(	mimeType , "text/html");
	}
    else if(strcmp(extension,".css")==0){
		strcpy(	mimeType , "text/css");
	}
	else if(strcmp(extension,".txt")==0){ // plain
		strcpy(	mimeType , "text/plain");
	}
    else if(strcmp(extension,".js")==0){ // javascipt
		strcpy(	mimeType , "text/javascript");
	}
	else if(strcmp(extension,".jpg")==0 || strcmp(extension,".jpeg")==0){ // ori
		strcpy(mimeType,"image/jpeg");
	}
    else if(strcmp(extension,".png")==0){
		strcpy(	mimeType , "image/png");
	}
    else if(strcmp(extension,".gif")==0){
		strcpy(	mimeType , "image/gif");
	}
    // support more ext

    // Date
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("Date: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	printf("mimeType (%s)\n", mimeType);
	char newBuf[MAXBUF];
	sprintf(newBuf,"%s%s",filePath,uri);
	printf("path (%s)\n",newBuf);
    int inputFd = open(newBuf,O_RDONLY);
    fstat(inputFd, &sb);
    size_t sizeOfFile = sb.st_size;
	printf("sizeOfFile: %lu\n",sizeOfFile);
    printf("Last Modified: %s", ctime(&sb.st_mtime));
    // printf("Last Modified: %lld\n", (long long) lastMod(NULL));



    char dateBuf[MAXBUF];
    time_t now = time(0);
    struct tm tm2 = *gmtime(&now);
    strftime(dateBuf, sizeof dateBuf, "%a, %d %b %Y %H:%M:%S %Z", &tm2);
    printf("Time is: [%s]\n", dateBuf);

// "Date: %s\r\n"


    sprintf(buf, 
            "HTTP/1.1 200 OK\r\n"
            "Server: icws\r\n"
            "Connection: close\r\n"
            "Content-length: %lu\r\n"
            "Content-type: %s\r\n\r\n", sizeOfFile,mimeType);

	printf("buf: \n%s",buf);
    printf("--HEADER------------------------------------------------------------------\n\n");       
	char content[MAXBUF];
    write_all(connFd, buf, strlen(buf));
	ssize_t numRead;

    // while (read_line(inputFd, content, MAXBUF) > 0) {
    //     write_all(connFd,content,MAXBUF);
    //     if (strcmp(buf, "\r\n") == 0) break;    
    // }


    // for HEAD
    if (isGet) {
        while((numRead = read(inputFd,content,MAXBUF))>0){	
            // printf("[] %s ]", content);
            write_all(connFd,content,MAXBUF);
        } 
    }

	close(inputFd);
}





void serve_http(int connFd,char *path) {
    char buf[MAXBUF];


    // this read the header
    //
	printf("path (%s)\n",path);	
    if (!read_line(connFd, buf, MAXBUF)) 
        return ;  /* Quit if we can't read the first line */

    printf("LOG: %s\n", buf);



    /* [METHOD] [URI] [HTTPVER] */
    // This is default method for parsing requests
    char method[MAXBUF], uri[MAXBUF], httpVer[MAXBUF];
    sscanf(buf, "%s %s %s", method, uri, httpVer);

    // puts("\n-------------------------------------------------------------\n");
    // printf("(%s)\n", method);
    // printf("(%s)\n", uri);
    // printf("(%s)\n", httpVer);
    // puts("\n-------------------------------------------------------------\n");

    int readRet;
    int totalReqSize = 0;
    // Reads the reast of the header into buf
	while((readRet = read_line(connFd,buf,MAXBUF)) > 0) {

        totalReqSize = totalReqSize + readRet;
        if (totalReqSize > MAXBUF) { // Check if request size is too big
            printf("\n!!! Request too big !!!\n");
            exit(400);
        }
        printf("++LOG in loop: %s\n", buf);
        if (strcmp(buf, "\r\n") == 0) break;

    }

            










    // //////////////////////////////////////////// aj parser 

    // /* Pasing using starter code's parser */

    // printf("\nGAGXYXGYAXGAXXGA %s XGYAXGYXGAGXYAXGYX\n", path);

    // // int fd_in = open(path, O_RDONLY); // open request as specified by the given input path
    // int index;

    // // if (fd_in < 0) { // In case of failed file opening
    // //     printf("Failed to open the file\n");
    // //     return 0;
    // // }

    // int readRet;
    // int totalReqSize = 0;

    // // Accumulate size from read and compare it to MAXBUF, if over change size of buf as well
    // readRet = read(connFd,buf,MAXBUF);


	// // while((readRet = read_line(connFd,buf,MAXBUF))>0){

    // //     // Check of request size is too big
    // //     totalReqSize = totalReqSize + readRet;
    // //     if (totalReqSize > MAXBUF) {
    // //         printf("\n!!! Request too big !!!\n");
    // //         exit(1);
    // //     }
    // //     printf("++LOG in loop: %s\n", buf);
    // //     if (strcmp(buf, "\r\n") == 0) break;
    // // }



    // // Check if Request size is too large. Still WIP
    // // while ((readRet = read(connFd,buf,MAXBUF)) > 0) { // This is obtaining the header to buf
    // //     //readRet = read(connFd,buf,MAXBUF);
    // //     totalReqSize += readRet;
    // //     if (totalReqSize > MAXBUF) {
    // //         printf("!!! Request Size too Large !!!\n");
    // //         exit(0);
    // //     }
    // //     printf("- %d\n", readRet);
    // // }

    // // printf("--BUFFER--------------------------------------------------------\n");
    // // printf("\n%s\n", buf);
    // // printf("----------------------------------------------------------------\n");


    // Request *request = parse(buf,readRet,connFd);
    // if (request == NULL) { // Check if what is parsed is NULL
    //     printf("!!! NULL Request !!!\n");
    //     exit(0);
    // }
    

    // //Parse the buffer to the parse function. You will need to pass the socket fd and the buffer would need to
    // //be read from that fd
    
    // // Store parsed data

    // // if (request)


    // char method[MAXBUF], uri[MAXBUF], httpVer[MAXBUF];
    // strcpy(method, request->http_method);
    // strcpy(uri, request->http_uri);
    // strcpy(httpVer, request->http_version);

    // /////////////////////////// Unchanged here on







    // If its a GET request
    if (strcasecmp(method, "GET") == 0 &&
            uri[0] == '/') {
        printf("--HEADER------------------------------------------------------------------\n\n");
        printf("LOG: GET Request\n");
        respond_GET_HEAD(connFd,path,uri,99);
    }
    else if (strcasecmp(method, "HEAD") == 0 &&
            uri[0] == '/') {
        printf("LOG: HEAD Request\n");
        respond_GET_HEAD(connFd,path,uri,0);
    }
    else {
        printf("LOG: Unknown request\n");
        char bufx[MAXBUF];
        sprintf(bufx, "LOG: Unknown request\r\n\r\n");
        // printf("+++++++%s", bufx);
        write(connFd,bufx,MAXBUF);
        exit(501); // Error 501: Method Not Implemented
    }

    // free(buf);
    // free(request->headers); // corrupted size vs. prev_size Aborted (core dumped)
    // free(request);



    
}

////// OLD CON HANDLER
// void* conn_handler(void *args){
// 	// struct survival_bag *context = (struct survival_bag*) args;
// 	// pthread_detach(pthread_self());
// 	// serve_http(context->connFd, context->path);
// 	// close(context->connFd);
// 	// free(context);
// 	// return NULL;	

//     int context = (int) args; 
//     // printf("Thread-based server A\n");
//     printf(">> %d\n", context);
//     // printf("Thread-based server C\n");
// }


void* conn_handler(void *args){

    struct survival_bag *context = (struct survival_bag *) args;
    pthread_detach(pthread_self());
    
    serve_http(context->connFd, context->path);
    close(context->connFd);
    close(context);
}




// run -> ./icws 222 src/sample-www/ 10 5

int main(int argc, char* argv[]) {

    int listenFd = open_listenfd(argv[1]); // Open and listen on port argv[1]. The web page on this port "request" for icws' sample-www 
    if (listenFd < 0) { // open_listenfd error checking
        printf("Error Listening on the port.\n");
        printf("Error %d", errno);
    }
    fds[0].fd = listenFd; // Put listenFd into poll struct
    fds[0].events = POLLIN;

    printf("Starting...\n");

    // Timeouts
    if (argv[4] != NULL) {
        timeout_sec = atoi(argv[4]);
    } else {
        timeout_sec = 5; // If no specified timeout is given, 5 will be the default
    }

    // Number of threads
    int numThread;
    if (argv[3] != NULL) {
        numThread = atoi(argv[3]);
    } else {
        numThread = 1; // If no numThread is given, set to default 1 thread
    }
    // Thread pool
    tpool_t *tm;
    tm   = tpool_create(numThread);
    
    
    // Some data for creating struct context
    struct sockaddr_storage clientAddr;
    socklen_t clientLen = sizeof(struct sockaddr_storage);


    for (;;) { //  for (int i=0 ; i<numThread ; i++)    // for(;;) repeats til all data is send. // Gotta reuse threads

        // Check for connection timeouts 
        ret = poll(fds, 2, timeout_sec * 1000);
        if (ret <= 0) {
            printf("\nRequest Timeout\n");
            exit(408); // Exit code 408 for Request Timeout
        }

        // Establish connection
        int connFd = accept(listenFd, (SA *) &clientAddr, &clientLen);
        if (connFd < 0) { 
            fprintf(stderr, "Failed to accept\n"); return; 
        }

        // Making struct to be passed into threads
        struct survival_bag *context = (struct survival_bag *) malloc(sizeof(struct survival_bag));
        context->connFd = connFd;
        context->path = argv[2];
        memcpy(&context->clientAddr, &clientAddr, sizeof(struct sockaddr_storage));

        char hostBuf[MAXBUF], svcBuf[MAXBUF];
        if (getnameinfo((SA *) &clientAddr, clientLen, 
                        hostBuf, MAXBUF, svcBuf, MAXBUF, 0)==0) 
            printf("Connection from %s:%s\n", hostBuf, svcBuf);
        else
            printf("Connection from ?UNKNOWN?\n");

        // Add work to queue
        tpool_add_work(tm, conn_handler, (void *) context);
        //pthread_create(&threadInfo, NULL, conn_handler, (void *) context); // pthread_create(&threadInfo[i], NULL, conn_handler, (void *) context);
    }        
    









    // // More poll stuff
    // ret = poll(fds, 2, timeout_seconds * 1000);

    // if (ret > 0) {
    //     for (;;) { //  for (int i=0 ; i<numThread ; i++)    // for(;;) repeats til all data is send. // Gotta reuse threads

    //         // Establish connection
    //         int connFd = accept(listenFd, (SA *) &clientAddr, &clientLen);
    //         if (connFd < 0) { 
    //             fprintf(stderr, "Failed to accept\n"); return; 
    //         }

    //         // Making struct to be passed into threads
    //         struct survival_bag *context = (struct survival_bag *) malloc(sizeof(struct survival_bag));
    //         context->connFd = connFd;
    //         context->path = argv[2];
    //         memcpy(&context->clientAddr, &clientAddr, sizeof(struct sockaddr_storage));

    //         char hostBuf[MAXBUF], svcBuf[MAXBUF];
    //         if (getnameinfo((SA *) &clientAddr, clientLen, 
    //                         hostBuf, MAXBUF, svcBuf, MAXBUF, 0)==0) 
    //             printf("Connection from %s:%s\n", hostBuf, svcBuf);
    //         else
    //             printf("Connection from ?UNKNOWN?\n");

    //         // Add work to queue
    //         tpool_add_work(tm, conn_handler, (void *) context);
    //         //pthread_create(&threadInfo, NULL, conn_handler, (void *) context); // pthread_create(&threadInfo[i], NULL, conn_handler, (void *) context);
    //     }        
    // }
    // else {
    //     printf("\n408 Request Timeout\n");
    //     exit(408);
    // }



    return 0;
}
