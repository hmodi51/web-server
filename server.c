#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#define PORT "8080"
#define bufSIZE 4096
#define http_version "HTTP/1.1"

typedef struct request_Line {
    char method[8];
    char path[bufSIZE];
} request_Line;


typedef struct request {
    int clientfd;
    request_Line line;
    char requestLine[bufSIZE+bufSIZE];
    char recbuf[bufSIZE*3];
    char headers[bufSIZE];
} request;


typedef enum HTTP_STATUS {
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404,
    HTTP_NOT_IMPLEMENTED = 501
} HTTP_STATUS;


typedef enum HTTP_METHODS {
    GET,
    HEAD
} HTTP_METHODS;



void handle_method(char* requestLine , request* req){
    snprintf(req->line.method , bufSIZE , "%s" , requestLine);
    printf("hndle methid %s\n" , req->line.method);
}

void handle_404(request* req , char* statusLine){
    char data[bufSIZE];
    char entity[bufSIZE];
    FILE *fptr;
    snprintf(req->headers, sizeof(req->headers), "Content-Type: text/html\r\nConnection: close\r\n\r\n");
    snprintf(entity , sizeof(entity) , "%s%s" , statusLine , req->headers);
    send(req->clientfd , entity , strlen(entity) , 0);
    fptr = fopen("404.html" , "r");
    while(fgets(data , bufSIZE , fptr) != NULL){
        send(req->clientfd , data , strlen(data) , 0);
    }
    close(req->clientfd);
}

void handle_200(request* req , char* statusLine){
    char data[bufSIZE];
    char entity[bufSIZE];
    FILE *fptr;
    snprintf(req->headers, sizeof(req->headers), "Content-Type: text/html\r\nConnection: close\r\n\r\n");
    snprintf(entity , sizeof(entity) , "%s%s" , statusLine , req->headers);
    send(req->clientfd , entity , strlen(entity) , 0);
    if(strcmp(req->line.method , "GET")==0){
        fptr = fopen(req->line.path , "r");
        while(fgets(data , bufSIZE , fptr) != NULL){
            send(req->clientfd , data , strlen(data) , 0);
        }
    }
    close(req->clientfd);
}

int checkPath(char* filePath , request* req){
    char* res;
    if (strcmp(filePath , "/") == 0){
        char* relativePath = "index.html";
        res = realpath(relativePath , req->line.path);
    }
    else{
        res = realpath(filePath+1 , req->line.path);
        printf("default path is %s\n" , req->line.path);
    }


    if(res == NULL){
        return -1;
    }
    else{
        return 0;
    }
}

void checkFile(request* req){
    struct stat statbuf;
    stat(req->line.path , &statbuf);
    if(S_ISDIR(statbuf.st_mode)){
        snprintf(req->line.path + strlen(req->line.path), bufSIZE - strlen(req->line.path) , "%s" , "/index.html");
        int length = strlen(req->line.path);
        printf("Length of string is : %d", length);
    }
}

void build_statusLine(HTTP_STATUS statusCode , char* statusLine){
    char* http_ver = http_version;
    char* statuscode;
    switch (statusCode){
        case 200:
            statuscode = "200 OK";
            break;
        case 404:
            statuscode = "404 Not Found";
    }
    snprintf(statusLine , bufSIZE , "%s %s\r\n" , http_ver , statuscode);
    printf("status line is %s\n", statusLine);
}

void build_headers(HTTP_STATUS statusCode , char* statusLine , request* req){
    switch(statusCode){
        case 200:
            handle_200(req , statusLine);
            break;
        case 404:
            handle_404(req , statusLine);
            break;
    }
}


void handle_client(int connfd){
    request req;
    req.clientfd = connfd;
    char statusLine[bufSIZE];
    recv(req.clientfd , req.recbuf , bufSIZE , 0);
    char* tokens = strtok(req.recbuf , "\r\n");
    char *requestLine = strtok(tokens , " ");
    handle_method(requestLine , &req);
    printf("request line is %s\n" , req.line.method);
    char *filePath = strtok(NULL , " ");
    printf("filpath is %s\n" , filePath);
    int ret = checkPath(filePath , &req);
    HTTP_STATUS statusCode;
    if(ret == -1){
        statusCode = HTTP_NOT_FOUND;
    }
    else{
        statusCode = HTTP_OK;
        checkFile(&req);
    }
    build_statusLine(statusCode , statusLine);
    printf("status code is %d\n" , statusCode);
    build_headers(statusCode ,statusLine , &req);
}


int main(){
    struct addrinfo hints , *res;
    struct sockaddr_storage their_addr;
    socklen_t addrlen = sizeof their_addr;
    memset(&hints , 0 , sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL , PORT , &hints , &res);

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int optval = 1;
    if(sockfd < 0){ 
        printf("error opening socket\n");
        return -1;
    }
    if(setsockopt(sockfd , SOL_SOCKET , SO_REUSEADDR , &optval , sizeof(optval)) < 0){
        printf("error in setsockopt\n");
        return -1;
    }

    if(bind(sockfd , res->ai_addr , res->ai_addrlen) < 0){
        printf("binding failed\n");
        perror("bind");
        return -1;
    }

    if(listen(sockfd , 5) < 0){
        printf("error in listen\n");
        return -1;
    }

    while(1){
        printf("accepting connection\n");
        int connfd = accept(sockfd , (struct sockaddr *)&their_addr , &addrlen);
        handle_client(connfd);
    }
    close(sockfd);

    return 0;
}
