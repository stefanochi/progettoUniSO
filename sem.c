#include "sem.h"

#define R_MUTEX 0
#define W_MUTEX 1
#define READ_TRY 2
#define RESOURCE 3
#define NUM_SEM 4


void check_sem(int id_sem){
    int val;
    val = semctl(id_sem, RESOURCE, GETVAL);
    if(val > 1){
        printf("[%d]---------------------\n------------------\n------------------- val %d: %d\n", getpid(), id_sem, val);
        sleep(5);
    }
    //printf("[%d] RESOURCE %d: %d\n", getpid(), id_sem,  val);
}

int get_sem_id(int key){
    return semget (key, NUM_SEM, 0600 | IPC_CREAT);
}

void set_shm_sem(int id_sem){
    semctl (id_sem, R_MUTEX, SETVAL, 1);
    semctl (id_sem, W_MUTEX, SETVAL, 1);
    semctl (id_sem, READ_TRY, SETVAL, 1);
    semctl (id_sem, RESOURCE, SETVAL, 1);
}

void entry_read(int id_sem, int * readCount){
    BLOCK_SIGNALS
    //fprintf(stderr, "[%d] entry read start\n", getpid());
    check_sem(id_sem);
    struct sembuf sem_ctl[1];
    sem_ctl[0].sem_num = READ_TRY;//try reading
    sem_ctl[0].sem_op = -1;
    sem_ctl[0].sem_flg = 0;
    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}

    sem_ctl[0].sem_num = R_MUTEX;//readcout mutex
    sem_ctl[0].sem_op = -1;
    sem_ctl[0].sem_flg = 0;
    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}

    (*readCount)++;

    if(*readCount == 1){
        sem_ctl[0].sem_num = RESOURCE;
        sem_ctl[0].sem_op = -1;
        sem_ctl[0].sem_flg = 0;

        if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
        //printf("[%d] starting read!!!!!\n\n", getpid());
    }

    sem_ctl[0].sem_num = R_MUTEX;
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;
    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}

    sem_ctl[0].sem_num = READ_TRY;
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;

    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
    //fprintf(stderr, "[%d] entry read finish\n", getpid());
    UNBLOCK_SIGNALS
}
void exit_read(int id_sem, int * readCount){
    BLOCK_SIGNALS
    //fprintf(stderr, "[%d] exit read start\n", getpid());
    check_sem(id_sem);
    struct sembuf sem_ctl[1];
    sem_ctl[0].sem_num = R_MUTEX;
    sem_ctl[0].sem_op = -1;
    sem_ctl[0].sem_flg = 0;

    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
    (*readCount) --;

    if(*readCount == 0){
        sem_ctl[0].sem_num = RESOURCE;
        sem_ctl[0].sem_op = 1;
        sem_ctl[0].sem_flg = 0;

        if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
        //printf("[%d] ending read!!!!!\n\n", getpid());
    }

    sem_ctl[0].sem_num = R_MUTEX;
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;

    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
    //fprintf(stderr, "[%d] exit read finish\n", getpid());
    UNBLOCK_SIGNALS
}
void entry_write(int id_sem, int * writeCount){
    BLOCK_SIGNALS
    //fprintf(stderr, "[%d] entry write start\n", getpid());
    check_sem(id_sem);
    struct sembuf sem_ctl[1];
    sem_ctl[0].sem_num = W_MUTEX;//try reading
    sem_ctl[0].sem_op = -1;
    sem_ctl[0].sem_flg = 0;
    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}

    (*writeCount)++;
    if(*writeCount == 1){
        sem_ctl[0].sem_num = READ_TRY;//try reading
        sem_ctl[0].sem_op = -1;
        sem_ctl[0].sem_flg = 0;
        if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
    }

    sem_ctl[0].sem_num = W_MUTEX;//try reading
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;
    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}

    sem_ctl[0].sem_num = RESOURCE;//try reading
    sem_ctl[0].sem_op = -1;
    sem_ctl[0].sem_flg = 0;

    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
    //printf("[%d] starting write!!!!!\n\n", getpid());
    //fprintf(stderr, "[%d] entry write finish\n", getpid());
    UNBLOCK_SIGNALS
}
void exit_write(int id_sem, int * writeCount){
    BLOCK_SIGNALS
    //fprintf(stderr, "[%d] exit write start\n", getpid());
    check_sem(id_sem);
    struct sembuf sem_ctl[1];
    sem_ctl[0].sem_num = RESOURCE;
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;

    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
    //printf("[%d] ending write!!!!!\n\n", getpid());

    sem_ctl[0].sem_num = W_MUTEX;
    sem_ctl[0].sem_op = -1;
    sem_ctl[0].sem_flg = 0;

    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}

    (*writeCount)--;
    if(*writeCount == 0){
        sem_ctl[0].sem_num = READ_TRY;
        sem_ctl[0].sem_op = 1;
        sem_ctl[0].sem_flg = 0;

        if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
    }

    sem_ctl[0].sem_num = W_MUTEX;
    sem_ctl[0].sem_op = 1;
    sem_ctl[0].sem_flg = 0;

    if(semop(id_sem, sem_ctl, 1) == -1){fprintf(stderr, "[%d] ------------------error in semop----------\n%s\n", getpid(), strerror(errno));}
    //fprintf(stderr, "[%d] exit write finish\n", getpid());
    UNBLOCK_SIGNALS
}
