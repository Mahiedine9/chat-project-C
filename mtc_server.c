#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "utils.h"
#include "protocol.h"

int init_sd(int myport);
void do_service(int sd);

static int verbose = 0;

#define PRINT(...)                         \
    do {                                   \
        if (verbose)                       \
            fprintf(stderr, __VA_ARGS__);  \
    } while (0)


int main(int argc, char *argv[])
{
    struct sockaddr_in c_add;
    int base_sd, curr_sd;
    socklen_t addrlen;
    int myport;
    int err = 0;
    int opt;
  
    if (argc < 2)
        USR_ERR("usage: server [-v] <port>");

    while ((opt = getopt(argc, argv, "v")) != -1) {
        if (opt == 'v') verbose = 1;
        else USR_ERR("usage: server [-v] <port>");
    }

    if (optind >= argc) USR_ERR("Missing port. Usage: server [-v] <port>");
        
    myport = atoi(argv[optind]);
  
    base_sd = init_sd(myport);

    while (!err) {
        addrlen = sizeof(c_add);
        curr_sd = accept(base_sd, CAST_ADDR(&c_add), &addrlen);
        if (curr_sd < 0)
            SYS_ERR("Accept failed!");

        PRINT("Client connected\n");
    
        do_service(curr_sd);
        close(curr_sd);  
    }
    close(base_sd);
}


int init_sd(int myport)
{
    struct sockaddr_in my_addr;
    int sd;
    
    bzero(&my_addr, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myport);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    PRINT("Preparing socket\n");
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) SYS_ERR("Error in creating socket");
    
    if (bind(sd, CAST_ADDR(&my_addr), sizeof(my_addr)) < 0)
        SYS_ERR("Bind failed!");

    if (listen(sd, 5) < 0)
        SYS_ERR("listen failed!");
    
    PRINT("Server listening on port %d\n", myport);

    return sd;
}

void do_service(int sd)
{
    int i, l=1;
    char buffer[MSG_SIZE];

    struct message msg;

    // first read the pseudo
    int r = read(sd, buffer, PSEUDO_SIZE);
    if (r < 0) SYS_ERR("Error in reading from the socket");

    buffer[r] = 0;
    char mypseudo[PSEUDO_SIZE];
    strcpy(mypseudo, buffer);
    PRINT("Received pseudo %s\n", mypseudo);
        
    do {
        l = read(sd, &msg, sizeof(msg));
        if (l == 0) break;
        strcpy(buffer, msg.msg);
        PRINT("Server: received %s\n", buffer);

        i = 0;
        while (buffer[i] != 0) {
            buffer[i] = toupper(buffer[i]);
            i++;
        }
        PRINT("Server: sending %s\n", buffer);

        strcpy(msg.pseudo, mypseudo);
        strcpy(msg.msg, buffer);
        
        write(sd, &msg, sizeof(msg));
    } while (l!=0);
}
