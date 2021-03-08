/*******************************************************************************
 * tlx/math/full_mul.hpp
 *
 * full_mul(a, b) computes the unsigned full multiplication of a * b which needs
 * up to twice as many bits as the input data types. The result is returned as a
 * pair p where p.first contains the upper half and p.second the lower half.
 *
 * Part of tlx - http://panthema.net/tlx
 *
 * Copyright (C) 2021 Manuel Penschuck <tlx@manuel.jetzt>
 *
 * All rights reserved. Published under the Boost Software License, Version 1.0
 ******************************************************************************/

#ifndef TLX_MATH_FULL_MUL_HEADER
#define TLX_MATH_FULL_MUL_HEADER

#include <cstdint>
#include <utility>

#if defined(_MSC_VER) && _WIN64
#include <intrin.h>
#endif

#include <tlx/define/attribute_nodiscard.hpp>

namespace tlx {

// unfortunately needed since the `constexpr std::pair<T, T>(T a, T b)` constructor is available only
// starting with C++14 while the `constexpr std::pair<T, T>()` constructor is already in C++11
#define TLX_FULLMUL_RETURN_PAIR(T, F, S) \
    std::pair<T, T> result;              \
    result.first = static_cast<T>(F);    \
    result.second = static_cast<T>(S);   \
    return result

namespace details {

template <typename T>
TLX_NODISCARD
constexpr typename
std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, std::pair<T, T> >::type
full_mul_generic(T a, T b) noexcept {
/* Multiplication schema for 64 bit:
 * a * b = ((a_hi << 32) + a_lo) * ((b_hi << 32) + b_lo)
 *       =   (a_hi * b_hi) << 64   HHHHLLLL|
 *         + (a_hi * b_lo) << 32       HHHH|LLLL
 *         + (a_lo * b_hi) << 32       HHHH|LLLL
 *         + (a_lo * b_lo) <<  0           |HHHHLLLL
 *                                          ^^^^ - may trigger carry
 */
    auto lo = [](T x) { return x & ((T(1) << (4 * sizeof(T))) - 1); };
    auto hi = [](T x) { return x >> (4 * sizeof(T)); };

    const auto a_lo = lo(a);
    const auto a_hi = hi(a);
    const auto b_lo = lo(b);
    const auto b_hi = hi(b);

    const auto albl = a_lo * b_lo;
    const auto albh = a_lo * b_hi;
    const auto ahbl = a_hi * b_lo;
    const auto ahbh = a_hi * b_hi;

    const auto carry = hi(lo(albh) + lo(ahbl) + hi(albl));

    TLX_FULLMUL_RETURN_PAIR(T, ahbh + hi(ahbl) + hi(albh) + carry, albl + ((lo(ahbl) + lo(albh)) << (4 * sizeof(T))));
}
}

//! \addtogroup tlx_math
//! \{

//! compute full multiplication x = full_mul(a,b) <=> a*b = (x.first << 8) + x.second
TLX_NODISCARD
constexpr std::pair<uint8_t, uint8_t> full_mul(uint8_t a, uint8_t b) noexcept {
    auto m = static_cast<uint16_t>(a) * static_cast<uint16_t>(b);
    TLX_FULLMUL_RETURN_PAIR(uint8_t, m >> 8, m);
}

//! compute full multiplication x = full_mul(a,b) <=> a*b = (x.first << 16) + x.second
TLX_NODISCARD
constexpr std::pair<uint16_t, uint16_t> full_mul(uint16_t a, uint16_t b) noexcept {
    auto m = static_cast<uint32_t>(a) * b;
    TLX_FULLMUL_RETURN_PAIR(uint16_t, m >> 16, m);
}

//! compute full multiplication x = full_mul(a,b) <=> a*b = (x.first << 32) + x.second
TLX_NODISCARD
constexpr std::pair<uint32_t, uint32_t> full_mul(uint32_t a, uint32_t b) noexcept {
    auto m = static_cast<uint64_t>(a) * b;
    TLX_FULLMUL_RETURN_PAIR(uint32_t, m >> 32, m);
}

#if defined(_MSC_VER) && _WIN64
#pragma intrinsic(_umul128)
#endif

//! compute full multiplication x = full_mul(a,b) <=> a*b = (x.first << 64) + x.second
TLX_NODISCARD
inline std::pair<uint64_t, uint64_t> full_mul(uint64_t a, uint64_t b) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    auto m = static_cast<__uint128_t>(a) * b;
    TLX_FULLMUL_RETURN_PAIR(uint64_t, m >> 64, m);
#elif defined(_MSC_VER) && _WIN64
    uint64_t l = 0;
    uint64_t h = 0;
    l = _umul128(a, b, &h);
    TLX_FULLMUL_RETURN_PAIR(uint64_t, h, l);
#else
    return details::full_mul_generic(a, b);
#endif
}

//! constexpr variant of full_mul(uint64_t a, uint64_t b) which cannot be constexpr on MSVC.
//! this implementation is probably slower than `full_mul` and should only be used for
//! compile-time computation.
TLX_NODISCARD
constexpr std::pair<uint64_t, uint64_t> full_mul_ce(uint64_t a, uint64_t b) noexcept {
    return details::full_mul_generic(a, b);
}

//! \}
}

#undef TLX_FULLMUL_RETURN_PAIR

#endif // !TLX_MATH_FULL_MUL_HEADER

/******************************************************************************/
