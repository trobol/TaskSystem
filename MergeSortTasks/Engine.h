#ifndef ENGINE_H
#define ENGINE_H

#pragma once

#include "Worker.h"
#include "StaticVector.h"
class Worker;
class Engine {
public:
	Engine(std::size_t threads, std::size_t jobsPerThread);

	Worker* randomWorker();
	Worker* threadWorker();
private:
	StaticVector<Worker> _workers;

	Worker* findThreadWorker(const std::thread::id threadId);

};
#endif // !ENGINE_H

