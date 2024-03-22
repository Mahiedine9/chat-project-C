#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#include <pthread.h>

#include "utils.h"
#include "console.h"
#include "protocol.h"

struct sockaddr_in s_addr;   // server address
struct hostent     *server;  // address structure

int  sd, s_port;             // socket and port
char *pseudo;                // my pseudo


static int verbose = 0;
static int batch = 0;
char *batch_name = NULL;
char *batch_outname = NULL;
FILE *batch_file = NULL;
FILE *batch_outfile = NULL;

#define PRINT(...)                         \
    do {                                   \
        if (verbose)                       \
            fprintf(stderr, __VA_ARGS__);  \
    } while (0)


/**
   Receiver thread.
*/
void *receiver(void *arg)
{
    struct message msg;
    
    while(1) {
        char buffer[OUTPUT_BUFFER_SIZE];
        
        int n = read(sd, &msg, sizeof(msg));
        if (n <= 0) SYS_ERR("Error in reading from socket");

        sprintf(buffer, "%s : %s", msg.pseudo, msg.msg);
        if (!batch) console_output(buffer);

        if (batch_outfile) fprintf(batch_outfile, "%s\n", buffer);
    }
}


int main(int argc, char *argv[])
{
    pthread_t receiver_td;
    
    if (argc < 4)
        USR_ERR("usage: %s [-v | -b input_file | -l out_file] <servername> <port> <pseudo>\n", argv[0]);


    int opt;
    while ((opt = getopt(argc, argv, "vb:l:")) != -1) {
        //printf("%c\n", opt);
        switch(opt) {
        case 'v' : verbose = 1;
            break;
        case 'b' :
            batch =  1;
            batch_name = optarg;
            break;
        case 'l' :
            batch_outname = optarg;
            break;
        default : 
            USR_ERR("Wrong option. Usage: %s [-v | -b input_file | -l out_file] <servername> <port> <pseudo>\n", argv[0]);
        }
    }
    
    pseudo = argv[optind + 2];
    if (strlen(pseudo) > (PSEUDO_SIZE - 1))
        USR_ERR("Pseudo must be no more than %d characters\n", PSEUDO_SIZE);
        
    // look for server
    PRINT("Server address lookup...\n");
    server = gethostbyname(argv[optind]);
    if (server == NULL)
        SYS_ERR("Error in gethostbyname, cannot find server");
    
    PRINT("... found\n");
    s_port = atoi(argv[optind+1]);
    PRINT("Server port : %d\n", s_port);

    bzero(&s_addr, sizeof(struct sockaddr_in));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(s_port);
    
    bcopy((char*)server->h_addr, 
          (char*)&s_addr.sin_addr.s_addr, 
          server->h_length);

    sd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(sd, CAST_ADDR(&s_addr), sizeof(struct sockaddr_in)) < 0)
        SYS_ERR("Cannot connect");

    PRINT("Connection ok\n");

    PRINT("Creating receiving thread\n");
    // create a thread to receive messages and update the screen
    int r = pthread_create(&receiver_td, NULL, receiver, NULL);
    if (r < 0) SYS_ERR("Cannot create the receiving thread");

    PRINT("thread created\n");

    PRINT("Sending pseudo\n");
    write(sd, pseudo, strlen(pseudo));
    PRINT("Pseudo sent\n");
    // pseudo has been sent, now I am ready to interact

    if (!batch) {
        console_init();
        char info[128];
        sprintf(info, "Server: %s ; port: %d; pseudo: %s", argv[optind], s_port, pseudo);
        console_info(info);
    } else {
        PRINT("Opening input file %s\n", batch_name);
        batch_file = fopen(batch_name, "r");
    }
    if (batch_outname) {
        PRINT("Opening output file %s\n", batch_outname);
        batch_outfile = fopen(batch_outname, "w");
    }
    
    char *buffer = NULL;

    if (!batch) buffer = malloc(MSG_SIZE);
    
    while(1) {
        struct message msg;
        strcpy(msg.pseudo, pseudo); 
        if (batch) {
            size_t len;
            r = getline(&buffer, &len, batch_file);
            if (r > 0) buffer[len-1] = 0;
            usleep(5000); // When in batch mode, wait a little before sending next message 
        }
        else {
            r = console_readline(buffer, MSG_SIZE);
        }
        if (r == -1 || (strcmp(buffer, "[quit]") == 0)) {
            if (!batch) console_exit();
            PRINT("Quit\n");
            break;
        }
        strcpy(msg.msg, buffer);
        write(sd, &msg, sizeof(msg));
    }

    PRINT("Closing ...\n");
    if (batch) sleep(1);
    
    free(buffer);
    PRINT("Free completed\n");
    if (batch) fclose(batch_file);
    if (batch_outfile) fclose(batch_outfile);
}
