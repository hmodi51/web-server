#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT "8080"
#define bufSIZE 4096
#define PATH "index.html"

char recbuf[bufSIZE];
char temprecbuf[bufSIZE];
char buf[bufSIZE];
char data[bufSIZE];
const char* del = "\r\n";
const char* pwd = "/home/harsh/Desktop/PersonalProjects/web-server/";
char path[bufSIZE];


typedef enum HTTP_STATUS {
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404
} HTTP_STATUS;


void handle_method(char* requestLine){

}

void handle_404(){

}

void handle_200(){

}

void checkPath(char* filePath){
    if (strcmp(filePath , "/") == 0){
        snprintf(path , bufSIZE , "%s%s" , pwd , PATH);
    }
    else{
        snprintf(path , bufSIZE , "%s%s" , pwd , filePath+1);
        printf("default path is %s\n" , path);
    }
}

HTTP_STATUS checkFile(char* path){
    FILE *fptr;
    fptr = fopen(path , "r");
    if(fptr == NULL){
        return HTTP_NOT_FOUND;
    }
    else{
        return HTTP_OK;
    }
}


void handle_client(int connfd){
    recv(connfd , recbuf , bufSIZE , 0);
    strcpy(temprecbuf , recbuf);
    char* tokens = strtok(temprecbuf , del);
    char *requestLine = strtok(tokens , " ");
    handle_method(requestLine);
    printf("request line is %s\n" , requestLine);
    char *filePath = strtok(NULL , " ");
    printf("filpath is %s\n" , filePath);
    // printf(tokens);
    printf(recbuf);
    checkPath(filePath);
    HTTP_STATUS statusCode = checkFile(path);
    printf("status code is %d\n" , statusCode);
    // if (strcmp(filePath ,  "/") ==0){
    //     snprintf(path , bufSIZE , "%s%s" , pwd , PATH);
    //     printf("default path is %s\n" , path);
    // }
    // else{
    //     snprintf(path , bufSIZE , "%s%s" , pwd , filePath+1);
    //     printf("default path is %s\n" , path);
    // }
    // FILE *fptr;
    // fptr = fopen(path , "r");
    // if(fptr == NULL){
    //     handle_404();
    // }
    // char *buf = "HTTP/1.1 200 OK\r\n"
    // "Content-Type: text/html\r\n"
    // "Connection: close\r\n"
    // "\r\n";
    // send(connfd , buf , strlen(buf) , 0);
    // printf(buf);
    // while(fgets(data , bufSIZE , fptr) != NULL){
    //     send(connfd , data , strlen(data) , 0);
    //     printf(data);
    // }
    close(connfd);
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
