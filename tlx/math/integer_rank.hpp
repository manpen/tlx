/*******************************************************************************
 * tlx/math/integer_rank.hpp
 *
 * Part of tlx - http://panthema.net/tlx
 *
 * Copyright (C) 2018 Manuel Penschuck <tlx@manuel.jetzt>
 *
 * All rights reserved. Published under the Boost Software License, Version 1.0
 ******************************************************************************/

#ifndef TLX_MATH_INTEGER_RANK_HEADER
#define TLX_MATH_INTEGER_RANK_HEADER

#include <limits>
#include <type_traits>

namespace tlx {

//! \addtogroup tlx_math
//! \{

/*!
 * Compute the rank of an integer x (i.e. the number of elements smaller than x
 * that are representable using type Int) and vice versa.
 * If Int is an unsigned integral type, all computations yield identity.
 * If Int is a signed integrals, the smallest (negative) number is mapped to
 * rank zero, the next larger value to one and so on.
 *
 * The implementation assumes negative numbers are implemented as Two's
 * complement and contains static_asserts failing if this is not the case.
 */
template <typename Int>
class integer_rank
{
    static_assert(std::is_integral<Int>::value,
                  "SignedInt has to be an integral type");

public:
    using int_type = Int;
    using rank_type = typename std::make_unsigned<int_type>::type;

    //! Maps value i to its rank in int_type. For any pair T x < y the invariant
    //! integer_rank<T>::rank_of_int(x) < integer_rank<T>::rank_of_int(y) holds.
    static constexpr rank_type rank_of_int(int_type i) {
        return use_identity_
               ? static_cast<rank_type>(i)
               : static_cast<rank_type>(i) ^ sign_bit_;
    }

    //! Returns the r-th smallest number of int_r. It is the inverse of
    //! rank_of_int, i.e. int_at_rank(rank_of_int(i)) == i for all i.
    static constexpr int_type int_at_rank(rank_type r) {
        return use_identity_
               ? static_cast<int_type>(r)
               : static_cast<int_type>(r ^ sign_bit_);
    }

private:
    constexpr static bool use_identity_ = !std::is_signed<int_type>::value;

    constexpr static rank_type sign_bit_
        = (rank_type(1) << (8 * sizeof(rank_type) - 1));

    // These test fail if a signed type does not use Two's complement
    static_assert(rank_of_int(std::numeric_limits<int_type>::min()) == 0,
                  "Rank of minimum is not zero");
    static_assert(rank_of_int(std::numeric_limits<int_type>::min() + 1) == 1,
                  "Rank of minimum+1 is not one");
    static_assert(rank_of_int(std::numeric_limits<int_type>::max())
                  == std::numeric_limits<rank_type>::max(),
                  "Rank of maximum is not maximum rank");
    static_assert(rank_of_int(std::numeric_limits<int_type>::max()) >
                  rank_of_int(int_type(0)),
                  "Rank of maximum is not larger than rank of zero");
};

//! \}

} // namespace tlx

#endif // !TLX_MATH_INTEGER_RANK_HEADER

/******************************************************************************/
