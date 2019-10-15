#include "Pool.h"
#include "Job.h"


Pool::Pool(std::size_t maxJobs) :
	_allocatedJobs{ 0 },
	_storage{ maxJobs }
{
	//_sortage.size() should equal maxJobs
}

Job* Pool::allocate()
{
	if (!full())
	{
		return &_storage[_allocatedJobs++];
	}
	else
	{
		return nullptr;
	}
}

void Pool::clear()
{
	_allocatedJobs = 0;
}

bool Pool::full() const
{
	return _allocatedJobs == _storage.size();
}

Job* Pool::createJob(JobFunction jobFunction) {
	Job* job = allocate();

	if (job != nullptr) {
		new(job) Job{ jobFunction };
		return job;
	}
	else {
		return nullptr;
	}
}