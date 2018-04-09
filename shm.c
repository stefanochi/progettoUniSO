#include "shm.h"

population* createAttach(int key, int dimension){
    population * pop;
    int shm_id = shmget(key, dimension, 0600 | IPC_CREAT);
    pop = shmat(shm_id, NULL, 0);
    return pop;
}

void removeShm(int key, int dimension){
    int shm_id = shmget(key, dimension, 0600 | IPC_CREAT);
    shmctl(shm_id, IPC_RMID, NULL);
}
