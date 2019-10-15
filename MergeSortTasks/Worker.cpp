
#include "Worker.h"
#include "Engine.h"


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
}

Worker::~Worker() {}