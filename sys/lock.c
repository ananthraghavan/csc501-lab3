/*
 * lock.c
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


int lock(int ldes, int type, int priority)
{
	STATWORD ps;
	struct	sentry	*sptr;
	struct	pentry	*pptr;
	int bit_s,ldes1,ver;
#ifdef DEBUGF
kprintf("\nCalled lock\n");
#endif
	disable(ps);
	ldes1=GET_LOCK(ldes);
	ver=GET_VER(ldes);
#ifdef DEBUGF
	kprintf("\nLDES:%04x, VER:%d, LOCK:%d\n",ldes,ver,ldes1);
#endif
	if (isbadlock(ldes1) || (sptr= &semaph[ldes1])->sstate==SFREE || (type>2) || (type<1)
			|| (priority<0)){
#ifdef DEBUGF
	kprintf("\nInside first error\n");
#endif

		restore(ps);
		return(SYSERR);
	}

	if (ver!=sptr->lversion)
	{
		restore(ps);
#ifdef DEBUGG
	kprintf("\nVersion error\n");
#endif
		return(SYSERR);

	}
	//semcnt is negative if the lock is held by another process
	if (--(sptr->semcnt) < 0) {
#ifdef DEBUGB
	kprintf("\nInside first if\n");
#endif

		pptr = &proctab[currpid];
		pptr->ltime=ctr1000;

		if (type==READ)
		{
			if ((sptr->lstate==READ) && (priority>sptr->hpwrt)){
				sptr->nordu++;   /*no of readers using */
				pptr->lnosheld++;
				pptr->ltime=MAXINT;
				if (ldes1-100>31)
				{
					bit_s=ldes1-132;
					SET(pptr->lheld50,bit_s);
				}
				else
				{
					bit_s=ldes1-100;
					SET(pptr->lheld32,bit_s);
				}

				restore(ps);
				return(OK);
			}
			sptr->nord++;
			if (priority > sptr->hprd){
				sptr->hprd=(priority>sptr->hprd)?priority:sptr->hprd;
				sptr->rtime=pptr->ltime;
			}
		}

		if (type==WRITE)
		{
#ifdef DEBUGB
	kprintf("\nInside write if\n");
#endif
			sptr->nowrt++;
			if (priority > sptr->hpwrt){
				sptr->hpwrt=(priority>sptr->hpwrt)?priority:sptr->hpwrt;
#ifdef DEBUGE
	kprintf("\nPid: %d, Lock: %d, Priority:%d, hpwrt:%d\n");
#endif
				sptr->wtime=pptr->ltime;
			}

		}
#ifdef DEBUGB
	kprintf("\nOutside write if\n");
#endif
	/* wait on lock*/
		pptr->pstate = PRLOCK;
		pptr->plock = ldes1;
		pptr->lstate=type;
		pptr->lprio=priority;

#ifdef DEBUGD
	kprintf("\nBefore insert calling PLQue, pid:%d\n",currpid);
	plque();
#endif

		insert(currpid,sptr->sqhead,priority);

#ifdef DEBUGD
	kprintf("\nAfter insert calling PLQue, pid:%d\n",currpid);
	plque();
#endif
#ifdef DEBUGB
	kprintf("\nReturn from insert\n");
#endif
		pptr->pwaitret = OK;
		resched();
#ifdef DEBUGC
	kprintf("\nReturn from resched\n");
#endif
		//acquire lock
	if (pptr->pwaitret==OK){
		pptr=&proctab[currpid];
		if (ldes1-100>31)
		{
			bit_s=ldes1-132;
			SET(pptr->lheld50,bit_s);
		}
		else
		{
			bit_s=ldes1-100;
			SET(pptr->lheld32,bit_s);
		}
		sptr->lstate=type;
		pptr->lnosheld++;
		if (type==READ)
			sptr->nordu++;
	}
	restore(ps);
	return pptr->pwaitret;
	}

	//if semcount is nonnegative
	else
	{
		pptr=&proctab[currpid];
#ifdef DEBUGF
		kprintf("\nInside else ldes=%d, pid=%d, lheld32=%d\n",ldes1,currpid,pptr->lheld32);
#endif
		if (ldes1-100>31)
		{
			bit_s=ldes1-132;
			SET(pptr->lheld50,bit_s);
		}
		else
		{
			bit_s=ldes1-100;
			SET(pptr->lheld32,bit_s);
		}
		sptr->lstate=type;
		pptr->lnosheld++;
		if (type==READ)
			sptr->nordu++;
#ifdef DEBUGF
		kprintf("\nExiting else ldes=%d, pid=%d, lheld32=%d\n",ldes1,currpid,pptr->lheld32);
#endif

	}
	restore(ps);
	return(OK);
}

void plque(ldes)
{
	struct	sentry	*sptr;
	int tail;
	int prev;


	struct	pentry	*pptr;

	sptr=&semaph[ldes];
	tail=sptr->sqtail;

	prev=q[tail].qprev;
	pptr=&proctab[prev];
	kprintf("\nPLQue called by process PID:%d\n",getpid());
	kprintf("\nPLQue: tail=%d prev=%d tailkey:%d prevkey:%d\n",tail,prev,q[tail].qkey,q[prev].qkey);
	kprintf("\nPLQue: q[prev].qkey>MININT :%d\n",q[prev].qkey>-1);
	while (q[prev].qkey>-1)
		{
			kprintf("\nLock:%d PID:%d WaitPrio:%d LState:%d",ldes,prev,q[prev].qkey,pptr->lstate);
			prev=q[prev].qprev;
		}
}


