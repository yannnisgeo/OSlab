#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#include "tree.h" 

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

/*
 * Create given process tree:
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

	/* ... */
        
	/* Forking B process */
	//pid_t pB;//, mypid;
	pid_t pid;
	int status;
		
	pid = fork();
	if (pid < 0) {
		perror("B: fork");
		exit(1);
	} else if (pid == 0) {
		fork_procB();
	}
	// Done with B, D. On to C.

	//pid_t pC;
	pid = fork();
	if (pid < 0) {
		perror("C: fork");
		exit(1);
	} else if (pid == 0) {
		fork_proc_leaf("C", 17);
	}
	//Force A to wait for children to finish
	printf("A: Waiting for children (B, C) to finish...\n");
	
	pid = wait(&status);
        explain_wait_status(pid, status);
	//A has to wait for 2 children
	pid = wait(&status);
        explain_wait_status(pid, status);
	/* ... */
		
	printf("A: Exiting...\n");
	exit(16);
}

/* proc B will fork D */
void fork_procB(void)
{
	change_pname("B");
        printf("B: Initiating...\n");

       	/* Forking D process */
        //pid_t pD;
	pid_t pid;
	int status;
	
       	pid = fork();
       	if (pid < 0) {
               	perror("D: fork");
               	exit(1);
       	} else if (pid == 0) {
               	fork_proc_leaf("D", 13);
       	}
	
	//Force B to wait for child to finish
	printf("B: Waiting for child (D) to finish...\n");	

	pid = wait(&status);
        explain_wait_status(pid, status);
	
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
int main(int argc,char *argv[])
{
	/*check for input, get tree and print it*/
	struct tree_node *root;

        if (argc != 2) {
                fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
                exit(1);
        }

        root = get_tree_from_file(argv[1]);
        print_tree(root);

	
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
