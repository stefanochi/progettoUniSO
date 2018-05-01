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

typedef struct msg_response{
    long mtype;
    int res;
} msg_response;

int get_message_id(int msg_key);
int remove_msq(int msq_id);
int send_request(int msq_id, request * req);
int send_response(int msq_id, int res);
int wait_request(int msq_id, request * req);
int wait_response(int msg_id, int * res);
