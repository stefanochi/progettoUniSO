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
    struct msqid_ds buf;
    if(msgctl(msq_id, IPC_STAT, &buf) == -1){
        fprintf(stderr, "[%d] IPC_STAT:%s\n", getpid(), strerror(errno));
    }
    buf.msg_perm.mode = 0400;
    if(msgctl(msq_id, IPC_SET, &buf) == -1){
        fprintf(stderr, "[%d] IPC_STAT:%s\n", getpid(), strerror(errno));
    }
    while(msgrcv(msq_id, &msg_req, sizeof(request), 1, IPC_NOWAIT) != -1){
        req = msg_req.req;
        int msq_b = msgget(req.pid, 0600);
        send_response(msq_b, 0);
    }
}

long next_target(long gene){
    static int index = 0;
    for(int i=2; i<gene/2; i++){
        if(gene%i == 0){
            if(i-2 == index){
                index++;
                return gene/i;
            }
        }
    }
    return 1;
}

int main(int argc, char ** argv){

    set_handler();

    population * pop;
    pop = createAttach (getppid(), 0);
    individual* ind_list;
    ind_list = (individual*) (pop + 1);
    relationship * rel = get_list_relationships(pop);

    int id_sem_ready, id_sem_shm, id_sem_relation;
    int key_semReady = getppid();

    id_sem_ready = get_sem_ready(key_semReady);
    id_sem_shm = get_sem_id(1234);
    id_sem_relation = get_sem_id(5432);

    int msq_a = create_msq(getpid());

    //wait for all the other individuals
    ind_ready(id_sem_ready);
    wait_ready(id_sem_ready);

    individual * my_ind_ptr, my_ind;
    entry_write(id_sem_shm, &(pop->writeCount_shm));
    my_ind_ptr = (get_ind_by_pid(getpid(), ind_list, pop));
    my_ind_ptr->status = 0;
    my_ind = *my_ind_ptr;
    exit_write(id_sem_shm, &(pop->writeCount_shm));

    request req;

    unsigned long target = my_ind.gene;

    while(keepRunning){
        wait_ready(id_sem_ready);
        if(wait_request(msq_a, &req) != -1){

            int msq_b = msgget(req.pid, 0600);
            int gcd_partner = gcd(my_ind.gene, req.gene);
            if(gcd_partner >= target){
                if(send_response(msq_b, 1) != -1){

                    entry_write(id_sem_shm, &(pop->writeCount_shm));
                    my_ind_ptr->status = req.pid;
                    exit_write(id_sem_shm, &(pop->writeCount_shm));

                    refuse_all(msq_a);

                    keepRunning = 0;
                }
            }else{
                //add b to the list of refused
                entry_write(id_sem_relation, &(pop->writeCount_relation));
                insert_relationship(rel, pop, my_ind.pid, req.pid);
                exit_write(id_sem_relation, &(pop->writeCount_relation));

                send_response(msq_b, 0);

                //check if every individual b contacted, if so reduce target and reset contacted list
                entry_read(id_sem_relation, &(pop->readCount_relation));
                entry_read(id_sem_shm, &(pop->readCount_shm));
                int reset = request_from_all(rel, pop, my_ind.pid);
                exit_read(id_sem_shm, &(pop->readCount_shm));
                exit_read(id_sem_relation, &(pop->readCount_relation));
                if(reset){
                    target = next_target(my_ind.gene);
                    entry_write(id_sem_relation, &(pop->writeCount_relation));
                    remove_relationship(rel, pop, my_ind.pid);
                    exit_write(id_sem_relation, &(pop->writeCount_relation));
                }
            }
        }
    }

    entry_write(id_sem_shm, &(pop->writeCount_shm));
    if(my_ind_ptr->status == 0){
        my_ind_ptr->status = -1;
    }
    exit_write(id_sem_shm, &(pop->writeCount_shm));
    refuse_all(msq_a);

    //remove individual a from contacted list
    entry_write(id_sem_relation, &(pop->writeCount_relation));
    remove_relationship(rel, pop, my_ind.pid);
    exit_write(id_sem_relation, &(pop->writeCount_relation));

    if(remove_msq(msq_a)){
        fprintf(stderr, "[%d] failed to remove msq: %s\n", getpid(),  strerror(errno));
    }

    exit(0);
}
