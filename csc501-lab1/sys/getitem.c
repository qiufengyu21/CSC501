/* getitem.c - getfirst, getlast */

#include <conf.h>
#include <kernel.h>
#include <q.h>

/*------------------------------------------------------------------------
 * getfirst  --	 remove and return the first process on a list
 *------------------------------------------------------------------------
 */
int getfirst(int head)
{
	int	proc;			/* first process on the list	*/

	if ((proc=q[head].qnext) < NPROC)
		return( dequeue(proc) );
	else
		return(EMPTY);
}



/*------------------------------------------------------------------------
 * getlast  --  remove and return the last process from a list
 *------------------------------------------------------------------------
 */
int getlast(int tail)
{
	int	proc;			/* last process on the list	*/

	if ((proc=q[tail].qprev) < NPROC)
		return( dequeue(proc) );
	else
		return(EMPTY);
}

int get_exp(int priority){
	int next;
	int prev;
	int proc;
	next = q[rdyhead].qnext;
	prev = q[rdytail].qprev;
	
	if(priority < q[next].qkey){
		return (dequeue(next));
	}
	else if(priority >= q[prev].qkey){
		return (dequeue(prev));
	} 
	else{
		while(q[next].qkey <= priority){
			next = q[next].qnext;
		}
		return (dequeue(next));
		
	}
}