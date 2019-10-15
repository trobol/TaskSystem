#include "JobQueue.h"

//adds Job at the bottom
bool JobQueue::push(Job* job) {
	//https://en.cppreference.com/w/cpp/atomic/memory_order
	int bottom = _bottom.load(std::memory_order_acquire);

	if (bottom < static_cast<int>(_jobs.size())) {
		_jobs[bottom] = job;
		_bottom.store(bottom + 1, std::memory_order_release);

		return true;
	}
	else {
		return false;
	}
}

/*
"First, pop() decrements bottom and then reads the current value of top,
this ordering in the reads protects pop() against concurrent calls to steal()
in the meantime (steal() works only if there are jobs to steal left in the range [top, bottom).
If you decrement bottom first you reduce the chance of concurrent steal()s to return jobs).

The most important part of pop() is the initialization of job:
pop() and steal() access to different ends of the queue,
so the only case where they could be fighting for the same job is when only one job is left in the queue.
In that case, both bottom and top point to the same queue slot,
and we have to make sure only one thread is returning this last job.

pop() ensures this doing a nice trick: Before returning the job,
it checks if a concurrent call to steal() happened after we read top,
by doing a CAS to increment top.
If the CAS fails, it means pop() has lost a race against a concurrent steal()call,
returning nullptr Job in that case. If the CAS succeeds,
pop() won and incremented top preventing further steal() calls to extract the last job again.

*/


//decrement bottom, and make sure no concurrent steal() calls are trying to return the same job
Job* JobQueue::pop() {
	int bottom = _bottom.load(std::memory_order_acquire);
	bottom = std::max(0, bottom - 1);
	_bottom.store(bottom, std::memory_order_release);
	int top = _top.load(std::memory_order_acquire);

	if (top <= bottom) {
		Job* job = _jobs[bottom];

		if (top != bottom) {
			//more then one job in the queue
			return job;
		}
		else {
			std::size_t expectedTop = top;
			std::size_t desiredTop = top + 1;
			//https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange
			if (!_top.compare_exchange_weak(expectedTop, desiredTop, std::memory_order_acq_rel)) {
				//someone already took the last item, abort
				job = nullptr;
			}

			_bottom.store(top + 1, std::memory_order_release);
			return job;
		}
	}
	else {
		//Queue already empty
		_bottom.store(top, std::memory_order_release);
		return nullptr;
	}
	
}




Job* JobQueue::steal()
{
	std::size_t top = _top.load(std::memory_order_acquire);

	// Put a barrier here to make sure bottom is read after reading
	// top
	std::atomic_thread_fence(std::memory_order_acquire);

	std::size_t bottom = _bottom.load(std::memory_order_acquire);

	if (top < bottom)
	{
		Job* job = _jobs[top % _jobs.size()];
		const std::size_t nextTop = top + 1;
		std::size_t       desiredTop = nextTop;

		if (!_top.compare_exchange_weak(
			top, desiredTop, std::memory_order_acq_rel))
		{
			// Some concurrent pop()/steal() operation
			// changed the current top
			return nullptr;
		}
		else
		{
			
			return job;
		}
	}
	else
	{
		// The queue is empty
		
		return nullptr;
	}
}

std::size_t JobQueue::size() const
{
	return _bottom.load(std::memory_order_seq_cst) -
		_top.load(std::memory_order_seq_cst);
}

bool JobQueue::empty() const
{
	return size() == 0;
}

