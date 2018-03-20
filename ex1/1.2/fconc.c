#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void doWrite(int fd, const char *buff, int len){
	ssize_t wcnt;
	size_t idx;
	idx = 0;
	do {
		wcnt = write(fd, buff+idx,len-idx);
		if (wcnt == -1) {	// error when writing in out file
			perror("err at write outfile");
			exit(1);
	
		}
		idx =+ wcnt;
	} while (idx < len);
	
}

	
void write_file(int fd, const char *infile){
	int fdd;
	fdd = open(infile, O_RDONLY);
	if (fdd <0){				// error when opening this infile
		perror(infile);
		exit(1);
	}
	
	char buff[1024];
	ssize_t rcnt;
	/* read a bunch of stuff, save them on buff, and then write them through
	doWrite in the output file*/
	do {					
		rcnt = read(fdd, buff, sizeof(buff)-1);
		if (rcnt == -1) {		// error when reading this infile
			perror("err at infile read");
			exit(1);
		}
		doWrite(fd, buff, rcnt);
	} while (rcnt > 0);
      	if (rcnt == 0)   /*end-of file*/
             //   return 0;
 
	if (close(fdd) < 0){ 			// error when closing this infile
		perror("err at infile close");	
		exit(1);
	}
}


int main(int argc, char **argv)
{
	char* filename;

	if (argc < 3){				// <2 arguments -> Error
		printf("We must construct additional pyl..arguments!\n");
		printf("Usage: ./fconc infile1 infile2 [outfile (default:fconc.out)\n");
		exit(1);
		}				
	else if (argc == 3){			//  2 arguments -> filename = default 
		 filename = "fconc.out"; 
  	}
	else { 					// >2 arguments -> filename = last argument
		 filename = argv[argc-1];
	}		

	int fd, oflags, mode;
	oflags = O_CREAT | O_WRONLY | O_TRUNC;
	mode = S_IRUSR | S_IWUSR;
	fd = open(filename, oflags, mode);	 // Open output file & check errs 
	if (fd <0){
		perror(filename);
		exit(1);
	}
	
	/*now for da good stuff*/
	for (int i = 1; i < argc-1; i++) {
                printf("%s\n", argv[i]);
		write_file(fd, argv[i]);
	}	
			
	if (close(fd) < 0) {			// Close output file & check errs 
		perror("err at output file close");
		exit(1);
	}
	return 0;
}
