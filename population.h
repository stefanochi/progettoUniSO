#include <stdio.h>
#include <stdlib.h>


#define MAX_NAME_SIZE 100
#define TYPE_A 0
#define TYPE_B 1

typedef struct individual{
    int pid;
    int type;
    char name[MAX_NAME_SIZE];
    unsigned long gene;
} individual;

typedef struct population{
    unsigned int size;
    unsigned int numbers_of_a;
    unsigned int numbers_of_b;
} population;

int start_individual(individual * ind);
int generate_individual(individual* ind, int type, unsigned long parent_gcd, unsigned long genes);

int generate_population(population* pop, individual* ind_list, unsigned long genes);
int start_population(population * pop, individual * ind_list);
int print_population(population* pop, individual* ind_list);

long gcd(long gene_a, long gene_b);
