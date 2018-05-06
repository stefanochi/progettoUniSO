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

int main(int argc, char ** argv){

    set_handler();

    population * pop;
    pop = createAttach (getppid(), 0);
    individual* ind_list;
    ind_list = (individual*) pop + sizeof(population);

    int id_semReady;
    int key_semReady = getppid();

    id_semReady = get_sem_id(key_semReady);

    ind_ready(id_semReady);
    wait_ready(id_semReady);

    int msq_a = get_message_id(getpid());

    individual my_ind;
    my_ind = *(get_ind_by_pid(getpid(), ind_list, pop));

    request req;

    while(keepRunning){
        if(wait_request(msq_a, &req) != -1){
            printf("[%d] request from:%d\n", getpid(), req.pid);
            int msq_b = get_message_id(req.pid);
            if(req.gene == my_ind.gene){
                printf("[%d] accepting\n", getpid());
                send_response(msq_b, 1);
                keepRunning = 0;
            }else{
                printf("[%d] refusing\n", getpid());
                send_response(msq_b, 0);
            }
        }
    }

    remove_msq(msq_a);

    exit(0);
}
