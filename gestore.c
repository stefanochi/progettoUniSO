#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "shm.h"
#include "population.h"

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int argc, char ** argv){

    int init_people = 20, i=0, j=0, status;
    unsigned long genes = 5;

    int key = getpid();
    int id_sem = get_sem_id(key);
    set_ready(id_sem, init_people);
    set_shm_sem(id_sem);

    population* pop;
    individual* ind_list;

    int dimension = sizeof(individual)*init_people + sizeof(population) + ((sizeof(relationship)*init_people)/2+1) * ((sizeof(relationship)*init_people)/2+1);
    pop = createAttach(key, dimension);
    ind_list = (individual*) (pop + 1);

    struct sigaction act;
    struct sigaction oldact;
    sigset_t my_mask;
    sigemptyset(&my_mask);
    act.sa_handler = &intHandler;
    act.sa_mask = my_mask;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, &oldact);


    for(i=0;i<init_people/sizeof(individual);i++){
        for(j=0;j<sizeof(ind_list[i].name);j++){
            ind_list[i].name[j] = '\0';
        }
    }
    pop->size = init_people;
    pop->numbers_of_a = 0;
    pop->numbers_of_b = 0;
    pop->readCount = 0;
    pop->writeCount = 0;

    srand(0);

    generate_population(pop, ind_list, genes);
    start_population(pop, ind_list);
    print_population(pop, ind_list);

    ind_ready(id_sem);
    wait_ready(id_sem);

    while(wait(&status) > 0 && keepRunning){
        //do nothing
    }

    kill(-getpid(), SIGINT);

    removeShm(key, dimension);
    semctl(id_sem, 0, IPC_RMID);

    exit(0);
}
