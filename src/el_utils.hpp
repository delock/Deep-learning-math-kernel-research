#include <stdio.h>
#include <cxxabi.h>
#include <chrono>
#include <omp.h>

#ifndef __EL_UTILS_HPP__
#define __EL_UTILS_HPP__

#define _T(x) x
typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<float, std::milli> Duration;

#define __tstart(n) _T(Time::time_point __s##n =  Time::now());
#define __tend(n) \
    _T(Time::time_point __e##n = Time::now()); \
    _T(printf("time: %s, th=%d, %.2f ms\n", #n, \
        omp_get_thread_num(), Duration(__e##n - __s##n).count()));

#define MD(type, array, dims, ptr) \
    type (&array)dims = *reinterpret_cast<type (*)dims>(ptr)

namespace euler {

enum {
    ISA_GENERIC     = 0,
    ISA_SKX_AVX512  = 512,
};

template <typename T, typename P>
inline bool one_of(T val, P item) { return val == item; }
template <typename T, typename P, typename... Args>
inline bool one_of(T val, P item, Args... item_others) {
    return val == item || one_of(val, item_others...);
}

template <typename... Args>
inline bool any_null(Args... ptrs) { return one_of(nullptr, ptrs...); }

template<typename T> inline T accumulate(T tn) {return tn; }
template<typename T, typename... Args>
inline T accumulate(T a, Args... args) { return a * accumulate(args...); }


inline void eld_error(const char *msg) {
    printf("Euler:d: %s\n", msg);
}

inline void elx_error(const char *msg) {
    printf("Euler:x: %s\n", msg);
}

}

#endif // __EL_UTILS_HPP__
