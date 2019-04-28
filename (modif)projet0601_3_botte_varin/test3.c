#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>

static void print_siginfo(siginfo_t *si)
       {
        timer_t *tidp;
        int or;

        tidp = si->si_value.sival_ptr;

        printf("    sival_ptr = %p; ", si->si_value.sival_ptr);
        printf("    *sival_ptr = 0x%lx\n", (long) *tidp);

        or = timer_getoverrun(*tidp);
        if (or == -1){
            perror("Erreur positionnement ");
            exit(EXIT_FAILURE);
        }
        else
            printf("    overrun count = %d\n", or);
       }

void timer_handler (int sig, siginfo_t *info, void *rien){
    printf("Caught signal %d\n", sig);
    print_siginfo(info);
}

int main () {
    struct sigaction sa;
    struct sigevent evt;

    timer_t timerid;

    int * myPtr;
    /* Install timer_handler as the signal handler for SIGVTALRM. */
    memset (&sa, 0, sizeof (sa));
    sa.sa_flags=SA_SIGINFO;
    sa.sa_sigaction = &timer_handler;
    sigaction (SIGRTMIN, &sa, NULL);

    /* sigevent */
    (*myPtr) = 5;
    evt.sigev_value.sival_ptr=myPtr;
    evt.sigev_notify = SIGEV_SIGNAL;
    evt.sigev_signo = SIGRTMIN;

    /* timer_t */

    if(timer_create(CLOCK_MONOTONIC,&evt,&timerid)==-1) {
        perror("Erreur positionnement ");
        exit(EXIT_FAILURE);
    }

    /* Do busy work. */
    while (1);
}