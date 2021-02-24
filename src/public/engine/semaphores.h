#pragma once

#if __has_include("semaphore")
#include <semaphore>
#else
#define CUSTOM_SEMAPHORES
#endif

#ifdef CUSTOM_SEMAPHORES
namespace std
{

    inline constexpr ptrdiff_t _Semaphore_max = (1ULL << (sizeof(ptrdiff_t) * CHAR_BIT - 1)) - 1;

    template <ptrdiff_t _Least_max_value = _Semaphore_max>
    class counting_semaphore {
    public:
        _NODISCARD static constexpr ptrdiff_t(max)() noexcept {
            return _Least_max_value;
        }

        constexpr explicit counting_semaphore(const ptrdiff_t _Desired) noexcept /* strengthened */
            : _Counter(_Desired) {
            _STL_VERIFY(_Desired >= 0 && _Desired <= _Least_max_value,
                "Precondition: desired >= 0, and desired <= max() (N4861 [thread.sema.cnt]/5)");
        }

        counting_semaphore(const counting_semaphore&) = delete;
        counting_semaphore& operator=(const counting_semaphore&) = delete;

        void release(ptrdiff_t _Update = 1) noexcept /* strengthened */ {
            if (_Update == 0) {
                return;
            }
            _STL_VERIFY(_Update > 0 && _Update <= _Least_max_value,
                "Precondition: update >= 0, and update <= max() - counter (N4861 [thread.sema.cnt]/8)");

            // We need to notify (wake) at least _Update waiting threads.
            // Errors towards waking more cannot be always avoided, but they are performance issues.
            // Errors towards waking fewer must be avoided, as they are correctness issues.

            // release thread: Increment semaphore counter, then load waiting counter;
            // acquire thread: Increment waiting counter, then load semaphore counter;

            // memory_order_seq_cst for all four operations guarantees that the release thread loads
            // the incremented value, or the acquire thread loads the incremented value, or both, but not neither.
            // memory_order_seq_cst might be superfluous for some hardware mappings of the C++ memory model,
            // but from the point of view of the C++ memory model itself it is needed; weaker orders don't work.

            const ptrdiff_t _Prev = _Counter.fetch_add(static_cast<ptrdiff_t>(_Update));
            _STL_VERIFY(_Prev + _Update > 0 && _Prev + _Update <= _Least_max_value,
                "Precondition: update <= max() - counter (N4861 [thread.sema.cnt]/8)");

            const ptrdiff_t _Waiting_upper_bound = _Waiting.load();

            if (_Waiting_upper_bound == 0) {
                // Definitely no one is waiting
            }
            else if (_Waiting_upper_bound <= _Update) {
                // No more waiting threads than update, can wake everyone.
                _Counter.notify_all();
            }
            else {
                // Wake at most _Update. Though repeated notify_one() is somewhat less efficient than single notify_all(),
                // the amount of OS calls is still the same; the benefit from trying not to wake unnecessary threads
                // is expected to be greater than the loss on extra calls and atomic operations.
                for (; _Update != 0; --_Update) {
                    _Counter.notify_one();
                }
            }
        }

        void _Wait(const unsigned long _Remaining_timeout) noexcept {
            // See the comment in release()
            _Waiting.fetch_add(1);
            ptrdiff_t _Current = _Counter.load();
            if (_Current == 0) {
                __std_atomic_wait_direct(&_Counter, &_Current, sizeof(_Current), _Remaining_timeout);
            }
            _Waiting.fetch_sub(1, memory_order_relaxed);
        }

        void acquire() noexcept /* strengthened */ {
            ptrdiff_t _Current = _Counter.load(memory_order_relaxed);
            for (;;) {
                while (_Current == 0) {
                    _Wait(_Atomic_wait_no_timeout);
                    _Current = _Counter.load(memory_order_relaxed);
                }
                _STL_VERIFY(_Current > 0 && _Current <= _Least_max_value,
                    "Invariant: counter >= 0, and counter <= max() "
                    "possibly caused by preconditions violation (N4861 [thread.sema.cnt]/8)");

                // "happens after release" ordering is provided by this CAS, so loads and waits can be relaxed
                if (_Counter.compare_exchange_weak(_Current, _Current - 1)) {
                    return;
                }
            }
        }

        _NODISCARD bool try_acquire() noexcept {
            ptrdiff_t _Current = _Counter.load();
            if (_Current == 0) {
                return false;
            }
            _STL_VERIFY(_Current > 0 && _Current <= _Least_max_value,
                "Invariant: counter >= 0, and counter <= max() "
                "possibly caused by preconditions violation (N4861 [thread.sema.cnt]/8)");

            return _Counter.compare_exchange_weak(_Current, _Current - 1);
        }

        template <class _Rep, class _Period>
        _NODISCARD bool try_acquire_for(const chrono::duration<_Rep, _Period>& _Rel_time) {
            auto _Deadline = _Semaphore_deadline(_Rel_time);
            ptrdiff_t _Current = _Counter.load(memory_order_relaxed);
            for (;;) {
                while (_Current == 0) {
                    const auto _Remaining_timeout = __std_atomic_wait_get_remaining_timeout(_Deadline);
                    if (_Remaining_timeout == 0) {
                        return false;
                    }
                    _Wait(_Remaining_timeout);
                    _Current = _Counter.load(memory_order_relaxed);
                }
                _STL_VERIFY(_Current > 0 && _Current <= _Least_max_value,
                    "Invariant: counter >= 0, and counter <= max() "
                    "possibly caused by preconditions violation (N4861 [thread.sema.cnt]/8)");

                // "happens after release" ordering is provided by this CAS, so loads and waits can be relaxed
                if (_Counter.compare_exchange_weak(_Current, _Current - 1)) {
                    return true;
                }
            }
        }

        template <class _Clock, class _Duration>
        _NODISCARD bool try_acquire_until(const chrono::time_point<_Clock, _Duration>& _Abs_time) {
            ptrdiff_t _Current = _Counter.load(memory_order_relaxed);
            for (;;) {
                while (_Current == 0) {
                    const unsigned long _Remaining_timeout = _Semaphore_remaining_timeout(_Abs_time);
                    if (_Remaining_timeout == 0) {
                        return false;
                    }
                    _Wait(_Remaining_timeout);
                    _Current = _Counter.load(memory_order_relaxed);
                }
                _STL_VERIFY(_Current > 0 && _Current <= _Least_max_value,
                    "Invariant: counter >= 0, and counter <= max() "
                    "possibly caused by preconditions violation (N4861 [thread.sema.cnt]/8)");

                // "happens after release" ordering is provided by this CAS, so loads and waits can be relaxed
                if (_Counter.compare_exchange_weak(_Current, _Current - 1)) {
                    return true;
                }
            }
        }

    private:
        atomic<ptrdiff_t> _Counter;
        atomic<ptrdiff_t> _Waiting;
    };

    template <>
    class counting_semaphore<1> {
    public:
        _NODISCARD static constexpr ptrdiff_t(max)() noexcept {
            return 1;
        }

        constexpr explicit counting_semaphore(const ptrdiff_t _Desired) noexcept /* strengthened */
            : _Counter(static_cast<unsigned char>(_Desired)) {
            _STL_VERIFY((_Desired & ~1) == 0, "Precondition: desired >= 0, and desired <= max() "
                "(N4861 [thread.sema.cnt]/5)");
        }

        counting_semaphore(const counting_semaphore&) = delete;
        counting_semaphore& operator=(const counting_semaphore&) = delete;

        void release(const ptrdiff_t _Update = 1) noexcept /* strengthened */ {
            if (_Update == 0) {
                return;
            }
            _STL_VERIFY(_Update == 1, "Precondition: update >= 0, "
                "and update <= max() - counter (N4861 [thread.sema.cnt]/8)");
            // TRANSITION, GH-1133: should be memory_order_release
            _Counter.store(1);
            _Counter.notify_one();
        }

        void acquire() noexcept /* strengthened */ {
            for (;;) {
                // "happens after release" ordering is provided by this exchange, so loads and waits can be relaxed
                // TRANSITION, GH-1133: should be memory_order_acquire
                unsigned char _Prev = _Counter.exchange(0);
                if (_Prev == 1) {
                    break;
                }
                _STL_VERIFY(_Prev == 0, "Invariant: semaphore counter is non-negative and doesn't exceed max(), "
                    "possibly caused by preconditions violation (N4861 [thread.sema.cnt]/8)");
                _Counter.wait(0, memory_order_relaxed);
            }
        }

        _NODISCARD bool try_acquire() noexcept {
            // TRANSITION, GH-1133: should be memory_order_acquire
            unsigned char _Prev = _Counter.exchange(0);
            _STL_VERIFY((_Prev & ~1) == 0, "Invariant: semaphore counter is non-negative and doesn't exceed max(), "
                "possibly caused by preconditions violation (N4861 [thread.sema.cnt]/8)");
            return reinterpret_cast<const bool&>(_Prev);
        }

        template <class _Rep, class _Period>
        _NODISCARD bool try_acquire_for(const chrono::duration<_Rep, _Period>& _Rel_time) {
            auto _Deadline = _Semaphore_deadline(_Rel_time);
            for (;;) {
                // "happens after release" ordering is provided by this exchange, so loads and waits can be relaxed
                // TRANSITION, GH-1133: should be memory_order_acquire
                unsigned char _Prev = _Counter.exchange(0);
                if (_Prev == 1) {
                    return true;
                }
                _STL_VERIFY(_Prev == 0, "Invariant: semaphore counter is non-negative and doesn't exceed max(), "
                    "possibly caused by preconditions violation (N4861 [thread.sema.cnt]/8)");
                const auto _Remaining_timeout = __std_atomic_wait_get_remaining_timeout(_Deadline);
                if (_Remaining_timeout == 0) {
                    return false;
                }
                __std_atomic_wait_direct(&_Counter, &_Prev, sizeof(_Prev), _Remaining_timeout);
            }
        }

        template <class _Clock, class _Duration>
        _NODISCARD bool try_acquire_until(const chrono::time_point<_Clock, _Duration>& _Abs_time) {
            for (;;) {
                // "happens after release" ordering is provided by this exchange, so loads and waits can be relaxed
                // TRANSITION, GH-1133: should be memory_order_acquire
                unsigned char _Prev = _Counter.exchange(0);
                if (_Prev == 1) {
                    return true;
                }
                _STL_VERIFY(_Prev == 0, "Invariant: semaphore counter is non-negative and doesn't exceed max(), "
                    "possibly caused by preconditions violation (N4861 [thread.sema.cnt]/8)");

                const unsigned long _Remaining_timeout = _Semaphore_remaining_timeout(_Abs_time);
                if (_Remaining_timeout == 0) {
                    return false;
                }

                __std_atomic_wait_direct(&_Counter, &_Prev, sizeof(_Prev), _Remaining_timeout);
            }
        }

    private:
        atomic<unsigned char> _Counter;
    };

    using binary_semaphore = counting_semaphore<1>;
}
#endif
