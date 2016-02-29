/*
 * ldelete.c
 *
 *  Created on: 25 Apr 2015
 *      Author: Ananth
 */

#include <lock.h>

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

int ldelete(int ldes1)
{
	STATWORD ps;
	int	pid,lck;
	struct	sentry	*sptr;

	disable(ps);
	lck=GET_LOCK(ldes1);
	if ((isbadlock(lck)) || (semaph[lck].sstate==SFREE)) {
		restore(ps);
		return(SYSERR);
	}
	sptr = &semaph[lck];
	sptr->sstate = SFREE;
	if (nonempty(sptr->sqhead)) {
		while( (pid=getfirst(sptr->sqhead)) != EMPTY)
		  {
		    proctab[pid].pwaitret = DELETED;
		    ready(pid,RESCHNO);
		  }
		resched();
	}
	restore(ps);
	return(OK);
}

