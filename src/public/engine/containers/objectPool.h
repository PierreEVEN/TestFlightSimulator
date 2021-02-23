#pragma once

#include <mutex>
#include <atomic>

static const unsigned int NUMBER_OF_JOBS = 4096;
static const unsigned int MASK = NUMBER_OF_JOBS - 1;

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
		long new_bottom = pool_bottom;
		pool[pool_bottom & MASK] = object;
		std::atomic_thread_fence(std::memory_order_acq_rel);
		pool_bottom = new_bottom + 1;
	}

	virtual ObjectType* pop()
	{
		std::lock_guard lock(pool_lock);
		long new_bottom = pool_bottom - 1;
		pool_bottom = new_bottom;

		std::atomic_thread_fence(std::memory_order_acq_rel);

		long top = pool_top;

		if (top <= new_bottom)
		{
			ObjectType* object = pool[new_bottom & MASK];
			if (top != new_bottom)
			{
				// there's still more than one item left in the queue
				return object;
			}

			// this is the last item in the queue
			if (_InterlockedCompareExchange(&pool_top, top + 1, top) != top)
			{
				// failed race against steal operation
				object = nullptr;
			}

			pool_bottom = top + 1;
			return object;
		}
		else {
			// deque was already empty
			pool_bottom = top;
			return nullptr;
		}
	}

	bool is_empty() const { return pool_top == pool_bottom; }

private:

	ObjectType** pool = nullptr;
	std::mutex pool_lock;
	long pool_top = 0;
	long pool_bottom = 0;
	const size_t mask;
};