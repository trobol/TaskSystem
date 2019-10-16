
#include <algorithm>
#include <random>
#include <chrono>
#include "sort.h"
#include <iostream>

#include "Engine.h"
#include "Job.hpp"

#define ITEM_COUNT 500000

#define MAX_SIZE 500000000000
//max number of operations
Engine engine{ std::thread::hardware_concurrency(), 100000 };



struct SortData {
	int *list;
	long lowerBound, upperBound;
};

void sortTask(Job& job);
int mN = 0;
void quickSortTasks(int list[], long lowerBound, long upperBound, Job* parent) {
	if (upperBound - lowerBound  > ITEM_COUNT/engine.workers().size()) {
		mN++;
		SortData d;
		d.list = list;
		d.lowerBound = lowerBound;
		d.upperBound = upperBound;

		auto* worker = engine.threadWorker();
		Job* job = worker->pool().createJobAsChild(sortTask, d, parent);
		worker->submit(job);
	
	} else {
		quickSort(list, lowerBound, upperBound);
	}
	
}

void sortTask(Job& job) {
	const SortData d = job.getData<SortData>();

	long i = d.lowerBound;
	long j = d.upperBound;

	int pivot = d.list[(d.lowerBound + d.upperBound) / 2];
	int tmp;
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
		quickSortTasks(d.list, d.lowerBound, j, &job);
	}

	if (i < d.upperBound) {
		quickSortTasks(d.list, i, d.upperBound, &job);
	}
}

void syncSort(int list[], long size) {

}

unsigned int seed;

void shuffle(int list[]) {
	std::shuffle(list, list + ITEM_COUNT, std::default_random_engine(seed));
}

int main() {

	Worker* worker = engine.threadWorker();

	seed = std::chrono::system_clock::now().time_since_epoch().count();
	
	
	int *list = new int[ITEM_COUNT];
	for (int i = 0; i < ITEM_COUNT; i++) {
		list[i] = i;
	}


	shuffle(list);

	auto start = std::chrono::high_resolution_clock::now();
	
	quickSort(list, 0, ITEM_COUNT-1);


	auto singleTime = std::chrono::high_resolution_clock::now() - start;


	shuffle(list);

	start = std::chrono::high_resolution_clock::now();

	Job* root = worker->pool().createJob([](Job& job) {});
	quickSortTasks(list, 0, ITEM_COUNT - 1, root);

	worker->submit(root);
	worker->wait(root);

	auto multiTime = std::chrono::high_resolution_clock::now() - start;

	bool sorted = true;
	for (long size = 1; size < MAX_SIZE; size += 1000) {

	}
	for (int i = 0; i < ITEM_COUNT; i++) {
		//std::cout << i <<": " << list[i] << std::endl;
		if (list[i] != i) {
			sorted = false;
		
		}
	}
	if (sorted) {
		std::cout << "SORTED" << std::endl;
	}
	else {
		std::cout << "NOT SORTED!!!" << std::endl;
	}
	std::cout << "N: " << mN << std::endl;

	delete[] list;
	std::cout << "SINGLE THREAD SORT TIME: " << std::chrono::duration_cast<std::chrono::microseconds>(singleTime).count()  << std::endl;
	std::cout << "MULTI THREAD SORT TIME:  " << std::chrono::duration_cast<std::chrono::microseconds>(multiTime).count() << std::endl;
}
