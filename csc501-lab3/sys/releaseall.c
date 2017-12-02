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
	int index;
	int wait_diff;
	//locktab[lock_index];
	curr = q[locktab[lock_index].lqhead].qnext;

    if(curr == locktab[lock_index].lqtail){
		// the queue is actually empty
		return SYSERR;
	}
	
	/////////////////////////////////////////////////////////////////
	//     Now find the index with highest prority     //////////////
	/////////////////////////////////////////////////////////////////
	int last = q[locktab[lock_index].lqtail].qprev; // last
	int oneb4last = q[last].qprev; // one before last
	
	if(q[last].qprev == locktab[lock_index].lqhead){
		// means this queue only has one element
		// so just return this element
		return last;
	}
	if(q[oneb4last].qprev == locktab[lock_index].lqhead){
		// means this queue only has two elements
		return last;
	}
	
	// now, this queue has three or more elements
	while(q[oneb4last].qprev != lptr -> lqhead)
	{
		oneb4last = q[oneb4last].qprev;
		
		if(q[oneb4last].qkey < q[last].qkey) 
		{
			return last;
		}

		if(q[oneb4last].qkey == q[last].qkey)
		{
			wait_diff = abs(q[last].qtime - q[oneb4last].qtime);
			if(wait_diff < 1)
			{
				if((q[last].qtype == READ) && (q[oneb4last].qtype == WRITE))
				{
					last = oneb4last;
				}
				else
				{
					last = last;
				}
			}

			if(q[last].qtime > q[oneb4last].qtime) 
			{
				last = oneb4last;
			}

			if(q[last].qtime < q[oneb4last].qtime) 
			{
				last = last;
			}
		}
	}
	return last;
	
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
