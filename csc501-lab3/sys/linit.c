#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

struct lentry locktab[NLOCKS];
int nextlock;

void linit()
{

	int i = 0;
	int j = 0;
	nextlock = NLOCKS-1;

	for(i = 0; i < NLOCKS; i++)
	{
		locktab[i].lstate = LFREE;
		locktab[i].lid = i;
		locktab[i].lqtail = 1 + (locktab[i].lqhead = newqueue());
		locktab[i].nr = 0;
		locktab[i].nw = 0;
		locktab[i].touched = 0;
	}

	for(i = 0; i < NPROC; i ++){
		for(j = 0; j < NLOCKS; j ++){
			proctab[i].owned[j] = 0;
		}
	}

}
