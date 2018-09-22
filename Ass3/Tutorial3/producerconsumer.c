/*
    In this version the buffer is a single number.
    The producer is putting numbers into the shared buffer
    And the consumer is taking them out.
    If the buffer contains zero, that indicates that the buffer is empty.
    Any other value is valid.
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MAX 10			/* Numbers to produce */

/* 
 * Globally declare the mutex and condition variables
 * so we can use them with the threads
 * You can also initialize them with compiler flags like this:
 *      pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 *      pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 */
pthread_mutex_t buffer_mutex;
pthread_cond_t isfull, isempty;
int buffer = 0;


struct data {
    int a;
    int b;
};  //Remember the ; here!

void* producer(void *_data) {
  int i;

  //Threads only accept void* arguments, so you
  //need to cast back to the original pointer type
  struct data* mydata = (struct data*)_data;

  for (i = 1; i <= MAX; i++) {
    /* This function attempts to lock the mutex, or block the thread 
     * if the mutex is already locked by another thread. 
     * In this case, when the mutex is unlocked by the consumer thread,
     * we can set a new buffer value */
    pthread_mutex_lock(&buffer_mutex);	
    
    // If there is something in the buffer, then wait
    while (buffer != 0) { 
      /* Block this thread until another thread signals isempty. 
       * While blocked, the mutex is released, then re-aquired before this
       * thread is woken up and the call returns. */  
      printf("%s    : waiting on %s signal, buffer = %d\n", "producer", "isempty", buffer);
      pthread_cond_wait(&isempty, &buffer_mutex);
    }
    
    //Update buffer
    buffer = (i % 2) ? i + mydata->b : i+mydata->a;
    
    printf("%s : buffer = %d\n", "producer", buffer); 
    
    /* Signal that the buffer is full, so the consumer can read it
     * Since I have multiple consumers, and I don't care who reads, I'll 
     * use _broadcast()!  However, if order matters (i.e. FCFS)), then using broadcast
     * is a bad idea.
     */
    pthread_cond_broadcast(&isfull);	

    /* After the thread did what it had to,
     * it should free the mutex, so other threads
     * can progress. If we don't, they deadlock!
     */
    pthread_mutex_unlock(&buffer_mutex);	
  }
    
  pthread_exit(0);
}

void* consumer(void *_id) {
  int i;

  int id = *((int*)_id);

  for (i = 1; i <= MAX; i++) {
    /* This function attempts to lock the mutex, or block the thread 
     * if the mutex is already locked by another thread. 
     * In this case, when the mutex is unlocked by the producer thread,
     * we can consume the buffer value */
    pthread_mutex_lock(&buffer_mutex);	
    
    // If there is nothing in the buffer, then wait
    while (buffer == 0) {			
      /* Block this thread until another thread signals isfull. 
       * While blocked, the mutex is released, then re-aquired before this
       * thread is woken up and the call returns. */  
      printf("%s:%d : waiting on %s signal, buffer = %d\n", "consumer", id, "isfull", buffer);
      pthread_cond_wait(&isfull, &buffer_mutex);
    }
   
    //Read the buffer 
    printf("%s:%d  : buffer = %d\n", "consumer", id, buffer); 
    buffer = 0;

    /* Signal that the buffer is empty, so the producer can write to it
     * You can also you _broadcast(), however, this will release EVERY
     * thread that is attached to the mutex! 
     */
    pthread_cond_signal(&isempty);	
    
    /* After the thread did what it had to,
     * it should free the mutex, so other threads
     * can progress. If we don't, they deadlock!
     */
    pthread_mutex_unlock(&buffer_mutex);	/* release the buffer */
  }
  pthread_exit(0);
}

int main(int argc, char **argv) {
  pthread_t pro, con1, con2;

  // Initialize the mutex and condition variables
  // There are mutex attributes you can use, but using defaults (NULL) is fine
  pthread_mutex_init(&buffer_mutex, NULL);	
  pthread_cond_init(&isfull, NULL);		/* Initialize consumer condition variable */
  pthread_cond_init(&isempty, NULL);		/* Initialize producer condition variable */

  /*
   * pthread_create() gets 4 arguments: 
   *    The first argument is a pointer to thread_id, used by pthread_create() to supply the program with the thread's identifier. 
   *    The second argument is used to set some attributes for the new thread (you can set this to NULL for default values)
   *    The third argument is the function (must be a void* type!)
   *    The fourth argument is the pointer to any additional function arguments we want to use in our function (also void* !)
   *        - NOTE: If you want to pass more values, you need to use a struct
   */
  struct data mydata = {.a=1, .b=2};
  const int id1 = 1;
  const int id2 = 2;
   
  pthread_create(&con1, NULL, consumer, (void*)&id1);
  pthread_create(&con2, NULL, consumer, (void*)&id2);
  pthread_create(&pro, NULL, producer, (void*)&mydata);

  // Wait for the threads to finish
  // Otherwise main might run to the end
  // and kill the entire process when it exits.
  pthread_join(con1,  NULL);
  pthread_join(con2,  NULL);
  pthread_join(pro, NULL);

  // Cleanup -- would happen automatically at end of program
  pthread_mutex_destroy(&buffer_mutex);	/* Free up buffer_mutex */
  pthread_cond_destroy(&isfull);		/* Free up consumer condition variable */
  pthread_cond_destroy(&isempty);		/* Free up producer condition variable */

}
