/*******************************************************************************
 * tlx/bitarray.hpp
 *
 * Part of tlx - http://panthema.net/tlx
 *
 * Copyright (C) 2018 Manuel Penschuck <tlx@manuel.jetzt>
 *
 * All rights reserved. Published under the Boost Software License, Version 1.0
 ******************************************************************************/

#ifndef TLX_BITARRAY_HEADER
#define TLX_BITARRAY_HEADER

#include <array>
#include <cassert>
#include <limits>
#include <type_traits>

#include <tlx/math/div_ceil.hpp>
#include <tlx/math/ffs.hpp>
#include <tlx/meta/log2.hpp>

namespace tlx {
namespace details {

//! Internal implementation of tlx::bitarray; do not invoke directly
//! \tparam Size  Number of bits the data structure is supposed to store
//! \tparam SizeIsAtmost64  Switch between inner node implementation (false)
//!                         and leaf implementation (true)
template <size_t Size, bool SizeIsAtmost64>
class bitarray_recursive;

template <size_t Size>
class bitarray_recursive<Size, false>
{
    static constexpr size_t leaf_width = 6;
    static constexpr size_t width = tlx::Log2<Size>::ceil;
    static_assert(width > leaf_width,
                  "Size has to be larger than 2**leaf_width");
    static constexpr size_t root_width = (width % leaf_width)
                                         ? (width % leaf_width)
                                         : leaf_width;
    static constexpr size_t child_width = width - root_width;
    using child_type = bitarray_recursive<1llu << child_width, child_width <= 6>;

    static constexpr size_t root_size = div_ceil(Size, child_type::size);
    using root_type = bitarray_recursive<root_size <= 32 ? 32 : 64, true>;

    using child_array_type = std::array<child_type, root_size>;

public:
    static constexpr size_t size = Size;

    explicit bitarray_recursive() noexcept = default;
    bitarray_recursive(const bitarray_recursive&) noexcept = default;
    bitarray_recursive(bitarray_recursive&&) noexcept = default;
    bitarray_recursive& operator = (const bitarray_recursive&) noexcept = default;
    bitarray_recursive& operator = (bitarray_recursive&&) noexcept = default;

    void set_bit(const size_t i) {
        const auto idx = get_index_(i);
        root_.set_bit(idx.first);
        children_[idx.first].set_bit(idx.second);
    }

    void clear_bit(const size_t i) {
        const auto idx = get_index_(i);
        children_[idx.first].clear_bit(idx.second);
        if (children_[idx.first].empty())
            root_.clear_bit(idx.first);
    }

    bool is_set(const size_t i) const {
        const auto idx = get_index_(i);
        return children_[idx.first].is_set(idx.second);
    }

    void clear_all() {
        root_.clear_all();
        for (auto& child : children_)
            child.clear_all();
    }

    bool empty() const {
        return root_.empty();
    }

    size_t find_lsb() const {
        assert(!empty());

        const size_t child_idx = root_.find_lsb();
        const size_t child_val = children_[child_idx].find_lsb();

        return child_idx * child_type::size + child_val;
    }

private:
    child_array_type children_;
    root_type root_;

    std::pair<size_t, size_t> get_index_(size_t i) const {
        assert(i < size);
        return { i / child_type::size, i % child_type::size };
    }
};

template <size_t Size>
class bitarray_recursive<Size, true>
{
    static_assert(Size <= 64, "Support at most 64 bits");
    using uint_type = typename std::conditional<
              Size <= 32, uint32_t, uint64_t>::type;

public:
    static constexpr size_t size = Size;

    explicit bitarray_recursive() noexcept : flags_(0) { }
    bitarray_recursive(const bitarray_recursive&) noexcept = default;
    bitarray_recursive(bitarray_recursive&&) noexcept = default;
    bitarray_recursive& operator = (const bitarray_recursive&) noexcept = default;
    bitarray_recursive& operator = (bitarray_recursive&&) noexcept = default;

    void set_bit(const size_t i) {
        assert(i < size);
        flags_ |= uint_type(1) << i;
    }

    void clear_bit(const size_t i) {
        assert(i < size);
        flags_ &= ~(uint_type(1) << i);
    }

    bool is_set(const size_t i) const {
        assert(i < size);
        return (flags_ & (uint_type(1) << i));
    }

    void clear_all() {
        flags_ = 0;
    }

    bool empty() const {
        return !flags_;
    }

    size_t find_lsb() const {
        assert(!empty());
        return tlx::ffs(flags_) - 1;
    }

private:
    uint_type flags_;
};

} // namespace details

/*!
 * A bitarray of fixed size supporting reading, setting, and clearing
 * of individual bits. The data structure is optimized to find the bit with
 * smallest index that is set (find_lsb).
 *
 * The bitarray is implemented as a search tree with a fan-out of up to 64.
 * It is thus very flat, and all operations but with the exception of clear_all
 * have a complexity of O(log_64(Size)) which is << 10 for all practical purposes.
 */
template <size_t Size>
class bitarray
{
    using impl_type = details::bitarray_recursive<Size, Size <= 64>;

public:
    static constexpr size_t size = Size;

    explicit bitarray() noexcept = default;
    bitarray(const bitarray&) noexcept = default;
    bitarray(bitarray&&) noexcept = default;
    bitarray& operator = (const bitarray&) noexcept = default;
    bitarray& operator = (bitarray&&) noexcept = default;

    //! Set the i-th bit to true
    void set_bit(const size_t i) {
        impl_.set_bit(i);
    }

    //! Set the i-th bit to false
    void clear_bit(const size_t i) {
        impl_.clear_bit(i);
    }

    //! Returns value of the i-th
    bool is_set(const size_t i) const {
        return impl_.is_set(i);
    }

    //! Sets all bits to false
    void clear_all() {
        impl_.clear_all();
    }

    //! True if all bits are false
    bool empty() const {
        return impl_.empty();
    }

    //! Finds the bit with smallest index that is set
    //! \Warning: If empty() is true, the result is undefined
    size_t find_lsb() const {
        return impl_.find_lsb();
    }

private:
    impl_type impl_;
};

} // namespace tlx

#endif // !TLX_BITARRAY_HEADER

/******************************************************************************/
