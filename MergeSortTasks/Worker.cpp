
#include "Worker.h"
#include "Engine.h"


Worker::Worker(
	const std::uint64_t id,
	Engine* engine,
	std::size_t         poolSize,
	Worker::Mode        mode)
	: _queue{ poolSize + 1 },
	_pool{ poolSize },
	_engine{ engine },
	_mode{ mode },
	_state{ State::Idle },
	_totalJobsRun{ 0 },
	_totalJobsDiscarded{ 0 },
	_cyclesWithoutJobs{ 0 },
	_maxCyclesWithoutJobs{ 0 },
	_id{ id }
{
}

void Worker::submit(Job* job) {
	_queue.push(job);
}

void Worker::wait(Job* waitJob) {
	while (!waitJob->finished()) {
		Job* job = getJob();

		if (job != nullptr) {
			job->run();
		}
	}
}


Job* Worker::getJob() {
	//try to get a job from own queue
	Job* job = _queue.pop();

	
	if (job != nullptr) {
		job->run();

	} else {
		//no more jobs
		//try to steal from another worker
		Worker* worker = _engine->randomWorker();

		if (worker != this) {
			Job* job = worker->_queue.steal();

			if (job != nullptr) {
				return job;
			}
			else {
				std::this_thread::yield();
				return nullptr;
			}
		}
		else {
			std::this_thread::yield();
			return nullptr;
		}
	}
}

bool Worker::running() const {
	return _state == State::Running;
}
void Worker::run() {
	while (running())
	{
		Job* job = getJob();

		if (job != nullptr)
		{
			job->run();
		}
	}

	if (running())
	{
		return;
	}

	auto mainLoop = [this] {
		_state = State::Running;


		while (running())
		{
			Job* job = getJob();

			if (job != nullptr)
			{
				if (job->run())
				{
					++_totalJobsRun;
					_cyclesWithoutJobs = 0;
				}
			}
			else
			{
				++_cyclesWithoutJobs;
				_maxCyclesWithoutJobs =
					std::max(_cyclesWithoutJobs, _maxCyclesWithoutJobs);
			}
		}
	};

	if (_mode == Mode::Background)
	{
		_thread = std::thread{ mainLoop };
		_threadId = _thread.get_id();
	}
	else
	{
		_state = State::Running;
		_threadId = std::this_thread::get_id();

	}
}

Worker::~Worker() {}