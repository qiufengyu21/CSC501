#ifndef _LOCK_H_
#define _LOCK_H_
#define NLOCKS	50

#define READ	1
#define WRITE	2

#define	LFREE	1 
#define	LUSED	2

#define isbadlock(l) (l<0 || l>=NLOCKS)


struct lentry {
	char	lstate;
	int		lid;
	int		lqhead;
	int 	lqtail;
	int		nr;
	int 	nw;
	int		touched;
};

extern struct lentry locktab[];

extern int nextlock;


void linit();
int lcreate();
int ldelete(int lockdescriptor);
int lock(int ldes1, int type, int priority);
int releaseall(int numlocks, int ldes1, ...);
int find_prio(int lock);
int release(int pid, int ldes);

#endif