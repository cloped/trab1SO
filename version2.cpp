#include <iostream>
#include <cstdio>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()
#include <semaphore.h>

using namespace std;

#define STOCK_SIZE 5
#define DEFAULT_AMOUNT 40
#define STORES 5

time_t rawtime;
struct tm * timeinfo;
static sem_t stock_semaphore;

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
  
  sem_wait(&stock_semaphore);
  
  printf("--> Thread OF READ tid[%ld] starting!! ** ENTERING CRITIC REGION !! ** %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  sleep(2);
  
  time_now();
  printf("__TID[%ld]__ %d:%d:%d\tReading data from PRODUCT_ID=[%d]. AMOUNT=[%d]\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec, product_id, stock[product_id].amount);
  
  time_now();
  printf("XXX Thread OF READ tid[%ld] finished!! ** LEFTING  CRITIC REGION !! ** %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  
  sem_post(&stock_semaphore);
  
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
  
  sem_wait(&stock_semaphore);

  printf("--> Thread OF WRITE tid[%ld] starting!! ** ENTERING CRITIC REGION !! ** %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  sleep((rand() % 3 + 1));
  
  time_now();
  printf("__TID[%ld]__ %d:%d:%d\t Writing data from PRODUCT_ID=[%d]. AMOUNT=[%d] -> [%d]\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec, product_id, stock[product_id].amount, stock[product_id].amount + quantity);
  stock[product_id].amount += quantity;
  
  time_now();
  printf("XXX Thread OF WRITE tid[%ld] finished!! ** LEFTING  CRITIC REGION !! ** %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  
  sem_post(&stock_semaphore);
  
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
  int type_threads[] = { 1, 0, 1, 0, 1};

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
