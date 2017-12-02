#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
#include <sleep.h>

int lpreempt(int ldes, int type, int priority, int wait){
	int curr = q[locktab[ldes].lqtail].qprev;
	
	if(wait == 1){
		goto WAIT;
	}
	
	LOOP:
	if(curr != locktab[ldes].lqhead){
		if(priority < q[curr].qkey){
			int qtype = q[curr].qtype;
			if(qtype == READ){
				curr = q[curr].qprev;
				goto LOOP;
			}
			else{
				WAIT:
				proctab[currpid].pstate = PRLOCK;
				proctab[currpid].plockret = OK;
				insert(currpid, locktab[ldes].lqhead, priority);
				q[currpid].qtype = type;
				q[currpid].qtime = clktime;
				return 1;
			}
		}
	}
	
	return -1;
}

int lock(int ldes, int type, int priority)
{
	STATWORD ps;
	struct  lentry  *lptr;
	int lock;
	int num_touched;

	disable(ps);

	lock = ldes/10;
	
	num_touched = ldes - lock*10;

	lptr = &locktab[lock];
	
	if (isbadlock(lock) || locktab[lock].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	
	if(num_touched != lptr->touched)
	{
		restore(ps);
		return(SYSERR);
	}

	int nr = lptr->nr;
	int nw = lptr->nw;
	int flag = 0;
	
	// We want a READ lock
	if(type == READ){
		// first check if no one is using the lock
		if(nr == 0 && nw == 0){
			// there is no one using the lock
			// we can assign it to the process
			proctab[currpid].owned[lock]++;
			lptr->nr ++;
			restore(ps);
			return OK;
		}
		
		// then, there are 2 possibilities
		// 1. this lock is a read lock
		// 2. this lock is a write lock
		if(nr != 0 && nw == 0){ // this is a READ lock!
			if(lpreempt(lock, type, priority, 0) == 1){
				resched();
				restore(ps);
				return proctab[currpid].plockret;
			}else{
				;
			}
			
			proctab[currpid].owned[lock]++;
			lptr->nr ++;

        	restore(ps);
       		return(OK);
		}
		
		if(nr == 0 && nw != 0){ // this is a WRITE lock, you wait!
			//proctab[currpid].pstate = PRLOCK;
			//proctab[currpid].plockret = OK;
			//insert(currpid, lptr -> lqhead, priority);
			//q[currpid].qtype = type;
			//q[currpid].qtime = clktime;
			
			if(lpreempt(lock, type, priority, 1) == 1){
				resched();
				restore(ps);
				return proctab[currpid].plockret;
			}
			else{
				kprintf("ERROR!\n");
				restore(ps);
				return SYSERR;
			}
		}
	}
	
	
	// Now, if we want a WRITE lock
	if(type == WRITE){
		// first check if no one is using the lock
		if(nr == 0 && nw == 0){
			// there is no one using the lock
			// we can assign it to the process
			proctab[currpid].owned[lock]++;
			lptr->nw ++;
			restore(ps);
			return OK;
		}
		
		// then, there are 2 possibilities
		// 1. this lock is a read lock
		// 2. this lock is a write lock
		if(nr != 0 && nw == 0){ // this is a READ lock.
			if(lpreempt(lock, type, priority, 1) == 1){
				resched();
				restore(ps);
				return proctab[currpid].plockret;
			}
			else{
				kprintf("ERROR!\n");
				restore(ps);
				return SYSERR;
			}
		}
		
		if(nr == 0 && nw != 0){ // this is a WRITE lock, you wait!
			if(lpreempt(lock, type, priority, 1) == 1){
				resched();
				restore(ps);
				return proctab[currpid].plockret;
			}
			else{
				kprintf("ERROR!\n");
				restore(ps);
				return SYSERR;
			}
		}
	}
}