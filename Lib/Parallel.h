#include <atomic>
#include <exception>
#include <functional>
#include <iterator>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "ConcurrentDeque.h"
#include "ConcurrentMap.h"
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
		for(;Start < End; Start++)
		{
			auto in = Input[Start];
			Tasks.PushBack([=]{
				F(in);
			});
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
						f = Tasks.TryPopFront();
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
		for(;Start != End; Start++)
		{
			Tasks.PushBack([F, Start]{F(*Start);});
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
						f = Tasks.TryPopFront();
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
		joinWhenEmpty = true;
		for(auto it = Workers.begin(); it != Workers.end(); it++)
		{
			it->join();
		}
	}

	template< class Iter, class Func>
	auto Map( Iter Start, Iter End, Func F )
	    -> concurrent_map<typename Iter::value_type, decltype(F(*Start))>
	{
		std::vector<std::thread> Workers;
		concurrent_deque<std::function<void()>> Tasks;
		std::atomic<bool> joinWhenEmpty(false);
		std::mutex OutputMutex;
		size_t ThreadCount = std::thread::hardware_concurrency();
		
		concurrent_map<typename Iter::value_type, decltype(F(*Start))> Output;
		

		// ThreadCount can be 0 if number of hardware threads is not computable or well-defined
		// Gracefully degrade to serial operation
		if(ThreadCount == 0)
		{
			ThreadCount = 1;
		}
		for(;Start != End; Start++)
		{
			Tasks.PushBack([F, Start, &Output]{Output.SetItem(*Start, F(*Start));});
		}
		for(size_t i = 0; i < ThreadCount; i++)
		{
			Workers.push_back(std::thread([&]{
				while(true)
				{
					if(Tasks.Empty() && joinWhenEmpty)
					{
						return;
					}
					if(!Tasks.Empty())
					{
						auto f = Tasks.TryPopFront();
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
				}
			}));
		}
		joinWhenEmpty = true;
		for(auto it = Workers.begin(); it != Workers.end(); it++)
		{
			it->join();
		}
		return Output;
	}
}
