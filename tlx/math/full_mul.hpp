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

namespace tlx {

namespace details {

template <typename T>
[[nodiscard]]
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
    constexpr size_t kBitHalfWord = 4 * sizeof(T);
    auto lo = [](T x) { return x & ((T(1) << kBitHalfWord) - 1); };
    auto hi = [](T x) { return x >> kBitHalfWord; };

    const auto a_lo = lo(a);
    const auto a_hi = hi(a);
    const auto b_lo = lo(b);
    const auto b_hi = hi(b);

    const auto albl = a_lo * b_lo;
    const auto albh = a_lo * b_hi;
    const auto ahbl = a_hi * b_lo;
    const auto ahbh = a_hi * b_hi;

    const auto carry = hi(lo(albh) + lo(ahbl) + hi(albl));

    return { ahbh + hi(ahbl) + hi(albh) + carry, albl + ((lo(ahbl) + lo(albh)) << kBitHalfWord) };
}
}

//! \addtogroup tlx_math
//! \{

[[nodiscard]]
constexpr std::pair<uint8_t, uint8_t> full_mul(uint8_t a, uint8_t b) noexcept {
    auto m = static_cast<uint16_t>(a) * static_cast<uint16_t>(b);
    return { m >> 8, m };
}

[[nodiscard]]
constexpr std::pair<uint16_t, uint16_t> full_mul(uint16_t a, uint16_t b) noexcept {
    auto m = static_cast<uint32_t>(a) * b;
    return { m >> 16, m };
}

[[nodiscard]]
constexpr std::pair<uint32_t, uint32_t> full_mul(uint32_t a, uint32_t b) noexcept {
    auto m = static_cast<uint64_t>(a) * b;
    return { m >> 32, m };
}

[[nodiscard]]
constexpr std::pair<uint64_t, uint64_t> full_mul(uint64_t a, uint64_t b) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    auto m = static_cast<__uint128_t>(a) * b;
    return { m >> 64, m };
#elif defined(_MSC_VER) && _WIN64
#pragma intrinsic(_umul128)
    uint64_t l, h;
    l = _umul128(a, b, h);
    return { h, l };
#else
    return details::full_mul_generic(a, b);
#endif
}

//! \}
}

#endif // !TLX_MATH_FULL_MUL_HEADER

/******************************************************************************/
