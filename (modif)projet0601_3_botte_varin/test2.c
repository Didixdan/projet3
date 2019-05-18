#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

void timer_handler (int signum)
{
if(signum==SIGVTALRM) {
    static int count = 0;
    printf ("timer expired %d times\n", ++count);
    }
}

typedef struct {
  int *val;
}structA_t;

int main ()
{
 /*struct sigaction sa;
 struct itimerval timer;

  Install timer_handler as the signal handler for SIGVTALRM. 
 memset (&sa, 0, sizeof (sa));
 sa.sa_handler = &timer_handler;
 sigaction (SIGVTALRM, &sa, NULL);

  Configure the timer to expire after 250 msec...
 timer.it_value.tv_sec = 1;
 timer.it_value.tv_usec = 0;
  ... and every 250 msec after that. 
 timer.it_interval.tv_sec = 1;
 timer.it_interval.tv_usec = 0;
  Start a virtual timer. It counts down whenever this process is
   executing.
 setitimer (ITIMER_VIRTUAL, &timer, NULL);

 Do busy work. 
 while (1);*/

 structA_t varA;
 varA.val = (int*)malloc(sizeof(int));
 varA.val[0] = 6;

 structA_t varB;
 varB = varA;

 varA.val[0] = 7;

 printf("%d;%d\n",varA.val[0],varB.val[0]);
}