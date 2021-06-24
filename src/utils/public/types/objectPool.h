#pragma once

#include <mutex>
#include <vector>
#include <cpputils/logger.hpp>

template<typename ObjectType>
class IObjectPool
{
public:
	virtual ~IObjectPool() = default;
private:
	virtual void push(std::shared_ptr<ObjectType> object) = 0;
	virtual std::shared_ptr<ObjectType> pop() = 0;
};

template<typename ObjectType, size_t PoolSize>
class TObjectPool : public IObjectPool<ObjectType>
{
public:

	TObjectPool() {
		pool.resize(PoolSize);
	}

	virtual ~TObjectPool() {}

	void for_each(void(*function)(std::shared_ptr<ObjectType>))
	{
		for (size_t i = pool_bottom; i != pool_top; i = (i + 1) % PoolSize)
		{
			function(pool[i]);
		}
	}

	
	virtual void push(std::shared_ptr<ObjectType> object)
	{
		std::lock_guard lock(pool_lock);
		if ((pool_bottom + 1) % PoolSize == pool_top)
		{ LOG_FATAL("object pool overflow : max=%d", PoolSize);
		}
		
		pool[pool_bottom] = object;
		pool_bottom = (pool_bottom + 1) % PoolSize;
	}

	virtual std::shared_ptr<ObjectType> pop()
	{
		std::lock_guard lock(pool_lock);
		if (is_empty()) return nullptr;

		std::shared_ptr<ObjectType> object = pool[pool_top];
		pool[pool_top] = nullptr;
		pool_top = (pool_top + 1) % PoolSize;
		
		return object;
	}

	[[nodiscard]] bool is_empty() const { return pool_top == pool_bottom; }
	
private:

	std::vector<std::shared_ptr<ObjectType>> pool;
	//std::shared_ptr<ObjectType>* pool = nullptr;
	std::mutex pool_lock;
	long pool_top = 0;
	long pool_bottom = 0;
};