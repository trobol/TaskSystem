
#include <algorithm>
#include <random>
#include <chrono>
#include "sort.h"
#include <iostream>

#include "Engine.h"
#include "Job.hpp"


Engine engine{ std::thread::hardware_concurrency(), 1000 };

struct SortData {
	int *arr, l, m, r;
};
unsigned int index = 0;
void mergeTask(Job& job) {
	const SortData d = job.getData<SortData>();
	std::cout << d.r - d.l << std::endl;
	merge(d.arr, d.l, d.m, d.r);
}
/*
void taskSort(Job& job) {
	const SortData d = job.getData<SortData>();

	if (d.l < d.r)
	{
		auto* worker = engine.randomWorker();
		int m = d.l + (d.r - d.l) / 2;
		SortData data;
		data.arr = arr;
		data.l = l;
		data.m = m;
		data.r = r;
		Job* job = worker->pool().createJobAsChild(mergeTask, data, job);
		// Same as (l+r)/2, but avoids overflow for 
		// large l and h 

		//add two new tasks as children
		taskSort(arr, l, m, job);
		taskSort(arr, m + 1, r, job);

		worker->submit(job);
	}

}
*/

Job* taskSort(int arr[], int l, int r, Job* parent)
{
	if (l < r)
	{	
		auto* worker = engine.randomWorker();
		int m = l + (r - l) / 2;
		SortData data;
		data.arr = arr;
		data.l = l;
		data.m = m;
		data.r = r;
		Job* job = worker->pool().createJobAsChild(mergeTask, data, parent);
		// Same as (l+r)/2, but avoids overflow for 
		// large l and h 
		
		//add two new tasks as children
		taskSort(arr, l, m, job);
		taskSort(arr, m + 1, r, job);

		worker->submit(job);
		return job;
	}


}

#define ITEM_COUNT 100

unsigned int seed;

void shuffle(int list[]) {
	std::shuffle(list, list + ITEM_COUNT, std::default_random_engine(seed));
}

int main() {


	Worker* worker = engine.threadWorker();

	seed = std::chrono::system_clock::now().time_since_epoch().count();

	int list[ITEM_COUNT];
	for (int i = 0; i < ITEM_COUNT; i++) {
		list[i] = i;
	}


	shuffle(list);
	Job* root = worker->pool().createJob([](Job& job) { // NOP 
		});

	taskSort(list, 0, ITEM_COUNT - 1, root);

	worker->submit(root);
	worker->wait(root);


	bool sorted = true;
	for (int i = 0; i < ITEM_COUNT-1; i++) {
		//std::cout << list[i] << std::endl;
		if (list[i] > list[i + 1]) {
			sorted = false;
		}
	}

	std::cout << worker->totalJobsDiscarded() << " : DISTARDED" << std::endl;
	if (sorted) {
		std::cout << "SORTED" << std::endl;
	}
	else {
		std::cout << "NOT SORTED!!!" << std::endl;
	}
}
