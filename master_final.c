#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<ctype.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include "config.h"

int slave_max = 20;

// global variables
pid_t *children;
int shmid_choosing, shmid_turnNum;
// globals relating to shared memory
key_t shmkey;
int *choosing; int *turnNum;
//int shmid_sharedNum;
//int *sharedNum;

void handle_sigalrm(int signum, siginfo_t *info, void *ptr)
{
   // prevents multiple interrupts
   signal(SIGINT, SIG_IGN);

   fprintf(stderr, "Master ran out of time\n");

   // detaching and deleting shared memory
   shmdt(choosing);
   shmctl(shmid_choosing, IPC_RMID, NULL);
    shmdt(turnNum);
   shmctl(shmid_turnNum, IPC_RMID, NULL);
   

   // creating tmp_children to replace children
   // this way children can be freed before SIGTERM
   pid_t tmp_children[slave_max];
   int i;
   for (i = 0; i < slave_max; i++)  {
      tmp_children[i] = children[i];
   }

   // freeing allocated memory
   free(children);

   // terminate child processes
   for (i = 0; i < slave_max; i++)
   {
      kill(tmp_children[i], SIGTERM);
   }
}

void handle_sigint(int signum, siginfo_t *info, void *ptr)
{
   // prevents multiple interrupts
   signal(SIGINT, SIG_IGN);
   signal(SIGALRM, SIG_IGN);

   fprintf(stderr, " interrupt was caught by master\n");

   // detaching and deleting shared memory
   shmdt(choosing);
   shmctl(shmid_choosing, IPC_RMID, NULL);
    shmdt(turnNum);
   shmctl(shmid_turnNum, IPC_RMID, NULL);

   // creating tmp_children to replace children
   // this way children can be freed before SIGTERM
   pid_t tmp_children[slave_max];
   int i;
   for (i = 0; i < slave_max; i++)
   {
      tmp_children[i] = children[i];
   }

   // freeing allocated memory
   free(children);

   // terminate child processes
   for (i = 0; i < slave_max; i++)
   {
      kill(tmp_children[i], SIGTERM);
   }
}

void catch_sigalrm()
{
   static struct sigaction _sigact;
   memset(&_sigact, 0, sizeof(_sigact));
   _sigact.sa_sigaction = handle_sigalrm;
   _sigact.sa_flags = SA_SIGINFO;
   sigaction(SIGALRM, &_sigact, NULL);
}

void catch_sigint()
{
   static struct sigaction _sigact;
   memset(&_sigact, 0, sizeof(_sigact));
   _sigact.sa_sigaction = handle_sigint;
   _sigact.sa_flags = SA_SIGINFO;
   sigaction(SIGINT, &_sigact, NULL);
}

int main(int argc, char *argv[])
{
   // local variables
   int i = 0; //  counter variable
   char slave_max_str[25]; 
   char *log_filename = NULL;
   int slave_increment = 3;
   char slave_increment_str[25]; 
   int master_time = 100;

   // shared memory initialization
    // shared memory initialization

shmkey = ftok("./master", KEY_CHOOSING); // key is defined in config.h
shmid_choosing = shmget(shmkey, sizeof(choosing), 0600 | IPC_CREAT);
choosing = (int *)shmat(shmid_choosing, NULL, 0);
shmkey = ftok("./master", KEY_NUMBER); // key is defined in config.h
shmid_turnNum = shmget(shmkey, sizeof(turnNum), 0600 | IPC_CREAT);
turnNum = (int *)shmat(shmid_turnNum, NULL, 0);

   // handling command line args 
   int t = -1;
   t = getopt(argc, argv, ":t:");
   
   if(t==-1) {printf("option -t missing, use -t and specify max time for master");exit(0);}
    else{
        master_time = atoi(optarg);
        printf("master time %d ",master_time);
    }
   if(argv[3]==NULL) {
   //printf("number of process is missing, correct way is ");exit(0);
   }
   else if(slave_max >20){
   	printf("number of process exceeeding max limit, max 20 process allowed. ");exit(0);
   }
   else{
       slave_max = atoi(argv[3]);
       printf("child count %d",slave_max);  
   }
   
   catch_sigint();
   catch_sigalrm();
   alarm(master_time);

   // if log_filename wasn't passed in by -l,
   // its default value is set here...
   if (!log_filename)
      log_filename = "test.out";

   // setting slave_increment_str and slave_max_str
   // for use in future execl
   snprintf(slave_increment_str, 25, "%i", slave_increment);
   snprintf(slave_max_str, 25, "%i", slave_max);

   // initializing pids
   if ((children = (pid_t *)(malloc(slave_max * sizeof(pid_t)))) == NULL)
   {
      errno = ENOMEM;
      perror("children malloc");
      exit(1);
   }
   pid_t p;

   // forking off child processes
   for (i = 0; i < slave_max; i++)
   {
      p = fork();
      if (p < 0)
      {
         fprintf(stderr,"Error: fork failed\n");
         continue;
      }
      if (p == 0)
      {
         children[i] = p;
         execl("./slave", "slave",(char *) NULL);
         exit(0);
      }
   }

   // waiting for all child processes to finish
   for (i = 0; i < slave_max; i++)
   {
      int status;
      waitpid(children[i], &status, 0);
   }

   // clean up and finish
   free(children);
   //shmdt(sharedNum);
   //shmctl(shmid_sharedNum, IPC_RMID, NULL);
   return 0;
}