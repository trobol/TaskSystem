#include "../include/Engine.hpp"

namespace TaskSystem {
	Engine::Engine(
		const std::size_t               workerThreads,
		const std::vector<std::size_t>& tasksPerThread,
		const std::size_t               fallbackTasksPerThread)
		: _workers{ workerThreads },
		_randomEngine{ std::random_device()() },
		_dist{ 0, workerThreads - 1 }
	{

		std::size_t tasksPerQueue = fallbackTasksPerThread;

		if (tasksPerThread.size() > 0)
		{
			tasksPerQueue = static_cast<std::size_t>(tasksPerThread[0]);
		}

		_workers.emplace_back(0ull, this, tasksPerQueue, Worker::Mode::Foreground);

		for (std::size_t i = 1; i < workerThreads; ++i)
		{

			if (tasksPerThread.size() > i)
			{
				tasksPerQueue = static_cast<std::size_t>(tasksPerThread[i]);
			}
			else
			{
				tasksPerQueue = fallbackTasksPerThread;
			}

			_workers.emplace_back(i, this, tasksPerQueue, Worker::Mode::Background);
		}

		for (auto& worker : _workers)
		{
			worker.run();
		}
	}

	Engine::Engine(const std::size_t workerThreads, const std::size_t tasksPerThread)
		: Engine{ workerThreads,
				 std::vector<std::size_t>(workerThreads, tasksPerThread),
				 tasksPerThread }
	{
	}

	Worker* Engine::randomWorker()
	{
		Worker* worker = &_workers[_dist(_randomEngine)];

		if (worker->running())
		{
			return worker;
		}
		else
		{
			return nullptr;
		}
	}

	Worker* Engine::findThreadWorker(const std::thread::id threadId)
	{
		for (auto& worker : _workers)
		{
			if (threadId == worker.threadId())
			{
				return &worker;
			}
		}

		return nullptr;
	}

	std::size_t Engine::totalTasksRun() const
	{
		std::size_t total = 0;

		for (const auto& worker : _workers)
		{
			total += worker.totalTasksRun();
		}

		return total;
	}

	std::size_t Engine::totalTasksAllocated() const
	{
		std::size_t total = 0;

		for (const auto& worker : _workers)
		{
			total += worker.pool().tasks();
		}

		return total;
	}

	Worker* Engine::threadWorker()
	{
		static thread_local Engine* engine = this;
		static thread_local Worker* worker =
			findThreadWorker(std::this_thread::get_id());

		if (engine != this)
		{
			engine = this;
			worker = findThreadWorker(std::this_thread::get_id());
		}

		return worker;
	}

	const StaticVector<Worker>& Engine::workers() const
	{
		return _workers;
	}
}