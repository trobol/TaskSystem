#include "../include/Pool.hpp"


namespace TaskSystem {

	Pool::Pool(std::size_t maxTasks) :
		_storage{ maxTasks },
		_allocatedTasks{ 0 }
	{}

	Task* Pool::allocate()
	{
		if (full())
		{
			return nullptr;
		}
		else
		{
		
			Task* taskStorage = &_storage[_allocatedTasks];
			_allocatedTasks++;
			return taskStorage;
		}
	}

	Task* Pool::createTask(TaskFunction taskFunction)
	{
		auto* taskStorage = allocate();

		if (taskStorage != nullptr)
		{
			return new(taskStorage) Task{ taskFunction };
		}
		else
		{
			return nullptr;
		}
	}

	Task* Pool::createTaskAsChild(TaskFunction taskFunction, Task* parent)
	{
		auto* taskStorage = allocate();

		if (taskStorage != nullptr)
		{
			return new(taskStorage) Task{ taskFunction, parent };
		}
		else
		{
			return nullptr;
		}
	}

	void Pool::clear()
	{
		_allocatedTasks = 0;
	}

	std::size_t Pool::tasks() const
	{
		return _allocatedTasks;
	}

	std::size_t Pool::maxTasks() const
	{
		return _storage.size();
	}

	float Pool::tasksFactor() const
	{
		return static_cast<float>(tasks()) / maxTasks();
	}

	bool Pool::full() const
	{
		return tasks() >= maxTasks();
	}
}