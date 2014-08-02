#include <atomic>
#include <exception>
#include <functional>
#include <iterator>
#include <mutex>
#include <queue>
#include <deque>
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

	//Chunking code from http://stackoverflow.com/a/14227282/1514535
	template <typename It>
	std::vector<std::pair<It,It>> chunk(It range_from, It range_to, const std::ptrdiff_t num)
	{
		/* Aliases, to make the rest of the code more readable. */
		using std::vector;
		using std::pair;
		using std::make_pair;
		using std::distance;
		using diff_t = std::ptrdiff_t;
		/* Total item number and portion size. */
		const diff_t total
		{ distance(range_from,range_to) };
		
		const diff_t portion
		{ total / num };
		
		vector<pair<It,It>> chunks(num);

		It portion_end
		{ range_from };

		/* Use the 'generate' algorithm to create portions. */    
		std::generate(begin(chunks),end(chunks),[&portion_end,portion]()
		{
			It portion_start
			{ portion_end };
	
	        portion_end += portion;
			return make_pair(portion_start,portion_end);
		});

		/* The last portion's end must always be 'range_to'. */    
		chunks.back().second = range_to;

		return chunks;
	}
	
	template<typename Iterator, typename Func>
	void ForeachChunking(Iterator Start, Iterator End, Func F, size_t grainSize = 4)
	{
		std::vector<std::thread> Workers;
		concurrent_deque<std::deque<std::function<void()>>> Tasks;
		std::atomic<bool> joinWhenEmpty(false);
		std::mutex OutputMutex;
		size_t ThreadCount = std::thread::hardware_concurrency();
		// ThreadCount can be 0 if number of hardware threads is not computable or well-defined
		// Gracefully degrade to serial operation
		if(ThreadCount == 0)
		{
			ThreadCount = 1;
		}

		size_t numOfChunks = ThreadCount * grainSize;
		auto chunks = chunk(Start, End, numOfChunks);
		for(auto chunkit = chunks.begin(); chunkit != chunks.end(); chunkit++)
		{
			std::deque<std::function<void()>> d;
			for(auto it = (*chunkit).first;it != (*chunkit).second; it++)
			{
				d.push_back([=]{F(*it);});
			}
			Tasks.PushBack(d);
		}

		// for(;Start != End; Start++)
		// {
		// 	Tasks.PushBack([F, Start]{F(*Start);});
		// }
		for(size_t i = 0; i < ThreadCount; i++)
		{
			Workers.push_back(std::thread([&]{
				while(true)
				{
					std::deque<std::function<void()>> work;
					std::function<void()> f;
					if(work.empty() && joinWhenEmpty)
					{
						return;
					}
					if(!work.empty())
					{
						f = work.front();
						work.pop_front();
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
	auto MapChunking( Iter Start, Iter End, Func F )
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
