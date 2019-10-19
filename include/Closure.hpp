#ifndef TASKSYSTEM_CLOSURE_H
#define TASKSYSTEM_CLOSURE_H

#include "Task.hpp"

namespace TaskSystem {
	template<typename Function>
	class Closure : public Function
	{
	public:
		Closure(Function function) :
			Function{ std::move(function) }
		{}

		template<typename... Args>
		void run(Task& task, Args&& ... args)
		{
			(*this)(task, std::forward<Args>(args)...);
		}
	};

	template<typename Function>
	Task* closure(Task* task, Function function, Task* parent = nullptr)
	{
		if (task == nullptr)
		{
			return nullptr;
		}

		// What this function does is to inject a non-pod payload
		// into a previously allocated task. Since tasks are PODs, we must
		// manually destroy the payload after running the task. This is safe
		// as long as there are no other C++ semantics involved (copy, move,
		// etc) which is true for our preallocated array of POD tasks

		auto taskFunction = [](Task& task)
		{
			if constexpr (sizeof(Closure<Function>) <= Task::maxDataSize())
			{
				auto& closure = task.getData<Closure<Function>>();
				closure.run(task);

				// Install a finished callback to destroy the closure
				// when the task is marked as finished. This allows any child task
				// to capture references to the parent closure
				task.whenFinished([](Task& task)
					{
						rt::utils::destroy(task.getData<Closure<Function>>());
					});
			}
			else
			{
				auto& closure = task.getData<std::unique_ptr<Closure<Function>>>();
				closure->run(task);

				// Install a finished callback to destroy the closure
				// when the task is marked as finished. This allows any child task
				// to capture references to the parent closure
				task.whenFinished([](Task& task)
					{
						rt::utils::destroy(task.getData<std::unique_ptr<Closure<Function>>>());
					});
			}
		};

		// Initialize the allocated task:
		rt::utils::construct<Task>(task, taskFunction, parent);

		if constexpr (sizeof(Closure<Function>) <= Task::maxDataSize())
		{
			// Construct the closure in the task payload:
			task->constructData<Closure<Function>>(function);
		}
		else
		{
			static_assert(sizeof(Function) != sizeof(Function));
			// The closure object does not fit in the task payload,
			// dynamically allocate it:
			task->constructData<std::unique_ptr<Closure<Function>>>(std::make_unique<Closure<Function>>(function));
		}

		return task;
	}

}



#endif // TASKSYSTEM_CLOSURE_H