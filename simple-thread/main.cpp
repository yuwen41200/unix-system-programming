#include <iostream>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>

int *data;
sem_t *sems;

void *partition(void *);
void *sort(void *);

int main() {
	std::string filename;
	std::cout << "> ";
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

	sems = new sem_t[15];
	for (int i = 0; i < 15; ++i)
		sem_init(&sems[i], 0, 0);

	pthread_t *threads = new pthread_t[15];
	for (long i = 0; i < 7; ++i)
		pthread_create(&threads[i], NULL, partition, (void *) i);
	for (long i = 7; i < 15; ++i)
		pthread_create(&threads[i], NULL, sort, (void *) i);

	for (int i = 7; i < 15; ++i)
		sem_wait(&sems[i]);

	delete[] threads;
	delete[] sems;
	delete[] data;

	output2.close();
	output1.close();
	input.close();

	return 0;
}
