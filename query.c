#include <string.h>
#include "query.h"

// TODO account for parameters
char* parseFileType(char* query) {
    char* ext = strrchr(query, '.');
    if (!ext) {
        return "";
    } else {
        return ext + 1;
    }
} 

char* parseMIMEType(char* query) {
    const char* filetype = parseFileType(query);
    if (strcmp(filetype, "js") == 0){
        return "text/javascript";
    } else if (strcmp(filetype, "css") == 0) {
        return "text/css";
    } else if (strcmp(filetype, "html") == 0) {
        return "text/html";
    } else {
        return "text/plain";
    }
}