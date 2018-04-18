#include "population.h"
#include <unistd.h>

int main(int argc, char ** argv){

    int id_semReady;
    int key_semReady = getppid();


    id_semReady = get_sem_id(key_semReady);
    ind_ready(id_semReady);
    wait_ready(id_semReady);

    exit(0);
}
