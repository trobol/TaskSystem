#include "../include/Task.hpp"

namespace TaskSystem {
	Task::Task(TaskFunction taskFunction, Task* parent) :
		_payload{
			taskFunction,
			parent
	}
	{
		_payload.unfinishedChildrenTasks.store(1, std::memory_order_seq_cst);

		if (_payload.parent != nullptr)
		{
			_payload.parent->incrementUnfinishedChildrenTasks();
		}
	}

	bool Task::run()
	{
		if (finished())
		{
			return false;
		}

		TaskFunction taskFunction = _payload.function;

		if (taskFunction != nullptr)
		{
			taskFunction(*this);


			// When the task is marked as finished, we run the task function
			// again as a callback with teardown work to run when the task is
			// marked as finished. To do so, the user reassigns the task function
			// by calling Task::whenFinished() in the body of the task function
			// (When the task is run). We later check if the task function has changed,
			// and mark a task with the same previous function, that is, a task with no
			// custom whenFinished() callback assigned during run; as having null function
			// Later, Task::finish() executes the function again only if not null
			if (_payload.function == taskFunction)
			{
				_payload.function = nullptr;
			}

			finish();
		}

		return true;
	}

	bool Task::finished() const
	{
		return _payload.unfinishedChildrenTasks.load(std::memory_order_seq_cst) == 0;
	}

	void Task::incrementUnfinishedChildrenTasks()
	{
		_payload.unfinishedChildrenTasks.fetch_add(1, std::memory_order_seq_cst);
	}

	bool Task::decrementUnfinishedChildrenTasks()
	{
		return _payload.unfinishedChildrenTasks.fetch_sub(1, std::memory_order_seq_cst) == 1;
	}

	std::int32_t Task::unfinishedChildrenTasks() const
	{
		return _payload.unfinishedChildrenTasks.load(std::memory_order_seq_cst);
	}

	Task* Task::parent() const
	{
		return _payload.parent;
	}

	void Task::finish()
	{
		if (decrementUnfinishedChildrenTasks())
		{
			if (_payload.function != nullptr)
			{
				// If the task function was left as a whenFinished() callback,
				// execute it
				_payload.function(*this);
			}

			if (_payload.parent != nullptr)
			{
				_payload.parent->finish();
			}
		}
	}

	void Task::discard()
	{
		finish();
	}

	void* Task::data()
	{
		return &_padding[0];
	}

	void Task::whenFinished(TaskFunction taskFunction)
	{
		_payload.function = taskFunction;
	}

	TaskFunction Task::function() const
	{
		return _payload.function;
	}

	std::uintptr_t Task::id() const
	{
		return reinterpret_cast<std::uintptr_t>(this);
	}


	/*
	requirements:
		Every copy constructor is trivial or deleted true

		Every move constructor is trivial or deleted true

		Every copy assignment operator is trivial or deleted true

		Every move assignment operator is trivial or deleted

		at least one copy constructor, move constructor, copy assignment operator, or move assignment operator is non-deleted

		Trivial non-deleted destructor

		Has one or more default constructors, all of which are either trivial or deleted, and at least one of which is not deleted.

	functions:
		

	*/

	/*
	is_trivially_constructible true
	is_trivially_default_constructible true
	is_trivially_move_constructible true
	is_trivially_copyable no

	is_volatile
	*/
	n_is_trivially_copyable<Task> copyable;


	static_assert(copyable.value, "Failed");
}