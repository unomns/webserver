//
//  main.c
//  webserver
//
//  Created by Murad Nurmukhamedov on 1/7/25.
//

#include "lib/httpd.h"

int main(int argc, const char *argv[])
{
    const char *port;
    int srv_d, cli_d;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <listening port>\n", argv[0]);

        return -1;
    }
    else
        port = argv[1];

    srv_d = srv_init(atoi(port));

    if (!srv_d)
    {
        return -1;
    }

    printf("Listening on %s:%s\n", LISTENADDR, port);

    while (1)
    {
        cli_d = cli_accept(srv_d);
        if (!cli_d)
        {
            continue;
        }

        printf("Client connected\n");
        if (!fork())
            cli_conn(srv_d, cli_d);
    }

    return -1;
}
