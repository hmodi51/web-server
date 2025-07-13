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
#define PATH "index.html"
#define http_version "HTTP/1.1"

// struct stats file_stats;

char recbuf[bufSIZE];
char temprecbuf[bufSIZE];
char buf[bufSIZE];
char data[bufSIZE];
const char* del = "\r\n";
// const char* pwd = "/home/harsh/Desktop/PersonalProjects/web-server/";
char path[bufSIZE];
char statusLine[bufSIZE];
char headers[bufSIZE];
char entity[bufSIZE];
char* method;


typedef enum HTTP_STATUS {
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404
} HTTP_STATUS;


void handle_method(char* requestLine){
    method = requestLine;
}

void handle_404(int connfd){
    FILE *fptr;
    snprintf(headers, sizeof(headers), "Content-Type: text/html\r\nConnection: close\r\n\r\n");
    snprintf(entity , sizeof(entity) , "%s%s" , statusLine , headers);
    send(connfd , entity , strlen(entity) , 0);
    fptr = fopen("404.html" , "r");
    while(fgets(data , bufSIZE , fptr) != NULL){
        send(connfd , data , strlen(data) , 0);
    }
    close(connfd);
}

void handle_200(int connfd){
    FILE *fptr;
    snprintf(headers, sizeof(headers), "Content-Type: text/html\r\nConnection: close\r\n\r\n");
    snprintf(entity , sizeof(entity) , "%s%s" , statusLine , headers);
    send(connfd , entity , strlen(entity) , 0);
    if(strcmp(method , "GET")==0){
        fptr = fopen(path , "r");
        while(fgets(data , bufSIZE , fptr) != NULL){
            send(connfd , data , strlen(data) , 0);
        }
    }
    close(connfd);
}

int checkPath(char* filePath){
    char* res;
    if (strcmp(filePath , "/") == 0){
        char* relativePath = "index.html";
        res = realpath(relativePath , path);
    }
    else{
        res = realpath(filePath+1 , path);
        printf("default path is %s\n" , path);
    }


    if(res == NULL){
        return -1;
    }
    else{
        return 0;
    }
}

void checkFile(){
    struct stat statbuf;
    stat(path , &statbuf);
    if(S_ISDIR(statbuf.st_mode)){
        strcat(path , "/index.html");
    }
}

void build_statusLine(HTTP_STATUS statusCode){
    char* http_ver = http_version;
    char* statuscode;
    switch (statusCode){
        case 200:
            statuscode = "200 OK";
            break;
        case 404:
            statuscode = "404 Not Found";
    }
    snprintf(statusLine , sizeof(statusLine) , "%s %s\r\n" , http_ver , statuscode);
    printf("status line is %s\n", statusLine);
}

void build_headers(HTTP_STATUS statusCode , int connfd){
    switch(statusCode){
        case 200:
            handle_200(connfd);
            break;
        case 404:
            handle_404(connfd);
            break;
    }
}


void handle_client(int connfd){
    recv(connfd , recbuf , bufSIZE , 0);
    strcpy(temprecbuf , recbuf);
    char* tokens = strtok(temprecbuf , del);
    char *requestLine = strtok(tokens , " ");
    handle_method(requestLine);
    // handle_method(requestLine);
    printf("request line is %s\n" , requestLine);
    char *filePath = strtok(NULL , " ");
    printf("filpath is %s\n" , filePath);
    // printf(tokens);
    printf(recbuf);
    int ret = checkPath(filePath);
    HTTP_STATUS statusCode;
    if(ret == -1){
        statusCode = HTTP_NOT_FOUND;
    }
    else{
        statusCode = HTTP_OK;
        checkFile();
    }
    build_statusLine(statusCode);
    printf("status code is %d\n" , statusCode);
    build_headers(statusCode , connfd);
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
