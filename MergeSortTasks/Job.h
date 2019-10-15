#pragma once
#include <atomic>
#include <array>
#include <type_traits>


//src: https://blog.molecular-matters.com/2015/08/24/job-system-2-0-lock-free-work-stealing-part-1-basics/

//and https://manu343726.github.io/2017-03-13-lock-free-job-stealing-task-system-with-modern-c/

typedef void (*JobFunction)(Job&);

class Job
{


private:
	void(*_jobFunction)(Job&);
	Job* _parent;
	std::atomic_size_t _unfinishedJobs;

	void finish();

	//align task with cache boundary
	static constexpr std::size_t JOB_PAYLOAD_SIZE = sizeof(_jobFunction)
		+ sizeof(_parent)
		+ sizeof(_unfinishedJobs);


	static constexpr std::size_t JOB_MAX_PADDING_SIZE = 64; //https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
	static constexpr std::size_t JOB_PADDING_SIZE = JOB_MAX_PADDING_SIZE - JOB_PAYLOAD_SIZE;

	std::array<unsigned char, JOB_PADDING_SIZE> _padding;


public:

	//handle non-"plain old data" objects
	template<typename T, typename... Args>
	void constructData(Args&&... args) {
		//https://en.cppreference.com/w/cpp/utility/forward
		//
		new(_padding.data())(std::forward<Args>(args)...);
	}


	template<typename Data>
	std::enable_if_t < std::is_pod<Data>::value && (sizeof(Data) <= JOB_PADDING_SIZE)>
	setData(const Data& data) {
		std::memcpy(_padding.data(), &data, sizeof(Data));
	}
	
	template<typename Data>
	const Data& getData() const
	{
		return *reinterpret_cast<const Data*>(_padding.data());
	}




	template<typename Data>
	Job(JobFunction jobFunction, const Data& data, Job* parent = nullptr) : Job{ jobFunction, parent }
	{
		setData(data);
	}

	Job(JobFunction jobFunction, Job* parent = nullptr);
	Job() = default;

	void run();
	bool finished() const;

	void onFinished(JobFunction callback) {
		_jobFunction = callback;
	}


};


//run lambda functions with non POD objects
template<typename Function>
void closure(Job* job, Function function)
{
	auto jobFunction = [](Job& job)
	{
		const auto& function = job.getData<Function>();

		function(job);

		//destroy bound object after running job
		function.~Function();
	}

	new(job) Job{ jobFunction };
	job.constructData<Function>(function);
}