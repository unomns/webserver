//
//  httpd.c
//  webserver
//
//  Created by Murad Nurmukhamedov on 1/7/25.
//

#include "httpd.h"

HttpReq *parse_http(char *str)
{
    HttpReq *req;
    char *p;

    req = malloc(sizeof(HttpReq));
    memset(req, 0, sizeof(HttpReq) - 1);

    for (p = str; *p && *p != ' '; p++)
        ;

    if (*p == ' ')
    {
        *p = 0;
    }
    else
    {
        printf("parse_http() NOSPACE error");

        return 0;
    }

    strncpy(req->method, str, 7);

    for (str = ++p; *p && *p != ' '; p++)
        ;

    if (*p == ' ')
    {
        *p = 0;
    }
    else
    {
        printf("parse_http() NOSPACE2 error");

        return 0;
    }

    strncpy(req->url, str, 127);

    return req;
}

char *cli_read(int cli_fd)
{
    static char buf[BUFFERSIZE];

    memset(buf, 0, BUFFERSIZE);
    if (read(cli_fd, buf, BUFFERSIZE - 1) < 0)
    {
        fprintf(stderr, "read() error\n");

        return 0;
    }
    else
        return buf;
}

int srv_init(int port)
{
    int sock_fd;
    struct sockaddr_in srv;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        fprintf(stderr, "socket() error\n");

        return 0;
    }

    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(LISTENADDR);
    srv.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr *)&srv, sizeof(srv)))
    {
        fprintf(stderr, "bind() error\n");
        close(sock_fd);

        return 0;
    }

    if (listen(sock_fd, 5))
    {
        close(sock_fd);
        fprintf(stderr, "listen() error\n");

        return 0;
    }

    return sock_fd;
}

int cli_accept(int sock_fd)
{
    int cli_fd;
    socklen_t addrlen;
    struct sockaddr_in cli;

    memset(&cli, 0, sizeof(cli));
    cli_fd = accept(sock_fd, (struct sockaddr *)&cli, &addrlen);

    if (cli_fd < 0)
    {
        fprintf(stderr, "accept() error\n");

        return 0;
    }

    return cli_fd;
}

void http_response(int cli_fd, char *content_type, char *data)
{
    char buf[BUFFERSIZE];
    int n;

    n = (int)strlen(data);

    memset(buf, 0, BUFFERSIZE);
    snprintf(buf, BUFFERSIZE - 1,
             "Content-Type: %s\n"
             "Content-Length: %d\n"
             "\n%s\n",
             content_type, n, data);

    n = (int)strlen(buf);
    write(cli_fd, buf, n);
}

void http_headers(int cli_fd, int status_code, char *status_text)
{
    char buf[BUFFERSIZE];
    int n;

    memset(buf, 0, BUFFERSIZE);
    snprintf(buf, BUFFERSIZE - 1,
             "HTTP/1.0 %d %s\n"
             "Server: lib/httpd.c\n"
             "Cache-Control: no-store, no-cache, max-age=0, private\n"
             "Content-Language: en\n"
             "Enpries: -1\n"
             "X-Frame-Options: SAMEORIGIN\n",
             status_code, status_text);

    n = (int)strlen(buf);
    write(cli_fd, buf, n);
}

File *read_file(char *filename)
{
    char buf[BUFFERSIZE];
    int n, x, fd;
    File *f;

    fd = open(filename, O_RDONLY);
    if (fd < 0)
        return 0;

    f = malloc(sizeof(struct sFile));
    if (!f)
    {
        close(fd);
        return 0;
    }

    strncpy(f->filename, filename, 63);
    f->content = malloc(BUFFERSIZE);

    x = 0; /* bytes read */
    while (1)
    {
        memset(buf, 0, BUFFERSIZE);
        n = read(fd, buf, BUFFERSIZE);

        if (!n)
            break;
        else if (x == -1)
        {
            close(fd);
            free(f->content);
            free(f);

            return 0;
        }

        memcpy((f->content) + x, buf, n);
        x += n;
        f->content = realloc(f->content, (BUFFERSIZE + x));
    }

    f->size = x;
    close(fd);

    return f;
}

int send_file(int cli_fd, char *content_type, File *file)
{
    char buf[BUFFERSIZE];
    char *p;
    int n, x;

    if (!file)
        return 0;

    memset(buf, 0, BUFFERSIZE);
    snprintf(buf, BUFFERSIZE - 1,
             "Content-Type: %s\n"
             "Content-Length: %d\n\n",
             content_type, file->size);

    n = strlen(buf);
    write(cli_fd, buf, n);

    n = file->size;
    p = file->content;
    while (1)
    {
        x = (int)write(cli_fd, p, (n < BUFFERSIZE) ? n : BUFFERSIZE);
        if (x < 1)
            return 0;

        n -= x;

        if (n < 1)
            break;
        else
            p += x;
    }

    return 1;
}

void cli_conn(int srv_fd, int cli_fd)
{
    HttpReq *req;
    char *p;
    char *res;
    char str[96];
    File *f;

    p = cli_read(cli_fd);
    if (!p)
    {
        close(cli_fd);

        return;
    }

    req = parse_http(p);
    if (!req)
    {
        close(cli_fd);

        return;
    }

    if (strstr(req->url, ".."))
    {
        http_headers(cli_fd, 403, "Forbidden");

        res = "Access denied";
        http_response(cli_fd, "text/plain", res);
    }
    else if (!strcmp(req->method, "GET") && !strncmp(req->url, "/img/", 5))
    {
        memset(str, 0, 96);
        snprintf(str, 95, ".%s", req->url);

        f = read_file(str);
        if (!f)
        {
            res = "File not found";
            http_headers(cli_fd, 404, "Not Found");
            http_response(cli_fd, "text/plain", res);
        }
        else
        {
            http_headers(cli_fd, 200, "OK");

            if (!send_file(cli_fd, "image/png", f))
            {
                res = "HTTP server error";
                http_headers(cli_fd, 500, "Internal Server Error");
                http_response(cli_fd, "text/plain", res);
            }
        }
    }
    else if (!strcmp(req->method, "GET") && !strcmp(req->url, "/api/home"))
    {
        res = "<html><img src='/img/burger.png' alt='burger image' /></html>";
        http_headers(cli_fd, 200, "OK");
        http_response(cli_fd, "text/html", res);
    }
    else
    {
        res = "File not found";
        http_headers(cli_fd, 404, "Not Found");
        http_response(cli_fd, "text/plain", res);
    }

    free(req);
    close(cli_fd);
}
