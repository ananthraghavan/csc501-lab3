/*
 * lock.h
 *
 *  Created on: 24 Apr 2015
 *      Author: Ananth
 */

#ifndef H_LOCK_H_
#define H_LOCK_H_

#define NLOCKS 50

//#define DELETED 0
#define READ 1
#define WRITE 2

#define	isbadlock(l)	(l<NSEM || l>=NSEM+NLOCKS)

//#define SET(status,ldes)   (status | (1UL<<ldes))
//#define CLEAR(status,ldes) (status & ~(1UL<<ldes))
#define CLEAR(p,n) ((p) &= ~((1UL) << (n)))
#define SET(p,n) ((p) |= ((1UL) << (n)))
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#define GET_VER(var) ((var)>>8)
#define GET_LOCK(var) ((var) & (0xFF))



/* extern variables */
extern int nextlock;
extern unsigned long	ctr1000;



/* functions */

int lock (int , int , int );
int lcreate(void);
int releaseall (int, int, ...);
int release(int ldes, int pid);
int fpidinlock (int pid,int ldes);
int hpp(int ldes,int type); //find highest priority
int lkill(int);     //upon killing of process release held locks
//maintainence functions
void plque(ldes);

int ldelete(int lck);


//#define DEBUGB
//#define DEBUGC
//#define DEBUGG
//#define DEBUGF
//#define DEBUGH
#endif /* H_LOCK_H_ */
