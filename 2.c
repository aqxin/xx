/*
  要求：一个生产者向一个缓冲区发消息，每当发出一个消息后，要等待三个消费者都接收这条消息后，生产者才能发送新消息。用信号量和P、V操作，写出他们同步工作的程序。（提示：可以把缓冲区看作是三个缓冲块组成的缓冲区，生产者等到这三个缓冲块为空时，才发送新消息到这个缓冲区。每个消费者从一个缓冲块取走数据。）
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXSEM 3

// 对信号量操作时需要的数据结构
union semun { 
	int val; 
	struct semid_ds *buf; 
	unsigned short int *array; 
	struct seminfo *__buf;
}arg;

// 申请三个信号量ID
int full_id;
int empty_id;
int mutx_id;

// 对信号量操作时需要的数据结构
struct sembuf P, V, p[MAXSEM], v[MAXSEM];


int main(int argc, char *argv[])
{
	// 创建虚拟主存
	int *buf = (int*)mmap(NULL, sizeof(int)*MAXSEM, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	int times = MAXSEM;
	if (2 == argc)
		times = atoi(argv[1]);

	// 获得信号量的描述符
	full_id  = semget(IPC_PRIVATE, 3, IPC_CREAT|0666);
	empty_id = semget(IPC_PRIVATE, 3, IPC_CREAT|0666);
	mutx_id  = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);

	// 初始化P，V操作;
	arg.val = 0;
	semctl(full_id, 0, SETVAL, arg);
	semctl(full_id, 1, SETVAL, arg);
	semctl(full_id, 2, SETVAL, arg);
	arg.val = 1;
	semctl(empty_id, 0, SETVAL, arg);
	semctl(empty_id, 1, SETVAL, arg);
	semctl(empty_id, 2, SETVAL, arg);
	semctl(mutx_id, 0, SETVAL, arg);

	p[0].sem_num = 0;
	v[0].sem_num = 0;
	p[1].sem_num = 1;
	v[1].sem_num = 1;
	p[2].sem_num = 2;
	v[2].sem_num = 2;

	p[0].sem_op = -1;
	v[0].sem_op = 1;
	p[1].sem_op = -1;
	v[1].sem_op = 1;
	p[2].sem_op = -1;
	v[2].sem_op = 1;

	p[0].sem_flg = v[0].sem_flg = SEM_UNDO;
	p[1].sem_flg = v[1].sem_flg = SEM_UNDO;
	p[2].sem_flg = v[2].sem_flg = SEM_UNDO;

	P.sem_op = -1;
	V.sem_op = 1;
	P.sem_flg = V.sem_flg = SEM_UNDO;
	P.sem_num = V.sem_num = 0;


//	Init_sem();

	// 消费者1进程
	if (fork() == 0) {
		int i, message;
		printf("child 1 process(PID=%d) is running.\n", getpid());
		for (i=0; i<times; i++) {
			// 申请数据;
			semop(full_id, &p[0], 1);
			// 申请对缓冲区的互斥操作
			semop(mutx_id, &P, 1);
			message = buf[0];
			buf[0] = 0;
			printf("child 1 PID=%d) get message:%d.\n", getpid(), message);
			// 释放对缓冲区的互斥操作
			semop(mutx_id, &V, 1);
			// 释放申请数据
			semop(empty_id, &v[0], 1);
		}
		printf("child 1 process(PID=%d) is over.\n", getpid());	
		exit(0);
	}

	// 消费者2进程
	if (fork() == 0) {
		int i, message;
		printf("child 2 process(PID=%d) is running.\n", getpid());
		for (i=0; i<times; i++) {
			// 申请数据
			semop(full_id, &p[1], 1);
			// 申请对缓冲区的互斥操作
			semop(mutx_id, &P, 1);
			message = buf[1];
			buf[1] = 0;
			printf("child 2 PID=%d) get message:%d.\n", getpid(), message);
			// 释放对缓冲区的互斥操作
			semop(mutx_id, &V, 1);
			// 释放申请数据;
			semop(empty_id, &v[1], 1);
		}
		printf("child 2 process(PID=%d) is over.\n", getpid());	
		exit(0);
	}

	// 消费者3进程
	if (fork() == 0) {
		int i, message;
		printf("child 3 process(PID=%d) is running.\n", getpid());
		for (i=0; i<times; i++) {
			// 申请数据
			semop(full_id, &p[2], 1);
			// 申请对缓冲区的互斥操作
			semop(mutx_id, &P, 1);
			message = buf[2];
			buf[2] = 0;
			printf("child 3 PID=%d) get message:%d.\n", getpid(), message);
			// 释放对缓冲区的互斥操作
			semop(mutx_id, &V, 1);
			// 释放申请数据
			semop(empty_id, &v[2], 1);
		}
		printf("child 3 process(PID=%d) is over.\n", getpid());	
		exit(0);
	}

	// 生产者进程
	int i, j;
	printf("parent process(PID=%d) is running.\n", getpid());
	for (i=0; i<times; i++) {
		// 申请空闲缓冲区3快
		semop(empty_id, p, 3);
		// 申请对缓冲区的互斥操作
		semop(mutx_id, &P, 1);
		// 将数据写入缓冲区
		for (j=0;j<MAXSEM;++j) {
			buf[j] = i+1;
		}
		printf("parent (PID=%d) product message:%d.\n", getpid(), i+1);
		// 释放对缓冲区的互斥操作
		semop(mutx_id, &V, 1);
		// 释放空闲缓冲区3块
		semop(full_id, v, 3);
	}
	printf("parent process(PID=%d) is over.\n", getpid());
	
	sleep(1);
	return 0;
}
