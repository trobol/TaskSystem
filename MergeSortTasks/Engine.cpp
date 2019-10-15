#include "Engine.h"
#include <random>

//allocate all the workersand start them
Engine::Engine(std::size_t workerThreads, std::size_t jobsPerThread) :
	_workers{ workerThreads }
{
	std::size_t jobsPerQueue = jobsPerThread;
	_workers.emplace_back(this, jobsPerQueue, Worker::Mode::Foreground);

	for (std::size_t i = 1; i < workerThreads; ++i) {
		_workers.emplace_back(this, jobsPerQueue, Worker::Mode::Background);
	}

	for (auto& worker : _workers) {
		worker.run();
	}
}

//searches for the engine Worker that is running on the given thread
Worker* Engine::findThreadWorker(const std::thread::id threadId) {
	for (auto& worker : _workers) {
		if (worker.threadId() == threadId) {
			return &worker;
		}
	}
}

//get the worker running the current thread
Worker* Engine::threadWorker()
{
	return findThreadWorker(std::this_thread::get_id());
}

Worker* Engine::randomWorker() {
	std::uniform_int_distribution<std::size_t> dist{ 0, _workers.size() };
	std::default_random_engine randomEngine{ std::random_device()() };

	Worker* worker = &_workers[dist(randomEngine)];

	if (worker->running()) {
		return worker;
	}
	else {
		return nullptr;
	}
}