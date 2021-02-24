#pragma once

#if __has_include("semaphore")
#include <semaphore>
#else
#define CUSTOM_SEMAPHORES
#endif

#ifdef CUSTOM_SEMAPHORES


#define SEM_VERIFY(cond, msg) {if (cond) {} else logger::fail(msg);}

namespace std
{

    inline constexpr ptrdiff_t _semaphore_max = (1ULL << (sizeof(ptrdiff_t) * CHAR_BIT - 1)) - 1;

    template <ptrdiff_t LeastMaxValue = _semaphore_max>
    class counting_semaphore {
    public:
        [[nodiscard]] static constexpr ptrdiff_t(max)() noexcept {
            return LeastMaxValue;
        }

        constexpr explicit counting_semaphore(const ptrdiff_t desired) noexcept
            : count(desired) {
            SEM_VERIFY(desired >= 0 && desired <= LeastMaxValue, "Precondition: desired >= 0, and desired <= max() (N4861 [thread.sema.cnt]/5)");
        }

        counting_semaphore(const counting_semaphore&) = delete;
        counting_semaphore& operator=(const counting_semaphore&) = delete;

        void release(ptrdiff_t update = 1) noexcept {

            if (update == 0) return;
            {
                auto lock = std::unique_lock<std::mutex>(lock_mutex);
                count += update;
            }
            condition.notify_all();
        }

        void acquire(ptrdiff_t update = 1) noexcept {
            if (update == 0) return;

            auto lock = std::unique_lock<std::mutex>(lock_mutex);
            condition.wait(lock, [&] () -> bool {
                if (count > 0 && count < update) {
                    update -= count;
                    count = 0;
                }
                else if (count >= update) {
                    count -= update;
                    update = 0;
                }
                return update == 0;
                });        	
        }

    private:
        std::mutex lock_mutex;
        std::condition_variable condition;
        std::ptrdiff_t count;
    };

    using binary_semaphore = counting_semaphore<1>;
}
#endif
