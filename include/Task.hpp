#ifndef TASKSYSTEM_TASK_H
#define TASKSYSTEM_TASK_H

#pragma once
#include <utility>
#include <atomic>
#include <type_traits>
#include <cstdint>
#include <cstring>
#include <thread>
#include <new>

//src: https://blog.molecular-matters.com/2015/08/24/task-system-2-0-lock-free-work-stealing-part-1-basics/

//and https://manu343726.github.io/2017-03-13-lock-free-task-stealing-task-system-with-modern-c/

namespace TaskSystem {

	class Task;

	using TaskFunction = void(*)(Task& task);



	/**
	 Represents a unit of work to be executed by the system
	 
	 Work is submitted to the system in the form of Tasks. A Task basically
	 stores a pointer to the function implementing the work. When the task
	 is scheduled the function will be invoked in the thread of the worker
	 that picked the task. The task is considered done if its function has been executed
	 and **all its child tasks are done too** (See `Task::finished()`).
	 
	 Users can wait for the completion of a task by calling `Engine::wait()`.
	 
	 Finally, user defined POD data can be associated to the task during its construction,
	 which is available later by calling `Task::getData()`. For non-POD data, see `tasks::closure()`.
	 */
	class Task
	{
	public:
		/**
		 * \brief Default constructs a task. Used by pool pre-allocation only.
		 */
		Task() = default;

		/**
		 Initializes a task given the function to execute and a parent task
		 
		 * \param taskFunction Function to be executed when the task is processed. Must be
		 * a pointer to function of signature `void(*)(Task&)`. See `TaskFunction`.
		 * \param parent Parent task, if any. When a task is associated to an existing parent
		 * one, the parent will not be considered finished until the child task is finished first.
		 * This allows to implement fork-join by simply associating multiple tasks to a parent task,
		 * and waiting for the finalization of the parent. If nullptr the task has no parent task.
		 */
		Task(TaskFunction taskFunction, Task* parent = nullptr);

		/**
		 * \brief Initializes a task given a function to execute, data associated with the task, and
		 * a parent task
		 *
		 * \tparam Data Must be a POD type
		 * \param taskFunction Function to be executed when the task is processed
		 * \param data Data associated with the task. The data is memcpy-ed into the
		 * task, and can be accessed later using `Task::getData()`
		 * \param parent Parent task. If nullptr the ob has no associated parent task
		 */
		template<typename Data>
		Task(TaskFunction taskFunction, const Data& data, Task* parent = nullptr) :
			Task{ taskFunction, parent }
		{
			setData(data);
		}

		/**
		 * \brief Executes the task function in the caller thread.
		 *
		 * Invokes the task function from the caller thread, passing itself as argument
		 * for the task function. After executing the function, tries to mark the task as finished.
		 */
		bool run();

		/**
		 * \brief Checks whether the task and all its child tasks have been run
		 */
		bool finished() const;

		/**
		 * \brief Returns the number of children unfinished tasks left
		 */
		std::int32_t unfinishedChildrenTasks() const;

		/**
		 * \brief Returns the parent of the task
		 *
		 * \returns A pointer to the parent task, nullptr if the task has no parent
		 */
		Task* parent() const;

		/**
		 Returns the function associated with the task
		 */
		TaskFunction function() const;

		/**
		 * \brief Discards the task, marking it as processed even if the task function
		 * has not been executed.
		 */
		void discard();

		/**
		 * \brief Changes the task function so an action is executed when the
		 * task is marked as finished. Invoking this method outside the task function
		 * has undefined behavior.
		 *
		 * \param taskFunction Function that will be invoked when the task finishes
		 */
		void whenFinished(TaskFunction taskFunction);

		std::uintptr_t id() const;

	private:
		struct Payload
		{
			TaskFunction function;
			Task* parent;
			std::atomic<std::int32_t> unfinishedChildrenTasks;
		};

		Payload _payload;

		static constexpr const std::size_t TASK_PAYLOAD_SIZE = sizeof(Payload);
		static constexpr const std::size_t TASK_MAX_PADDING_SIZE = 64 * 2;
		static constexpr const std::size_t TASK_PADDING_SIZE = TASK_MAX_PADDING_SIZE - TASK_PAYLOAD_SIZE;
		static_assert(TASK_PAYLOAD_SIZE < TASK_MAX_PADDING_SIZE, "Task payload does not fit in a cache line");

		char _padding[TASK_PADDING_SIZE];

		void finish();
		void incrementUnfinishedChildrenTasks();
		bool decrementUnfinishedChildrenTasks();

		template<typename Data>
		std::enable_if_t<std::is_pod<Data>::value && sizeof(Data) <= TASK_PADDING_SIZE>
			setData(const Data & data)
		{
			std::memcpy(&_padding[0], &data, sizeof(Data));
		}

	public:
		/**
		 * \brief Returns a const reference to the data associated to the task
		 *
		 * \tparam Data Must be the same type passed to the task constructor. The behavior
		 * is undefined if this type is different.
		 *
		 * The behavior is undefined if this function is invoked on tasks with no
		 * data associated
		 */
		template<typename Data>
		const Data& getData() const
		{
			static_assert(sizeof(Data) <= TASK_PADDING_SIZE, "Objects of that type do not fit in "
				"the task data storage");

			return *reinterpret_cast<const Data*>(data());
		}

		/**
		 * \brief Returns a non-const reference to the data associated to the task
		 *
		 * \tparam Data Must be the same type passed to the task constructor. The behavior
		 * is undefined if this type is different.
		 *
		 * The behavior is undefined if this function is invoked on tasks with no
		 * data associated
		 */
		template<typename Data>
		Data& getData()
		{
			static_assert(sizeof(Data) <= TASK_PADDING_SIZE, "Objects of that type do not fit in "
				"the task data storage");

			return *reinterpret_cast<Data*>(data());
		}

		/**
		 * \brief Returns the storage address of the data associated with the task
		 */
		void* data();

		/**
		 * \brief Returns the storage address of the data associated with the task
		 */
		const void* data() const;

		/**
		 * \brief Constructs an object in the data storage of the task
		 *
		 * This function constructs in place an object of the given type
		 * at the beginning of the task data storage. Users must manually
		 * invoke object destructor later.
		 *
		 * \tparam T Type of the object that will be constructed
		 * \param args Constructor arguments
		 */
		template<typename T, typename... Args>
		void constructData(Args&& ... args)
		{
			static_assert(sizeof(T) <= TASK_PADDING_SIZE, "Objects of that type do not fit in "
				"the task data storage");

			new(data()) T{ std::forward<Args>(args)... };
		}

		/**
		 * \brief Returns the maximum size of the data that can be associated with a task
		 */
		static constexpr std::size_t maxDataSize()
		{
			return TASK_PADDING_SIZE;
		}
	};
}
//static_assert(std::is_pod<Task>::value, "Task type must be a POD");


#endif