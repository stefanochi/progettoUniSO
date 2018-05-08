#include "population.h"

#define SEM_READY 0
#define NUM_TOTAL_SEM 5
#define R_MUTEX 1
#define W_MUTEX 2
#define READ_TRY 3
#define RESOURCE 4

int get_sem_id(int key){
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

void set_shm_sem(int id_sem){
    semctl (id_sem, R_MUTEX, SETVAL, 1);
    semctl (id_sem, W_MUTEX, SETVAL, 1);
    semctl (id_sem, READ_TRY, SETVAL, 1);
    semctl (id_sem, RESOURCE, SETVAL, 1);
}
void entry_read(int id_sem, population * pop){
    struct sembuf sem_ctl[2];
    sem_ctl[0].sem_num = READ_TRY;//try reading
    sem_ctl[0].sem_op = -1;
    sem_ctl[0].sem_flg = 0;

    sem_ctl[1].sem_num = R_MUTEX;//readcout mutex
    sem_ctl[1].sem_op = -1;
    sem_ctl[1].sem_flg = 0;
    semop(id_sem, sem_ctl, 2);

    pop->readCount++;

    if(pop->readCount == 1){
        sem_ctl[0].sem_num = RESOURCE;
        sem_ctl[0].sem_op = -1;
        sem_ctl[0].sem_flg = 0;

        semop(id_sem, sem_ctl, 1);
    }

    sem_ctl[0].sem_num = R_MUTEX;
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;

    sem_ctl[1].sem_num = READ_TRY;
    sem_ctl[1].sem_op = 1;
    sem_ctl[1].sem_flg = 0;

    semop(id_sem, sem_ctl, 2);
}
void exit_read(int id_sem, population * pop){
    struct sembuf sem_ctl[2];
    sem_ctl[0].sem_num = R_MUTEX;
    sem_ctl[0].sem_op = -1;
    sem_ctl[0].sem_flg = 0;

    semop(id_sem, sem_ctl, 1);
    pop->readCount --;

    if(pop->readCount == 0){
        sem_ctl[0].sem_num = RESOURCE;
        sem_ctl[0].sem_op = 1;
        sem_ctl[0].sem_flg = 0;

        semop(id_sem, sem_ctl, 1);
    }

    sem_ctl[0].sem_num = R_MUTEX;
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;

    semop(id_sem, sem_ctl, 1);
}
void entry_write(int id_sem, population * pop){
    struct sembuf sem_ctl[2];
    sem_ctl[0].sem_num = W_MUTEX;//try reading
    sem_ctl[0].sem_op = -1;
    sem_ctl[0].sem_flg = 0;
    semop(id_sem, sem_ctl, 1);

    pop->writeCount++;
    if(pop->writeCount == 1){
        sem_ctl[0].sem_num = READ_TRY;//try reading
        sem_ctl[0].sem_op = -1;
        sem_ctl[0].sem_flg = 0;
        semop(id_sem, sem_ctl, 1);
    }

    sem_ctl[0].sem_num = W_MUTEX;//try reading
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;

    sem_ctl[1].sem_num = RESOURCE;//try reading
    sem_ctl[1].sem_op = -1;
    sem_ctl[1].sem_flg = 0;

    semop(id_sem, sem_ctl, 2);
}
void exit_write(int id_sem, population * pop){
    struct sembuf sem_ctl[2];
    sem_ctl[0].sem_num = RESOURCE;//try reading
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;

    sem_ctl[1].sem_num = W_MUTEX;//try reading
    sem_ctl[1].sem_op = -1;
    sem_ctl[1].sem_flg = 0;

    semop(id_sem, sem_ctl, 2);

    pop->writeCount--;
    if(pop->writeCount == 0){
        sem_ctl[0].sem_num = READ_TRY;//try reading
        sem_ctl[0].sem_op = 1;
        sem_ctl[0].sem_flg = 0;

        semop(id_sem, sem_ctl, 1);
    }

    sem_ctl[0].sem_num = W_MUTEX;//try reading
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;

    semop(id_sem, sem_ctl, 1);
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

    return ind->type;
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
    for(int i; i<pop->size; i++){
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

    printf("PID\tTYPE\tNAME\tGENE\n");
    for(i=0; i<pop->size; i++){
        printf("%d\t%i\t%s\t%lu\n", (ind_list+i)->pid, (ind_list+i)->type, (ind_list+i)->name, (ind_list+i)->gene);
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
  return pop+sizeof(population)+sizeof(individual)*pop->size;
}

void insert_relationship(relationship * rel, population * pop,int pid_a, int pid_b){
  int end = ((sizeof(relationship)*pop->size)/2+1 * (sizeof(relationship)*pop->size)/2+1);
  int i = 0;
  while (i<end){
    if (rel[i].individual_a == 0){
      rel[i].individual_a = pid_a;
      rel[i].individual_b = pid_b;
    }
    i++;
  }
}

void remove_relationship(relationship * rel, population * pop,int pid){
  int end = ((sizeof(relationship)*pop->size)/2+1 * (sizeof(relationship)*pop->size)/2+1);
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
  int end = ((sizeof(relationship)*pop->size)/2+1 * (sizeof(relationship)*pop->size)/2+1);
  int i = 0;
  while (i<end){
    if (rel[i].individual_a != 0 || rel[i].individual_b != 0){
      printf("pid a: %d, pid b = %d\n", rel[i].individual_a, rel[i].individual_b);
    }
    i++;
  }
}
