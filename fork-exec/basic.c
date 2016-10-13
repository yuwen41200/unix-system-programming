#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "fork() error\n");
		exit(1);
	}
	else if (pid == 0) {
		execlp("ls", "ls", "-lAh", ".", NULL);
		_exit(1);
	}
	else {
		wait(NULL);
		printf("child process done\n");
		exit(0);
	}
}
