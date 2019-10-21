#ifndef TASKSYSTEM_POOL_HPP
#define TASKSYSTEM_POOL_HPP
#pragma once
#include <vector>
#include "Task.hpp"

//one pool per worker NOT THREAD SAFE
namespace TaskSystem {
	class Pool
	{
	public:
		Pool(std::size_t maxTasks);

		Task* allocate();

		Task* createTask(TaskFunction taskFunction);
		Task* createTaskAsChild(TaskFunction taskFunction, Task* parent);

		template<typename Data>
		Task* createTask(TaskFunction taskFunction, const Data& data)
		{
			auto* taskStorage = allocate();

			if (taskStorage != nullptr)
			{
				return new(taskStorage) Task{ taskFunction, data };
			}
			else
			{
				return nullptr;
			}
		}

		template<typename Data>
		Task* createTaskAsChild(TaskFunction taskFunction, const Data& data, Task* parent)
		{
			auto* taskStorage = allocate();

			if (taskStorage != nullptr)
			{
				return new(taskStorage) Task{ taskFunction, data, parent };
			}
			else
			{
				return nullptr;
			}
		}

		template<typename Function>
		Task* createClosureTask(Function function)
		{
			return tasks::closure(allocate(), function);
		}

		template<typename Function>
		Task* createClosureTaskAsChild(Function function, Task* parent)
		{
			return tasks::closure(allocate(), function, parent);
		}

		Task* next() {
			return &_storage[_head];
		}

		void clear();
		std::size_t tasks() const;
		std::size_t maxTasks() const;
		float tasksFactor() const;
		bool full() const;

	private:
		std::vector<Task> _storage;
		std::size_t _allocatedTasks;
		std::size_t _head;
	};
}
#endif
