/*******************************************************************************
 * tests/integer_rank_test.cpp
 *
 * Part of tlx - http://panthema.net/tlx
 *
 * Copyright (C) 2018 Manuel Penschuck <tlx@manuel.jetzt>
 *
 * All rights reserved. Published under the Boost Software License, Version 1.0
 ******************************************************************************/

#include <array>
#include <cstdint>
#include <limits>
#include <random>

#include <tlx/die.hpp>
#include <tlx/math/integer_rank.hpp>

template <typename T>
void test_suite(std::mt19937& prng) {
    using ir = tlx::integer_rank<T>;

    constexpr T min = std::numeric_limits<T>::min();
    constexpr T max = std::numeric_limits<T>::max();
    constexpr T zero = 0;
    constexpr T one = 1;

    // Check that int_at_rank and rank_of_int are inverse
    die_unequal(min, ir::int_at_rank(ir::rank_of_int(min)));
    die_unequal(min + one, ir::int_at_rank(ir::rank_of_int(min + one)));
    die_unequal(zero, ir::int_at_rank(ir::rank_of_int(zero)));
    die_unequal(one, ir::int_at_rank(ir::rank_of_int(one)));
    die_unequal(max - one, ir::int_at_rank(ir::rank_of_int(max - one)));
    die_unequal(max, ir::int_at_rank(ir::rank_of_int(max)));

    // Check that random pairs keep their relative order
    std::uniform_int_distribution<T> dist;
    for (size_t i = 0; i < 1000; i++) {
        // draw two random numbers a, b s.t. a < b
        T a = dist(prng);
        T b;
        do {
            b = dist(prng);
        } while (b == a);
        if (a > b) std::swap(a, b);

        die_unless(ir::rank_of_int(a) < ir::rank_of_int(b));

        // Check inverse for random numbers
        die_unequal(a, ir::int_at_rank(ir::rank_of_int(a)));
        die_unequal(b, ir::int_at_rank(ir::rank_of_int(b)));
    }
}

int main() {
    std::mt19937 prng(1);

    test_suite<uint8_t>(prng);
    test_suite<uint16_t>(prng);
    test_suite<uint32_t>(prng);
    test_suite<uint64_t>(prng);

    test_suite<int8_t>(prng);
    test_suite<int16_t>(prng);
    test_suite<int32_t>(prng);
    test_suite<int64_t>(prng);

    return 0;
}
/******************************************************************************/
