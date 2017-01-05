#define _GNU_SOURCE

#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "timer.h"

void * global_param = NULL;

// Return number of elapsed µsec since... a long time ago
static unsigned long get_time (void)
{
    struct timeval tv;

    gettimeofday (&tv ,NULL);

    // Only count seconds since beginning of 2016 (not jan 1st, 1970)
    tv.tv_sec -= 3600UL * 24 * 365 * 46;

    return tv.tv_sec * 1000000UL + tv.tv_usec;
}

void handler(int signal){
    printf("sdl_push_event(%p) appelée au temps %lu\n", global_param, get_time());
}

void * routine(void * arg){
    struct sigaction sa;
    sigset_t mask;

    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    while(1){
        sigprocmask(0, NULL, &mask);
        sigdelset(&mask, SIGALRM);
        sigsuspend(&mask);
    }
}

#ifdef PADAWAN

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
  pthread_t tid;
  pthread_create(&tid, NULL, routine, NULL);
  return 1; // Implementation not ready
}

void timer_set (Uint32 delay, void *param)
{
  global_param = param;

  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = delay * 1000;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;

  setitimer(ITIMER_REAL, &timer, NULL);
}

#endif
