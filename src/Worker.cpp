
#include "../include/Worker.hpp"
#include "../include/Engine.hpp"

namespace TaskSystem {
	Worker::Worker(
		const std::uint64_t id,
		Engine* engine,
		std::size_t         poolSize,
		Worker::Mode        mode)
		: _workQueue{ poolSize + 1 },
		_pool{ poolSize },
		_engine{ engine },
		_mode{ mode },
		_state{ State::Idle },
		_totalTasksRun{ 0 },
		_totalTasksDiscarded{ 0 },
		_cyclesWithoutTasks{ 0 },
		_maxCyclesWithoutTasks{ 0 },
		_id{ id }
	{
	}

	void Worker::run()
	{
		if (running())
		{
			return;
		}

		auto mainLoop = [this] {
			_state = State::Running;


			while (running())
			{
				Task* task = getTask();

				if (task != nullptr)
				{
					if (task->run())
					{
						++_totalTasksRun;
						_cyclesWithoutTasks = 0;
					}
				}
				else
				{
					++_cyclesWithoutTasks;
					_maxCyclesWithoutTasks =
						std::max(_cyclesWithoutTasks, _maxCyclesWithoutTasks);
				}
			}
		};

		if (_mode == Mode::Background)
		{
			_workerThread = std::thread{ mainLoop };
			_workerThreadId = _workerThread.get_id();
		}
		else
		{
			_state = State::Running;
			_workerThreadId = std::this_thread::get_id();


		}
	}

	void Worker::stop()
	{
		State expected = State::Running;
		while (!_state.compare_exchange_weak(expected, State::Stopping))
			;


		join();
		_state = State::Idle;

	}

	Worker::~Worker()
	{
		stop();
	}

	void Worker::join()
	{
		if (std::this_thread::get_id() != threadId() && _workerThread.joinable())
		{
			_workerThread.join();
		}
	}

	bool Worker::running() const
	{
		return _state == State::Running;
	}

	void Worker::submit(Task* task)
	{
		if (task != nullptr && !_workQueue.push(task))
		{
			task->discard();
			++_totalTasksDiscarded;
		}
	}

	void Worker::wait(Task* waitTask)
	{
		while (!waitTask->finished())
		{
			Task* task = getTask();

			if (task != nullptr)
			{
				task->run();
				++_totalTasksRun;
				_cyclesWithoutTasks = 0;
			}
			else
			{
				++_cyclesWithoutTasks;
				_maxCyclesWithoutTasks =
					std::max(_cyclesWithoutTasks, _maxCyclesWithoutTasks);
			}
		}
	}

	Pool& Worker::pool()
	{
		return _pool;
	}

	const Pool& Worker::pool() const
	{
		return _pool;
	}

	Task* Worker::getTask()
	{
		Task* task = _workQueue.pop();

		if (task != nullptr)
		{
			return task;
		}
		else
		{
			// Steal task from another worker

			Worker* worker = _engine->randomWorker();

			if (worker == this)
			{
				std::this_thread::yield();
				return nullptr;
			}
			else
			{
				if (worker != nullptr)
				{
					return worker->_workQueue.steal();
				}
				else
				{
					std::this_thread::yield();
					return nullptr;
				}
			}
		}
	}

	std::uint64_t Worker::id() const
	{
		return _id;
	}

	std::thread::id Worker::threadId() const
	{
		return _workerThreadId;
	}

	const std::atomic<Worker::State>& Worker::state() const
	{
		return _state;
	}

	std::size_t Worker::totalTasksRun() const
	{
		return _totalTasksRun;
	}

	std::size_t Worker::cyclesWithoutTasks() const
	{
		return _cyclesWithoutTasks;
	}

	std::size_t Worker::maxCyclesWithoutTasks() const
	{
		return _maxCyclesWithoutTasks;
	}

	std::size_t Worker::totalTasksDiscarded() const
	{
		return _totalTasksDiscarded;
	}
}