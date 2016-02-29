/*
 * lcreate.c
 *
 *  Created on: 24 Apr 2015
 *      Author: Ananth
 */

#include <lock.h>

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */

int lcreate(void)
{
	STATWORD ps;
	int	lock,ldes;

#ifdef DEBUGB
kprintf("\nInside lcreate Nextlock: %d\n",nextlock);
#endif DEBUGB

	disable(ps);
	if ( (ldes=newlock())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	lock=GET_LOCK(ldes);

	semaph[lock].semcnt=1;  //minimum positive value
	semaph[lock].islock=1; //is a lock
	semaph[lock].nowrt=0;
	semaph[lock].nord=0;
	semaph[lock].nordu=0;
	semaph[lock].hpwrt=-1;  //negative value because all lprios are +ve
	semaph[lock].hprd=-1;  //negative value because all lprios are +ve
	/* sqhead and sqtail were initialized at system startup */
	restore(ps);
	return(ldes);
}

/*------------------------------------------------------------------------
 * newlock  --  allocate an unused lock and return its index
 *------------------------------------------------------------------------
 */
LOCAL int newlock()
{
	int	lock,ldes;
	int	i;
#ifdef DEBUGE
kprintf("\nInside newlock Nextlock: %d\n",nextlock);
#endif DEBUGE

	for (i=NSEM ; i<NSEM+NLOCKS ; i++) {
		lock=nextlock--;
		if (nextlock < NSEM)
			nextlock = NSEM+NLOCKS-1;
		if (semaph[lock].sstate==SFREE) {
			semaph[lock].sstate = SUSED;
			semaph[lock].lversion++;
			ldes=(semaph[lock].lversion<<8)|(lock);
#ifdef DEBUGE
kprintf("\nInside newlock, ldes: %04x, lock=%02x\n",ldes,lock);
#endif DEBUGE
			return(ldes);
		}
	}
	return(SYSERR);
}
