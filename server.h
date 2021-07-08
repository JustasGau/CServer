#ifndef SERVER
#define SERVER

typedef enum
{
    GET,
    POST,
    PUT,
    PATCH,
    DELETE

}HTTPTypeEnum;

struct Request {
   char  HTTPType[10];
   char  version[20];
   char  contentType[100];
   char  query[100];
   int   requestLength;
   char* body;
};

enum ParsePart {
    PART_FIRSTLINE,
    PART_HEADERS,
    PART_BODY
};

void parseString(const char * str, const char * format, ...);
void parseRequest();

#endif
