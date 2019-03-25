#include <stdlib.h>
#include "msq.h"

int create_msq(int msq_key){
    return msgget(msq_key, IPC_CREAT | 0600);
}

int remove_msq(int msq_id){
    //printf("[%d] removing msq\n", getpid());
    return msgctl(msq_id, IPC_RMID, NULL);
}

int send_request(int msq_id, request * req){
    msg_request msg_req;
    msg_req.mtype = 1;
    msg_req.req = *req;

    if(msgsnd(msq_id, &msg_req, sizeof(request), 0) == -1){
        //fprintf(stderr, "[%d] send_request: %s\n", getpid(), strerror(errno));
        return -1;
    }
}

int send_response(int msq_id, int res){
    msg_response msg_res;
    msg_res.mtype = 2;
    msg_res.res = res;
    if(msgsnd(msq_id, &msg_res, sizeof(int), 0) == -1){
        //fprintf(stderr, "[%d] send_response: %s\n",getpid(), strerror(errno));
        return -1;
    }
    return 0;
}

int wait_request(int msq_id, request * req){
    msg_request msg_req;
    if(msgrcv(msq_id, &msg_req, sizeof(request), 1, 0) == -1){
        //fprintf(stderr, "[%d] wait_request: %s\n", getpid(), strerror(errno));
        return -1;
    }
    *req = msg_req.req;
    return 0;
}

int wait_response(int msq_id, int * res){
    msg_response msg_res;
    if(msgrcv(msq_id, &msg_res, sizeof(int), 2, 0) == -1){
        //fprintf(stderr, "[%d] wait_response: %s\n", getpid(), strerror(errno));
        return -1;
    }
    *res = msg_res.res;
    return 0;
}
