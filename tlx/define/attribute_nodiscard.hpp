/*******************************************************************************
 * tlx/define/attribute_nodiscard.hpp
 *
 * Part of tlx - http://panthema.net/tlx
 *
 * Copyright (C) 2021 Manuel Penschuck <tlx@manuel.jetzt>
 *
 * All rights reserved. Published under the Boost Software License, Version 1.0
 ******************************************************************************/

#ifndef TLX_DEFINE_ATTRIBUTE_NODISCARD_HEADER
#define TLX_DEFINE_ATTRIBUTE_NODISCARD_HEADER

namespace tlx {

//! \addtogroup tlx_define
//! \{

/******************************************************************************/
// [[nodiscard]]

#if defined(__cplusplus) && __cplusplus >= 201703L
#define TLX_NODISCARD [[nodiscard]]
#else
#define TLX_NODISCARD
#endif

//! \}

} // namespace tlx

#endif // !TLX_DEFINE_ATTRIBUTE_NODISCARD_HEADER

/******************************************************************************/
