#include "../include/TaskQueue.hpp"

namespace TaskSystem {
	TaskQueue::TaskQueue(std::size_t maxTasks)
		: _tasks{ maxTasks, nullptr }, _top{ 0 }, _bottom{ 0 }
	{
	}

	bool TaskQueue::push(Task* task)
	{

		std::size_t bottom = _bottom.load(std::memory_order_acquire);

		if (bottom < _tasks.size())
		{
			_tasks[bottom % _tasks.size()] = task;

			// Make sure the task is written before publishing the new botton
			std::atomic_thread_fence(std::memory_order_release);

			_bottom.store(bottom + 1, std::memory_order_release);

			return true;
		}
		else
		{
			return false;
		}
	}

	Task* TaskQueue::pop()
	{
		std::size_t bottom = _bottom.load(std::memory_order_acquire);

		if (bottom > 0)
		{
			bottom = bottom - 1;
			_bottom.store(bottom, std::memory_order_release);
		}

		std::atomic_thread_fence(std::memory_order_release);

		std::size_t top = _top.load(std::memory_order_acquire);

		if (top <= bottom)
		{
			Task* task = _tasks[bottom % _tasks.size()];
			// assert(task != nullptr && "null task returned from queue");

			if (top == bottom)
			{
				// This is the last item in the queue. It could happen
				// multiple concurrent access "fight" for this last item.
				// The atomic compare+exchange operation ensures this last item
				// is extracted only once

				std::size_t       expectedTop = top;
				const std::size_t nextTop = top + 1;
				std::size_t       desiredTop = nextTop;

				if (!_top.compare_exchange_strong(
					expectedTop, desiredTop, std::memory_order_acq_rel))
				{
					// Someone already took the last item, abort
					task = nullptr;
				}

				_bottom.store(nextTop, std::memory_order_release);
			}

			return task;
		}
		else
		{
			// Queue already empty
			_bottom.store(top, std::memory_order_release);

			return nullptr;
		}
	}

	Task* TaskQueue::steal()
	{
		std::size_t top = _top.load(std::memory_order_acquire);

		// Put a barrier here to make sure bottom is read after reading
		// top
		std::atomic_thread_fence(std::memory_order_acquire);

		std::size_t bottom = _bottom.load(std::memory_order_acquire);

		if (top < bottom)
		{
			Task* task = _tasks[top % _tasks.size()];
			const std::size_t nextTop = top + 1;
			std::size_t       desiredTop = nextTop;

			if (!_top.compare_exchange_weak(
				top, desiredTop, std::memory_order_acq_rel))
			{
				// Some concurrent pop()/steal() operation
				// changed the current top
				return nullptr;
			}
			else
			{

				return task;
			}
		}
		else
		{
			// The queue is empty

			return nullptr;
		}
	}

	std::size_t TaskQueue::size() const
	{
		return _bottom.load(std::memory_order_seq_cst) -
			_top.load(std::memory_order_seq_cst);
	}

	bool TaskQueue::empty() const
	{
		return size() == 0;
	}
}