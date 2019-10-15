#pragma once
#include <mutex>

template <typename T>
class ObjectPool {
public:


	ObjectPool(int initialSize, bool resizeAble = true) {
		size = initialSize;
		objects = new Object[size];
		this->resizeAble = resizeAble;
	}

	~ObjectPool() {
		delete[] objects;
	}


	struct SmartPtr {

		explicit SmartPtr(T* p) {
			ptr = p;
		}
		SmartPtr(const T* p) {
			ptr = p;
		}
		~SmartPtr() {
			ptr->inUse = false;
		}
		T& operator * () {
			return *ptr;
		}

		T* operator -> () {
			return ptr;
		}
		

		private:
			T* ptr;
	};

	struct Iterator {
		Iterator(ObjectPool* op) {
			pool = op;
		}
		T* next() {
			if (hasObject()) {
				return &pool->objects[index++].object;
			}
			else {
				return nullptr;
			}
		}
		
		bool hasObject() {
			while (index < pool->size) {
				if (pool->objects[index].inUse) {
					return true;
				}
				index++;
			}
			return false;
		}
		ObjectPool* pool;
		int index;
	};


	Iterator getIterator() {
		return Iterator(this);
	}
;

	void free(T* ptr) {
		ptr->inUse = false;
	}

	T* create() {
		mtx.lock();
		for (int i = 0; i < size; i++) {
			if (!objects[i]->inUse) {
				objects[i]->inUse = true;
				mtx.unlock();
				return objects + i;
				
			}
		}
		mtx.unlock();
		if (resizeAble) {
			resize(size * 2);
			return create();
		}
	}

	T* create(T o) {
		T* p = create();
		*p = o;
		return p;
	}


	private:

		struct Object {
			T object;
			bool inUse;
		};

	void resize(int s) {
		mtx.lock();
		Object* newObjects = realloc(objects, s);
		size = s;
		if (!newObjects) {
			throw "Failed to resize";
		}
		else {
			objects = newObjects;
		}
		mtx.unlock();
	}

	bool resizeAble;
	int size;

	std::mutex mtx;

	Object* objects;
};