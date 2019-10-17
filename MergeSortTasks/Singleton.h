#ifndef SINGLETON_H
#define SINGLETON_H
#pragma once

class Singleton {
public:
	static Singleton Instance();
private:
	Singleton() {};
	static Singleton _instance;
};


#endif // !SINGLETON_H


