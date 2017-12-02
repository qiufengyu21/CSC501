#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
#include <sleep.h>

int lock(int ldes, int type, int priority)
{
    STATWORD ps;
    struct  lentry  *lptr;
	int item;
	int lock;
	int num_touched;

    disable(ps);

	lock = ldes/10;
	
	num_touched = ldes - lock*10;

	lptr = &locktab[lock];
	
	if(lptr->lstate==LFREE)
		{
        restore(ps);
        return(SYSERR);
    	}
	
	if(num_touched != lptr->touched)
		{
        restore(ps);
        return(SYSERR);
    	}

	if((lptr->nr == 0) && (lptr->nw == 0)) 
		{
		proctab[currpid].owned[lock]++;
		if(type == READ) 
			lptr->nr ++;
		else
			lptr->nw ++;
        restore(ps);
       	return(OK);
		}

	if((lptr->nr == 0) && (lptr->nw == 1)) 
		{
		proctab[currpid].pstate = PRLOCK;
		proctab[currpid].plockret = OK;
		insert(currpid, lptr -> lqhead, priority);
		q[currpid].qtype = type;
		q[currpid].qtime = clktime;
		resched();
		restore(ps);
		return proctab[currpid].plockret;
		}
	
	if((lptr->nr > 0) && (lptr->nw == 0))
		{

		if(type == WRITE) 
			{
			proctab[currpid].pstate = PRLOCK;
			proctab[currpid].plockret = OK;
			insert(currpid, lptr -> lqhead, priority);
			q[currpid].qtype = type;
			q[currpid].qtime = clktime;
			resched();
			restore(ps);
			return proctab[currpid].plockret;
			}
		
		if(type == READ)
			{
			item = q[lptr -> lqtail].qprev;
			while((item != lptr -> lqhead) && (priority < q[item].qkey))
				{
				if(q[item].qtype == WRITE)
					{
					proctab[currpid].pstate = PRLOCK;
					proctab[currpid].plockret = OK;
					insert(currpid, lptr -> lqhead, priority);
					q[currpid].qtype = type;
					q[currpid].qtime = clktime;
					resched();
					restore(ps);
					return proctab[currpid].plockret;
					}
				item = q[item].qprev;
				}
	
			proctab[currpid].owned[lock]++;
			if(type == READ) 
				lptr->nr ++;
			else
				lptr->nw ++;
        	restore(ps);
       		return(OK);
	
			}
		}

}