For the extra work I added a client operation that allows it to retrieve the Resource statistics (# reads, # writes). 
The "op" command is "stats". In this project there are two different Resource structures, so you specify which
one you want using the "val" key in the URL:

	curl "localhost:15253/?op=stats&val=meetup"
	curl "localhost:15253/?op=stats&val=rw"

If an invalid value for "val" is given the resulting response is a small usage statement.

What I have to modify in order to make it work:

requests.h --> added "STATS" item to the enum.

network.c --> in the parse_request() function, added code to set the return value to STATS when op=stats line 69 & 70 
	
69						} else if (strncmp(s, "stats", 5) == 0) {
70							result = STATS;
			  
			  The code continues after this to parse the "val" parameter the same as the "write" and "meetup"
			  operations.
		

myserver.c --> added a case block to handle the case when operation == STATS; calls the function stats() when this occurs. line119-125. It checks the operand ("rw" or "meetup") and calls the appropriate X_stats() method.

         			case STATS:
120                         printf("server: STATS\n");
121                         if (strcmp(operand, "meetup") == 0) meetup_stats(operand);
122                         else if (strcmp(operand, "rw") == 0) rw_stats(operand);
123                         else strcpy(operand, "Invalid val parameter. Choose \"meetup\" or \"rw\"");
124                         sprintf(result_message, "server: STATS -- %s\n", operand);
125                         break;


meetup.h --> added prototype of meetup_stats() function. line 10

10					void meetup_stats(char *);


meetup.c --> added meetup_stats() function which retrieves .num_reads & .num_writes from the meetup Resource struct, 
	     and copies a formatted output string to the array that will be returned to the client. lines 102-107

102					void meetup_stats(char * stats) {
103						pthread_mutex_lock(&position_lock);
104						sprintf(stats, "Resource: %s; # reads: %d; # writes: %d\n",
105				        	r.label, r.num_reads, r.num_writes);
106						pthread_mutex_unlock(&position_lock);
107					}

rw.h --> added prototype of rw_stats() function. line 10

9					void rw_stats(char *);


meetup.c --> added rw_stats() function which retrieves .num_reads & .num_writes from the rw Resource struct, 
	     and copies a formatted output string to the array that will be returned to the client. lines 42-47

42					void rw_stats(char * stats) {
42						sem_wait(&mutex);
44						sprintf(stats, "Resource: %s; # reads: %d; # writes: %d\n",
45				        	data.label, data.num_reads, data.num_writes);
46						sem_post(&mutex);
47					}

do_scenario4.sh --> (copied from do_scenario2.sh) line 44 to add the stats command after the meetup commands.

44					tmux send-keys -t $NAME:0.5 'curl "localhost:15253/?op=stats&val=meetup"' C-m

do_scenario5.sh --> (copied from do_scenario1.sh) line 44 to add the stats command after the rw commands.

44					tmux send-keys -t $NAME:0.5 'curl "localhost:15253/?op=stats&val=rw"' C-m

