#include <condition_variable>
#include <deque>
#include <mutex>
#include <functional>

namespace Parallel
{
	template<typename T>
	class concurrent_deque
	{
		std::deque<T> _deque;
		mutable std::mutex _mutex;
		std::condition_variable _emptyCondition;
	public:
		void PushBack(T Value);
		T PopFront();
		T TryPopFront();
		void PushFront(T Value);
		T PopBack();
		T TryPopBack();
		bool Empty() const;
	};

	template<typename T>
	void concurrent_deque<T>::PushBack(T Value)
	{
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_deque.push_back(Value);
		}
		_emptyCondition.notify_one();
	}

	template<typename T>
	T concurrent_deque<T>::PopFront()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_emptyCondition.wait(lock, [this]{return !Empty();});
		T ret = _deque.front();
		_deque.pop_front();
		return ret;
	}

	template<typename T>
	T concurrent_deque<T>::TryPopFront()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if(Empty())
		{
			return T();
		}
		T ret = _deque.front();
		_deque.pop_front();
		return ret;		
	}
	
	template<typename T>
	void concurrent_deque<T>::PushFront(T Value)
	{
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_deque.push_front(Value);
		}
		_emptyCondition.notify_one();
	}

	template<typename T>
	T concurrent_deque<T>::PopBack()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_emptyCondition.wait(lock, [this]{return !Empty();});
		T ret = _deque.back();
		_deque.pop_back();
		return ret;
	}

	template<typename T>
	T concurrent_deque<T>::TryPopBack()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if(Empty())
		{
			return T();
		}
		T ret = _deque.back();
		_deque.back_front();
		return ret;		
	}

	template<typename T>
	bool concurrent_deque<T>::Empty() const
	{
		return _deque.empty();
	}
}
