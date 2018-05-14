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

        printf("PID = %ld, name %s, starting...\n",
                        (long)getpid(), root->name);
        change_pname(root->name);

	/*
	 *Forking recursively to next
	 * child process in DFS order
	 */

	pid_t pid;
	int i, status;
	double val, ans[2];
	
	for (i=0; i < root->nr_children; i++) {
		pid = fork();
		if (pid < 0) {
			perror("fork_procs: fork");
			exit(1);
		} else if (pid == 0) {
			__fork_procs(root->children + i, level + 1, i);
		}
	}

	//Force root to wait for children to finish
	printf("%s: waiting for %d children's values...\n",
			root->name, root->nr_children);

	/*
	 *root has to wait for nr_children
	 * if nr_children = 0, it's a leaf,
	 * doesn't get in the for loop
	 */
	for (i=0; i < root->nr_children; i++){
		
		/* Read value from the pipe */
		if (read(pfd[0], &val, sizeof(val)) != sizeof(val)) {
			perror("parent: read from pipe");
			exit(1);
		}
		ans[i]= val;
		/*Now wait for child to exit*/
		pid = wait(&status);
		explain_wait_status(pid, status);
	}
	
	/*
	 *compute val for parent,
	 *checked by the root name
	 *currently works for multiplication
	 *and addition, could be scaled
	 */
	if ((root->name[0])  == '+') {
		val = ans[0] + ans[1];
	} else if ((root->name[0]) == '*') {
		val = ans[0] * ans[1];
	} else 	sscanf(root->name, "%lf", &val);
	
	
	/*write val into pipe for parent*/
	if (write(pdf[1], &val, sizeof(val)) != sizeof(val)) {
		perror("child: write to pipe");
		exit(1);
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
	/*check for input, get tree and print it*/
	pid_t pid;
	int status;
	double ans;
	struct tree_node *root;

        if (argc != 2) {
                fprintf(stderr, "Usage: %s <input_tree_file>\n\n",
				 		argv[0]);
                exit(1);
        }

	/* Read tree into memory*/
	root = get_tree_from_file(argv[1]);
	print_tree(root);

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
	
	/* Read value from the pipe */
        if (read(pfd[0], &ans, sizeof(ans)) != sizeof(ans)) {
                perror("parent: read from pipe");
                exit(1);
	}
	
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
