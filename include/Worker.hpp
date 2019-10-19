#ifndef TASKSYSTEM_WORK_HPP
#define TASKSYSTEM_WORK_HPP

#pragma once
#include "Task.hpp"
#include "Pool.hpp"
#include <thread>
#include "TaskQueue.hpp"


namespace TaskSystem {

	class Engine;

	using WorkQueue = TaskQueue;

	/**
	 * \ingroup tasks
	 * \
	 */
	class Worker
	{
	public:
		enum class Mode
		{
			Background,
			Foreground
		};

		enum class State
		{
			Idle,
			Running,
			Stopping
		};

		Worker(const std::uint64_t id, Engine* engine, std::size_t poolSize, Mode mode = Mode::Background);
		~Worker();

		std::uint64_t id() const;
		std::thread::id threadId() const;
		bool running() const;
		void run();
		void stop();
		void submit(Task* task);
		void wait(Task* task);
		Pool& pool();
		const Pool& pool() const;
		void join();

		const std::atomic<State>& state() const;
		std::size_t totalTasksRun() const;
		std::size_t totalTasksDiscarded() const;
		std::size_t cyclesWithoutTasks() const;
		std::size_t maxCyclesWithoutTasks() const;

	private:
		WorkQueue _workQueue;
		Pool _pool;
		Engine* _engine;
		std::thread _workerThread;
		std::thread::id _workerThreadId;
		Mode _mode;
		std::atomic<State> _state;
		std::size_t _totalTasksRun;
		std::size_t _totalTasksDiscarded;
		std::size_t _cyclesWithoutTasks;
		std::size_t _maxCyclesWithoutTasks;
		std::uint64_t _id;

		Task* getTask();
		void getTasks();
	};
}
#endif