
#include <algorithm>
#include <random>
#include <chrono>
#include "sort.h"


#define ITEM_COUNT 1000

unsigned int seed;

void shuffle(int list[]) {
	std::shuffle(list, list + ITEM_COUNT, std::default_random_engine(seed));
}

int main() {
	seed = std::chrono::system_clock::now().time_since_epoch().count();

	int list[ITEM_COUNT];
	for (int i = 0; i < ITEM_COUNT; i++) {
		list[i] = i;
	}


	shuffle(list);


	mergeSort(list, 0, ITEM_COUNT - 1);
}