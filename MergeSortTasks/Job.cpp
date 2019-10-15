#include "Job.h"

bool Job::run()
{
	if (finished())
	{
		return false;
	}

	JobFunction jobFunction = _jobFunction;

	if (jobFunction != nullptr)
	{
		jobFunction(*this);


		// When the job is marked as finished, we run the job function
		// again as a callback with teardown work to run when the job is
		// marked as finished. To do so, the user reassigns the job function
		// by calling Job::whenFinished() in the body of the job function
		// (When the job is run). We later check if the job function has changed,
		// and mark a job with the same previous function, that is, a job with no
		// custom whenFinished() callback assigned during run; as having null function
		// Later, Job::finish() executes the function again only if not null
		if (_jobFunction == jobFunction)
		{
			_jobFunction = nullptr;
		}

		finish();
	}

	return true;
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