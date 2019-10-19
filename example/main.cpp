
#include <algorithm>
#include <random>
#include <chrono>
#include "sort.h"
#include <iostream>
#include <sstream>
#include <fstream>

#include "../include/Engine.hpp"
#include "../include/Task.hpp"

#define MAX_SIZE 200000

long sortSize = 2;
//max number of operations

using namespace TaskSystem;

struct SortData {
	long* list;
	long lowerBound, upperBound;
};

void sortTask(Task& task);
void quickSortTasks(long list[], long lowerBound, long upperBound, Task* parent) {
	if (upperBound - lowerBound > sortSize / Engine::Instance().workers().size()) {
		SortData d;
		d.list = list;
		d.lowerBound = lowerBound;
		d.upperBound = upperBound;

		auto* worker = Engine::Instance().threadWorker();
		Task* task = worker->pool().createTaskAsChild(sortTask, d, parent);
		worker->submit(task);

	}
	else {
		quickSort(list, lowerBound, upperBound);
	}

}

void sortTask(Task& task) {
	const SortData d = task.getData<SortData>();

	long i = d.lowerBound;
	long j = d.upperBound;

	long pivot = d.list[(d.lowerBound + d.upperBound) / 2];
	long tmp;
	while (i <= j) {
		while (d.list[i] < pivot) {
			i++;
		}

		while (d.list[j] > pivot) {
			j--;
		}
		if (i <= j) {
			tmp = d.list[i];
			d.list[i] = d.list[j];
			d.list[j] = tmp;
			i++;
			j--;
		}
	}

	if (d.lowerBound < j) {
		quickSortTasks(d.list, d.lowerBound, j, &task);
	}

	if (i < d.upperBound) {
		quickSortTasks(d.list, i, d.upperBound, &task);
	}
}

unsigned int seed;
void shuffle(long list[], long size) {
	std::shuffle(list, list + size, std::default_random_engine(seed));
}

void syncSort(long* list, long size) {
	sortSize = size;
	Worker* worker = Engine::Instance().threadWorker();
	Task* root = worker->pool().createTask([](Task& task) {});
	quickSortTasks(list, 0, size - 1, root);

	worker->submit(root);
	worker->wait(root);
}


bool isSorted(long* list, long size) {
	for (long i = 0; i < size; i++) {
		if (list[i] != i) {
			return false;
		}
	}
	return true;
}

long long now() {
	return std::chrono::system_clock::now().time_since_epoch().count();
}

long long end(long long start) {
	return now() - start;
}

#define ITERATIONS 20
#define TO_MS CLOCKS_PER_SEC/1000

int main() {



	seed = now();


	long long start = 0, singleEnd = 0, multiEnd = 0;

	std::vector<long long> singleTimes, multiTimes;

	for (long size = 2; size < MAX_SIZE; size += size * 1.5) {

		singleEnd = 0;
		multiEnd = 0;
		long* list = new long[size];
		for (long i = 0; i < size; i++) {
			list[i] = i;
		}
		for (int a = 0; a < ITERATIONS; a++) {

			shuffle(list, size);
			start = now();

			quickSort(list, 0, size - 1);


			singleEnd += end(start);
			shuffle(list, size);
			start = now();

			syncSort(list, size);

			multiEnd += end(start);

		}

		singleEnd /= ITERATIONS;
		singleEnd /= TO_MS;

		multiEnd /= ITERATIONS;
		multiEnd /= TO_MS;

		std::cout << "Sorted: " << size << " items Single: " << singleEnd << " Multi: " << multiEnd << std::endl;

		delete[] list;
	}
}
