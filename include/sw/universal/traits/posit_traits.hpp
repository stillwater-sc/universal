#pragma once
//  posit_traits.hpp : traits for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/traits/integral_constant.hpp>
//#include <universal/number/posit/posit_fwd.hpp>
// if we want to make this traits header self reliant, we need to include posit.hpp
// but as this is posit2, this design breaks as posit_fwd.hpp is for the old posit design without BlockType.
// All of this works at the present because we are treating traits include files as
// part of the number system included file.

namespace sw { namespace universal {

// define a trait for posit types
template<typename _Ty>
struct is_posit_trait
	: false_type
{
};
template<unsigned nbits, unsigned es>
struct is_posit_trait< sw::universal::posit<nbits, es> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_posit = is_posit_trait<_Ty>::value;

template<typename _Ty, typename Type = void>
using enable_if_posit = std::enable_if_t<is_posit<_Ty>, Type>;

}} // namespace sw::universal
