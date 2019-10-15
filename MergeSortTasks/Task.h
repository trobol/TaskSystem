#pragma once
#include <vector>
#include <mutex>
#include "ObjectPool.h"
#include <stack>



class Task {
public:
	void* function;
	void* parameters;
	template<typename F, typename... Args>
	Task() {
		
	}

	//the tasks that this task relies on
	std::vector<Task*> dependants;
	int dependancies = 0;

	bool dependancyComplete() {
		--dependancies;
		if (dependancies <= 0)
			TaskManager::Instance()->readyTasks.push(this);
	}

	void onComplete() {
		for (int i = 0; i < dependants.size(); i++) {
			dependants[i]->dependancyComplete();
		}
	}
};


class TaskManager {
public:
	static TaskManager* Instance() {
		if (!m_pInstance)
			m_pInstance = new TaskManager();
		return m_pInstance;
	}
	Task getNext() {
		
	}
	std::stack<ObjectPool<Task>::SmartPtr> readyTasks;

	void add(Task& task) {
	
	}

private:
	TaskManager() {};
	TaskManager(TaskManager const&) {};
	TaskManager& operator=(TaskManager const&);
	std::mutex mtx;
	ObjectPool<Task> pool = ObjectPool<Task>(10);
	static TaskManager* m_pInstance;
};