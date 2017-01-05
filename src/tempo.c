#define _XOPEN_SOURCE 600

#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "timer.h"
void* global_param;
// Return number of elapsed µsec since... a long time ago
static unsigned long get_time (void)
{
    struct timeval tv;

    gettimeofday (&tv ,NULL);

    // Only count seconds since beginning of 2016 (not jan 1st, 1970)
    tv.tv_sec -= 3600UL * 24 * 365 * 46;

    return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN

void handler_demon(int sig) {
	if(sig == SIGALRM)
		printf ("[%lu]: sdl_push_event(%p) appelée au temps %ld\n", pthread_self(), global_param, get_time ());
  else
    printf("[%lu]: reçu %d\n", pthread_self(), sig);
}

void * demon() {
	//on crée la sigaction pour traiter les signaux
	struct sigaction s;
	s.sa_flags = 0;
	sigemptyset(&s.sa_mask);
	s.sa_handler = handler_demon;
  sigaction(SIGALRM, &s, NULL);
	//on initialise le mask du thread demon
	sigset_t demon_mask;
	//on supprime SIGALARM des signaux bloqués pour le recevoir
	sigdelset(&demon_mask, SIGALRM);

	while(1) {
		timer_set(200,oui);//Test timer_set simple
		sigsuspend(&demon_mask);
	}

}
// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
  //on crée un thread quelconque
  pthread_t thread;
  //on crée un masque
  sigset_t blocked_mask;
  //on peut recevoir tous les signaux
  sigemptyset(&blocked_mask);
  //on ajoute le signal SIGALARM dans les signaux à traiter 
  sigaddset(&blocked_mask, SIGALRM);
  //on bloque le signal SIGALARM pour le thread (impossible de le recevoir)
  sigprocmask(SIG_BLOCK, &blocked_mask, NULL);
  //on crée notre thread demon
  pthread_create(&thread, NULL, demon, NULL);
  
  return 0; // Implementation not ready
}

void timer_set (Uint32 delay, void *param)
{
  global_param = param;// On stocke le paramètre vers sdl_push_event

  struct itimerval timer; // On crée un timer 
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = delay * 1000; // On le paramètre pour envoyer SIGALRM dans delay ms
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;


  setitimer(ITIMER_REAL, &timer, NULL); // On l'enclenche
}

#endif
