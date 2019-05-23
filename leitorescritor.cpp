#include <iostream>
#include <cstdio>
#include <pthread.h>
#include <unistd.h>

using namespace std;

#define STOCK_SIZE 5
#define DEFAULT_AMOUNT 40
#define STORES 2

#define SLEEP_MAIN 20
#define SLEEP_READ 5
#define SLEEP_WRITE 8

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
};

void *changeAmount(void *args) {
  Thread_Args arguments;
  arguments = *(struct Thread_Args*)args;

  long thread_id = arguments.thread_id;
  int operation = arguments.operation;
  int product_id = arguments.product_id;
  int amount;
  
  // Thread start
  printf("\n--> Thread tid[%ld] starting!!\n", thread_id);
  // Show what is doing
  printf("\t__TID[%ld]__\tWill run operation [%d] to PRODUCT_ID=[%d]\n", thread_id, operation, product_id);
  
  // Reading the data of product
  printf("\t__TID[%ld]__\tReading data from PRODUCT_ID=[%d]\n", thread_id, product_id);
  // Simulate slow and processing the read
  sleep(SLEEP_READ);
  amount = stock[product_id].amount;
  // Show what you read
  printf("\t__TID[%ld]__\tFinished reading data from PRODUCT_ID=[%d]  ---  Read the AMOUNT=[%d]\n", thread_id, product_id, amount);

  // Writing the new amount of the product
  printf("\t__TID[%ld]__\tChanging data from PRODUCT_ID=[%d] with operation [%d]  ---  Actual value of AMOUNT=[%d]\n", thread_id, product_id, operation, amount);
  // Simulate slow and processing the write
  sleep(SLEEP_WRITE);
  stock[product_id].amount = amount + operation;
  printf("\t__TID[%ld]__\tFinished writing amount of PRODUCT_ID=[%d] the operation [%d]  ---  Past value of AMOUNT=[%d].  ---  Actual value of AMOUNT=[%d]\n", thread_id, product_id, operation, amount, stock[product_id].amount);
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

  printf("Infos:\n\t- Threads to run: %d\n\n", STORES);

  initStock();
  printStock();

  printf("\n**** starting main ****\n");
  /* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  

  for(int i = 0; i < STORES; ++i) {
    Thread_Args args = {
      1,      // tid 
      add(4), // operation
      2,      //pid
    };

    stopper_flag = pthread_create(&stores[0], &attr, changeAmount, (void *)&args);
    
    if (stopper_flag) {
      printf("ERROR; return code from pthread_create() is %d\n", stopper_flag);
      exit(-1);
    }
    break;
  }
  
  // printf("Depois de chamar o create da stopper_flag, valor: %d", stopper_flag);
  /* We're done with the attribute object, so we can destroy it */
  pthread_attr_destroy(&attr);

  sleep(SLEEP_MAIN);
  printf("\n**** ending main ****\n\n");
  printStock();
  return 0;
}
