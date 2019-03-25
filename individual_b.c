#include <unistd.h>
#include <signal.h>
#include "shm.h"
#include "population.h"
#include "msq.h"
#include "sem.h"

static volatile int keepRunning = 1;

void intHandler(int signum) {
    keepRunning = 0;
}

void block_responses(int msq_id){
    struct msqid_ds buf;
    if(msgctl(msq_id, IPC_STAT, &buf) == -1){
        fprintf(stderr, "[%d] IPC_STAT:%s\n", getpid(), strerror(errno));
    }
    buf.msg_perm.mode = 0400;
    if(msgctl(msq_id, IPC_SET, &buf) == -1){
        fprintf(stderr, "[%d] IPC_STAT:%s\n", getpid(), strerror(errno));
    }
}

int empty_responses(int msq_id){
    msg_response msg_res;
    while(msgrcv(msq_id, &msg_res, sizeof(int), 2, IPC_NOWAIT) != -1){
        if(msg_res.res == 1){
            return 1;
        }
    }
    return 0;
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
    ind_a.pid = -1;
    relationship * rel = get_list_relationships(pop);

    if(pop->numbers_of_a > 0){
        long max_gcd=0;
        for (int j=0; j<pop->size; j++){
            if ((ind_list+j)->type == 0 && (ind_list + j)->status == 0 && find_relationship(rel, pop, (ind_list+j)->pid, my_ind.pid)==0){
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

  int id_sem_ready, id_sem_shm, id_sem_relation;
  int key_semReady = getppid();

  id_sem_ready = get_sem_ready(key_semReady);
  id_sem_shm = get_sem_id(1234);
  id_sem_relation = get_sem_id(5432);

  int msq_b = create_msq(getpid());

  set_handler();

  ind_ready(id_sem_ready);
  wait_ready(id_sem_ready);

  individual* ind_list;
  ind_list = (individual*) (pop + 1);

  relationship * rel = get_list_relationships(pop);

  individual *my_ind_ptr, my_ind;
  entry_write(id_sem_shm, &(pop->writeCount_shm));
  my_ind_ptr = get_ind_by_pid(getpid(), ind_list, pop);
  my_ind_ptr->status = 0;
  my_ind = *my_ind_ptr;
  exit_write(id_sem_shm, &(pop->writeCount_shm));

  individual ind_a;

  while(keepRunning){

      do{
          wait_ready(id_sem_ready);
          entry_read(id_sem_shm, &(pop->readCount_shm));
          ind_a = get_best_partner(pop, ind_list, my_ind);
          exit_read(id_sem_shm, &(pop->readCount_shm));
      }while(keepRunning && ind_a.pid == -1);

      int msq_a = msgget(ind_a.pid, 0600);

      request req;
      req.pid = my_ind.pid;
      req.gene = my_ind.gene;

      int res;

      if(send_request(msq_a, &req) != -1){

          if(keepRunning && wait_response(msq_b, &res) != -1){

             if(res == 1){
                
                keepRunning = 0;
                entry_write(id_sem_shm, &(pop->writeCount_shm));
                my_ind_ptr->status = ind_a.pid;
                exit_write(id_sem_shm, &(pop->writeCount_shm));

                entry_write(id_sem_relation, &(pop->writeCount_relation));
                remove_relationship(rel, pop, my_ind.pid);
                exit_write(id_sem_relation, &(pop->writeCount_relation));
             }
         }
     }
  }

  block_responses(msq_b);
  while(empty_responses(msq_b)){
      entry_write(id_sem_shm, &(pop->writeCount_shm));
      my_ind_ptr->status = ind_a.pid;
      exit_write(id_sem_shm, &(pop->writeCount_shm));
  }

  entry_write(id_sem_relation, &(pop->writeCount_relation));
  remove_relationship(rel, pop, my_ind.pid);
  exit_write(id_sem_relation, &(pop->writeCount_relation));

  if(remove_msq(msq_b)){
      fprintf(stderr, "[%d] failed to remove msq: %s\n", getpid(), strerror(errno));
  }

  exit(0);
}
