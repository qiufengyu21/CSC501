/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>
#include <math.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
* resched  --  reschedule processor to highest priority ready process
*
* Notes:	Upon entry, currpid gives current process id.
*		Proctab[currpid].pstate gives correct NEXT state for
*			current process if other than PRREADY.
*------------------------------------------------------------------------
*/
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	
	if(schedclass == EXPDISTSCHED){ // EXP
		int exp_prio = (int)expdev(0.1);
		//int exp_proc = exp_get(exp_prio);
		
		optr= &proctab[currpid];
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}
		nptr = &proctab[ (currpid = get_exp(exp_prio)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	}
	else if(schedclass == LINUXSCHED){ // Linux-like
		optr = &proctab[currpid];
		if(preempt <= 0){
			//this process used all CPU time.
			optr->counter = 0;
			optr->goodness = 0;
		}
		else{
			//this process has some CPU time left.
			optr->goodness = optr->goodness - optr->counter + preempt;
			optr->counter = preempt;
		}

		if(currpid == NULLPROC){
			optr->counter = 0;
			optr->goodness = 0;
		}

		

		// traverse the ready queue and find the highest goodness value
		int max_counter = 0;
		int max_goodness = 0;
		int next = q[rdyhead].qnext;
		int prev = q[rdytail].qprev;
		while(next != rdytail){
			
			if(proctab[next].counter <=0){ //update goodness
				proctab[next].goodness = 0;
			}
			else{ // update goodness
				proctab[next].goodness = proctab[next].counter + proctab[next].pprio;
			}
			
			if(proctab[next].goodness > max_goodness){
				max_goodness = proctab[next].goodness;
				next = q[next].qnext;
			}else{
				next = q[next].qnext;
			}
		}
		
		if(max_goodness < proctab[currpid].goodness){
			max_goodness = proctab[currpid].goodness;
		}

		next = q[rdyhead].qnext;
		while(next != rdytail){
			if(proctab[next].counter > max_counter){
				max_counter = proctab[next].counter;
				next = q[next].qnext;
			}
			else{
				next = q[next].qnext;
			}
		}
		
		if(max_counter < proctab[currpid].counter){
			max_counter = proctab[currpid].counter;
		}

		
		// find max goodness, max counter and next runnable pid
		if(max_goodness <=0){ // this means that no runnable processes in the queue;
			if(optr->counter == 0 || optr->pstate != PRCURR){
				// start a new epoch
				int pindex = 0;
				for(pindex = 0; pindex < NPROC; pindex++){
					if(proctab[pindex].counter <=0 || proctab[pindex].counter == proctab[pindex].quantum){
						proctab[pindex].quantum = proctab[pindex].pprio;
						proctab[pindex].goodness = proctab[pindex].counter + proctab[pindex].pprio;
						proctab[pindex].counter = proctab[pindex].quantum;
					}
					else{
						proctab[pindex].quantum = (int)(proctab[pindex].counter/2) + proctab[pindex].pprio;
						proctab[pindex].goodness = proctab[pindex].counter + proctab[pindex].pprio;
						proctab[pindex].counter = proctab[pindex].quantum;
					}
				}
				//preempt = optr->counter;
			}
			
		}
		
		// current process still has left quantum, so no need to start a new epoch
		// now find the highest goodness value in the proc table
		int proc = 0;
		int max = 0;
		int next_pid = 0;
		
		int next2  = q[rdyhead].qnext;
		while(next2 != rdytail){
			if(proctab[next2].goodness > max){
				max = proctab[next2].goodness;
				next_pid = next2;
				next2 = q[next2].qnext;
			}else{
				next2 = q[next2].qnext;
			}
			
		}
		if(proctab[currpid].goodness > max){
			max = proctab[currpid].goodness;
			next_pid = currpid;
		}

		if(next_pid == currpid){
			preempt = proctab[next_pid].counter;
			return OK;
		}
		else{
			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid,rdyhead,optr->pprio);
			}

			nptr = &proctab[next_pid];

			currpid = next_pid;
			nptr->pstate = PRCURR;
			
			nptr = &proctab[ (currpid = dequeue(next_pid)) ];

			#ifdef  RTCLOCK
			preempt = nptr->counter;              // reset preemption counter
			#endif

			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
			return(OK);
		}

	}
	else{ // Default Xinu Scheduler
		/* no switch needed if current process priority higher than next*/
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
				(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
		
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	}

	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

	/* The OLD process returns here when resumed. */
	return OK;
}
