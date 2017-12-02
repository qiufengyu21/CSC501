#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

int ldelete(int ldes)
{
        STATWORD ps;
        int     pid, i;
        struct  lentry  *lptr;
		int lock;
		int num_touched;
		
        disable(ps);
		lock = ldes/10;
		num_touched = ldes - lock*10;
		lptr=&locktab[lock];
		
        if (lptr->lstate==LFREE) 
			{
            restore(ps);
            return(SYSERR);
        	}
		
		if(lptr->touched != num_touched)
			{
            restore(ps);
            return(SYSERR);
        	}

        lptr->lstate = LFREE;
        if(nonempty(lptr->lqhead)) 
			{
            while( (pid=getfirst(lptr->lqhead)) != EMPTY)
                {
                proctab[pid].plockret = DELETED;
                ready(pid,RESCHNO);
                }
                resched();
        	}
		lptr->nr = 0;
		lptr->nw = 0;

		for(i = 0; i < NPROC; i++)
			proctab[i].owned[lock] = 0;

        restore(ps);
        return(OK);
}
