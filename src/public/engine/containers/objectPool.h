#pragma once

#include <mutex>
#include "ios/logger.h"

static const unsigned int NUMBER_OF_JOBS = 4096;

template<typename ObjectType>
class IObjectPool
{
public:
	virtual ~IObjectPool() = default;
private:
	virtual void push(ObjectType* object) = 0;
	virtual ObjectType* pop() = 0;
};

template<typename ObjectType, size_t PoolSize>
class TObjectPool : public IObjectPool<ObjectType>
{
public:

	TObjectPool()
		: mask(PoolSize - 1)
	{
		pool = static_cast<ObjectType**>(std::malloc(sizeof(ObjectType*) * PoolSize));
	}

	virtual ~TObjectPool() {
		std::free(pool);
	}
	
	virtual void push(ObjectType* object)
	{
		std::lock_guard lock(pool_lock);
		if ((pool_bottom + 1) % NUMBER_OF_JOBS == pool_top)
		{
			logger_fail("job pool overflow : %d", NUMBER_OF_JOBS);
		}
		
		pool[pool_bottom] = object;		
		pool_bottom = (pool_bottom + 1) % NUMBER_OF_JOBS;
	}

	virtual ObjectType* pop()
	{
		std::lock_guard lock(pool_lock);
		if (is_empty()) return nullptr;

		ObjectType* object = pool[pool_top];
		pool_top = (pool_top + 1) % NUMBER_OF_JOBS;
		
		return object;
	}

	[[nodiscard]] bool is_empty() const { return pool_top == pool_bottom; }

private:

	ObjectType** pool = nullptr;
	std::mutex pool_lock;
	long pool_top = 0;
	long pool_bottom = 0;
	const size_t mask;
};