#include "population.h"
#include <unistd.h>
#include "msg_request.h"
int main(int argc, char ** argv){

    int id_semReady;
    int key_semReady = getppid();

    msg_request msg_req;

    int msq_id = get_message_id(getpid());
    id_semReady = get_sem_id(key_semReady);

    ind_ready(id_semReady);
    wait_ready(id_semReady);

    msgrcv(msq_id, &msg_req, sizeof(request), 1,  0);

    printf("[%d] request from:%d\n", getpid(), msg_req.req.pid);


    exit(0);
}
