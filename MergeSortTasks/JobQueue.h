#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#pragma once
#include "Job.h"
#include <vector>

class JobQueue {
public:
	JobQueue(std::size_t maxJobs);

	bool push(Job* job);
	Job* pop();
	Job* steal();
	std::size_t size() const;
	bool empty() const;

private:
	std::vector<Job*> _jobs;
	std::atomic<std::size_t> _top, _bottom;
};

#endif