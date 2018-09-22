/*Required Headers*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "rw.h"
#include "resource.h"

/*
 * Declarations for reader-writer shared variables -- plus concurrency-control
 * variables -- must START here.
 */

static resource_t data;
static sem_t mutex;

void initialize_readers_writer() {
    /*
     * Initialize the shared structures, including those used for
     * synchronization.
     */
	sem_init(&mutex, 0, 1);
	init_resource(&data, "resource");
}


void rw_read(char *value, int len) {
	sem_wait(&mutex);
	read_resource(&data, value, len);
	sem_post(&mutex);
}


void rw_write(char *value, int len) {
	sem_wait(&mutex);
	write_resource(&data, value, len);
	sem_post(&mutex);
}

void rw_stats(char * stats) {
	sem_wait(&mutex);
	sprintf(stats, "Resource: %s; # reads: %d; # writes: %d\n",
        data.label, data.num_reads, data.num_writes);
	sem_post(&mutex);
}

