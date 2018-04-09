#include <unistd.h>
#include "shm.h"

int main(int argc, char ** argv){
  population * pop;
  pop = createAttach (getppid(), 0);

  individual* ind_list;
  ind_list = (individual*) pop + sizeof(population);

  exit(0);
}
