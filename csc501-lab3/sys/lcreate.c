#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

int lcreate(void)
{
		STATWORD ps;
        int lock;
		int i = 0;
		int j = 0;
		int lock_value;

        disable(ps);
		
		for (i=0 ; i<NLOCKS ; i++) 
		{
        	lock = nextlock--;
            if (nextlock < 0)
            	nextlock = NLOCKS-1;
            if (locktab[lock].lstate==LFREE) 
			{
            	locktab[lock].lstate = LUSED;
				locktab[lock].nr = 0;
				locktab[lock].nw = 0;
				locktab[lock].touched++;
				if(locktab[lock].touched >= 10){
					locktab[lock].touched = 0;
				}
				for(j = 0; j < NPROC; j++) 
					proctab[j].owned[lock] = 0;
				restore(ps);
                return lock*10+locktab[lock].touched;
            }
        }
		
        restore(ps);
        return(SYSERR);
}
