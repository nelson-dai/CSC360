/*Required Headers*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "meetup.h"
#include "resource.h"

/*
 * Declarations for barrier shared variables -- plus concurrency-control
 * variables -- must START here.
 */
static int position;
static pthread_mutex_t position_lock;
static pthread_cond_t full_cv;
static resource_t r;
static int num;
static int type;

void initialize_meetup(int n, int mf) {
    char label[100];
    int i;

    if (n < 1) {
        fprintf(stderr, "Who are you kidding?\n");
        fprintf(stderr, "A meetup size of %d??\n", n);
        exit(1);
    }

    /*
     * Initialize the shared structures, including those used for
     * synchronization.
     */
	position = 0;
	num = n;
	type = mf;
	pthread_mutex_init(&position_lock, NULL);
	pthread_cond_init(&full_cv, NULL);
	init_resource(&r, "Hipsters");
	//printf("Meetup type is %d\n", mf);
	
}


void join_meetup(char *value, int len) {
	
	char name[100];
	strcpy(name, value);
	
	// Grab the lock that controls the "position" variable.
	pthread_mutex_lock(&position_lock);
	
	//printf("Serving hipster %s\n", name);
	
	// Find out if I'm the first or last for this group.
	if (type == MEET_FIRST && position == 0) {
		// I'm first, store the codeword in the resource.
		write_resource(&r, value, len);
		position++;
		
		// Block until everyone arrives. Since I had the codeword
		// I can return to the client as soon as the condition is
		// achieved, so we can release the lock and return.
		//printf("%s is waiting.\n", name);
		//fflush(stdout);
		pthread_cond_wait(&full_cv, &position_lock);

	} else if (position == num-1) {
		// I'm last. What I do depends on the meetup type.
		if (type == MEET_FIRST) {
			//copy the codeword into value.
			read_resource(&r, value, len);
		} else {
			// copy the client's word into the resource.
			write_resource(&r, value, len);
		}
		
		// In either case, we can reset the position back to zero, and broadcast
		// the cv.
		position = 0;
		//printf("%s is done and broadcasting\n", name);
		pthread_cond_broadcast(&full_cv);

	} else {
		// Neither first nor last, just wait for everyone to arrive.
		position++;
		
		// Block until the group arrives; once this returns the
		// codeword will be in the resource.
		//printf("%s is waiting.\n", name);
		pthread_cond_wait(&full_cv, &position_lock);
		
		// Copy the codeword into value to be returned to the client.
		read_resource(&r, value, len);
	}

	// We're done, unlock the mutex.
	//rintf("Releasing %s with codeword %s\n", name, value);
	//fflush(stdout);
	pthread_mutex_unlock(&position_lock);
}
