#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>
#include "shm.h"

int main(int argc, char ** argv){
    int init_people = 20, i=0, j=0, status;
    unsigned long genes = 5;
    int key = getpid();

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
    print_population(pop, ind_list);
    start_population(pop, ind_list);


    while(wait(&status) > 0){
        //do nothing
    }

    removeShm(key, dimension);

    exit(0);
}
