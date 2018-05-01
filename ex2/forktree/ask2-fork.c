#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

/*
 * Create this process tree:
 * A-+-B---D
 *   `-C
 */

/* Created a function for every fork handle */
void fork_procB(void);
void fork_proc_leaf(char * proc_name, int exit_number);

void fork_procs(void)
{
	/*
	 * initial process is A.
	 */

	change_pname("A");
	printf("A: Initiating...\n");
	//sleep(SLEEP_PROC_SEC);

	/* ... */
        
	/* Forking B process */
	pid_t pB;//, mypid;
	
	pB = fork();
	if (pB < 0) {
		perror("B: fork");
		exit(1);
	} else if (pB == 0) {
		//mypid = getpid();
		fork_procB();
	}
	// Done with B, D. On to C.

	pid_t pC;
	pC = fork();
	if (pC<0) {
		perror("C: fork");
		exit(1);
	} else if (pC == 0) {
		//mypid = getpid();
		fork_proc_leaf("C", 17);
	}
	/* ... */
	
	printf("A: Exiting...\n");
	exit(16);
}

/* proc B will fork D */
void fork_procB(void)
{
	change_pname("B");
        printf("B: Initiating...\n");
        //sleep(SLEEP_PROC_SEC);

       	/* Forking D process */
       	pid_t pD;

       	pD = fork();
       	if (pD < 0) {
               	perror("D: fork");
               	exit(1);
       	} else if (pD == 0) {
            	//mypid = getpid();
               	fork_proc_leaf("D", 13);
       	}

        printf("B: Exiting...\n");
        exit(19);
}

/* proc leaf (C, D) shall sleep for a while, then return */
void fork_proc_leaf(char * proc_name, int exit_number)
{
        change_pname(proc_name);
	printf("%s: Initiating...\n", proc_name);
	printf("%s: Sleeping...\n", proc_name);
        sleep(SLEEP_PROC_SEC);

        printf("%s: Exiting...\n", proc_name);
        exit(exit_number);
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * How to wait for the process tree to be ready?
 * In ask2-{fork, tree}:
 *      wait for a few seconds, hope for the best.
 * In ask2-signals:
 *      use wait_for_ready_children() to wait until
 *      the first process raises SIGSTOP.
 */
int main(void)
{
	pid_t pid;
	int status;

	/* Fork root of process tree */
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		/* Child */
		fork_procs();
		exit(1);
	}

	/*
	 * Father
	 */
	/* for ask2-signals */
	/* wait_for_ready_children(1); */

	/* for ask2-{fork, tree} */
	sleep(SLEEP_TREE_SEC);

	/* Print the process tree root at pid */
	show_pstree(pid);

	/* for ask2-signals */
	/* kill(pid, SIGCONT); */

	/* Wait for the root of the process tree to terminate */
	pid = wait(&status);
	explain_wait_status(pid, status);

	return 0;
}
