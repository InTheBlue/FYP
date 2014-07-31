#include <condition_variable>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace Parallel
{
	template<typename KeyType, typename ValType>
	class concurrent_map
	{
		std::unordered_map<KeyType, ValType> _unordered_map;
		mutable std::mutex _mutex;
		std::condition_variable _emptyCondition;
	public:
		concurrent_map(){};
		concurrent_map(concurrent_map<KeyType, ValType>&& map) //move constructor
		{
			_unordered_map.swap(map._unordered_map);
		};
		concurrent_map<KeyType, ValType>& operator=(concurrent_map<KeyType, ValType>&& map)
		{
			_unordered_map.swap(map._unordered_map);
			return this;
		};
		void SetItem(KeyType Key, ValType Val);
		ValType GetItem(KeyType Key);
		size_t Size() const;
		auto Begin();
		auto End();
	};
	
	template<typename KeyType, typename ValType>
	void concurrent_map<KeyType, ValType>::SetItem(KeyType Key, ValType Val)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_unordered_map[Key] = Val;	
	}

	template<typename KeyType, typename ValType>
	ValType concurrent_map<KeyType, ValType>::GetItem(KeyType Key)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _unordered_map[Key];
	}

	template<typename KeyType, typename ValType>
	size_t concurrent_map<KeyType, ValType>::Size() const
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _unordered_map.size();
	}

	template<typename KeyType, typename ValType>
	auto concurrent_map<KeyType, ValType>::Begin()
	{
		return _unordered_map.begin();
	}

	template<typename KeyType, typename ValType>
	auto concurrent_map<KeyType, ValType>::End()
	{
		return _unordered_map.end();
	}
}
