#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>

void * createAttach(int key, int dimension);
void removeShm(int key, int dimension);
