#include <exception>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
namespace Parallel
{
	template<typename IndexType, typename Container, typename Func>
	void For(IndexType Start, IndexType End, const Container Input, Func F)
	{
		std::vector<std::thread> Workers;
		std::queue<std::function<void()>> Tasks;
		std::mutex Mutex;
		std::mutex OutputMutex;
		size_t ThreadCount = std::thread::hardware_concurrency();
		// ThreadCount can be 0 if number of hardware threads is not computable or well-defined
		// Gracefully degrade to serial operation
		if(ThreadCount == 0)
		{
			ThreadCount = 1;
		}
		
		for(;Start < End; Start++)
		{
			auto in = Input[Start];
			Tasks.push([=]{
				F(in);
			});
		}
		
		for(size_t i = 0; i < ThreadCount; i++)
		{
			Workers.push_back(std::thread([&]{
				while(true)
				{
					std::function<void()> f;
					Mutex.lock();
					if(Tasks.empty())
					{
						Mutex.unlock();
						return;
					}
					if(!Tasks.empty())
					{
						f = Tasks.front();
						Tasks.pop();
					}
					Mutex.unlock();
					try
					{
						if(f)
						{
							f();
						}
					}
					catch(std::exception& e)
					{
						OutputMutex.lock();
						cerr<<e.what()<<endl;
						OutputMutex.unlock();
					}
				}
			}));
		}
		for(auto it = Workers.begin(); it != Workers.end(); it++)
		{
			it->join();
		}
	}
	
	template<typename Iterator, typename Func>
	void Foreach(Iterator Start, Iterator End, Func F)
	{
		std::vector<std::thread> Workers;
		std::queue<std::function<void()>> Tasks;
		std::mutex Mutex;
		std::mutex OutputMutex;
		size_t ThreadCount = std::thread::hardware_concurrency();
		// ThreadCount can be 0 if number of hardware threads is not computable or well-defined
		// Gracefully degrade to serial operation
		if(ThreadCount == 0)
		{
			ThreadCount = 1;
		}
		
		for(;Start != End; Start++)
		{
			Tasks.push([F, Start]{F(*Start);});
		}
		
		for(size_t i = 0; i < ThreadCount; i++)
		{
			Workers.push_back(std::thread([&]{
				while(true)
				{
					std::function<void()> f;
					Mutex.lock();
					if(Tasks.empty())
					{
						Mutex.unlock();
						return;
					}
					if(!Tasks.empty())
					{
						f = Tasks.front();
						Tasks.pop();
					}
					Mutex.unlock();
					try
					{
						if(f)
						{
							f();
						}
					}
					catch(std::exception& e)
					{
						OutputMutex.lock();
						cerr<<e.what()<<endl;
						OutputMutex.unlock();
					}
				}
			}));
		}
		for(auto it = Workers.begin(); it != Workers.end(); it++)
		{
			it->join();
		}
	}
}
