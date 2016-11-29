#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/time.h>
#include <utility>
#include <deque>
#include <errno.h>

typedef void (*job)();

int *data;
sem_t *sems, *ready, *idle;
int pivots[7];
int size;
std::deque<job> jobs;
job job_table[8];

void job0();
void job1();
void job2();
void job3();
void job4();
void job5();
void job6();
void job7();
void job8();
void job9();
void job10();
void job11();
void job12();
void job13();
void job14();

void *worker(void *);
inline int part(int, int);
inline void bsort(int *, size_t);

int main() {
	std::ifstream input("input.txt");
	if (!input.is_open()) {
		std::cerr << "error: cannot open \"input.txt\"" << std::endl;
		return 1;
	}

	input >> size;
	data = new int[size];
	for (int i = 0; i < size; ++i)
		input >> data[i];

	sems = new sem_t[15];
	for (int i = 0; i < 15; ++i)
		sem_init(&sems[i], 0, 0);

	ready = new sem_t[8];
	idle = new sem_t[8];
	for (int i = 0; i < 8; ++i) {
		sem_init(&ready[i], 0, 0);
		sem_init(&idle[i], 0, 0);
		sem_post(&idle[i]);
	}

	pthread_t *threads = new pthread_t[8];
	for (long i = 0; i < 8; ++i) {
		pthread_create(&threads[i], NULL, worker, (void *) i);
		pthread_detach(threads[i]);
	}

	for (int pool_size = 1; pool_size <= 8; ++pool_size) {
		struct timeval begin, end;
		std::cout << "pool_size = " << pool_size << std::endl;
		gettimeofday(&begin, NULL);

		jobs.push_back(&job0);
		jobs.push_back(&job1);
		jobs.push_back(&job2);
		jobs.push_back(&job3);
		jobs.push_back(&job4);
		jobs.push_back(&job5);
		jobs.push_back(&job6);
		jobs.push_back(&job7);
		jobs.push_back(&job8);
		jobs.push_back(&job9);
		jobs.push_back(&job10);
		jobs.push_back(&job11);
		jobs.push_back(&job12);
		jobs.push_back(&job13);
		jobs.push_back(&job14);

		while (!jobs.empty()) {
			for (int i = 0; i < pool_size; ++i) {
				if (jobs.empty())
					break;
				sem_trywait(&idle[i]);
				if (errno == EAGAIN)
					continue;
				job_table[i] = jobs.front();
				jobs.pop_front();
				sem_post(&ready[i]);
			}
		}

		for (int i = 7; i < 15; ++i)
			sem_wait(&sems[i]);

		gettimeofday(&end, NULL);
		double duration = (end.tv_sec - begin.tv_sec) * 1000 + (end.tv_usec - begin.tv_usec) / 1000.0;
		std::cout << "elapsed " << duration << " ms" << std::endl;

		std::string filename = "output_";
		filename += pool_size;
		filename += ".txt";

		std::ofstream output(filename, std::ios::out | std::ios::trunc);
		if (!output.is_open()) {
			std::cerr << "error: cannot open \"" << filename << "\"" << std::endl;
			return 1;
		}

		for (int i = 0; i < size; ++i)
			output << data[i] << " ";
		output.close();

		input.clear();
		input.seekg(0);
		input >> size;
		for (int i = 0; i < size; ++i)
			input >> data[i];
	}

	for (int i = 0; i < 8; ++i) {
		sem_destroy(&ready[i]);
		sem_destroy(&idle[i]);
	}

	for (int i = 0; i < 15; ++i)
		sem_destroy(&sems[i]);

	delete[] threads;
	delete[] ready;
	delete[] idle;
	delete[] sems;
	delete[] data;

	input.close();

	return 0;
}

void job0() {
	pivots[3] = part(0, size - 1);
	sem_post(&sems[0]);
	sem_post(&sems[0]);
}

void job1() {
	sem_wait(&sems[0]);
	pivots[1] = part(0, pivots[3] - 1);
	sem_post(&sems[1]);
	sem_post(&sems[1]);
}

void job2() {
	sem_wait(&sems[0]);
	pivots[5] = part(pivots[3] + 1, size - 1);
	sem_post(&sems[2]);
	sem_post(&sems[2]);
}

void job3() {
	sem_wait(&sems[1]);
	pivots[0] = part(0, pivots[1] - 1);
	sem_post(&sems[3]);
	sem_post(&sems[3]);
}

void job4() {
	sem_wait(&sems[1]);
	pivots[2] = part(pivots[1] + 1, pivots[3] - 1);
	sem_post(&sems[4]);
	sem_post(&sems[4]);
}

void job5() {
	sem_wait(&sems[2]);
	pivots[4] = part(pivots[3] + 1, pivots[5] - 1);
	sem_post(&sems[5]);
	sem_post(&sems[5]);
}

void job6() {
	sem_wait(&sems[2]);
	pivots[6] = part(pivots[5] + 1, size - 1);
	sem_post(&sems[6]);
	sem_post(&sems[6]);
}

void job7() {
	long nol = 7;
	sem_wait(&sems[(nol - 1) / 2]);

	int *base = nol == 7 ? data : data + pivots[nol - 8] + 1;
	int nmemb = nol == 7 ? pivots[nol - 7] : nol == 14 ?
	            size - pivots[nol - 8] - 1 : pivots[nol - 7] - pivots[nol - 8] - 1;
	bsort(base, (size_t) nmemb);

	sem_post(&sems[nol]);
}

void job8() {
	long nol = 8;
	sem_wait(&sems[(nol - 1) / 2]);

	int *base = nol == 7 ? data : data + pivots[nol - 8] + 1;
	int nmemb = nol == 7 ? pivots[nol - 7] : nol == 14 ?
	            size - pivots[nol - 8] - 1 : pivots[nol - 7] - pivots[nol - 8] - 1;
	bsort(base, (size_t) nmemb);

	sem_post(&sems[nol]);
}

void job9() {
	long nol = 9;
	sem_wait(&sems[(nol - 1) / 2]);

	int *base = nol == 7 ? data : data + pivots[nol - 8] + 1;
	int nmemb = nol == 7 ? pivots[nol - 7] : nol == 14 ?
	            size - pivots[nol - 8] - 1 : pivots[nol - 7] - pivots[nol - 8] - 1;
	bsort(base, (size_t) nmemb);

	sem_post(&sems[nol]);
}

void job10() {
	long nol = 10;
	sem_wait(&sems[(nol - 1) / 2]);

	int *base = nol == 7 ? data : data + pivots[nol - 8] + 1;
	int nmemb = nol == 7 ? pivots[nol - 7] : nol == 14 ?
	            size - pivots[nol - 8] - 1 : pivots[nol - 7] - pivots[nol - 8] - 1;
	bsort(base, (size_t) nmemb);

	sem_post(&sems[nol]);
}

void job11() {
	long nol = 11;
	sem_wait(&sems[(nol - 1) / 2]);

	int *base = nol == 7 ? data : data + pivots[nol - 8] + 1;
	int nmemb = nol == 7 ? pivots[nol - 7] : nol == 14 ?
	            size - pivots[nol - 8] - 1 : pivots[nol - 7] - pivots[nol - 8] - 1;
	bsort(base, (size_t) nmemb);

	sem_post(&sems[nol]);
}

void job12() {
	long nol = 12;
	sem_wait(&sems[(nol - 1) / 2]);

	int *base = nol == 7 ? data : data + pivots[nol - 8] + 1;
	int nmemb = nol == 7 ? pivots[nol - 7] : nol == 14 ?
	            size - pivots[nol - 8] - 1 : pivots[nol - 7] - pivots[nol - 8] - 1;
	bsort(base, (size_t) nmemb);

	sem_post(&sems[nol]);
}

void job13() {
	long nol = 13;
	sem_wait(&sems[(nol - 1) / 2]);

	int *base = nol == 7 ? data : data + pivots[nol - 8] + 1;
	int nmemb = nol == 7 ? pivots[nol - 7] : nol == 14 ?
	            size - pivots[nol - 8] - 1 : pivots[nol - 7] - pivots[nol - 8] - 1;
	bsort(base, (size_t) nmemb);

	sem_post(&sems[nol]);
}

void job14() {
	long nol = 14;
	sem_wait(&sems[(nol - 1) / 2]);

	int *base = nol == 7 ? data : data + pivots[nol - 8] + 1;
	int nmemb = nol == 7 ? pivots[nol - 7] : nol == 14 ?
	            size - pivots[nol - 8] - 1 : pivots[nol - 7] - pivots[nol - 8] - 1;
	bsort(base, (size_t) nmemb);

	sem_post(&sems[nol]);
}

void *worker(void *no) {
	long nol = (long) no;
	sem_wait(&ready[nol]);

	job_table[nol]();

	sem_post(&idle[nol]);
	return NULL;
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

inline void bsort(int *arr, size_t n) {
	for (size_t i = 0; i < n - 1; ++i)
		for (size_t j = 0; j < n - i - 1; ++j)
			if (arr[j] > arr[j + 1])
				std::swap(arr[j], arr[j + 1]);
}
