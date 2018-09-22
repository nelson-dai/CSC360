#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

#define MAX_BUFFER_LEN 80

taskval_t *event_list = NULL;

float wsum = 0.0;
float tasum = 0.0;

void print_task(taskval_t *t, void *arg) {
    printf("task %03d: %5d %3.2f %3.2f\n",
        t->id,
        t->arrival_time,
        t->cpu_request,
        t->cpu_used
    );  
}


void increment_count(taskval_t *t, void *arg) {
    int *ip;
    ip = (int *)arg;
    (*ip)++;
}


/*
	functions for outputing reports
*/
void report_idle(int tick) {
#ifndef STATS
	printf("[%05d] IDLE\n", tick);
#endif
}

void report_running(int tick, taskval_t * job) {
#ifndef STATS
	printf("[%05d] id=%05d req=%0.2f used=%0.2f\n", tick, job->id, job->cpu_request, job->cpu_used);
#endif
}

void report_dispatching(int tick) {
#ifndef STATS
	printf("[%05d] DISPATCHING\n", tick);	
#endif
}

void report_finished(int tick, taskval_t * job) {
	float turnaround = job->finish_time - job->arrival_time;
	float wait_time = turnaround - job->cpu_request;
#ifndef STATS
	printf("[%05d] id=%05d EXIT w=%0.2f ta=%0.2f\n", tick, job->id, wait_time, turnaround);
#endif
	wsum += wait_time;
	tasum += turnaround;
	
}


void run_simulation(int qlen, int dlen) {
    taskval_t *ready_q = NULL;


	int tick = 0;					//current tick
	int tick_remaining;				//#ticks left in current turn
	int dispatch_remaining = -1;	//...
	char dispatching = 0;			//...
	taskval_t *running = NULL;		//nothing is running yet
	
	while (event_list != NULL || ready_q != NULL || running != NULL) {
		// 1. is anything in the event list arriving?
		while (event_list != NULL && event_list->arrival_time == tick) {
			// Yes, place it in the ready queue.
			taskval_t * transfer = event_list;
			event_list = remove_front(event_list);
			ready_q = add_end(ready_q, transfer);
			//printf("added to ready_q: ");
			//print_task(ready_q, NULL);
			//printf("next to be removed:");
			//if (event_list != NULL) {
			//	print_task(event_list, NULL);
			//} else {
			//	printf("none\n");
			//}
		}
		
		// 2. is there a task running?
		if (running != NULL) {
			// Task is running. Does it have any quantum ticks remaining? Is it finishing?
			if (running->cpu_used >= running->cpu_request) {
				// job finished.
				running->cpu_used = running->cpu_request;
				running->finish_time = tick;
				report_finished(tick, running);
				end_task(running);
				running = NULL;
			} else {
				// check if it needs to be stopped.
				if (tick_remaining == 0) {
					ready_q = add_end(ready_q, running);
					running = NULL;
				} else {
					report_running(tick, running);
				}
			}
		}
		
		// 3. dispatch if nothing is running.
		if (running == NULL) {
			if (dispatching) {
				if (dispatch_remaining > 1) {
					// we are currently dispatching a new job. Process one more tick for that.
					dispatch_remaining -= 1;
					report_dispatching(tick);
				} 

				else {
					// we can start the next process.
					dispatching = 0;
					running = ready_q;
					ready_q = remove_front(ready_q);
					tick_remaining = qlen;
					report_running(tick, running);
				}
			} else {
				// we can dispatch a new job (if one is waiting).
				if (ready_q != NULL) {
					dispatching = 1; // true
					dispatch_remaining = dlen;
					report_dispatching(tick);
				}
			}
		}
		
		// 4. idle.
		if (running == NULL && ready_q == NULL && event_list != NULL) {
			report_idle(tick);
		}
		
		// 5. increment the tick for the next round.
		tick += 1;
		if (running != NULL) {
			running->cpu_used += 1.0;
			tick_remaining -= 1;
		}
	}
}


int main(int argc, char *argv[]) {
    char   input_line[MAX_BUFFER_LEN];
    int    i;
    int    task_num;
    int    task_arrival;
    float  task_cpu;
    int    quantum_length = -1;
    int    dispatch_length = -1;
	int    num_tasks = 0;

    taskval_t *temp_task;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--quantum") == 0 && i+1 < argc) {
            quantum_length = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--dispatch") == 0 && i+1 < argc) {
            dispatch_length = atoi(argv[i+1]);
        }
    }

    if (quantum_length == -1 || dispatch_length == -1) {
        fprintf(stderr, 
            "usage: %s --quantum <num> --dispatch <num>\n",
            argv[0]);
        exit(1);
    }


    while(fgets(input_line, MAX_BUFFER_LEN, stdin)) {
        sscanf(input_line, "%d %d %f", &task_num, &task_arrival,
            &task_cpu);
        temp_task = new_task();
        temp_task->id = task_num;
        temp_task->arrival_time = task_arrival;
        temp_task->cpu_request = task_cpu;
        temp_task->cpu_used = 0.0;
        event_list = add_end(event_list, temp_task);
		num_tasks++;
    }

#ifdef DEBUG
    int num_events = 0;
    apply(event_list, increment_count, &num_events);
    printf("DEBUG: # of events read into list -- %d\n", num_events);
    printf("DEBUG: value of quantum length -- %d\n", quantum_length);
    printf("DEBUG: value of dispatch length -- %d\n", dispatch_length);
#endif

    run_simulation(quantum_length, dispatch_length);

#ifdef STATS
	printf("%d %d %f %f\n", quantum_length, dispatch_length, wsum / num_tasks, tasum / num_tasks);
#endif

    return (0);
}
