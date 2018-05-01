#include <unistd.h>
#include <signal.h>
#include "shm.h"
#include "population.h"
#include "msq.h"

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

void set_handelr(){
    struct sigaction act;
    struct sigaction oldact;
    sigset_t my_mask;
    sigemptyset(&my_mask);
    act.sa_handler = &intHandler;
    act.sa_mask = my_mask;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, &oldact);
}

int main(int argc, char ** argv){
  population * pop;
  pop = createAttach (getppid(), 0);

  int id_semReady;
  int key_semReady = getppid();

  id_semReady = get_sem_id(key_semReady);
  int msq_b = get_message_id(getpid());

  set_handelr();

  ind_ready(id_semReady);
  wait_ready(id_semReady);

  individual* ind_list;
  ind_list = (individual*) pop + sizeof(population);

  individual my_ind;
  my_ind = *(get_ind_by_pid(getpid(), ind_list, pop));

  individual ind_a;

  if(pop->numbers_of_a > 0){
      long max_gcd=0;
      for (int j=0; j<pop->size; j++){
          if ((ind_list+j)->type == 0){
              long gcd_a = gcd((ind_list+j)->gene, my_ind.gene);
              if (gcd_a > max_gcd){
                  max_gcd = gcd_a;
                  ind_a = *(ind_list+j);
              }
          }
      }
  }
  int msq_a = get_message_id(ind_a.pid);

  request req;
  req.pid = my_ind.pid;
  req.gene = my_ind.gene;

  int res;

  printf("[%d] sending request to pid A = %d\n",getpid(), ind_a.pid);
  if(send_request(msq_a, &req) != -1){
      if(keepRunning && wait_response(msq_b, &res) == -1){
         printf("[%d] individual: %d response's is: %d\n", getpid(), ind_a.pid, res);
      }
      fprintf(stderr, "[%d] failed to wait_response\n", getpid());
  }
  fprintf(stderr, "[%d] failed to send_request\n", getpid());

  remove_msq(msq_b);

  exit(0);
}
