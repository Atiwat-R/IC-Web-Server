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
#define MAXBUF 1024  

typedef struct sockaddr SA;
struct survival_bag{
	struct sockaddr_storage clientAddr;
	int connFd;
	char* path;
};
void respond_get(int connFd,char *filePath,char *uri) {
    char buf[MAXBUF];
	struct stat sb;
	printf("uri (%s)\n",uri);
    char tmpUri[MAXBUF];
	strcpy(tmpUri,uri);
    char *extension; strtok(tmpUri,".");
	extension = strtok(NULL," ");
	printf("extension (%s)\n",extension);	
	char mimeType[MAXBUF];
	if(strcmp(extension,"html")==0){
		strcpy(	mimeType , "text/html");
	}
	else if(strcmp(extension,"jpg")==0){
		strcpy(mimeType,"image/jpeg");
	}
	printf("mimeType (%s)\n",mimeType);
	char newBuf[MAXBUF];
	sprintf(newBuf,"%s%s",filePath,uri);
	printf("path (%s)\n",newBuf);
    int inputFd = open(newBuf,O_RDONLY);
    fstat(inputFd, &sb);
    size_t sizeOfFile = sb.st_size;
	printf("%lu\n",sizeOfFile);
    sprintf(buf, 
            "HTTP/1.1 200 OK\r\n"
            "Server: Tiny\r\n"
            "Connection: close\r\n"
            "Content-length: %lu\r\n"
            "Content-type: %s\r\n\r\n", sizeOfFile,mimeType);
	char content[MAXBUF];
    write_all(connFd, buf, strlen(buf));
	ssize_t numRead;
	printf("buf (%s)\n",buf);
    // while (read_line(inputFd, content, MAXBUF) > 0) {
    //     write_all(connFd,content,MAXBUF);
    //     // if (strcmp(buf, "\r\n") == 0) break;    
    // }
    while((numRead = read(inputFd,content,MAXBUF))>0){	
    	write_all(connFd,content,MAXBUF);
     }

	close(inputFd);
}



void respond_head(int connFd,char *filePath,char *uri) {
    char buf[MAXBUF];
	struct stat sb;
	printf("uri (%s)\n",uri);
    char tmpUri[MAXBUF];
	strcpy(tmpUri,uri);
    char *extension; strtok(tmpUri,".");
	extension = strtok(NULL," ");
	printf("extension (%s)\n",extension);	
	char mimeType[MAXBUF];
	if(strcmp(extension,"html")==0){
		strcpy(	mimeType , "text/html");
	}
	else if(strcmp(extension,"jpg")==0){
		strcpy(mimeType,"image/jpeg");
	}
	printf("mimeType (%s)\n",mimeType);
	char newBuf[MAXBUF];
	sprintf(newBuf,"%s%s",filePath,uri);
	printf("path (%s)\n",newBuf);
    int inputFd = open(newBuf,O_RDONLY);
    fstat(inputFd, &sb);
    size_t sizeOfFile = sb.st_size;
	printf("%lu\n",sizeOfFile);
    sprintf(buf, 
            "HTTP/1.1 200 OK\r\n"
            "Server: Tiny\r\n"
            "Connection: close\r\n"
            "Content-length: %lu\r\n"
            "Content-type: %s\r\n\r\n", sizeOfFile,mimeType);
	char content[MAXBUF];
    write_all(connFd, buf, strlen(buf));
	ssize_t numRead;
	printf("buf (%s)\n",buf);
    // while (read_line(inputFd, content, MAXBUF) > 0) {
    //     write_all(connFd,content,MAXBUF);
    //     // if (strcmp(buf, "\r\n") == 0) break;    
    // }
    // while((numRead = read(inputFd,content,MAXBUF))>0){	
    // 	write_all(connFd,content,MAXBUF);
    //  }

	close(inputFd);
}



void serve_http(int connFd,char *path) {
    char buf[MAXBUF];
	printf("path (%s)\n",path);	
    if (!read_line(connFd, buf, MAXBUF)) 
        return ;  /* Quit if we can't read the first line */

    printf("LOG: %s\n", buf);



    /* [METHOD] [URI] [HTTPVER] */
    // This is default method for parsing requests
    char method[MAXBUF], uri[MAXBUF], httpVer[MAXBUF];
    sscanf(buf, "%s %s %s", method, uri, httpVer);

    puts("\n-------------------------------------------------------------\n");
    printf("(%s)\n", method);
    printf("(%s)\n", uri);
    printf("(%s)\n", httpVer);
    puts("\n-------------------------------------------------------------\n");




    /* Pasing using starter code's parser */

    // printf("\nGAGXYXGYAXGAXXGA %s XGYAXGYXGAGXYAXGYX\n", path);

    // int fd_in = open(path, O_RDONLY); // open request as specified by the given input path
    // int index;

    // if (fd_in < 0) { // In case of failed file opening
    //     printf("Failed to open the file\n");
    //     return 0;
    // }
    // int readRet = read(fd_in,buf,MAXBUF);
    // //Parse the buffer to the parse function. You will need to pass the socket fd and the buffer would need to
    // //be read from that fd
    // Request *request = parse(buf,readRet,fd_in);
    // // Store parsed data


    // char method[MAXBUF], uri[MAXBUF], httpVer[MAXBUF];
    // strcpy(method, request->http_method);
    // strcpy(uri, request->http_uri);
    // strcpy(httpVer, request->http_version);




    // 
	while(read_line(connFd,buf,MAXBUF)>0){
        // printf("LOG in loop: %s\n", buf);
        if (strcmp(buf, "\r\n") == 0) break;
    }

    // If its a GET request
    if (strcasecmp(method, "GET") == 0 &&
            uri[0] == '/') {
        printf("LOG: Sending hello world\n");
        respond_get(connFd,path,uri);
    }
    else {
        printf("LOG: Unknown request\n");
    }
    
}

// void* conn_handler(void *args){
// 	struct survival_bag *context = (struct survival_bag*) args;
// 	pthread_detach(pthread_self());
// 	serve_http(context->connFd, context->path);
// 	close(context->connFd);
// 	free(context);
// 	return NULL;	
// }

int main(int argc, char* argv[]) {
    // printf("Thread-based server");
    int listenFd = open_listenfd(argv[1]); // Open and listen on port argv[1]

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
        //pthread_create(&threadInfo, NULL, conn_handler, (void *) context);        
        serve_http(connFd,argv[2]);
        close(connFd);
    }

    return 0;
}
