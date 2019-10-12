#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#include "simulator.h"

#define PERIOD_USEC (10000)

// wire[1] = Tx  de 1 et Rx  de 2
// wire[2] = Rx  de 1 et Tx  de 2
// wire[3] = RTS de 1 et CTS de 2
// wire[4] = CTS de 1 et RTS de 2
static unsigned char wire[4]={1,1,1,1};

static struct simulator simulator1=
{
	.broche_Tx=&wire[0],
	.broche_Rx=&wire[1],
	.broche_RTS=&wire[2],
	.broche_CTS=&wire[3],
	.Tx=1,
	.RTS=1
};

static struct simulator simulator2=
{
	.broche_Tx=&wire[1],
	.broche_Rx=&wire[0],
	.broche_RTS=&wire[3],
	.broche_CTS=&wire[2],
	.Tx=1,
	.RTS=1
}; 

void signal_handler(int sigsum){
	timer_interruption(&simulator1);
	//printf("RTS=%d;CTS=%d\tstate1=%d;state2=%d\n;index=%d\n",simulator1.RTS,simulator1.CTS,simulator1.state_emission,simulator2.state_reception,simulator1.index);
	timer_interruption(&simulator2);
}

int main ()
{
	struct sigaction sa;
	struct itimerval timer;

	pthread_t thread_simu1;
	pthread_t thread_simu2;

	/* Install timer_handler as the signal handler for SIGVTALRM. */
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &signal_handler;
	sigaction (SIGVTALRM, &sa, NULL);

	/* Configure the timer to expire after 250 msec... */
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = PERIOD_USEC;

	/* ... and every 250 msec after that. */
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = PERIOD_USEC;
	
	/* Start a virtual timer. It counts down whenever this process is
	executing. */
	setitimer (ITIMER_VIRTUAL, &timer, NULL);

	pthread_create(&thread_simu2,NULL,main_simulator2,&simulator2);
	pthread_create(&thread_simu1,NULL,main_simulator1,&simulator1);

	pthread_join(thread_simu1,NULL);
	pthread_join(thread_simu2,NULL);

	return 0;
}