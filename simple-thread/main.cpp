#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/time.h>
#include <utility>

int *data;
sem_t *sems;
int pivots[7];
int size;

void *partition(void *);
void *sort(void *);
int compare(const void *, const void *);
inline int part(int, int);

int main() {
	std::string filename;
	std::cin >> filename;
	std::ifstream input(filename);
	if (!input.is_open()) {
		std::cerr << "error: cannot open \"" << filename << "\"" << std::endl;
		return 1;
	}

	std::ofstream output1("output1.txt", std::ios::out | std::ios::trunc);
	if (!output1.is_open()) {
		std::cerr << "error: cannot open \"output1.txt\"" << std::endl;
		return 1;
	}

	std::ofstream output2("output2.txt", std::ios::out | std::ios::trunc);
	if (!output2.is_open()) {
		std::cerr << "error: cannot open \"output2.txt\"" << std::endl;
		return 1;
	}

	input >> size;
	data = new int[size];
	for (int i = 0; i < size; ++i)
		input >> data[i];

	struct timeval begin, end;
	std::cout << "multi-thread" << std::endl;
	gettimeofday(&begin, NULL);

	sems = new sem_t[15];
	for (int i = 0; i < 15; ++i)
		sem_init(&sems[i], 0, 0);

	pthread_t *threads = new pthread_t[15];
	for (long i = 0; i < 15; ++i) {
		if (i < 7)
			pthread_create(&threads[i], NULL, partition, (void *) i);
		else
			pthread_create(&threads[i], NULL, sort, (void *) i);
		pthread_detach(threads[i]);
	}

	for (int i = 7; i < 15; ++i)
		sem_wait(&sems[i]);
	delete[] threads;

	for (int i = 0; i < 15; ++i)
		sem_destroy(&sems[i]);
	delete[] sems;

	gettimeofday(&end, NULL);
	double duration = (end.tv_sec - begin.tv_sec) * 1000 + (end.tv_usec - begin.tv_usec) / 1000.0;
	std::cout << "elapsed " << duration << " ms" << std::endl;

	for (int i = 0; i < size; ++i)
		output1 << data[i] << " ";

	input.clear();
	input.seekg(0);
	for (int i = 0; i < size; ++i)
		input >> data[i];

	std::cout << "single thread" << std::endl;
	gettimeofday(&begin, NULL);

	qsort(data, (size_t) size, sizeof(int), compare);

	gettimeofday(&end, NULL);
	duration = (end.tv_sec - begin.tv_sec) * 1000 + (end.tv_usec - begin.tv_usec) / 1000.0;
	std::cout << "elapsed " << duration << " ms" << std::endl;

	for (int i = 0; i < size; ++i)
		output2 << data[i] << " ";
	delete[] data;

	output2.close();
	output1.close();
	input.close();

	return 0;
}

void *partition(void *no) {
	switch ((long) no) {
		case 0:
			pivots[3] = part(0, size - 1);
			sem_post(&sems[0]);
			sem_post(&sems[0]);
			break;

		case 1:
			sem_wait(&sems[0]);
			pivots[1] = part(0, pivots[3] - 1);
			sem_post(&sems[1]);
			sem_post(&sems[1]);
			break;

		case 2:
			sem_wait(&sems[0]);
			pivots[5] = part(pivots[3] + 1, size - 1);
			sem_post(&sems[2]);
			sem_post(&sems[2]);
			break;

		case 3:
			sem_wait(&sems[1]);
			pivots[0] = part(0, pivots[1] - 1);
			sem_post(&sems[3]);
			sem_post(&sems[3]);
			break;

		case 4:
			sem_wait(&sems[1]);
			pivots[2] = part(pivots[1] + 1, pivots[3] - 1);
			sem_post(&sems[4]);
			sem_post(&sems[4]);
			break;

		case 5:
			sem_wait(&sems[2]);
			pivots[4] = part(pivots[3] + 1, pivots[5] - 1);
			sem_post(&sems[5]);
			sem_post(&sems[5]);
			break;

		case 6:
			sem_wait(&sems[2]);
			pivots[6] = part(pivots[5] + 1, size - 1);
			sem_post(&sems[6]);
			sem_post(&sems[6]);
			break;

		default:
			break;
	}
	return NULL;
}

void *sort(void *no) {
	long nol = (long) no;
	sem_wait(&sems[(nol - 1) / 2]);

	int *base = nol == 7 ? data : data + pivots[nol - 8] + 1;
	int nmemb = nol == 7 ? pivots[nol - 7] : nol == 14 ?
	            size - pivots[nol - 8] - 1 : pivots[nol - 7] - pivots[nol - 8] - 1;
	qsort(base, (size_t) nmemb, sizeof(int), compare);

	sem_post(&sems[nol]);
	return NULL;
}

int compare(const void *a, const void *b) {
	return *(int *) a - *(int *) b;
}

inline int part(int left, int right) {
	int pivot = data[left];
	int idx = left;
	std::swap(data[left], data[right]);
	for (int i = left; i < right; ++i)
		if (data[i] <= pivot) {
			std::swap(data[idx], data[i]);
			++idx;
		}
	std::swap(data[right], data[idx]);
	return idx;
}
