#include <time.h>
#include "population.h"

int generate_individual(individual* ind, int type, unsigned long parent_gcd, unsigned long genes){
    char name;
    int i=0;

    if(type != -1)
        ind->type = type;
    else
        ind->type = rand() % 2;

    name = rand() % 26 + 65;
    while(i < MAX_NAME_SIZE && ind->name[i] != '\0'){
        i++;
    }
    printf("name:%s\n", ind->name);
    ind->name[i] = name;
    printf("name:%s\n", ind->name);

    ind->gene = rand() % genes + parent_gcd;

    return ind->type;
}

int generate_population(population* pop, individual* ind_list, unsigned long genes){
    int i;

    srand(time(NULL));
    for(i=0; i<pop->size; i++){
        if(pop->numbers_of_a == 0){
            generate_individual(ind_list + i, TYPE_A, 0, genes);
            pop->numbers_of_a++;
        }
        else if(pop->numbers_of_b == 0){
            generate_individual(ind_list + i, TYPE_B, 0, genes);
            pop->numbers_of_b++;
        }
        else{
            if(generate_individual(ind_list + i, -1, 0, genes) == 0)
                pop->numbers_of_a++;
            else
                pop->numbers_of_b++;
        }
    }
    return 0;
}

int print_population(population* pop, individual* ind_list){
    int i;

    printf("TYPE\tNAME\tGENE\n");
    for(i=0; i<pop->size; i++){
        printf("%i\t%s\t%lu\n", (ind_list+i)->type, (ind_list+i)->name, (ind_list+i)->gene);
    }
}
