#include <signal.h> /* Pour sigaction */
#include <stdio.h> /* Pour printf */
#include <string.h> /* pour strcmp */
#include <time.h>
#include <sys/time.h> /* pour itimer */
#include <stdlib.h>     /* Pour EXIT_SUCCESS, malloc */
#include <unistd.h>     /* Pour sleep */
#include "structures.h" /* Pour les requetes */
#include <sys/socket.h>  /* Pour socket, bind */
#include <arpa/inet.h>   /* Pour sockaddr_in */

#define PERIOD_SIG      SIGRTMIN
#define PERIOD_SEC      0
#define PERIOD_NSEC     1000000 // 1ms

//#define _POSIX_C_SOURCE 

/*Compil avec -lrt*/

void handler(int signum, siginfo_t*info,void*rien){
    if(signum==SIGALRM)
        printf("Signal SIGVALRM reçu dans handler avec la valeur : %d\n",info->si_value.sival_int);
    
}

int main(int argc, char *argv[])
{
	sigset_t mask;
	timer_t timerid;
    struct sigaction action;
	struct itimerspec its;

	siginfo_t si;
	int sig;

    sigval_t val;
	val.sival_int=5;
    struct sigevent evp = {
		.sigev_notify = SIGEV_SIGNAL, /*SIGEV_SIGNAL va notifier le process avec le signal renseigné dans signo*/
		.sigev_signo = SIGALRM,
        .sigev_value = val
	};

	/* Bloque le signal PERIOD_SIG (la réception de PERIOD_SIG n'entraîne plus
	   l'appel d'un handler déclaré avec sigaction) */
	/*if (sigemptyset(&mask) || sigaddset(&mask, PERIOD_SIG) || sigprocmask(SIG_BLOCK, &mask, NULL))
		return 1;*/

	if (timer_create(CLOCK_REALTIME, &evp, &timerid))
		return 2;

	its.it_value.tv_sec = 1;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 1;
	its.it_interval.tv_nsec = 0;
	if (timer_settime(timerid, 0, &its, NULL))
		return 3;

     /*On positionne un gestionnaire sur SIGALRM*/
    action.sa_sigaction = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    if(sigaction(SIGALRM, &action, NULL) == -1){
        perror("Erreur lors du positionnement !");
        exit(EXIT_FAILURE);
    }

    
	/*while (1)
	{
		if ((sig = sigwaitinfo(&mask, &si)) < 0)
			return 4;
		 Periodic task... */
		printf("AAAAAAH 1\n");
		sleep(5);
		printf("AAAAAAH 2\n");
		its.it_value.tv_sec = 0;
		its.it_interval.tv_sec = 0;
		if (timer_settime(timerid, 0, &its, NULL))
			return 3;

		printf("AAAAAAH 3\n");
		sleep(5);
		printf("AAAAAAH 4\n");

		its.it_value.tv_sec = 1;
		its.it_interval.tv_sec = 1;
		if (timer_settime(timerid, 0, &its, NULL))
			return 3;

		printf("AAAAAAH 5\n");
		sleep(5);

	/*}*/

	return 0;
}