#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/resource.h>

int search(char* data, int len, char *key, int start, int end);
void printMaxRss();

int main(int argc, char** argv) {
	int dataFd = open("new_data.txt", O_RDWR);
	struct stat dataSt;
	fstat(dataFd, &dataSt);
	char *data = (char *) mmap(
		NULL, (size_t) dataSt.st_size,
		PROT_READ | PROT_WRITE, MAP_PRIVATE,
		dataFd, 0
	);

	int *len = (int *) data;
	data += sizeof(int);

	FILE *testFp = fopen(argv[1], "r");
	char key[5];
	key[4] = 0;
	int start, end;
	while (fscanf(testFp, "%4s %d %d\n", key, &start, &end) != EOF)
		search(data, *len, key, start, end);

	munmap(data - sizeof(int), (size_t) dataSt.st_size);
	printMaxRss();

	fclose(testFp);
	close(dataFd);
	return 0;
}

void printMaxRss() {
	struct rusage ru;
	getrusage(RUSAGE_SELF, &ru);
	printf("max rss = %ld\n", ru.ru_maxrss);
}

int search(char* data, int len, char *key, int start, int end) {
	int low = 0, high = len / 4 - 1, mid, ret;
	static char buf[4096];
	static char midKey[5];
	midKey[4] = 0;

	while (low <= high) {
		mid = (low + high) / 2;
		strncpy(midKey, data + 4 * mid, 4);
		ret = strcmp(midKey, key);

		if (ret == 0) {
			memset(buf, 0, sizeof(buf));
			strncpy(
				buf,
				data + len + 4096 * mid + 4 + start,
				(size_t) (end - start + 1)
			);
			printf("key %s found : %s\n", key, buf);
			return mid;
		}

		else if (ret > 0)
			high = mid - 1;

		else
			low = mid + 1;
	}

	printf("key %s not found\n", key);
	return -1;
}
