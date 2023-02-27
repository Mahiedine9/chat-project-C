#pragma once

#define SYS_ERR(msg)     \
    do {                 \
        perror(msg);     \
        exit(-1);        \
    } while (0);

#define USR_ERR(...)                            \
    do {                                        \
        fprintf(stderr, __VA_ARGS__);           \
        exit(-1);                               \
    } while(0);


#define CAST_ADDR(x) (struct sockaddr *)(x)

