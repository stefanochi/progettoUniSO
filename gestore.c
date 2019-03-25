#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include "shm.h"
#include "sem.h"
#include "population.h"


static volatile int keepRunning = 1;
static volatile int checkTimer = 0;

void intHandler(int signum) {
    if(signum == SIGINT){
        keepRunning = 0;
    }else if(signum == SIGALRM){
        checkTimer = 1;
    }
}

void read_parameters(int * init_people, unsigned long * genes, time_t * sim_time, time_t * birth_death){
    printf("init_people: ");
    scanf("%d", init_people);
    printf("genes: ");
    scanf("%lu", genes);
    printf("sim_time: ");
    scanf("%li", sim_time);
    printf("birth_death: ");
    scanf("%li", birth_death);
}

void print_max_gene(population * pop, individual * ind_list){
    int i;
    individual ind_max;
    ind_max.gene = 0;
    for(i=0; i<pop->size; i++){
        if((ind_list + i)->gene > ind_max.gene){
            ind_max = *(ind_list + i);
        }
    }
    printf("individual with max gene at the end of the simulation:\n");
    printf("PID\tTYPE\tNAME\tGENE\tSTATUS\n");
    printf("%d\t%i\t%s\t%lu\t%d\n", ind_max.pid, ind_max.type, ind_max.name, ind_max.gene, ind_max.status);
}

void print_longest_name(population * pop, individual * ind_list){
    int i;
    int max_name_length = -1;
    individual longest_name_ind;
    for(i=0; i<pop->size; i++){
        int name_length = 0;
        int k=0;
        while(k < MAX_NAME_SIZE  && (ind_list + i)->name[k] != '\0'){
            k++;
        }
        if(k > max_name_length){
            longest_name_ind = *(ind_list + i);
            max_name_length = k;
        }
    }
    printf("individual with longest name at the end of the simulation:\n");
    printf("PID\tTYPE\tNAME\tGENE\tSTATUS\n");
    printf("%d\t%i\t%s\t%lu\t%d\n", longest_name_ind.pid, longest_name_ind.type, longest_name_ind.name, longest_name_ind.gene, longest_name_ind.status);
}


int main(int argc, char ** argv){

    int total_a=0, total_b=0;

    int init_people = 10, i=0, j=0, status;
    unsigned long genes = 20;

    time_t birth_death = 2;
    time_t sim_time = 30;
    read_parameters(&init_people, &genes, &sim_time, &birth_death);
    long int interval = birth_death;

    time_t start_time = time(NULL);
    time_t last_time = start_time;
    alarm(interval);

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
    sigaction(SIGALRM, &act, &oldact);


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

    //srand(0);

    generate_population(pop, ind_list, genes);
    total_a += pop->numbers_of_a;
    total_b += pop->numbers_of_b;
    start_population(pop, ind_list);
    print_population(pop, ind_list);

    //sleep(30);
    ind_ready(id_sem_ready);
    wait_ready(id_sem_ready);

    int pid;
    while(keepRunning && (pid=wait(&status))){
        time_t current_time = time(NULL);
        if((current_time - start_time) >= sim_time){
            printf("-----------------------------------------------------------\n");
            printf("GESTORE simulation stopping, finished time\n");
            keepRunning = 0;
        }
        if(keepRunning && checkTimer){
            printf("GESTORE: current_time - start_time: %li\n", current_time - start_time);
            printf("GESTORE: current_time - last_time: %li\n", current_time - last_time);
            if((current_time - last_time) >= birth_death){
                int pid_to_kill = -1;
                while(pid_to_kill == -1){
                    individual * ind_to_kill = ind_list+(rand() % init_people);
                    entry_read(id_sem_shm, &(pop->readCount_shm));
                    if(ind_to_kill->status != -1){
                        pid_to_kill = ind_to_kill->pid;
                    }
                    exit_read(id_sem_shm, &(pop->readCount_shm));
                }
                printf("---------------------------------------------------------\nGESTORE: killing process %d\n------------------------------------------------\n", pid_to_kill);
                kill(pid_to_kill, SIGINT);
                BLOCK_SIGNALS
                sleep(3);
                UNBLOCK_SIGNALS
                last_time = time(NULL);
                alarm(interval);
            }
            checkTimer = 0;
        }
        if(keepRunning && pid != -1){
            //sleep(3);
            printf("GESTORE: process %d just stopped\n", pid);

            stop_ready(id_sem_ready);
            entry_read(id_sem_shm, &(pop->readCount_shm));
            individual * partner1 = get_ind_by_pid(pid, ind_list, pop);
            int partner2_pid = partner1->status;
            exit_read(id_sem_shm, &(pop->readCount_shm));
            if(partner2_pid == 0 || partner2_pid == -1){
                printf("GESTORE: %d had no partner\n", pid);
                stop_ready(id_sem_ready);
                entry_write(id_sem_shm, &(pop->writeCount_shm));

                if(partner1->type == 0){
                    pop->numbers_of_a--;
                }else{
                    pop->numbers_of_b--;
                }

                reset_name(partner1);
                if(generate_individual_helper(pop, partner1, 2, genes) == 0){
                    total_a ++;
                }else{
                    total_b ++;
                }
                start_individual(partner1);

                exit_write(id_sem_shm, &(pop->writeCount_shm));

                ind_ready(id_sem_ready);
                wait_ready(id_sem_ready);

                printf("GESTORE: individual [%d] just started\n", partner1->pid);
                entry_read(id_sem_shm, &(pop->readCount_shm));
                print_population(pop, ind_list);
                exit_read(id_sem_shm, &(pop->readCount_shm));
                //sleep(3);


            }else{
                entry_read(id_sem_shm, &(pop->readCount_shm));
                individual * partner2 = get_ind_by_pid(partner2_pid, ind_list, pop);
                exit_read(id_sem_shm, &(pop->readCount_shm));

                BLOCK_SIGNALS
                partner2_pid = waitpid(partner2_pid, &status, 0);
                UNBLOCK_SIGNALS

                stop_ready(id_sem_ready);
                stop_ready(id_sem_ready);
                printf("GESTORE: process %d just stopped after %d\n", partner2_pid, pid);

                long gcd_parent = gcd(partner1->gene, partner2->gene);

                //printf("GESTORE numbers of a:%d\tnumbers of b: %d\n", pop->numbers_of_a, pop->numbers_of_b);

                entry_write(id_sem_shm, &(pop->writeCount_shm));

                pop->numbers_of_a--;
                pop->numbers_of_b--;

                if(generate_individual_helper(pop, partner1, gcd_parent, genes) == 0){
                    total_a ++;
                }else{
                    total_b ++;
                }

                if(generate_individual_helper(pop, partner2, gcd_parent, genes) == 0){
                    total_a ++;
                }else{
                    total_b ++;
                }

                start_individual(partner1);
                start_individual(partner2);

                exit_write(id_sem_shm, &(pop->writeCount_shm));

                ind_ready(id_sem_ready);
                wait_ready(id_sem_ready);

                printf("GESTORE: individual [%d] just started\n", partner1->pid);
                printf("GESTORE: individual [%d] just started\n", partner2->pid);

                entry_read(id_sem_shm, &(pop->readCount_shm));
                print_population(pop, ind_list);
                exit_read(id_sem_shm, &(pop->readCount_shm));

                //sleep(1);

            }
        }
    }

    kill(-getpid(), SIGINT);

    while(wait(&status) != -1){

    }
    printf("----------------------------------------------------------\n");
    printf("total number of individual a: %d\ntotal number of individual b:%d\n", total_a, total_b);
    print_max_gene(pop, ind_list);
    print_longest_name(pop, ind_list);

    removeShm(key, dimension);
    semctl(id_sem_shm, 0, IPC_RMID);
    semctl(id_sem_ready, 0, IPC_RMID);
    semctl(id_sem_relation, 0, IPC_RMID);

    exit(0);
}
