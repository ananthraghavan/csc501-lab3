/*
 * releaseall.c
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

int lreschedyes;

int releaseall (int numlocks, int args, ...)
{

	int *a;
	int ldes[numlocks];
	int i,r;
	r=0;
	STATWORD ps;

	disable(ps);
	lreschedyes=0;
	/*Getting args from stack*/
	a=(int *)(&args);
#ifdef DEBUGA
	//kprintf("\nArgs are:\n");
#endif
	for (i=0;i<numlocks;i++)
	{
#ifdef DEBUGD
		kprintf("Release all called by pid:%d: for lock number%d  \n",getpid(),*(a+i));
#endif
		ldes[i]=*(a+i);
		if (release(ldes[i],getpid())==SYSERR)
			r=SYSERR;

	}
	if (lreschedyes)
		resched();
	if (r==SYSERR){
		restore(ps);
		return SYSERR;
	}


	restore(ps);
	return OK;
}

int release(int ldes1, int pid)
{

	register struct	sentry	*sptr;
	struct	pentry	*pptr;
	STATWORD ps;
	//unsigned long ctime;
	int tail,prev,ldes,ver;
#ifdef DEBUGF
		kprintf("\nInside release\n");
#endif

	disable (ps);
	ldes=GET_LOCK(ldes1);
	ver=GET_VER(ldes1);
#ifdef DEBUGF
	kprintf("\nLDES:%04x, VER:%d, LOCK:%d\n",ldes1,ver,ldes);
#endif



	if (isbadlock(ldes) || (sptr= &semaph[ldes])->sstate==SFREE) {
#ifdef DEBUGF
		kprintf("\npid %d has found an error, ldes=%d, sptr->sstate= %d\n",pid,ldes,sptr->sstate);
#endif
			restore(ps);
			return(SYSERR);
		}
	if (ver!=sptr->lversion)
	{
#ifdef DEBUGF
	kprintf("\nVersion error\n");
#endif
			restore(ps);
			return(SYSERR);
		}
	if (fpidinlock(pid,ldes)==0)
	{
		restore(ps);
		return(SYSERR);
	}
#ifdef DEBUGF
		kprintf("\nPast error check sptr->semcnt:%d\n",sptr->semcnt);
#endif
		proctab[pid].lnosheld--;
	if ((sptr->semcnt++) < 0)
	{
#ifdef DEBUGD
		kprintf("\nInside if,sptr->lstate:%d, sptr->nordu:%d \n",sptr->lstate,sptr->nordu);
#endif
		if ( ((sptr->lstate==READ) && (--sptr->nordu==0)) || (sptr->lstate==WRITE))
		{
#ifdef DEBUGC
		kprintf("\nInside second if sptr->hprd:%d sptr->hpwrt:%d\n",sptr->hprd,sptr->hpwrt);
#endif
			if (sptr->hprd>sptr->hpwrt)
			{
				//release all readers with hprd
#ifdef DEBUGC
		kprintf("\nReleasing readers\n");
#endif
				tail=sptr->sqtail;
				prev=q[tail].qprev;

				while (q[prev].qkey==sptr->hprd)
				{
					pptr=&proctab[prev];
					if (pptr->lstate==READ)
					{
						dequeue(prev);
						ready(prev,RESCHNO);
						lreschedyes=1;
					}
					prev=q[prev].qprev;

				}
				sptr->hprd=hpp(ldes,READ);
				//resched();
				restore(ps);
				return (OK);
			}
			if (sptr->hpwrt>sptr->hprd)
			{
#ifdef DEBUGD
		kprintf("\nReleasing writer\n");
#endif
				//release hpwrt
				tail=sptr->sqtail;
				prev=q[tail].qprev;
				while (q[prev].qkey==sptr->hpwrt)
				{
					pptr=&proctab[prev];
					if (pptr->lstate==WRITE)
					{
						dequeue(prev);
						sptr->hpwrt=hpp(ldes,WRITE);
#ifdef DEBUGD
		kprintf("\nReleasing writer pid%d for lock %d, new hpwrt= %d\n",prev,ldes,sptr->hpwrt);
#endif
						ready(prev,RESCHNO);
						lreschedyes=1;
						restore(ps);
						return (OK);
					}
					prev=q[prev].qprev;

				}


			}

			if (sptr->hprd==sptr->hpwrt)
			{

				if (sptr->wtime-sptr->rtime>1000)
				{
#ifdef DEBUGC
		kprintf("\nReleasing readers priority equal\n");
#endif
					//release all readers with hprd
					tail=sptr->sqtail;
					prev=q[tail].qprev;

					while (q[prev].qkey==sptr->hprd)
					{
						pptr=&proctab[prev];
						if (pptr->lstate==READ)
						{
							dequeue(prev);
							ready(prev,RESCHNO);
							lreschedyes=1;
						}
						prev=q[prev].qprev;

					}
					sptr->hprd=hpp(ldes,READ);
					//resched();
					restore(ps);
					return (OK);

				}
				else
				{
					//release hpwrt
					tail=sptr->sqtail;
					prev=q[tail].qprev;
					while (q[prev].qkey==sptr->hprd)
					{
						pptr=&proctab[prev];
						if (pptr->lstate==WRITE)
						{
							dequeue(prev);
							sptr->hpwrt=hpp(ldes,WRITE);
							ready(prev,RESCHNO);
							lreschedyes=1;
							restore(ps);
							return (OK);
						}
						prev=q[prev].qprev;

					}


				}
			}
		}
	}
	restore(ps);
	return OK;

}

int fpidinlock (int pid,int ldes)
{

		struct	pentry	*pptr;
		int bit_s;
		int r=0;
		//unsigned long ctime;
		pptr=&proctab[pid];

#ifdef DEBUGD
		kprintf("\nInside find function ldes=%d, pid=%d, lheld50=%d\n",ldes,pid,pptr->lheld50);
#endif
		if (ldes-100>31)
		{
			bit_s=ldes-132;

			if (r=CHECK_BIT(pptr->lheld50,bit_s))
				CLEAR(pptr->lheld50,bit_s);
			return (r);
		}
		else
		{
			bit_s=ldes-100;

			if  (r=CHECK_BIT(pptr->lheld32,bit_s))
					CLEAR(pptr->lheld32,bit_s);
			return(r);
		}



		//should not come here
		return 0;
}

int hpp(int ldes,int type)
{
	struct	sentry	*sptr;
	int tail;
	int prev;
	struct	pentry	*pptr;

	sptr=&semaph[ldes];
	tail=sptr->sqtail;

	prev=q[tail].qprev;
	pptr=&proctab[prev];
	while (q[prev].qkey>-1)
	{
		if(pptr->lstate==type)
			{
			return q[prev].qkey;
			}
		prev=q[prev].qprev;
	}

	//means empty list
	return -1;


}
