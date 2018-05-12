#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"


static void
__fork_procs(struct tree_node *root, int level, int exit_no)
{
	/*
	 * Start creating child processes.
	 * Initial process is (*root).name.
	 */

	 printf("PID = %ld, name %s, starting...\n",
 			(long)getpid(), root->name);
 	change_pname(root->name);

	/*
	 *Forking recursively to next
	 *child process in DFS order 
	 */
 
	/*saving pids of immediate chilren in an array*/
	pid_t pid[root->nr_children];
	int i, status;

	for (i=0; i < root->nr_children; i++) {
		pid[i] = fork();
		if (pid[i] < 0) {
			perror("fork_procs: fork");
			exit(1);
		} else if (pid[i] == 0) {
			__fork_procs(root->children + i, level + 1, i);
		}
	}

	/*wait for children to stop*/
	wait_for_ready_children(root->nr_children);
	
        /*
         *Suspend self.
         */
        raise(SIGSTOP);
	/*Process woken up*/
        printf("PID = %ld, name = %s is awake\n",
                (long)getpid(), root->name);

	/*Wake up children, in DFS order*/
	printf("%s: waiting for %d children...\n",
		root->name, root->nr_children);
	
	for (i = 0; i < root->nr_children; i++) {
		/*wake up immidiate child*/
		kill(pid[i],SIGCONT); 
		
		/*wait for it to finish*/
		wait(&status);
        	explain_wait_status(pid[i], status);		
	}

	/*exit*/	
	printf("PID = %ld, name %s, exiting...\n",
                        (long)getpid(), root->name);
	
	/*
	 *creating a semi-unique exit number for
	 *each proccess considering
	 *exit() outputs (exit_no)mod256
	 */
	exit_no += level*(10);
	exit(exit_no);
}


/*
 *the fuction we call to use the above,
 *level = 0, cuz we start from the root
 */
void
fork_procs(struct tree_node *root)
{
	__fork_procs(root, 0, 1);
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

int main(int argc, char *argv[])
{
	pid_t pid;
	int status;
	struct tree_node *root;

	if (argc < 2){
		fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
		exit(1);
	}

	/* Read tree into memory */
	root = get_tree_from_file(argv[1]);

	/* Fork root of process tree */
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		/* Child */
		fork_procs(root);
		exit(1);
	}

	/*
	 * Father
	 */
	/* for ask2-signals */
	wait_for_ready_children(1);

	/* Print the process tree root at pid */
	show_pstree(pid);

	/* for ask2-signals */	
	kill(pid, SIGCONT);

	/* Wait for the root of the process tree to terminate */
	wait(&status);
	explain_wait_status(pid, status);

	return 0;
}
