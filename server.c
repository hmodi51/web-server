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

typedef struct response {
    char statusLine[bufSIZE];
    char headers[bufSIZE];
    char data[bufSIZE];
    char length[10];
} response;

typedef struct headers {
    char key[50];
    char value[bufSIZE];
} headers;

typedef enum HTTP_STATUS {
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404,
    HTTP_NOT_IMPLEMENTED = 501
} HTTP_STATUS;


typedef enum HTTP_METHODS {
    GET,
    HEAD
} HTTP_METHODS;

void parseHeaders(char* rest , headers* headerList){
    char* headersplit;
    int i=0;
    char * headerString = strtok_r(rest , "\r\n" , &rest);
    while(headerString){
        headers head;
        char* keyptr = strtok_r(headerString , ": " , &headersplit);
        char* valueptr = headersplit;
        snprintf(head.key , sizeof(head.key) , "%s" , keyptr);
        snprintf(head.value , sizeof(head.value) , "%s" , valueptr);
        headerList[i++] = head;
        headerString = strtok_r(rest , "\r\n" , &rest);
        printf("\nheader key is %s and value is %s\n" , head.key ,head.value);
    }
}

void handle_method(char* method , request* req){
    snprintf(req->line.method , 20 , "%s" , method);
    printf("hndle methid %s\n" , req->line.method);
}

void handle_404(request* req , response* res){
    char data[bufSIZE];
    char entity[bufSIZE];
    FILE *fptr;
    snprintf(res->headers, sizeof(res->headers), "Content-Type: text/html\r\nConnection: keep-alive\r\nContent-Length: 513\r\n\r\n");
    snprintf(entity , sizeof(entity) , "%s%s" , res->statusLine , res->headers);
    send(req->clientfd , entity , strlen(entity) , 0);
    printf("printing data sent\n %s\n data send end\n" , entity);
    fptr = fopen("404.html" , "r");
    while(fgets(data , bufSIZE , fptr) != NULL){
        send(req->clientfd , data , strlen(data) , 0);
        printf("printing data sent\n %s\n data send end\n" , data);
    }
    // close(req->clientfd);
}

void handle_200(request* req , response* res){
    char data[bufSIZE];
    char entity[bufSIZE];
    FILE *fptr;
    snprintf(res->headers, sizeof(res->headers), "Content-Type: text/html\r\nConnection: keep-alive\r\nContent-Length: %s\r\n\r\n" , res->length);
    snprintf(entity , sizeof(entity) , "%s%s" , res->statusLine , res->headers);
    send(req->clientfd , entity , strlen(entity) , 0);
    if(strcmp(req->line.method , "GET")==0){
        fptr = fopen(req->line.path , "r");
        while(fgets(data , bufSIZE , fptr) != NULL){
            send(req->clientfd , data , strlen(data) , 0);
        }
    }
    // close(req->clientfd);
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

void checkFile(request* req , response* res){
    struct stat statbuf;
    int fileLength;
    stat(req->line.path , &statbuf);
    if(S_ISDIR(statbuf.st_mode)){
        snprintf(req->line.path + strlen(req->line.path), bufSIZE - strlen(req->line.path) , "%s" , "/index.html");
        int length = strlen(req->line.path);
        printf("Length of string is : %d", length);
    }
    if(stat(req->line.path ,&statbuf)==0){
        fileLength = statbuf.st_size;
    }
    snprintf(res->length , sizeof(res->length) ,"%d" ,fileLength);

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

void build_headers(HTTP_STATUS statusCode , request* req, response* res){
    switch(statusCode){
        case 200:
            handle_200(req , res);
            break;
        case 404:
            handle_404(req , res);
            break;
    }
}

void handle_request(request* req){
    response res;
    char* requestLine;
    char* rest;
    char* tokens = strtok_r(req->recbuf , "\r\n" , &rest);
    char* host = strtok_r(NULL , "\r\n" , &rest);
    headers headerList[20];
    parseHeaders(rest , headerList);
    printf("host is %s \n" , host);
    requestLine = tokens;
    printf("requestLine starting %s requestLine end\n" , requestLine);
    char *method = strtok(tokens , " ");
    printf("method starting %s method end\n" , method);
    handle_method(method , req);
    char *filePath = strtok(NULL , " ");
    printf("filpath is %s\n" , filePath);
    int ret = checkPath(filePath , req);
    HTTP_STATUS statusCode;
    if(ret == -1){
        statusCode = HTTP_NOT_FOUND;
    }
    else{
        statusCode = HTTP_OK;
        checkFile(req , &res);
    }
    build_statusLine(statusCode , res.statusLine);
    printf("status code is %d\n" , statusCode);
    build_headers(statusCode , req , &res);
}


void handle_client(int connfd){
    request req;
    req.clientfd = connfd;
    while(1){
        printf("\nreceiving\n");
        int sizeRecv = recv(req.clientfd , req.recbuf , 8192 , 0);
        if(sizeRecv<=0 ){
            close(req.clientfd);
            break;
        }
        req.recbuf[sizeRecv] = '\0';
        printf("recbuf printing\n %s \nrecbuf printend\n" , req.recbuf);
        handle_request(&req);
    }
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
