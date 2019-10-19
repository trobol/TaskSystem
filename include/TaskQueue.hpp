#ifndef TASKSYSTEM_TASKQUEUE_H
#define TASKSYSTEM_TASKQUEUE_H

#pragma once
#include "Task.hpp"
#include <vector>

namespace TaskSystem {
	class TaskQueue {
	public:
		TaskQueue(std::size_t maxTasks);

		bool push(Task* task);
		Task* pop();
		Task* steal();
		std::size_t size() const;
		bool empty() const;

	private:
		std::vector<Task*> _tasks;
		std::atomic<std::size_t> _top, _bottom;
	};
}
#endif