//
//  httpd.c
//  webserver
//
//  Created by Murad Nurmukhamedov on 1/7/25.
//

#include "httpd.h"

HttpReq *parse_http(char *str) {
    HttpReq *req;
    char *p;
    
    req = malloc(sizeof(HttpReq));
    memset(req, 0, sizeof(HttpReq) - 1);
    
    for (p = str; *p && *p != ' '; p++);
    
    if (*p == ' ') {
        *p = 0;
    } else {
        printf("parse_http() NOSPACE error");
        
        return 0;
    }
    
    strncpy(req->method, str, 7);
    
    for (str = ++p; *p && *p != ' '; p++);
    
    if (*p == ' ') {
        *p = 0;
    } else {
        printf("parse_http() NOSPACE2 error");
        
        return 0;
    }
    
    strncpy(req->url, str, 127);
    
    return req;
}

char *cli_read(int cli_fd) {
    static char buf[BUFFERSIZE];
    
    memset(buf, 0, BUFFERSIZE);
    if (read(cli_fd, buf, BUFFERSIZE - 1) < 0) {
        fprintf(stderr, "read() error\n");
        
        return 0;
    }
    else
        return buf;
}

int srv_init(int port) {
    int sock_fd;
    struct sockaddr_in srv;
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        fprintf(stderr, "socket() error\n");
        
        return 0;
    }
    
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(LISTENADDR);
    srv.sin_port = htons(port);
    
    if (bind(sock_fd, (struct sockaddr *)&srv, sizeof(srv))) {
        fprintf(stderr, "bind() error\n");
        close(sock_fd);
        
        return 0;
    }
    
    if (listen(sock_fd, 5)) {
        close(sock_fd);
        fprintf(stderr, "listen() error\n");
        
        return 0;
    }
    
    return sock_fd;
}

int cli_accept(int sock_fd) {
    int cli_fd;
    socklen_t addrlen;
    struct sockaddr_in cli;
    
    memset(&cli, 0, sizeof(cli));
    cli_fd = accept(sock_fd, (struct sockaddr *)&cli, &addrlen);
    
    if (cli_fd < 0) {
        fprintf(stderr, "accept() error");
        
        return 0;
    }
    
    return cli_fd;
}

void http_response(int cli_fd, char *content_type, char *data) {
	char buf[BUFFERSIZE];
	int n;
    
    n = (int)strlen(data);

	memset(buf, 0, BUFFERSIZE);
    snprintf(buf, BUFFERSIZE - 1,
             "Content-Type: %s\n"
             "Content-Length: %d"
             "\n%s\n",
             content_type, n, data);
    
    n = (int)strlen(buf);
    write(cli_fd, buf, n);
}

void http_headers(int cli_fd, int code) {
	char buf[BUFFERSIZE];
	int n;

	memset(buf, 0, BUFFERSIZE);	
	snprintf(buf, BUFFERSIZE - 1,
			"HTTP/1.0 %d OK\n"
			"Server: httpd.c\n"
			"Cache-Control: no-store, no-cache, max-age=0, private\n"
			"Content-Language: en\n"
			"Enpries: -1\n"
			"X-Frame-Options: SAMEORIGIN\n",
			code
	);

	n = (int)strlen(buf);
	write(cli_fd, buf, n);
}

/*
			"Content-Type: %s\n"
*/

void cli_conn(int srv_fd, int cli_fd) {
    HttpReq *req;
    char *p;
		char *res;
    
    p = cli_read(cli_fd);
    if (!p) {
        close(cli_fd);
        
        return;
    }
    
    req = parse_http(p);
    if (!req) {
        close(cli_fd);
        
        return;
    }

		if (!strcmp(req->method, "GET") || !strcmp(req->url, "/api/home_page")) {
			res = "<html>Hello World</html>";
			http_headers(cli_fd, 200);
			http_response(cli_fd, "text/html", res);
		}
		else {
            res = "File not found";
            http_headers(cli_fd, 404);
			http_response(cli_fd, "text/plain", res);
		}
    
    free(req);
    close(cli_fd);
}

