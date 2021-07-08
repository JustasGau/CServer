#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdarg.h>
#include "server.h"
#include <stdbool.h>

#define PORT 8080

void parseString(const char * str, const char * format, ... ) {
    va_list args;
    va_start(args, format);
    vsscanf(str, format, args);
    va_end(args);
}

void parseRequest(char *str, struct Request *request) {
    enum ParsePart currentPart = PART_FIRSTLINE;
    char *token, *temp, *header;
    bool firstLine = true;
    while ((token = strtok_r(str, "\n", &str)) && currentPart != PART_BODY) {
        switch (currentPart)
        {
        case PART_FIRSTLINE:
            parseString(token, "%s %s %s", request->HTTPType, request->query, request->version);
            currentPart = PART_HEADERS;
            break;
        case PART_HEADERS:
            header = strtok_r(token, " ", &token);
            if (strcmp(header, "Content-Type:") == 0) {
                strcpy(request->contentType, token);
            } else if (strcmp(header, "Content-Length:") == 0) {
                request->requestLength = strtol(token, NULL, 10);
            } else if (strlen(header) == 1) {
                currentPart = PART_BODY;
                request->body = strdup(str);
            }
            break;
        default:
            break;
        }
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    struct Request request;
    
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    
    /* htonl converts a long integer (e.g. address) to a network representation */ 
    /* htons converts a short integer (e.g. port) to a network representation */ 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        char requestString[30000] = {0};
        valread = read(new_socket, requestString, 30000);
        printf("%s\n", requestString);
        parseRequest(requestString, &request);
        printf("%s\n", request.HTTPType);
        printf("%s\n", request.query);
        printf("%s\n", request.version);
        printf("%s\n", request.contentType);
        printf("%d\n", request.requestLength);
        printf("%s\n", request.body);
        char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
        write(new_socket , response , strlen(response));
        close(new_socket);
    }
    return 0;
}