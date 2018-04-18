#include <unistd.h>
#include "shm.h"

int main(int argc, char ** argv){
  population * pop;
  pop = createAttach (getppid(), 0);

  int id_semReady;
  int key_semReady = getppid();


  id_semReady = get_sem_id(key_semReady);
  ind_ready(id_semReady);
  wait_ready(id_semReady);

  individual* ind_list;
  ind_list = (individual*) pop + sizeof(population);

  individual * my_ind;
  my_ind = get_ind_by_pid(getpid(), ind_list, pop);

  long max_gcd=0;
  individual * ind_a;
  for (int j=0; j<pop->size; j++){
    if ((ind_list+j)->type == 0){
      long gcd_a = gcd((ind_list+j)->gene, my_ind->gene);
      if (gcd_a > max_gcd){
        max_gcd = gcd_a;
        ind_a = (ind_list+j);
      }
    }
  }
  printf("[%d] pid A = %d\n",getpid(), ind_a->pid);


  exit(0);
}
