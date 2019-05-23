#include <iostream>
#include <cstdio>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

using namespace std;

#define STOCK_SIZE 5
#define DEFAULT_AMOUNT 40
#define STORES 2

#define SLEEP_MAIN 20
// #define SLEEP_READ 2
// #define SLEEP_WRITE 4
#define SLEEP_BETWEEN_THREADS 4

time_t rawtime;
struct tm * timeinfo;

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
  int operation = 0; // 0 will make no effects
  int product_id = -1; // -1 if is not to read
  int sleep_read = 0; // 0 will make no effects
  int sleep_write = 0; // 0 will make no effects
};

void *changeAmount(void *args) {
  Thread_Args arguments;
  arguments = *(struct Thread_Args*)args;
  
  long thread_id = arguments.thread_id;
  int operation = arguments.operation;
  int product_id = arguments.product_id;
  int sleep_read = arguments.sleep_read;
  int sleep_write = arguments.sleep_write;
  int amount;

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  // Thread start
  printf("--> Thread tid[%ld] starting!! %d:%d:%d\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec);
  // Show what is doing
  // printf("__TID[%ld]__ %d:%d:%d\tWill run operation [%d] to PRODUCT_ID=[%d]\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec, operation, product_id);
  
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  // Reading the data of product
  printf("__TID[%ld]__ %d:%d:%d\t** ENTERING CRITIC REGION **\tReading data from PRODUCT_ID=[%d]\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec, product_id);
  // Simulate slow and processing the read
  sleep(sleep_read);
  amount = stock[product_id].amount;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  // Show what you read
  printf("__TID[%ld]__ %d:%d:%d\t** LEFTING CRITIC REGION  **\tFinished reading data from PRODUCT_ID=[%d]  ---  Read the AMOUNT=[%d]\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec, product_id, amount);

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  // Writing the new amount of the product
  printf("__TID[%ld]__ %d:%d:%d\t** ENTERING CRITIC REGION **\tChanging data from PRODUCT_ID=[%d] with operation [%d]  ---  Actual value of AMOUNT=[%d]\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec, product_id, operation, amount);
  // Simulate slow and processing the write
  sleep(sleep_write);
  stock[product_id].amount = amount + operation;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  printf("__TID[%ld]__ %d:%d:%d\t** LEFTING CRITIC REGION  **\tFinished writing amount of PRODUCT_ID=[%d] the operation [%d]  ---  Past value of AMOUNT=[%d].  ---  Actual value of AMOUNT=[%d]\n", thread_id, timeinfo->tm_hour, timeinfo->tm_hour, timeinfo->tm_sec, product_id, operation, amount, stock[product_id].amount);
  amount = stock[product_id].amount;

}

int add(int n) {
  return n;
}

int remove(int n) {
  return -n;
}

int main() {
  /* 
    O tema abordado será um estoque de uma distribuidora.
    Cada loja compradora de Product será uma thread.
  */
  pthread_t stores[STORES];
  pthread_attr_t attr;
  int stopper_flag;
  int operations[] = { add(4), remove(2) };
  int products_affected[] = { 2, 2 };
  int sleeps_read[] = { 2, 3 };
  int sleeps_write[] = { 6, 4 };

  printf("Infos:\n\t- Threads to run: %d\n\n", STORES);

  initStock();
  printStock();

  printf("\n**** starting main ****\n\n");
  /* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  
  Thread_Args args;

  for(int i = 0; i < STORES; ++i) {
    args = {
      i,                          // tid 
      operations[i],              // operation
      products_affected[i],       // pid
      sleeps_read[i],             // sleep_read
      sleeps_write[i],            // sleep_write
    };

    stopper_flag = pthread_create(&stores[i], &attr, changeAmount, &args);

    if (stopper_flag) {
      printf("ERROR; return code from pthread_create() is %d\n", stopper_flag);
      exit(-1);
    }
    sleep(SLEEP_BETWEEN_THREADS);
  }
  
  // printf("Depois de chamar o create da stopper_flag, valor: %d", stopper_flag);
  /* We're done with the attribute object, so we can destroy it */
  pthread_attr_destroy(&attr);

  sleep(SLEEP_MAIN);
  printf("\n**** ending main ****\n\n");
  printStock();
  return 0;
}
