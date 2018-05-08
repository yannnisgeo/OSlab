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


static void 
__fork_procs(struct tree_node *root, int level, int exit_no)
{
	/*
	 * initial process is (*root).name.
	 */
	
	change_pname(root->name);
	printf("%s: Initiating...\n", root->name);

	/* 
	 *Forking recursively to next 
	 * child process in DFS order 
	 */
	
	
	pid_t pid;
	int i, status;
	
	for (i=0; i < root->nr_children; i++) {	
		pid = fork();
		if (pid < 0) {
			perror("fork_procs: fork");
			exit(1);
		} else if (pid == 0) {
			__fork_procs(root->children + i, level + 1, i);
		}
	}
	
	if (root->nr_children > 0) {	
		//Force root to wait for children to finish
		printf("%s: waiting for %d children...\n", root->name, root->nr_children);
	
		//root has to wait for nr_children
		for (i=0; i < root->nr_children; i++){
			pid = wait(&status);
       			explain_wait_status(pid, status);
		}
	} else if (root->nr_children == 0) {
		//if root has no children, it's a leaf
		printf("%s: Sleeping...\n", root->name);		
		sleep(SLEEP_PROC_SEC);
	}	
	printf("%s: Exiting...\n", root->name);
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
		fork_procs(root);
		exit(1);
	}

	/*
	 * Father
	 */

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
