#pragma once

#include "Job.h"
#include "Pool.h"
#include <thread>
#include "Engine.h"
#include "JobQueue.h"

//a thread and a job pool
class Worker {
public:
	enum class Mode {
		Background, //work on own thread
		Foreground //work on the caller thread
	};

	enum class State {
		Idle,
		Running,
		Stopping
	};

	Worker(Engine* engine, std::size_t maxJobs, Mode mode = Mode::Background);
	~Worker();

	void start();
	void stop();
	bool running() const;
	Pool& pool();
	void submit(Job* job);
	void wait(Job* job);

private:
	Pool _pool;
	Engine* _engine;
	JobQueue _queue;
	std::thread::id _threadId;
	std::thread _thread;
	std::atomic<State> _state;
	std::atomic<Mode> _mode;


	Job* getJob();
	void join();


};