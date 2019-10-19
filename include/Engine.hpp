#ifndef TASKSYSTEM_ENGINE_H
#define TASKSYSTEM_ENGINE_H

#pragma once

#include "Worker.hpp"
#include "StaticVector.hpp"
#include <random>

namespace TaskSystem {
	class Worker;
	class Engine
	{
	public:
		inline static Engine& Instance() {
			static Engine _instance{ std::thread::hardware_concurrency(), 100000 };
			return _instance;
		}

		/**
		 * \brief Initializes an engine with /p workerThreads workers and
		 * a maximum of \p tasksPerThread tasks per worker
		 *
		 * \param workerThreads The number of workers this engine will manage.
		 * One of these workers will be run in the caller thread, which means only
		 * n-1 workers will have background threads polling for work to do.
		 * See Worker::wait().
		 * \param tasksPerThread Maximum number of tasks that can be allocated by
		 * thread. Once this limit is reached, the worker stops returning storage
		 * for more tasks, which means no more tasks can be submitted to the worker.
		 */
		Engine(
			const std::size_t               workerThreads,
			const std::vector<std::size_t>& tasksPerThread,
			const std::size_t               fallbackTasksPerThread);

		Engine(const std::size_t workerThreads, const std::size_t tasksPerThread);

		Engine(const Engine&) = delete;

		/**
		 * \brief Returns one of the workers, randomnly picked from all
		 * the available workers in the engine
		 *
		 * Note allocating and submitting tasks from a thread different from
		 * the worker associated to the caller thread has undefined behavior.
		 * See `threadWorker()`.
		 */
		Worker* randomWorker();

		/**
		 * \brief Returns the worker associated to a given thread
		 *
		 * \param threadId Id of the thread
		 *
		 * \returns A pointer to the worker which is using the given thread
		 * as worker thread, nullptr if no worker is using the give thread.
		 */
		Worker* findThreadWorker(const std::thread::id threadId);

		/**
		 * \brief Returns the worker associated to the caller thread
		 */
		Worker* threadWorker();

		/**
		 * \brief Returns the total number of tasks run by the engine
		 */
		std::size_t totalTasksRun() const;
		std::size_t totalTasksAllocated() const;

		const StaticVector<Worker>& workers() const;

	private:
		StaticVector<Worker>                  _workers;
		std::default_random_engine                 _randomEngine;
		std::uniform_int_distribution<std::size_t> _dist;
	};
}
#endif // !ENGINE_H

