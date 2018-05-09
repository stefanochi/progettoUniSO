#include <unistd.h>
#include <signal.h>
#include "shm.h"
#include "population.h"
#include "msq.h"

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

void set_handler(){
    struct sigaction act;
    struct sigaction oldact;
    sigset_t my_mask;
    sigemptyset(&my_mask);
    act.sa_handler = &intHandler;
    act.sa_mask = my_mask;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, &oldact);
}

individual get_best_partner(population * pop, individual * ind_list, individual my_ind){
    individual ind_a;

    if(pop->numbers_of_a > 0){
        long max_gcd=0;
        for (int j=0; j<pop->size; j++){
            if ((ind_list+j)->type == 0 && (ind_list + j)->status == 0){
                long gcd_a = gcd((ind_list+j)->gene, my_ind.gene);
                if (gcd_a > max_gcd){
                    max_gcd = gcd_a;
                    ind_a = *(ind_list+j);
                }
            }
        }
    }
    return ind_a;
}

int main(int argc, char ** argv){
  population * pop;
  pop = createAttach (getppid(), 0);

  int id_sem;
  int key_semReady = getppid();

  id_sem = get_sem_id(key_semReady);
  int msq_b = create_msq(getpid());

  set_handler();

  ind_ready(id_sem);
  wait_ready(id_sem);

  individual* ind_list;
  ind_list = (individual*) (pop + 1);

  individual my_ind;
  entry_read(id_sem, pop);
  my_ind = *(get_ind_by_pid(getpid(), ind_list, pop));
  exit_read(id_sem, pop);

  individual ind_a;

  while(keepRunning){

      entry_read(id_sem, pop);
      ind_a = get_best_partner(pop, ind_list, my_ind);
      exit_read(id_sem, pop);

      if(ind_a.pid == 0){
          fprintf(stderr, "[%d] error: can't find partner\n", getpid());
      }

      int msq_a = msgget(ind_a.pid, 0600);

      request req;
      req.pid = my_ind.pid;
      req.gene = my_ind.gene;

      int res;

      printf("[%d] sending request to pid A = %d\n",getpid(), ind_a.pid);
      if(send_request(msq_a, &req) != -1){
          printf("[%d] waiting response...\n", getpid());
          if(keepRunning && wait_response(msq_b, &res) != -1){
             printf("[%d] individual: %d response's is: %d\n", getpid(), ind_a.pid, res);
             if(res == 1){
                 keepRunning = 0;
             }
         }else{
             fprintf(stderr, "[%d] failed to wait_response\n", getpid());
         }
     }else{
         fprintf(stderr, "[%d] failed to send_request\n", getpid());
     }
  }

  if(remove_msq(msq_b)){
      fprintf(stderr, "[%d] failed to remove msq: %s\n", getpid(), strerror(errno));
  }

  exit(0);
}
