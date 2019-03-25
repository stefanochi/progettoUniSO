#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>
#include "population.h"

#define SEM_READY 0
#define NUM_TOTAL_SEM 1

int get_sem_ready(int key){
    return semget (key, NUM_TOTAL_SEM, 0600 | IPC_CREAT);
}
void set_ready(int id_sem, int init_people){
    semctl (id_sem, SEM_READY, SETVAL, init_people + 1);
}
void wait_ready(int id_sem){
  struct sembuf ops;
  ops.sem_num = SEM_READY;
  ops.sem_op =  0;
  ops.sem_flg = 0;

  semop(id_sem, &ops, 1);
}
void ind_ready(int id_sem){
    struct sembuf ops;
    ops.sem_num = SEM_READY;
    ops.sem_op = -1;
    ops.sem_flg = 0;

    semop(id_sem, &ops, 1);
}
void stop_ready(int id_sem){
    struct sembuf ops;
    ops.sem_num = SEM_READY;
    ops.sem_op = 1;
    ops.sem_flg = 0;

    semop(id_sem, &ops, 1);
}

int reset_name(individual * ind){
    int i;
    for(i =0; i < MAX_NAME_SIZE; i++){
        ind->name[i] = '\0';
    }
}


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
    ind->name[i] = name;

    ind->gene = rand() % genes + parent_gcd;
    ind->status = -1;

    return ind->type;
}

int generate_individual_helper(population * pop, individual* ind, unsigned long parent_gcd, unsigned long genes){
    if(pop->numbers_of_a == 0){
        generate_individual(ind, TYPE_A, parent_gcd, genes);
        pop->numbers_of_a++;
        return 0;
    }
    else if(pop->numbers_of_b == 0){
        generate_individual(ind, TYPE_B, parent_gcd, genes);
        pop->numbers_of_b++;
        return 1;
    }
    else{
        if(generate_individual(ind, -1, parent_gcd, genes) == 0){
            pop->numbers_of_a++;
            return 0;
        }else{
            pop->numbers_of_b++;
            return 1;
        }
    }
}

int start_individual(individual * ind){
    char * file[2] = {
        "individual_a",
        "individual_b"
    };
    char * argv[2];
    int pid;

    argv[0] = file[ind->type];
    argv[1] = NULL;

    pid = fork();
    switch(pid){
        case 0:
            execve(argv[0], argv, NULL);
            return -1;
            break;
        case -1:
            return -1;
            break;
        default:
            ind->pid = pid;
    }

    return pid;


}

individual* get_ind_by_pid(int pid, individual *ind_list, population *pop){
    individual * my_ind;
    for(int i=0; i<pop->size; i++){
      if ((ind_list+i)->pid == pid){
        my_ind = ind_list+i;
        return my_ind;
      }
    }
    return NULL;
}

int generate_population(population* pop, individual* ind_list, unsigned long genes){
    int i;

    for(i=0; i<pop->size; i++){
        if(pop->numbers_of_a == 0){
            generate_individual(ind_list + i, TYPE_A, 2, genes);
            pop->numbers_of_a++;
        }
        else if(pop->numbers_of_b == 0){
            generate_individual(ind_list + i, TYPE_B, 2, genes);
            pop->numbers_of_b++;
        }
        else{
            if(generate_individual(ind_list + i, -1, 2, genes) == 0)
                pop->numbers_of_a++;
            else
                pop->numbers_of_b++;
        }
    }
    return 0;
}
int start_population(population * pop, individual * ind_list){
    int i;

    for(i=0; i<pop->size; i++){
        if(start_individual(ind_list + i) == -1){
            return -1;
        }
    }
    return 0;
}
int print_population(population* pop, individual* ind_list){
    int i;

    printf("PID\tTYPE\tNAME\tGENE\tSTATUS\n");
    for(i=0; i<pop->size; i++){
        printf("%d\t%i\t%s\t%lu\t%d\n", (ind_list+i)->pid, (ind_list+i)->type, (ind_list+i)->name, (ind_list+i)->gene, (ind_list+i)->status);
    }
}

long gcd(long gene_a, long gene_b){
  if (gene_a == 0 || gene_b == 0){
    return 0;
  }
  if (gene_a == gene_b){
    return gene_a;
  }
  else{
    if (gene_a>gene_b){
      return gcd(gene_a-gene_b, gene_b);
    }
    else return gcd(gene_a, gene_b-gene_a);
  }
}

void * get_list_relationships(population * pop){
  return ((void *) pop) + sizeof(population) + pop->size*sizeof(individual);
}

void insert_relationship(relationship * rel, population * pop,int pid_a, int pid_b){
  int end = ((pop->size)/2+1) * ((pop->size)/2+1);
  int flag = 1;
  int i = 0;
  while (i<end && flag){
    if (rel[i].individual_a == 0){
      rel[i].individual_a = pid_a;
      rel[i].individual_b = pid_b;
      flag = 0;
    }
    i++;
  }
}

void remove_relationship(relationship * rel, population * pop,int pid){
  int end = ((pop->size)/2+1) * ((pop->size)/2+1);
  int i = 0;
  while (i<end){
    if (rel[i].individual_a == pid || rel[i].individual_b == pid){
      rel[i].individual_a = 0;
      rel[i].individual_b = 0;
    }
    i++;
  }
}

void print_relationship(relationship * rel, population * pop){
  int end = ((pop->size)/2+1) * ((pop->size)/2+1);
  int i = 0;
  while (i<end){
    if (rel[i].individual_a != 0 || rel[i].individual_b != 0){
      printf("[%d] pid a: %d, pid b = %d\n",getpid(),  rel[i].individual_a, rel[i].individual_b);
    }
    i++;
  }
}

int find_relationship(relationship * rel, population * pop, int pid_a, int pid_b){
  int end = ((pop->size)/2+1) * ((pop->size)/2+1);
  int i = 0;
  while (i<end){
    if (rel[i].individual_a == pid_a && rel[i].individual_b == pid_b){
      return 1;
    }
    i++;
  }
  return 0;
}

int request_from_all(relationship * rel, population * pop, int pid_a){
    individual * ind_list = (individual *)(pop + 1);
    int end = ((pop->size)/2+1) * ((pop->size)/2+1);
    int i, j;
    for(i=0; i<pop->size; i++){
        if((ind_list + i)->type == 1 && (ind_list + i)->status == 0){
            int flag = 0;
            for(j=0; j<end; j++){
                if((rel + j)->individual_a == pid_a){
                    if((rel+j)->individual_b == (ind_list + i)->pid){
                        flag = 1;
                    }
                }
            }
            if(flag == 0){
                return 0;
            }
        }
    }
    return 1;
}
