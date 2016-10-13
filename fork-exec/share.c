#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	int shared = 0;
	pid_t pid = vfork();
	if (pid < 0) {
		fprintf(stderr, "vfork() error\n");
		exit(1);
	}
	else if (pid == 0) {
		shared++;
		printf("child process id = %d\n", getpid());
		printf("child shared = %d\n", shared);
		_exit(0);
	}
	else {
		wait(NULL);
		printf("parent process id = %d\n", getpid());
		printf("parent shared = %d\n", shared);
		exit(0);
	}
}
