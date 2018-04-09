#include <sys/types.h>
#include <sys/shm.h>
#include "population.h"

int lock_write(int sem_id);
int unlock_write(int sem_id);
int lock_read(int sem_id);
int unlock_read(int sem_id);

population * createAttach(int key, int dimension);
void removeShm(int key, int dimension);
