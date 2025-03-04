#pragma once

#include <stdlib.h>
#include <assert.h>

namespace euler {

template <typename F, const int N> class mdarray {
  public:
  template <typename... Args>
  mdarray(void *p, Args... dims) : _dims{ dims... }
  {
    _p = (F *)p;
  }
  template <typename... Args> inline F &operator()(Args... dims)
  {
    return *(_p + offset(1, dims...));
  }

  private:
  template <typename... Args>
  inline size_t offset(size_t index, size_t off, size_t dim, Args... dims)
  {
    off = _dims[index] * off + dim;
    return offset(index + 1, off, dims...);
  }
  inline size_t offset(size_t index, size_t off, size_t dim)
  {
    return _dims[index] * off + dim;
  }
  inline size_t offset(size_t index, size_t dim)
  {
    return dim;
  }

  F *_p;
  const int _dims[N];
};

#define __SQUARE_1(d0) [d0]
#define __SQUARE_2(d1, ...)   [d1]__SQUARE_1(__VA_ARGS__)
#define __SQUARE_3(d2, ...)   [d2]__SQUARE_2(__VA_ARGS__)
#define __SQUARE_4(d3, ...)   [d3]__SQUARE_3(__VA_ARGS__)
#define __SQUARE_5(d4, ...)   [d4]__SQUARE_4(__VA_ARGS__)
#define __SQUARE_6(d5, ...)   [d5]__SQUARE_5(__VA_ARGS__)
#define __SQUARE_7(d6, ...)   [d6]__SQUARE_6(__VA_ARGS__)
#define __SQUARE_8(d7, ...)   [d7]__SQUARE_7(__VA_ARGS__)
#define __SQUARE_9(d8, ...)   [d8]__SQUARE_8(__VA_ARGS__)
#define __SQUARE_10(d9, ...)  [d9]__SQUARE_9(__VA_ARGS__)
#define __SQUARE_11(d10, ...) [d10]__SQUARE_10(__VA_ARGS__)
#define __SQUARE_12(d11, ...) [d11]__SQUARE_11(__VA_ARGS__)
#define __SQUARE_13(d12, ...) [d12]__SQUARE_12(__VA_ARGS__)
#define __SQUARE_14(d13, ...) [d13]__SQUARE_13(__VA_ARGS__)
#define __SQUARE_15(d14, ...) [d14]__SQUARE_14(__VA_ARGS__)
#define __SQUARE_16(d15, ...) [d15]__SQUARE_15(__VA_ARGS__)
#define __SQUARE_17(d16, ...) [d16]__SQUARE_16(__VA_ARGS__)
#define __SQUARE_18(d17, ...) [d17]__SQUARE_17(__VA_ARGS__)
#define __SQUARE_19(d18, ...) [d18]__SQUARE_18(__VA_ARGS__)
#define __SQUARE_20(d19, ...) [d19]__SQUARE_19(__VA_ARGS__)
#define __SQUARE_21(d20, ...) [d20]__SQUARE_20(__VA_ARGS__)
#define __SQUARE_22(d21, ...) [d21]__SQUARE_21(__VA_ARGS__)
#define __SQUARE_23(d22, ...) [d22]__SQUARE_22(__VA_ARGS__)
#define __SQUARE_24(d23, ...) [d23]__SQUARE_23(__VA_ARGS__)
#define __SQUARE_25(d24, ...) [d24]__SQUARE_24(__VA_ARGS__)

#define __SQUARE(n, ...) __SQUARE_##n(__VA_ARGS__)

#define __COMMA_1(d0) d0
#define __COMMA_2(d1, ...)   d1,__COMMA_1(__VA_ARGS__)
#define __COMMA_3(d2, ...)   d2,__COMMA_2(__VA_ARGS__)
#define __COMMA_4(d3, ...)   d3,__COMMA_3(__VA_ARGS__)
#define __COMMA_5(d4, ...)   d4,__COMMA_4(__VA_ARGS__)
#define __COMMA_6(d5, ...)   d5,__COMMA_5(__VA_ARGS__)
#define __COMMA_7(d6, ...)   d6,__COMMA_6(__VA_ARGS__)
#define __COMMA_8(d7, ...)   d7,__COMMA_7(__VA_ARGS__)
#define __COMMA_9(d8, ...)   d8,__COMMA_8(__VA_ARGS__)
#define __COMMA_10(d9, ...)  d9,__COMMA_9(__VA_ARGS__)
#define __COMMA_11(d10, ...) d10,__COMMA_10(__VA_ARGS__)
#define __COMMA_12(d11, ...) d11,__COMMA_11(__VA_ARGS__)
#define __COMMA_13(d12, ...) d12,__COMMA_12(__VA_ARGS__)
#define __COMMA_14(d13, ...) d13,__COMMA_13(__VA_ARGS__)
#define __COMMA_15(d14, ...) d14,__COMMA_14(__VA_ARGS__)
#define __COMMA_16(d15, ...) d15,__COMMA_15(__VA_ARGS__)
#define __COMMA_17(d16, ...) d16,__COMMA_16(__VA_ARGS__)
#define __COMMA_18(d17, ...) d17,__COMMA_17(__VA_ARGS__)
#define __COMMA_19(d18, ...) d18,__COMMA_18(__VA_ARGS__)
#define __COMMA_20(d19, ...) d19,__COMMA_19(__VA_ARGS__)
#define __COMMA_21(d20, ...) d20,__COMMA_20(__VA_ARGS__)
#define __COMMA_22(d21, ...) d21,__COMMA_21(__VA_ARGS__)
#define __COMMA_23(d22, ...) d22,__COMMA_22(__VA_ARGS__)
#define __COMMA_24(d23, ...) d23,__COMMA_23(__VA_ARGS__)
#define __COMMA_25(d24, ...) d24,__COMMA_24(__VA_ARGS__)

#define __COMMA(n, ...) __COMMA_##n(__VA_ARGS__)

#if (__INTEL_COMPILER || __GCC_COMPILER)

#define __MD(type, n, array, ptr, ...)                                         \
  auto &array = *reinterpret_cast<type(*) __SQUARE(n, __VA_ARGS__)>(ptr)

#define md1(array, ...)  ((array)__SQUARE(1, __VA_ARGS__))
#define md2(array, ...)  ((array)__SQUARE(2, __VA_ARGS__))
#define md3(array, ...)  ((array)__SQUARE(3, __VA_ARGS__))
#define md4(array, ...)  ((array)__SQUARE(4, __VA_ARGS__))
#define md5(array, ...)  ((array)__SQUARE(5, __VA_ARGS__))
#define md6(array, ...)  ((array)__SQUARE(6, __VA_ARGS__))
#define md7(array, ...)  ((array)__SQUARE(7, __VA_ARGS__))
#define md8(array, ...)  ((array)__SQUARE(8, __VA_ARGS__))
#define md9(array, ...)  ((array)__SQUARE(9, __VA_ARGS__))
#define md10(array, ...) ((array)__SQUARE(10, __VA_ARGS__))
#define md11(array, ...) ((array)__SQUARE(11, __VA_ARGS__))
#define md12(array, ...) ((array)__SQUARE(12, __VA_ARGS__))
#define md13(array, ...) ((array)__SQUARE(13, __VA_ARGS__))
#define md14(array, ...) ((array)__SQUARE(14, __VA_ARGS__))
#define md15(array, ...) ((array)__SQUARE(15, __VA_ARGS__))
#define md16(array, ...) ((array)__SQUARE(16, __VA_ARGS__))
#define md17(array, ...) ((array)__SQUARE(17, __VA_ARGS__))
#define md18(array, ...) ((array)__SQUARE(18, __VA_ARGS__))
#define md19(array, ...) ((array)__SQUARE(19, __VA_ARGS__))
#define md20(array, ...) ((array)__SQUARE(20, __VA_ARGS__))
#define md21(array, ...) ((array)__SQUARE(21, __VA_ARGS__))
#define md22(array, ...) ((array)__SQUARE(22, __VA_ARGS__))
#define md23(array, ...) ((array)__SQUARE(23, __VA_ARGS__))
#define md24(array, ...) ((array)__SQUARE(24, __VA_ARGS__))
#define md25(array, ...) ((array)__SQUARE(25, __VA_ARGS__))

#else

#define __MD(type, n, array, ptr, ...)                                         \
  mdarray<type, n> array(ptr, __COMMA(n, __VA_ARGS__))

#define md1(array, ...)  (array(__COMMA(1, __VA_ARGS__)))
#define md2(array, ...)  (array(__COMMA(2, __VA_ARGS__)))
#define md3(array, ...)  (array(__COMMA(3, __VA_ARGS__)))
#define md4(array, ...)  (array(__COMMA(4, __VA_ARGS__)))
#define md5(array, ...)  (array(__COMMA(5, __VA_ARGS__)))
#define md6(array, ...)  (array(__COMMA(6, __VA_ARGS__)))
#define md7(array, ...)  (array(__COMMA(7, __VA_ARGS__)))
#define md8(array, ...)  (array(__COMMA(8, __VA_ARGS__)))
#define md9(array, ...)  (array(__COMMA(9, __VA_ARGS__)))
#define md10(array, ...) (array(__COMMA(10, __VA_ARGS__)))
#define md11(array, ...) (array(__COMMA(11, __VA_ARGS__)))
#define md12(array, ...) (array(__COMMA(12, __VA_ARGS__)))
#define md13(array, ...) (array(__COMMA(13, __VA_ARGS__)))
#define md14(array, ...) (array(__COMMA(14, __VA_ARGS__)))
#define md15(array, ...) (array(__COMMA(15, __VA_ARGS__)))
#define md16(array, ...) (array(__COMMA(16, __VA_ARGS__)))
#define md17(array, ...) (array(__COMMA(17, __VA_ARGS__)))
#define md18(array, ...) (array(__COMMA(18, __VA_ARGS__)))
#define md19(array, ...) (array(__COMMA(19, __VA_ARGS__)))
#define md20(array, ...) (array(__COMMA(20, __VA_ARGS__)))
#define md21(array, ...) (array(__COMMA(21, __VA_ARGS__)))
#define md22(array, ...) (array(__COMMA(22, __VA_ARGS__)))
#define md23(array, ...) (array(__COMMA(23, __VA_ARGS__)))
#define md24(array, ...) (array(__COMMA(24, __VA_ARGS__)))
#define md25(array, ...) (array(__COMMA(25, __VA_ARGS__)))

#endif

#define MD1(type,  array, ptr, ...) __MD(type, 1,  array, ptr, __VA_ARGS__)
#define MD2(type,  array, ptr, ...) __MD(type, 2,  array, ptr, __VA_ARGS__)
#define MD3(type,  array, ptr, ...) __MD(type, 3,  array, ptr, __VA_ARGS__)
#define MD4(type,  array, ptr, ...) __MD(type, 4,  array, ptr, __VA_ARGS__)
#define MD5(type,  array, ptr, ...) __MD(type, 5,  array, ptr, __VA_ARGS__)
#define MD6(type,  array, ptr, ...) __MD(type, 6,  array, ptr, __VA_ARGS__)
#define MD7(type,  array, ptr, ...) __MD(type, 7,  array, ptr, __VA_ARGS__)
#define MD8(type,  array, ptr, ...) __MD(type, 8,  array, ptr, __VA_ARGS__)
#define MD9(type,  array, ptr, ...) __MD(type, 9,  array, ptr, __VA_ARGS__)
#define MD10(type, array, ptr, ...) __MD(type, 10, array, ptr, __VA_ARGS__)
#define MD11(type, array, ptr, ...) __MD(type, 11, array, ptr, __VA_ARGS__)
#define MD12(type, array, ptr, ...) __MD(type, 12, array, ptr, __VA_ARGS__)
#define MD13(type, array, ptr, ...) __MD(type, 13, array, ptr, __VA_ARGS__)
#define MD14(type, array, ptr, ...) __MD(type, 14, array, ptr, __VA_ARGS__)
#define MD15(type, array, ptr, ...) __MD(type, 15, array, ptr, __VA_ARGS__)
#define MD16(type, array, ptr, ...) __MD(type, 16, array, ptr, __VA_ARGS__)
#define MD17(type, array, ptr, ...) __MD(type, 17, array, ptr, __VA_ARGS__)
#define MD18(type, array, ptr, ...) __MD(type, 18, array, ptr, __VA_ARGS__)
#define MD19(type, array, ptr, ...) __MD(type, 19, array, ptr, __VA_ARGS__)
#define MD20(type, array, ptr, ...) __MD(type, 20, array, ptr, __VA_ARGS__)
#define MD21(type, array, ptr, ...) __MD(type, 21, array, ptr, __VA_ARGS__)
#define MD22(type, array, ptr, ...) __MD(type, 22, array, ptr, __VA_ARGS__)
#define MD23(type, array, ptr, ...) __MD(type, 23, array, ptr, __VA_ARGS__)
#define MD24(type, array, ptr, ...) __MD(type, 24, array, ptr, __VA_ARGS__)
#define MD25(type, array, ptr, ...) __MD(type, 25, array, ptr, __VA_ARGS__)


} // namespace euler
