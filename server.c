#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "server.h"
#include "query.h"

#define PORT 8080

void handelError(char* error) {
    perror(error);
    exit(EXIT_FAILURE);
}

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
char* respondGET(struct Request request) {
    char *body= NULL, *headers = NULL, *fullResponse = NULL, *path = NULL;
    FILE *fp;
    char http[] = "HTTP/1.1 200 OK\n";
    if (strcmp(request.query, "/") == 0) {
        path = malloc(sizeof(char) * (strlen(ROOT_FOLDER) + strlen(INDEX_FILE) + 2));
        sprintf(path, "%s%s", ROOT_FOLDER, INDEX_FILE);
    } else {
        path = malloc(sizeof(char) * (strlen(request.query) + strlen(ROOT_FOLDER) + 2));
        sprintf(path, "%s%s", ROOT_FOLDER, request.query);
    }
    fp = fopen(path, "r");
    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            long bufsize = ftell(fp);
            if (bufsize == -1) { 
                perror("in ftell");
            }
            char *contentType = malloc(50 * sizeof(char));
            sprintf(contentType, "Content-Type: %s\n", parseMIMEType(path));
            char *contentLength = malloc(25 * sizeof(char));
            sprintf(contentLength, "Content-Length: %ld\n\n", bufsize);
            /* Allocate our buffer to that size. */
            body = malloc(sizeof(char) * (bufsize + 1));
            headers = malloc(sizeof(char) * (strlen(contentLength) + strlen(http) + strlen(contentType) + 1));
            strcat(headers, http);
            strcat(headers, contentType);
            strcat(headers, contentLength);
            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) {
                handelError("in fseek");
            }
            /* Read the entire file into memory. */
            size_t newLen = fread(body, sizeof(char), bufsize, fp);
            if (ferror( fp ) != 0) {
                fputs("Error reading file", stderr);
            } else {
                body[newLen++] = '\0'; /* Just to be safe. */
            }
            fullResponse = malloc(strlen(body) + strlen(headers));
            strcpy(fullResponse, headers);
            strcat(fullResponse, body);
            free(headers);
            free(body);
        }
        free(path);
        fclose(fp);
    } else {
        char notFound[] = "HTTP/1.1 404 NOT FOUND\n";
        fullResponse = malloc(strlen(notFound));
        strcpy(fullResponse, notFound);
        printf("GET file not found \n");
    }
    return fullResponse;
}

char* respond(struct Request request) {
    if (strcmp(request.HTTPType, "GET") == 0) {
        return respondGET(request);
    } else if (strcmp(request.HTTPType, "PUT") == 0) {
        printf("PUT\n");
    } else if (strcmp(request.HTTPType, "DELETE") == 0) {
        printf("DELETE\n");
    } else if (strcmp(request.HTTPType, "HEAD") == 0) {
        printf("HEAD\n");
    } else if (strcmp(request.HTTPType, "PATCH") == 0) {
        printf("PATCH\n");
    } else if (strcmp(request.HTTPType, "POST") == 0) {
        printf("POST\n");
    }
}

void freeStruct(struct Request *request) {
    free(request->body);
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    struct Request request;
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        handelError("In socket");
    }
    // adds option to reuse socet while it is still active. Avoids "address in use" error while testing 
    // restarting server
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        handelError("setsockopt(SO_REUSEADDR) failed");

    /* htonl converts a long integer (e.g. address) to a network representation */ 
    /* htons converts a short integer (e.g. port) to a network representation */ 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        handelError("In bind");
    }
    if (listen(server_fd, 10) < 0) {
        handelError("In listen");
    }
    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            handelError("In accept");
        }
        
        char requestString[30000] = {0};
        valread = read(new_socket, requestString, 30000);
        printf("Request: \n");
        printf("%s\n", requestString);
        parseRequest(requestString, &request);
        char *response = respond(request);
        printf("Response: \n");
        printf("%s\n", response);
        if (response == NULL) {
            write(new_socket, "", 1);
            printf("No response for query: %s\n", request.query);
        } else {
            write(new_socket, response, strlen(response));
        }
        close(new_socket);
        freeStruct(&request);
        free(response);
    }
    return 0;
}