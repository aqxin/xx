#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXSIZE 1024

int main(void)
{
	pid_t pid;
	int pipe_fd[2];
	char buf[MAXSIZE];
	FILE *file;

	if (pipe(pipe_fd) < 0) {
		printf("create pipe error.\n");
		return -1;
	}

	if ((pid = fork()) < 0) {
		printf("create child process error.\n");
		return -1;
	}

	if (0 == pid) {
		close(pipe_fd[1]);
		read(pipe_fd[0], buf, MAXSIZE);
		printf("child process, receive your message:\n%s", buf);
		close(pipe_fd[0]);

		if ((file=fopen("1.2_file.txt", "w"))==NULL) {
			printf("open 1.2_file.txt error.\n");
			exit(0);
		}
		else {
			fputs(buf, file);
			fclose(file);
		}
		return 0;
	}
	else {
		close(pipe_fd[0]);
		printf("parent process, please input some message:\n");
		fgets(buf, MAXSIZE-1, stdin);
		write(pipe_fd[1], buf, MAXSIZE);
		close(pipe_fd[1]);
		waitpid(pid, NULL, 0);	
		return 0;
	}
}
