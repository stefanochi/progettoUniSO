#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "msq.h"

int get_message_id(int msq_key){
    return msgget(msq_key, IPC_CREAT | 0600);
}

int send_request(int msq_id, request * req){
    msg_request * msg_req = malloc(sizeof(msg_request));
    if(msg_req == NULL){
        fprintf(stderr, "Failed malloc in send_request:%s\n", strerror(errno));
        exit(1);
    }

    msg_req->mtype = 1;
    msg_req->req = *req;

    if(msgsnd(msq_id, msg_req, sizeof(request), 0) == -1){
        printf("%s\n", strerror(errno));
        exit(1);
    }

    free(req);
}
