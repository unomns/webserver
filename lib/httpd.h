//
//  httpd.h
//  webserver
//
//  Created by Murad Nurmukhamedov on 1/7/25.
//

#ifndef httpd_h
#define httpd_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define LISTENADDR "127.0.0.1"
#define BUFFERSIZE 512

struct http_req
{
    char method[8];
    char url[128];
};
typedef struct http_req HttpReq;

struct sFile
{
    char filename[64];
    char *content;
    int size;
};
typedef struct sFile File;

HttpReq *parse_http(char *);

/* return 0 on error, or it returns a socket fd */
int srv_init(int);

/* return 0 on error, or returns the new client's socket fd */
int cli_accept(int);

void cli_conn(int, int);

/* return 0 on error, or return the data */
char *cli_read(int);

void http_headers(int, int, char *);
void http_response(int, char *, char *);

/* return 0 on error, or 1 on success */
int send_file(int, char *, File *);

/* return 0 on error */
File *read_file(char *filename);

#endif /* httpd_h */
