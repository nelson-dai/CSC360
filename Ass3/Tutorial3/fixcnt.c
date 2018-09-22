// Original downloaded from here : http://www.csc.villanova.edu/~mdamian/threads/badcnt.c

#include <pthread.h> // for pthread_create, pthread_join, pthread_exit
#include <semaphore.h> //for the sem_* functions
#include <stdio.h>   
#include <stdlib.h>  

#define NITER 1000000

int cnt = 0;

//Declare global semaphore
sem_t mutex;

void *count(void * a)
{
    int i, tmp;
    
    
    
    for(i = 0; i < NITER; i++)   
    {
        /* To wait on a semaphore, use sem_wait:
         *       int sem_wait(sem_t *sem);
         *             
         *       - If the value of the semaphore is negative, the calling process blocks; 
         *         one of the blocked processes wakes up when another process calls sem_post.
         */
        sem_wait (&mutex);
       
        /* critical section */ 
        tmp = cnt;      /* copy the global cnt locally */
        tmp = tmp+1;    /* increment the local copy */
        cnt = tmp;      /* store the local value into the global cnt */ 
    
         /* To increment the value of a semaphore, use sem_post:
         *       int sem_post(sem_t *sem);
         * 
         *       - It increments the value of the semaphore and wakes 
         *         up a blocked process waiting on the semaphore, if any.
         */
        sem_post(&mutex);
    }
    
}

int main(int argc, char * argv[])
{
    pthread_t tid1, tid2;

    /*
     * To initialize a semaphore, use sem_init:
     *
     *
     *       int sem_init(sem_t *sem, int pshared, unsigned int value);
     *       
     *       - sem points to a semaphore object to initialize
     *       - pshared is a flag indicating whether or not the semaphore 
     *          should be shared with fork()ed processes (just set it to 0). 
     *       - value is an initial value to set the semaphore to
     */
    sem_init(&mutex, 0, 1);


    if(pthread_create(&tid1, NULL, count, NULL))
    {
        printf("\n ERROR creating thread 1");
        exit(1);
    }

    if(pthread_create(&tid2, NULL, count, NULL))
    {
        printf("\n ERROR creating thread 2");
        exit(1);
    }

    if(pthread_join(tid1, NULL))        /* wait for the thread 1 to finish */
    {
        printf("\n ERROR joining thread");
        exit(1);
    }

    if(pthread_join(tid2, NULL))        /* wait for the thread 2 to finish */
    {
        printf("\n ERROR joining thread");
        exit(1);
    }

    if (cnt < 2 * NITER) 
        printf("\n BOOM! cnt is [%d], should be %d\n", cnt, 2*NITER);
    else
        printf("\n OK! cnt is [%d]\n", cnt);
  
    pthread_exit(NULL);
    /* To destroy a semaphore, use
     *       int sem_destroy(sem_t *sem);
     *       destroys the semaphore; no threads should be waiting on the semaphore if its destruction is to succeed.
     */
    sem_destroy(&mutex);
}


