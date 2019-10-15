#include "Job.h"

void Job::run()
{
	auto jobFunction = _jobFunction;
	_jobFunction(*this);

	if (_jobFunction == jobFunction) {
		//has no callback
		_jobFunction = nullptr;
	}
	finish();
}

void Job::finish()
{
	_unfinishedJobs--;

	if (finished())
	{
		if (_parent != nullptr)
		{
			_parent->finish();
		}
		if (_jobFunction != nullptr) {
			_jobFunction(*this);
		}
	}
}

Job::Job(JobFunction jobFunction, Job* parent) :
	_jobFunction{ jobFunction },
	_parent{ parent },
	_unfinishedJobs{ 1 } // 1 means **this** job has not been run
{
	if (_parent != nullptr)
	{
		_parent->_unfinishedJobs++;
	} 
}

bool Job::finished() const
{
	return _unfinishedJobs == 0;
}