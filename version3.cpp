#include <iostream>
#include <cstdio>
#include <pthread.h>
#include <unistd.h>
#include <mutex>
#include <time.h>
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()
#include <semaphore.h>

using namespace std;

#define STOCK_SIZE 5
#define DEFAULT_AMOUNT 40
#define STORES 12 
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

time_t rawtime;
struct tm * timeinfo;
static sem_t stock_semaphore;
static mutex stock_mutex;
static mutex cont_op_semaphore;
int writers = 0;
int readers = 0;
int cont_op = 0;

void time_now() {
  time (&rawtime);
  timeinfo = localtime(&rawtime);
}

struct Product {
  int id;
  int amount;
};

Product stock[STOCK_SIZE];

void initStock() {
  for (int i=0; i < STOCK_SIZE; ++i) {
    stock[i].id = i;
    stock[i].amount = DEFAULT_AMOUNT;
  }
}

void printStock() {
  printf("--------AZEITE STOCK--------\n");
  for (int i=0; i < STOCK_SIZE; ++i) {
    printf("Product [%d]\tAmount: %d\n", stock[i].id, stock[i].amount);
  }
}

struct Thread_Args {
  long thread_id;
  int product_id = -1; // -1 if is not to read
  int type_operation;
  int quantity;
};

void *reader(void *args) {
  Thread_Args arguments;
  arguments = *(struct Thread_Args*)args;
  
  long thread_id = arguments.thread_id;
  int product_id = arguments.product_id;
  
  time_now();
  // Thread start
  
  printf("+++ Thread OF READ tid[%ld] invoked!! %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  // sem_wait(&stock_semaphore);
  cont_op_semaphore.lock();
  
  cont_op ++;
  if (cont_op == 1) {
    stock_mutex.lock();
  }

  printf(ANSI_COLOR_GREEN "[SYS][READ]->" ANSI_COLOR_RESET "TID[%ld] starting!!\n\t** ENTERING CRITIC REGION !!** %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  sleep(2);
  
  time_now();
  printf(ANSI_COLOR_YELLOW "[RUNNING][READ]" ANSI_COLOR_RESET "->TID[%ld] %d:%d:%d\n\tReading data from PRODUCT_ID=[%d]. AMOUNT=[%d]\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec, product_id, stock[product_id].amount);
  
  cont_op --;
  if (cont_op == 0) {
    stock_mutex.unlock();
  }

  time_now();
  printf(ANSI_COLOR_RED "[SYS][READ]" ANSI_COLOR_RESET "Thread OF READ tid[%ld] finished!!\n\t** LEFTING  CRITIC REGION !!** %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  
  // sem_post(&stock_semaphore);
  cont_op_semaphore.unlock();
  
  pthread_exit(NULL);  
}

void *writer(void *args) {
  srand(time(NULL));
  Thread_Args arguments;
  arguments = *(struct Thread_Args*)args;
  
  long thread_id = arguments.thread_id;
  int product_id = arguments.product_id;
  int quantity = arguments.quantity;
  
  time_now();
  // Thread start
  
  printf("+++ Thread OF WRITE tid[%ld] invoked!! %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  // sem_wait(&stock_semaphore);
  stock_mutex.lock();

  printf(ANSI_COLOR_GREEN "[SYS][WRITE]->" ANSI_COLOR_RESET "TID[%ld] starting!!\n\t** ENTERING CRITIC REGION !!** %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  // sleep((rand() % 3 + 1));
  sleep(4);
  
  time_now();
  printf(ANSI_COLOR_YELLOW "[RUNNING][WRITE]->" ANSI_COLOR_RESET "TID[%ld] %d:%d:%d\n\tWriting data from PRODUCT_ID=[%d]. AMOUNT=[%d] -> [%d]\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec, product_id, stock[product_id].amount, stock[product_id].amount + quantity);
  stock[product_id].amount += quantity;
  
  time_now();
  printf(ANSI_COLOR_RED "[SYS][WRITE]->" ANSI_COLOR_RESET "TID[%ld] finished!!\n\t** LEFTING  CRITIC REGION !!** %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  
  // sem_post(&stock_semaphore);
  stock_mutex.unlock();
  
  pthread_exit(NULL);   
}

int main() {
  /* 
    O tema abordado será um estoque de uma distribuidora.
    Cada loja compradora de Product será uma thread.
  */
  srand(time(NULL));
  pthread_t stores[STORES];
  int stopper_flag;
  int product_target = 1;
  // 0 read, 1 write
  int type_threads[] = { 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0};
  
  for (int i = 0; i < 5; i++) {
    if (type_threads[i] == 1) {
      writers ++;
    } else {
      readers ++;
    }
  }

  initStock();
  printStock();

  printf("\n**** starting main ****\n\n");
  sem_init(&stock_semaphore, 0, 1);
  Thread_Args args;

  for(int i = 0; i < STORES; ++i) {
    int random_amount = (rand() % 10 + 1) % 2 == 0 ? -1 * (rand() % 10 + 1) : (rand() % 10 + 1) % 2; // random add or remove from -10 to 10
    random_amount ++;
    args = {
      i,
      product_target,
      // (rand() % 10 + 1) % 2, // random operation
      type_threads[i],
      random_amount, 
    };

    if (args.type_operation % 2 == 0)
      stopper_flag = pthread_create(&stores[i], NULL, reader, &args);
    else
      stopper_flag = pthread_create(&stores[i], NULL, writer, &args);

    if (stopper_flag != 0) {
      cout << "Error creating thread " << i << ". Return code:" << stopper_flag <<  endl;
    }

    sleep(0.0005);
  }
  
  for(int i = 0; i < STORES; i++) {
    pthread_join(stores[i], NULL);
  }
  cout << "All threads completed." << endl;

  printf("\n**** ending main ****\n\n");
  printStock();
  return 0;
}
