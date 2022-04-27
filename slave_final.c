#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>
#include<sys/types.h>
#include<time.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
# include "config.h"

// global variables
pid_t parent;
pid_t child;
int childProc;

// globals for shared memory
key_t shmkey;
int  shmid_choosing, shmid_turnNum;
int *choosing; int *turnNum;

int main(int argc, char *argv[])
{
   // default variables
   parent = getppid();
   child = getpid();
   childProc = (int)(child - parent);
   int j, maxCounter; // to be used as a counter variables
   int slave_max = 20;
   char *log_filename = NULL;
   //int slave_incrementer = 3;
   srand(time(NULL));
   int napTime;

   // shared memory initialization
shmkey = ftok("./master", KEY_CHOOSING); // key is defined in config.h
shmid_choosing = shmget(shmkey, sizeof(choosing), 0600 | IPC_CREAT);
choosing = (int *)shmat(shmid_choosing, NULL, 0);
shmkey = ftok("./master", KEY_NUMBER); // key is defined in config.h
shmid_turnNum = shmget(shmkey, sizeof(turnNum), 0600 | IPC_CREAT);
turnNum = (int *)shmat(shmid_turnNum, NULL, 0);

   if (!log_filename)
      log_filename = "test.out";

   //struct timespec now;
   //long curTime;
   int max = 0;
  
      // execute code to enter critical section
      choosing[(childProc-1)] = 1;
      for (maxCounter = 0; maxCounter < slave_max; maxCounter++)
      {
          if((turnNum[maxCounter]) > max)
             max = (turnNum[maxCounter]);
      }
      turnNum[(childProc-1)] = 1 + max;
      printf("turnNum for process #%i = %i\n", childProc, turnNum[(childProc-1)]);
      choosing[(childProc-1)] = 0;
      for (j = 0; j < slave_max; j++)
      {
     while (choosing[j] == 1) {}
         while ((turnNum[j] != 0) && (turnNum[j] < turnNum[(childProc-1)])) {}
      }

      // critical section
      napTime = rand() % 3;
      sleep(napTime);
      
    /* get seconds since the Epoch */
time_t secs = time(0);

/* convert to localtime */
struct tm *local = localtime(&secs);

      // write into file
      
      FILE *fp;
         //char ch;
         fp = fopen("cstest.txt", "a");
         // fprintf(fp, "Hello file by fprintf...\n")
         fprintf(fp,"%02d:%02d:%02d Queue %d by process number %d \n", local->tm_hour, local->tm_min, local->tm_sec,childProc,child);
         fclose(fp);

      // exit from critical section
      turnNum[(childProc-1)] = 0;
   

   return 0;
}
	