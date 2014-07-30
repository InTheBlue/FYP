#include <atomic>
#include <exception>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "ConcurrentDeque.h"
namespace Parallel
{
	template<typename IndexType, typename Container, typename Func>
	void For(IndexType Start, IndexType End, const Container Input, Func F)
	{
		std::vector<std::thread> Workers;
		concurrent_deque<std::function<void()>> Tasks;
		std::mutex OutputMutex;
		size_t ThreadCount = std::thread::hardware_concurrency();
		std::atomic<bool> joinWhenEmpty(false);
		// ThreadCount can be 0 if number of hardware threads is not computable or well-defined
		// Gracefully degrade to serial operation
		if(ThreadCount == 0)
		{
			ThreadCount = 1;
		}
		for(size_t i = 0; i < ThreadCount; i++)
		{
			Workers.push_back(std::thread([&]{
				while(true)
				{
					std::function<void()> f;
					if(Tasks.Empty() && joinWhenEmpty)
					{
						return;
					}
					if(!Tasks.Empty())
					{
						f = Tasks.PopFront();
					}
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
		for(;Start < End; Start++)
		{
			auto in = Input[Start];
			Tasks.PushBack([=]{
				F(in);
			});
		}
		joinWhenEmpty = true;
		for(auto it = Workers.begin(); it != Workers.end(); it++)
		{
			it->join();
		}
	}
	
	template<typename Iterator, typename Func>
	void Foreach(Iterator Start, Iterator End, Func F)
	{
		std::vector<std::thread> Workers;
		concurrent_deque<std::function<void()>> Tasks;
		std::atomic<bool> joinWhenEmpty(false);
		std::mutex OutputMutex;
		size_t ThreadCount = std::thread::hardware_concurrency();
		// ThreadCount can be 0 if number of hardware threads is not computable or well-defined
		// Gracefully degrade to serial operation
		if(ThreadCount == 0)
		{
			ThreadCount = 1;
		}
		for(size_t i = 0; i < ThreadCount; i++)
		{
			Workers.push_back(std::thread([&]{
				while(true)
				{
					std::function<void()> f;
					if(Tasks.Empty() && joinWhenEmpty)
					{
						return;
					}
					if(!Tasks.Empty())
					{
						f = Tasks.PopFront();
					}
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
		for(;Start != End; Start++)
		{
			Tasks.PushBack([F, Start]{F(*Start);});
		}
		joinWhenEmpty = true;
		for(auto it = Workers.begin(); it != Workers.end(); it++)
		{
			it->join();
		}
	}
}
