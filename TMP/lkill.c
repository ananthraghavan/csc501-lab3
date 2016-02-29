/*
 * lkill.c
 *
 *  Created on: 26 Apr 2015
 *      Author: Ananth
 */

// handle locks acquired by process on death of process


#include <lock.h>

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

int lkill(int pid)
{
	STATWORD ps;
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	struct	sentry	*sptr;
	int i=0;
	int j=NSEM+NLOCKS-1;
	int bit_s;
	int ldes[50];
	int local_lnosheld;

	disable(ps);
	pptr=&proctab[pid];
#ifdef DEBUGH
	kprintf("\n Inside Lkill LNOSHELD:%d",pptr->lnosheld);
#endif DEBUGH



	while (i<pptr->lnosheld)
	{
		if (j-100>31)
		{
			bit_s=j-132;

			if (CHECK_BIT(pptr->lheld50,bit_s))
			{
				ldes[i]=(semaph[j].lversion<<8)|(j);
				i++;

			}
		}
		else
		{
			bit_s=j-100;

			if  (CHECK_BIT(pptr->lheld32,bit_s)){
				ldes[i]=(semaph[j].lversion<<8)|(j);
				i++;

			}
		}
		j--;
	}
#ifdef DEBUGH
	kprintf("\nLocks held by PID:%d are:\n",getpid());
	kprintf("\n before for LNOSHELD:%d",pptr->lnosheld);
#endif DEBUGH
	local_lnosheld=pptr->lnosheld;
	for (i=0;i<local_lnosheld;i++)
	{
#ifdef DEBUGH
		kprintf("\nLock Des:%04x\n",ldes[i]);
#endif DEBUGH
		release(ldes[i],pid);
	}

	restore(ps);
	return OK;


}
