#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>
#include "shm.h"
#include "population.h"

int main(int argc, char ** argv){

    int init_people = 20, i=0, j=0, status;
    unsigned long genes = 5;
    int key = getpid();

    int id_semReady = get_sem_id(key);
    semctl (id_semReady, 0, SETVAL, init_people + 1);


    population* pop;
    individual* ind_list;

    int dimension = sizeof(individual)*init_people + sizeof(population);

    pop = createAttach(key, dimension);
    ind_list = (individual*) pop + sizeof(population);

    for(i=0;i<init_people/sizeof(individual);i++){
        for(j=0;j<sizeof(ind_list[i].name);j++){
            ind_list[i].name[j] = '\0';
        }
    }
    pop->size = init_people;
    pop->numbers_of_a = 0;
    pop->numbers_of_b = 0;

    generate_population(pop, ind_list, genes);
    start_population(pop, ind_list);
    print_population(pop, ind_list);

    ind_ready(id_semReady);
    wait_ready(id_semReady);

    while(wait(&status) > 0){
        //do nothing
    }

    removeShm(key, dimension);
    semctl(id_semReady, 0, IPC_RMID);

    exit(0);
}
