#pragma once
#include <vector>
#include "Job.h"

//one pool per worker NOT THREAD SAFE

class Pool
{
public:
	Pool(std::size_t maxJobs);

	Job* allocate();
	bool full() const;
	void clear();

	Job* createJob(JobFunction jobFunction);
	Job* createJobAsChild(JobFunction jobFunction, Job* parent);

	template<typename Data>
	Job* createJob(JobFunction jobFunction, const Data& data);
	template<typename Data>
	Job* createJobAsChild(JobFunction jobFunction, const Data& data, Job*
		parent);
	template<typename Function>
	Job* createClosureJob(Function function);
	template<typename Function>
	Job* createClosureJobAsChild(Function function, Job* parent);

private:
	std::size_t _allocatedJobs;
	std::vector<Job> _storage;
};