#include <unistd.h>
#include <signal.h>
#include "population.h"
#include "msq.h"
#include "shm.h"
#include "sem.h"

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

    int id_sem_ready, id_sem_shm, id_sem_relation;
    int key_semReady = getppid();

    id_sem_ready = get_sem_ready(key_semReady);
    id_sem_shm = get_sem_id(1234);
    id_sem_relation = get_sem_id(5432);

    int msq_a = create_msq(getpid());

    
    ind_ready(id_sem_ready);
    wait_ready(id_sem_ready);

    individual * my_ind_ptr, my_ind;
    entry_write(id_sem_shm, &(pop->writeCount_shm));
    my_ind_ptr = (get_ind_by_pid(getpid(), ind_list, pop));
    my_ind_ptr->status = 0;
    my_ind = *my_ind_ptr;
    exit_write(id_sem_shm, &(pop->writeCount_shm));

    request req;

    int target = my_ind.gene;

    while(keepRunning){
        if(wait_request(msq_a, &req) != -1){
            printf("[%d] request from:%d\n", getpid(), req.pid);
            int msq_b = msgget(req.pid, 0600);
            if(req.gene == my_ind.gene){
                printf("[%d] accepting\n", getpid());
                send_response(msq_b, 1);
                
                entry_write(id_sem_shm, &(pop->writeCount_shm));
                my_ind_ptr->status = req.pid;
                refuse_all(msq_a);
                exit_write(id_sem_shm, &(pop->writeCount_shm));
                
                keepRunning = 0;
            }else{
                printf("[%d] refusing\n", getpid());
                relationship * rel = get_list_relationships(pop);
                

                entry_write(id_sem_relation, &(pop->writeCount_relation));
                insert_relationship(rel, pop, my_ind.pid, req.pid);
                exit_write(id_sem_relation, &(pop->writeCount_relation));
                
                entry_read(id_sem_relation, &(pop->readCount_relation));
                print_relationship(rel, pop);
                exit_read(id_sem_relation, &(pop->readCount_relation));
                
                send_response(msq_b, 0);
                entry_read(id_sem_relation, &(pop->readCount_relation));
                if(request_from_all(rel, pop, my_ind.pid)){
                    exit_read(id_sem_relation, &(pop->readCount_relation));
                    printf("[%d] resetting relationships\n", getpid());

                    entry_write(id_sem_relation, &(pop->writeCount_relation));
                    remove_relationship(rel, pop, my_ind.pid);
                    exit_write(id_sem_relation, &(pop->writeCount_relation));
                }else{
                    exit_read(id_sem_relation, &(pop->readCount_relation));
                }
            }
        }
    }

    if(remove_msq(msq_a)){
        fprintf(stderr, "[%d] failed to remove msq: %s\n", getpid(),  strerror(errno));
    }

    exit(0);
}
