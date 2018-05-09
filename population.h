#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <time.h>
#include <unistd.h>

#define MAX_NAME_SIZE 100
#define TYPE_A 0
#define TYPE_B 1

typedef struct relationship{
  int individual_a;
  int individual_b;
} relationship;

typedef struct individual{
    int pid;
    int type;
    char name[MAX_NAME_SIZE];
    unsigned long gene;
    int status;
} individual;

typedef struct population{
    unsigned int size;
    unsigned int numbers_of_a;
    unsigned int numbers_of_b;
    unsigned int readCount;
    unsigned int writeCount;
} population;

int get_sem_id(int key);

void set_ready(int id_sem, int init_people);
void wait_ready(int id_sem);
void ind_ready(int id_sem);

void set_shm_sem(int id_sem);
void entry_read(int id_sem, population * pop);
void exit_read(int id_sem, population * pop);
void entry_write(int id_sem, population * pop);
void exit_write(int id_sem, population * pop);

int start_individual(individual * ind);
int generate_individual(individual* ind, int type, unsigned long parent_gcd, unsigned long genes);
individual* get_ind_by_pid(int pid, individual *ind_list, population *pop);

int generate_population(population* pop, individual* ind_list, unsigned long genes);
int start_population(population * pop, individual * ind_list);
int print_population(population* pop, individual* ind_list);

long gcd(long gene_a, long gene_b);

void * get_list_relationships(population * pop);
void insert_relationship(relationship * rel, population * pop, int pid_a, int pid_b);
void remove_relationship(relationship * rel, population * pop,int pid);
void print_relationship(relationship * rel, population * pop);
