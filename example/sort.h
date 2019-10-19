#pragma once

void quickSort(long list[], long lowerBound, long upperBound) {
	long i = lowerBound;
	long j = upperBound;

	long pivot = list[(lowerBound + upperBound) / 2];
	long tmp;
	while (i <= j) {
		while (list[i] < pivot) {
			i++;
		}

		while (list[j] > pivot) {
			j--;
		}
		if (i <= j) {
			tmp = list[i];
			list[i] = list[j];
			list[j] = tmp;
			i++;
			j--;
		}
	}

	if (lowerBound < j) {
		quickSort(list, lowerBound, j);
	}

	if (i < upperBound) {
		quickSort(list, i, upperBound);
	}
}
