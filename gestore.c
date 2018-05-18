#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "shm.h"
#include "sem.h"
#include "population.h"


static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int argc, char ** argv){

    int init_people = 500, i=0, j=0, status;
    unsigned long genes = 10;

    int key = getpid();
    int id_sem_ready = get_sem_ready(key);
    int id_sem_shm = get_sem_id(1234);
    int id_sem_relation = get_sem_id(5432);
    set_ready(id_sem_ready, init_people);
    set_shm_sem(id_sem_shm);
    set_shm_sem(id_sem_relation);

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
    pop->readCount_shm = 0;
    pop->writeCount_shm = 0;
    pop->readCount_relation = 0;
    pop->writeCount_relation = 0;

    srand(0);

    generate_population(pop, ind_list, genes);
    start_population(pop, ind_list);
    print_population(pop, ind_list);

    //sleep(30);
    ind_ready(id_sem_ready);
    wait_ready(id_sem_ready);

    int pid;
    while((pid=wait(&status)) > 0 && keepRunning){
        printf("GESTORE: process %d just stopped\n", pid);

        stop_ready(id_sem_ready);
        
        entry_read(id_sem_shm, &(pop->readCount_shm));
        individual * partner1 = get_ind_by_pid(pid, ind_list, pop);
        int partner2_pid = partner1->status;
        individual * partner2 = get_ind_by_pid(partner2_pid, ind_list, pop);
        exit_read(id_sem_shm, &(pop->readCount_shm));

        waitpid(partner2_pid, &status, 0);

        stop_ready(id_sem_ready);
        stop_ready(id_sem_ready);
        printf("GESTORE: process %d just stopped after %d\n", partner2_pid, pid);

        long gcd_parent = gcd(partner1->gene, partner2->gene);

        entry_write(id_sem_shm, &(pop->writeCount_shm));
        if(pop->numbers_of_a == 0){
            generate_individual(partner1, TYPE_A, gcd_parent, genes);
            pop->numbers_of_a++;
        }
        else if(pop->numbers_of_b == 0){
            generate_individual(partner1, TYPE_B, gcd_parent, genes);
            pop->numbers_of_b++;
        }
        else{
            if(generate_individual(partner1, -1, gcd_parent, genes) == 0)
                pop->numbers_of_a++;
            else
                pop->numbers_of_b++;
        }
        if(pop->numbers_of_a == 0){
            generate_individual(partner2, TYPE_A, gcd_parent, genes);
            pop->numbers_of_a++;
        }
        else if(pop->numbers_of_b == 0){
            generate_individual(partner2, TYPE_B, gcd_parent, genes);
            pop->numbers_of_b++;
        }
        else{
            if(generate_individual(partner2, -1, gcd_parent, genes) == 0)
                pop->numbers_of_a++;
            else
                pop->numbers_of_b++;
        }

        start_individual(partner1);
        start_individual(partner2);

        ind_ready(id_sem_ready);
        wait_ready(id_sem_ready);

        printf("GESTORE: individual [%d] just started\n", partner1->pid);
        printf("GESTORE: individual [%d] just started\n", partner2->pid);

        print_population(pop, ind_list);

        //sleep(5);
        
        exit_write(id_sem_shm, &(pop->writeCount_shm));

       


    }

    kill(-getpid(), SIGINT);

    removeShm(key, dimension);
    semctl(id_sem_shm, 0, IPC_RMID);
    semctl(id_sem_ready, 0, IPC_RMID);
    semctl(id_sem_relation, 0, IPC_RMID);

    exit(0);
}
