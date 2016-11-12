#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

int *data;
sem_t *sems;
int pivots[11];

void *partition(void *);
void *sort(void *);
int compare(const void *, const void *);

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

	int size;
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
	for (long i = 0; i < 7; ++i)
		pthread_create(&threads[i], NULL, partition, (void *) i);
	for (long i = 7; i < 15; ++i)
		pthread_create(&threads[i], NULL, sort, (void *) i);

	sem_post(&sems[0]);
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
	return NULL;
}

void *sort(void *no) {
	return NULL;
}

int compare(const void *a, const void *b) {
	return *(int *) a - *(int *) b;
}
