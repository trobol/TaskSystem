#ifndef ENGINE_H
#define ENGINE_H

#pragma once

#include "Worker.h"
#include "StaticVector.h"
#include <random>

class Worker;
class Engine {
public:
	Engine(
		const std::size_t               workerThreads,
		const std::vector<std::size_t>& jobsPerThread,
		const std::size_t               fallbackJobsPerThread);
	Engine(const std::size_t workerThreads, const std::size_t jobsPerThread);

	Worker* randomWorker();
	Worker* threadWorker();
private:


	StaticVector<Worker> _workers;

	Worker* findThreadWorker(const std::thread::id threadId);


	std::default_random_engine _randomEngine;
	std::uniform_int_distribution<std::size_t> _dist;
};
#endif // !ENGINE_H

