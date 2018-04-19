#include "shm.h"

void* createAttach(int key, int dimension){
    void * p;
    int shm_id = shmget(key, dimension, 0600 | IPC_CREAT);
    p = shmat(shm_id, NULL, 0);
    return p;
}

void removeShm(int key, int dimension){
    int shm_id = shmget(key, dimension, 0600 | IPC_CREAT);
    shmctl(shm_id, IPC_RMID, NULL);
}
