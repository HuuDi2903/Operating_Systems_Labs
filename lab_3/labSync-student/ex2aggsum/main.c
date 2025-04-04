#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* for getopt */

#include "utils.h"
#include <errno.h>
#include <limits.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

/** process command line argument.
 *  values are made available through the 'conf' struct.
 *  using the parsed conf to get arguments: the arrsz, tnum, and seednum
 */
extern int processopts (int argc, char **argv, struct _appconf *conf); 

/** process string to number.
 *  string is stored in 'nptr' char array.
 *  'num' is returned the valid integer number.
 *  return 0 valid number stored in num
 *        -1 invalid and num is useless value.
 */
extern int tonum (const char * nptr, int * num);

/** validate the array size argument.
 *  the size must be splitable "num_thread".
 */
extern int validate_and_split_argarray (int arraysize, int num_thread, struct _range* thread_idx_range);

/** generate "arraysize" data for the array "buf"
 *  validate the array size argument.
 *  the size must be splitable "num_thread".
 */
extern int generate_array_data (int* buf, int arraysize, int seednum);

/** display help */
extern void help (int xcode);

void* sum_worker (void *arg);
long validate_sum(int arraysize);

/* Global sum buffer */
long sumbuf = 0;
int* shrdarrbuf;
pthread_mutex_t mtx;

void* sum_worker (void *arg) {
   struct _range idx_range = *(struct _range*)arg;
   int i;
   
   //printf("In worker from %d to %d\n", idx_range.start, idx_range.end);

   long local_sum = 0;  // Local sum for this thread
   for (int i = idx_range.start; i <= idx_range.end; i++) {
       local_sum += shrdarrbuf[i];  // Calculate the sum for this thread
   }
 
   pthread_mutex_lock(&mtx);  // Lock the mutex before updating total_sum
   sumbuf += local_sum;  // Update the global total sum
   pthread_mutex_unlock(&mtx);  // Unlock the mutex
 
   printf("Thread completed range %d to %d with local sum: %ld\n", idx_range.start, idx_range.end, local_sum);
      
   return 0;
		
}

int main(int argc, char * argv[]) {
   int i, arrsz, tnum, seednum;
   char *buf;
   struct _range* thread_idx_range;
   pthread_t* tid;
   int pid;

    if (argc < 3 || argc > 4) /* only accept 2 or 3 arguments */
      help(EXIT_SUCCESS);

#if(DBGSTDERR == 1)
   freopen("/dev/null","w",stderr); /* redirect stderr by default */
#endif
   
   processopts(argc, argv, &appconf); /* process all option and argument */

   fprintf(stdout,"%s runs with %s=%d \t %s=%d \t %s=%d\n", PACKAGE,
                  ARG1, appconf.arrsz, ARG2, appconf.tnum, ARG3, appconf.seednum);

   thread_idx_range = malloc(appconf.tnum * sizeof(struct _range));
   if(thread_idx_range == NULL)
   {
      printf("Error! memory for index storage not allocated.\n");
      exit(-1);
   }
	
   if (validate_and_split_argarray(appconf.arrsz, appconf.tnum, thread_idx_range) < 0)
   {
      printf("Error! array index not splitable. Each partition need at least %d item\n", THRSL_MIN);
      exit(-1);	   
   }
    
   /* Generate array data */
   shrdarrbuf = malloc(appconf.arrsz * sizeof(int));
   if(shrdarrbuf == NULL)
   {
      printf("Error! memory for array buffer not allocated.\n");
      exit(-1);
   }
 
   if(generate_array_data(shrdarrbuf, appconf.arrsz, appconf.seednum) < 0) 
   {
      printf("Error! array index not splitable.\n");
      exit(-1);	   
   }

   pid=fork();
   
   if(pid < 0)
   {
      printf("Error! fork failed.\n");
      exit(-1);   
   }  
   
   if(pid == 0) { /* child process do a validation sum */
      printf("sequence sum results %ld\n",validate_sum(appconf.arrsz));
      exit(0);
   } // parent process goes here
	   
   /** Create <tnum> thead to calculate partial non-volatile sum
    *  the non-volatile mechanism require value added to global sum buffer
    */
   tid = malloc (appconf.tnum * sizeof(pthread_t));

   for (i=0; i < appconf.tnum; i++)
      pthread_create(&tid[i], NULL, sum_worker, &thread_idx_range[i]);

   for (i=0; i < appconf.tnum; i++)
      pthread_join(tid[i], NULL);
      
   fflush(stdout);
	
   printf("%s gives sum result %ld\n", PACKAGE, sumbuf);

   int status;
   waitpid(pid, &status, 0); /* wait for child process to finish */
      exit(0);
}

long validate_sum(int arraysize)
{
   long validsum = 0;
   int i;

   for (i=0; i < arraysize; i++)
      validsum += shrdarrbuf[i];

   return validsum;
}
