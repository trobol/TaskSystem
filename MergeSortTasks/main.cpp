
#include <algorithm>
#include <random>
#include <chrono>
#include "sort.h"
#include <iostream>
#include <sstream>
#include <fstream>

#include "Engine.h"
#include "Job.hpp"

#define MAX_SIZE 20000

long sortSize = 2;
//max number of operations
Engine engine{ std::thread::hardware_concurrency(), 100000 };



struct SortData {
	long *list;
	long lowerBound, upperBound;
};

void sortTask(Job& job);
void quickSortTasks(long list[], long lowerBound, long upperBound, Job* parent) {
	if (upperBound - lowerBound  > sortSize/engine.workers().size()) {
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
		quickSortTasks(d.list, d.lowerBound, j, &job);
	}

	if (i < d.upperBound) {
		quickSortTasks(d.list, i, d.upperBound, &job);
	}
}

unsigned int seed;
void shuffle(long list[], long size) {
	std::shuffle(list, list + size, std::default_random_engine(seed));
}

void syncSort(long* list, long size) {
	sortSize = size;
	Worker* worker = engine.threadWorker();
	Job* root = worker->pool().createJob([](Job& job) {});
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

int main() {
	std::ofstream output;
	
	

	seed = now();
	
	
	std::stringstream singlePath, multiPath;

	singlePath << "<path stroke=\"red\" d=\"";
	multiPath << "<path stroke=\"blue\" d=\"";


	long long start = 0, singleEnd = 0, multiEnd = 0;
	

	for (long size = 2; size < MAX_SIZE; size += 1000) {
		

		long* list = new long[size];
		for (long i = 0; i < size; i++) {
			list[i] = i;
		}
		shuffle(list, size);
		start = now();

		quickSort(list, 0, size - 1);

		
		singleEnd = end(start);
		shuffle(list, size);
		start = now();

		syncSort(list, size);

		multiEnd = end(start);

		output << size << ",";
		if (isSorted(list, size)) {
			std::cout << "Sorted: " << size << " items Single: " << singleEnd << " Multi: " << multiEnd << std::endl;
			
			singlePath << "M " << size << " " << singleEnd;
			multiPath << "M " << size << " " << multiEnd;
		}
		else {
			std::cout << "ERROR at: size=" << size << std::endl;

		}
		delete[] list;
	}

	singlePath << "\"/>";
	multiPath << "\"/>";

	output.open("output.svg");
	output << "<svg version=\"1.1\" baseProfile=\"full\" width = \"300\" height = \"200\" xmlns =\"http://www.w3.org/2000/svg\" >";

	output << singlePath.str();
	output << multiPath.str();
	output << "</svg>";
	output.close();
}
