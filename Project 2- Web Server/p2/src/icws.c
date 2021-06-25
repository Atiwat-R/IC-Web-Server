#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netdb.h>
#include<pthread.h>
#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include "pcsa_net.h"
#include "parse.h"

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

void* conn_handler(void *args){
	// struct survival_bag *context = (struct survival_bag*) args;
	// pthread_detach(pthread_self());
	// serve_http(context->connFd, context->path);
	// close(context->connFd);
	// free(context);
	// return NULL;	

    int context = (int) args; 
    // printf("Thread-based server A\n");
    for (;;) {
        printf(">> %d\n", context);
        break;
    }
    // printf("Thread-based server C\n");
}

int main(int argc, char* argv[]) {
    // printf("Thread-based server");
    int listenFd = open_listenfd(argv[1]); // Open and listen on port argv[1]

    printf("Starter a\n");
    int numThread = atoi(argv[3]);
    pthread_t threadInfo[5];
    for (int i=0 ; i<numThread ; i++) {
        pthread_create(&threadInfo[i], NULL, conn_handler, (void *) i);    
        pthread_detach(threadInfo[i]);           
    }
    printf("Ender b\n");

    for (;;) {
        struct sockaddr_storage clientAddr;
        socklen_t clientLen = sizeof(struct sockaddr_storage);
	    // pthread_t threadInfo;
        int connFd = accept(listenFd, (SA *) &clientAddr, &clientLen);
        // printf("connFd (%d)\n",connFd);
        if (connFd < 0) { fprintf(stderr, "Failed to accept\n"); continue; }
        // struct survival_bag *context = (struct survival_bag *) malloc(sizeof(struct survival_bag));
        // context->connFd = connFd;
        // context->path = argv[2];
        char hostBuf[MAXBUF], svcBuf[MAXBUF];
        if (getnameinfo((SA *) &clientAddr, clientLen, 
                        hostBuf, MAXBUF, svcBuf, MAXBUF, 0)==0) 
            printf("Connection from %s:%s\n", hostBuf, svcBuf);
        else
            printf("Connection from ?UNKNOWN?\n");
        // for (int i=0 ; i<numThread ; i++) {
        //     //pthread_create(&threadInfo, NULL, conn_handler, (void *) context);             
        // }
       
        serve_http(connFd,argv[2]);
        close(connFd);
    }

    return 0;
}
