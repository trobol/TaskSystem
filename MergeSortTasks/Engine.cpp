#include "Engine.h"



Engine::Engine(
	const std::size_t               workerThreads,
	const std::vector<std::size_t>& jobsPerThread,
	const std::size_t               fallbackJobsPerThread)
	: _workers{ workerThreads },
	_randomEngine{ std::random_device()() },
	_dist{ 0, workerThreads - 1 }

{

	std::size_t jobsPerQueue = fallbackJobsPerThread;

	if (jobsPerThread.size() > 0)
	{
		jobsPerQueue = static_cast<std::size_t>(jobsPerThread[0]);
	}

	_workers.emplace_back(0ull, this, jobsPerQueue, Worker::Mode::Foreground);

	for (std::size_t i = 1; i < workerThreads; ++i)
	{

		if (jobsPerThread.size() > i)
		{
			jobsPerQueue = static_cast<std::size_t>(jobsPerThread[i]);
		}
		else
		{
			jobsPerQueue = fallbackJobsPerThread;
		}

		_workers.emplace_back(i, this, jobsPerQueue, Worker::Mode::Background);
	}


	for (auto& worker : _workers)
	{
		worker.run();
	}
}

Engine::Engine(const std::size_t workerThreads, const std::size_t jobsPerThread)
	: Engine{ workerThreads,
			 std::vector<std::size_t>(workerThreads, jobsPerThread),
			 jobsPerThread }
{
}

//searches for the engine Worker that is running on the given thread
Worker* Engine::findThreadWorker(const std::thread::id threadId) {
	for (auto& worker : _workers) {
		if (worker.threadId() == threadId) {
			return &worker;
		}
	}
	return nullptr;
}

//get the worker running the current thread
Worker* Engine::threadWorker()
{
	return findThreadWorker(std::this_thread::get_id());
}

Worker* Engine::randomWorker() {
	

	Worker* worker = &_workers[_dist(_randomEngine)];

	if (worker->running()) {
		return worker;
	}
	else {
		return nullptr;
	}
}