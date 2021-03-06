#include <stdio.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	size_t dimension;
	printf("Input the matrix dimension: ");
	scanf("%lu", &dimension);

	int shmid = shmget(
		IPC_PRIVATE,
		3 * dimension * dimension * sizeof(unsigned),
		IPC_CREAT | 0600
	);
	if (shmid < 0)
		perror("shmget() error");
	void *shmaddr = shmat(shmid, NULL, 0);
	if ((long) shmaddr < 0)
		perror("shmat() error");

	unsigned *matrix_a = (unsigned *) shmaddr;
	unsigned *matrix_b = matrix_a + dimension * dimension;
	unsigned *matrix_c = matrix_b + dimension * dimension;

	for (unsigned i = 0; i < dimension * dimension; ++i) {
		matrix_a[i] = i;
		matrix_b[i] = i;
	}

	for (int i = 0; i < 16; ++i) {
		struct timeval begin_tv, end_tv;
		printf("Multiplying matrices using %d process(es)\n", i + 1);
		gettimeofday(&begin_tv, NULL);

		for (int j = 0; j < i + 1; ++j) {
			if (fork() == 0) {
				size_t begin_p = dimension / (i + 1) * j;
				size_t end_p = j != i ? begin_p + dimension / (i + 1) : dimension;

				for (size_t p = begin_p; p < end_p; ++p) {
					for (size_t q = 0; q < dimension; ++q) {
						unsigned temp = 0;
						for (size_t r = 0; r < dimension; ++r)
							temp += matrix_a[p * dimension + r] * matrix_b[r * dimension + q];
						matrix_c[p * dimension + q] = temp;
					}
				}

				_exit(0);
			}
		}

		for (int j = 0; j < i + 1; ++j)
			wait(NULL);

		unsigned checksum = 0;
		for (unsigned j = 0; j < dimension * dimension; ++j)
			checksum += matrix_c[j];

		gettimeofday(&end_tv, NULL);
		printf(
			"Elapsed time: %.6f sec, Checksum: %u\n",
			end_tv.tv_sec - begin_tv.tv_sec + (end_tv.tv_usec - begin_tv.tv_usec) / 1000000.0,
			checksum
		);
	}

	if (shmdt(shmaddr) < 0)
		perror("shmdt() error");
	if (shmctl(shmid, IPC_RMID, NULL) < 0)
		perror("shmctl() error");

	return 0;
}
