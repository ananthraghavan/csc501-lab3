/* sem.h - isbadsem */

#ifndef _SEM_H_
#define _SEM_H_

#ifndef	NSEM
#define	NSEM		50	/* number of semaphores, if not defined	*/
#endif

#define	SFREE	'\01'		/* this semaphore is free		*/
#define	SUSED	'\02'		/* this semaphore is used		*/



struct	sentry	{		/* semaphore table entry		*/
	char	sstate;		/* the state SFREE or SUSED		*/
	int	semcnt;		/* count for this semaphore		*/
	int	sqhead;		/* q index of head of list		*/
	int	sqtail;		/* q index of tail of list		*/

	//for locks

	int islock;     /* is it a semaphore or a lock*/
	char lstate;    /*READ WRITE or DELETED*/

	int nordu;     /*no of readers using this lock */


	/* waiting */
	int hprd;    /* priority of highest reader waiting*/
	unsigned long rtime; /*earliest hp reader wait time */
	int nord;    /*no of readers waiting*/

	int hpwrt;    /* priority of highest writer waiting*/
	int nowrt;    /*no of writers waiting*/
	unsigned long wtime; /*earliest hp writer wait time */

	unsigned int lversion;        /*version for lock*/



};
extern	struct	sentry	semaph[];
extern	int	nextsem;

#define	isbadsem(s)	(s<0 || s>=NSEM)


#endif
