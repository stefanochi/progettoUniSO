#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>

typedef struct request{
    unsigned int pid;
    unsigned long gene;
} request;

typedef struct msg_request{
    long mtype;
    request req;
} msg_request;

int get_message_id(int msg_key);
int send_request(int msq_id, request * req);
