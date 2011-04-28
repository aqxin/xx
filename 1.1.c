#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define KEY1 1234
#define KEY2 5678

int main(void) {
	int shmid1, shmid2;
	pid_t pid1, pid2;
	char *shmaddr;
	shmid1 = shmget(KEY1, SIZE, IPC_CREAT|0600);
	shmid2 = shmget(KEY2, SIZE, IPC_CREAT|0600);

	if (0 == (pid1=fork())) {
		shmaddr = (char*)shmat(shmid1, NULL, 0);
		printf("child 1 process, please input some message:\n");
		fgets(shmaddr, SIZE-1, stdin);
		shmdt(shmaddr);
		return 0;
	}

	waitpid(pid1, NULL, 0);

	if (0 == (pid2=fork())) {
		shmaddr = (char*)shmat(shmid2, NULL, 0);
		printf("child 2 process, please input some message:\n");
		fflush(stdin);
		fgets(shmaddr, SIZE-1, stdin);
		shmdt(shmaddr);
		return 0;
	}

	waitpid(pid2, NULL, 0);

	shmaddr = (char*)shmat(shmid1, NULL, 0);
	printf("child 1 process (PID=%d) message:\n%s", pid1, shmaddr);
	shmdt(shmaddr);
	shmctl(shmid1, IPC_RMID, NULL);

	shmaddr = (char*)shmat(shmid2, NULL, 0);
	printf("child 2 process (PID=%d) message:\n%s", pid2, shmaddr);
	shmdt(shmaddr);
	shmctl(shmid2, IPC_RMID, NULL);

	return 0; 
}
