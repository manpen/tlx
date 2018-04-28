/*******************************************************************************
 * tests/bitarray_test.cpp
 *
 * Part of tlx - http://panthema.net/tlx
 *
 * Copyright (C) 2018 Manuel Penschuck <tlx@manuel.jetzt>
 *
 * All rights reserved. Published under the Boost Software License, Version 1.0
 ******************************************************************************/

#include <iostream>
#include <random>
#include <vector>

#include <tlx/bitarray.hpp>
#include <tlx/die.hpp>

/******************************************************************************/

// Idiot-proof reference implementation of flag field against which we test
class naive_bitarray
{
public:
    naive_bitarray(size_t n) : bits_(n, false) { }

    void set_bit(size_t i) {
        bits_.at(i) = true;
    }

    void clear_bit(size_t i) {
        bits_.at(i) = false;
    }

    bool is_set(size_t i) const {
        return bits_.at(i);
    }

    void clear_all() {
        for (size_t i = 0; i < bits_.size(); i++)
            bits_[i] = false;
    }

    bool empty() {
        for (size_t i = 0; i < bits_.size(); i++)
            if (bits_[i])
                return false;

        return true;
    }

    size_t find_lsb() {
        for (size_t i = 0; i < bits_.size(); i++)
            if (bits_[i])
                return i;

        die("find_lsb on empty field");
    }

    void swap(naive_bitarray& o) {
        std::swap(bits_, o.bits_);
    }

protected:
    std::vector<bool> bits_;
};

// Perform the same tests for a number of different compile-time settings
template <typename Field, size_t Size>
void test() {
    Field f;
    std::mt19937 rng(Size);

    // set each (for large instances, a random subset) bit, search it, clear it
    {
        std::uniform_int_distribution<int> skip_dist(0, Size / 4096);

        for (size_t i = 0; i < Size; i++) {
            die_unless(f.empty());
            f.set_bit(i);
            die_if(f.empty());
            die_unequal(i, f.find_lsb());
            f.clear_bit(i);
            die_unless(f.empty());

            if (Size > 64 * 64 + 1) {
                i += skip_dist(rng);
            }
        }
    }

    Field f0;
    std::swap(f0, f);

    // randomly flip bits
    std::uniform_int_distribution<unsigned> distr(0, Size - 1);
    std::uniform_real_distribution<float> rdist;

    naive_bitarray nf(Size);
    for (size_t i = 0; i < std::min<size_t>(100, Size * 10); i++) {
        const auto idx = distr(rng);

        die_unequal(nf.is_set(idx), f.is_set(idx));

        if (nf.is_set(idx)) {
            nf.clear_bit(idx);
            f.clear_bit(idx);
        }
        else {
            nf.set_bit(idx);
            f.set_bit(idx);
        }

        die_unequal(nf.empty(), f.empty());

        if (f.empty()) continue;

        die_unequal(nf.find_lsb(), f.find_lsb());

        // clear lsb to increase chance of observing changes on more
        // significant bits
        if (rdist(rng) > 0.3) {
            const auto first = f.find_lsb();
            nf.clear_bit(first);
            f.clear_bit(first);
        }
    }
    f.set_bit(0);
    die_unless(f0.empty());

    Field f1;
    die_unless(f1.empty());
    std::swap(f1, f);
    die_unless(f.empty());
    die_if(f1.empty());

    f1.clear_all();
    die_unless(f1.empty());
}

int main() {
    test<tlx::details::bitarray_recursive<32, true>, 32>();
    test<tlx::details::bitarray_recursive<64, true>, 64>();

    // 1 layer
    test<tlx::bitarray<32>, 32>();
    test<tlx::bitarray<33>, 33>();
    test<tlx::bitarray<63>, 63>();
    test<tlx::bitarray<64>, 64>();

    // 2 layers
    test<tlx::bitarray<65>, 65>();
    test<tlx::bitarray<500>, 500>();
    test<tlx::bitarray<1001>, 1001>();
    test<tlx::bitarray<4004>, 4004>();
    test<tlx::bitarray<4096>, 4096>();

    // 3 layers
    test<tlx::bitarray<1*64*64 + 1>, 1*64*64 + 1>();
    test<tlx::bitarray<64*64*64>, 64*64*64>();

    return 0;
}

/******************************************************************************/
