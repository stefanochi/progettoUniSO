#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "shm.h"

#define BLOCK_SIGNALS sigset_t set, oldset;\
                        sigfillset(&set);\
                        sigprocmask(SIG_SETMASK, &set, &oldset);

#define UNBLOCK_SIGNALS sigprocmask(SIG_SETMASK, &oldset, NULL);

int get_sem_id(int key);

void set_shm_sem(int id_sem);

void entry_read(int id_sem, int * readCount);

void exit_read(int id_sem, int * readCount);

void entry_write(int id_sem, int * writeCount);

void exit_write(int id_sem, int * writeCount);
