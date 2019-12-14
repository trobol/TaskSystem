#include "../include/Engine.hpp"
#include "../include/Worker.hpp"

#define TEST_COUNT 100
#define ITERATIONS 100

using namespace TaskSystem;


void test(Task &task) {
	for (int i = 0; i < ITERATIONS; i++) {

	}
}

int main() {
	Worker* worker = Engine::Instance().threadWorker();
	for (int i = 0; i < TEST_COUNT; i++) {
	
		Task* task = worker->pool().createTask(test);
		worker->submit(task);
		worker->wait(task);
	}
}