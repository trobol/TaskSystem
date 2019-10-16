#ifndef WORK_H
#define WORK_H

#pragma once

#include "Job.hpp"
#include "Pool.h"
#include <thread>
#include "JobQueue.h"


class Engine;
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

	Worker(
		const std::uint64_t id,
		Engine* engine,
		std::size_t         poolSize,
		Worker::Mode        mode);

	~Worker();

	void start();
	void stop();
	void join();

	bool running() const;
	void run();

	Pool& pool();
	const Pool& pool() const;

	void submit(Job* job);
	void wait(Job* job);
	
	std::uint64_t id() const;
	std::thread::id threadId() const;


	const std::atomic<State>& state() const;
	std::size_t totalJobsRun() const;
	std::size_t totalJobsDiscarded() const;
	std::size_t cyclesWithoutJobs() const;
	std::size_t maxCyclesWithoutJobs() const;

private:

	Pool _pool;
	Engine* _engine;
	JobQueue _queue;

	std::thread _thread;
	std::thread::id _threadId;
	
	
	Mode _mode;
	std::atomic<State> _state;

	std::size_t _totalJobsRun;
	std::size_t _totalJobsDiscarded;
	std::size_t _cyclesWithoutJobs;
	std::size_t _maxCyclesWithoutJobs;
	std::uint64_t _id;


	Job* getJob();
	void getJobs();


};

#endif