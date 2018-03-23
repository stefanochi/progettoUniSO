#include <sys/types.h>
#include <sys/wait.h>
#include "population.h"

int main(int argc, char ** argv){
    int init_people = 20, i=0, j=0, status;
    unsigned long genes = 5;

    population pop;
    individual ind_list[init_people];

    for(i=0;i<sizeof(ind_list)/sizeof(individual);i++){
        for(j=0;j<sizeof(ind_list[i].name);j++){
            ind_list[i].name[j] = '\0';
        }
    }

    pop.size = init_people;
    pop.numbers_of_a = 0;
    pop.numbers_of_b = 0;

    generate_population(&pop, &ind_list[0], genes);
    print_population(&pop, &ind_list[0]);
    start_population(&pop, &ind_list[0]);

    while(wait(&status) > 0){
        //do nothing
    }
    exit(0);
}
