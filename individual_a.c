#include <unistd.h>
#include <signal.h>
#include "population.h"
#include "msq.h"
#include "shm.h"

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

void set_handler(){
    struct sigaction act;
    struct sigaction oldact;
    sigset_t my_mask;
    sigemptyset(&my_mask);
    act.sa_handler = &intHandler;
    act.sa_mask = my_mask;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, &oldact);
}

void refuse_all(int msq_id){
    msg_request msg_req;
    request req;
    while(msgrcv(msq_id, &msg_req, sizeof(request), 1, IPC_NOWAIT) != -1){
        req = msg_req.req;
        int msq_b = msgget(req.pid, 0600);
        send_response(msq_b, 0);
    }
}

int main(int argc, char ** argv){

    set_handler();

    population * pop;
    pop = createAttach (getppid(), 0);
    individual* ind_list;
    ind_list = (individual*) (pop + 1);

    int id_sem;
    int key_semReady = getppid();

    id_sem = get_sem_id(key_semReady);

    int msq_a = create_msq(getpid());

    
    ind_ready(id_sem);
    wait_ready(id_sem);

    individual * my_ind_ptr, my_ind;
    entry_read(id_sem, pop);
    my_ind_ptr = (get_ind_by_pid(getpid(), ind_list, pop));
    my_ind = *my_ind_ptr;
    
    exit_read(id_sem, pop);

    request req;

    while(keepRunning){
        if(wait_request(msq_a, &req) != -1){
            printf("[%d] request from:%d\n", getpid(), req.pid);
            int msq_b = msgget(req.pid, 0600);
            if(req.gene == my_ind.gene){
                printf("[%d] accepting\n", getpid());
                send_response(msq_b, 1);
                //fprintf(stderr, "[%d] entry write\n", getpid());
                entry_write(id_sem, pop);
                //fprintf(stderr, "[%d] entry write finish\n", getpid());
                my_ind_ptr->status = 1;
                //fprintf(stderr, "[%d] exit write\n", getpid());
                exit_write(id_sem, pop);
                //fprintf(stderr, "[%d] refuse all\n", getpid());
                refuse_all(msq_a);
                keepRunning = 0;
            }else{
                printf("[%d] refusing\n", getpid());
                relationship * rel = get_list_relationships(pop);
                insert_relationship(rel, pop, my_ind.pid, req.pid);
                send_response(msq_b, 0);
            }
        }
    }

    if(remove_msq(msq_a)){
        fprintf(stderr, "[%d] failed to remove msq: %s\n", getpid(),  strerror(errno));
    }

    exit(0);
}
