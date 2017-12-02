#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>


int find_index(int lock_index)
{
	struct lentry *lptr;
    int curr;
	int temp;
	int wait_diff;
	lptr = &locktab[lock_index];
	curr = q[lptr->lqhead].qnext;

    if(curr == lptr -> lqtail){
		// the queue is actually empty
		return SYSERR;
	}
	
	int item = q[lptr -> lqtail].qprev;
	int best = q[lptr -> lqtail].qprev;

	item = q[best].qprev; 

	while(q[item].qprev != lptr -> lqhead)
	{
		item = q[item].qprev;
		
		if(q[item].qkey < q[best].qkey) 
		{
			return best;
		}

		if(q[item].qkey == q[best].qkey)
		{
			wait_diff = abs(q[best].qtime - q[item].qtime);
			if(wait_diff < 1)
			{
				if((q[best].qtype == READ) && (q[item].qtype == WRITE))
				{
					best = item;
				}
				else
				{
					best = best;
				}
			}

			if(q[best].qtime > q[item].qtime) 
			{
				best = item;
			}

			if(q[best].qtime < q[item].qtime) 
			{
				best = best;
			}
		}
	}
	return best;
} 


int releaseall(int numlocks, int ldes1, ...)
{
	int lock_value;
	int flag = 0;
	unsigned long *a = (unsigned long *)(&ldes1);
    for ( ; numlocks > 0 ; numlocks--)
		{
        lock_value = *a++; 
		if(release(currpid, lock_value) == SYSERR) 
			flag = 1;
		}
		
	resched();
	
	if(flag == 1) 
		return(SYSERR);
	else 
		return(OK);
}

int release(int pid, int lock_value)
{
    STATWORD ps;
    struct lentry  *lptr;
	int best;
	int lock;
	int num_touched;

	disable(ps);
	lock = lock_value/10;
	num_touched = lock_value - lock*10;
	lptr= &locktab[lock];
	
	if (lptr->lstate==LFREE) 
        {
        restore(ps);
        return(SYSERR);
        }
	
	if (lptr->touched != num_touched)
		{
        restore(ps);
        return(SYSERR);
        }
	
	if(proctab[pid].owned[lock] > 0) 
		{
		proctab[pid].owned[lock]--;
		}
	else
		{
		restore(ps);
		return(SYSERR);
		}

	if((lptr->nr > 0) && (lptr->nw == 0)) 
		{
		lptr->nr --;
		}
	
	else if((lptr->nr == 0) && (lptr->nw == 1)) 
		{
		lptr->nw --;
		}
	
	else 
		{
		kprintf("Impossible to get here. Something is wrong!!!\n");
		}
		
	if((lptr->nr == 0) && (lptr->nw == 0))
		{
		best = find_index(lock);
		while(best != -1)
			{
			if(q[best].qtype == READ)
				{
				lptr->nr ++;
				proctab[best].owned[lock] ++;
				dequeue(best);
				ready(best, RESCHNO);
				best = find_index(lock);
				
				if((best != -1) && (q[best].qtype == WRITE))
					{
					best = -1;
					break;
					}
				}
			else if(q[best].qtype == WRITE)
				{
				lptr->nw ++;
				proctab[best].owned[lock] ++;
				dequeue(best);
				ready(best, RESCHNO);
				break;
				}
			}
		}
	restore(ps);
	return OK;
}
