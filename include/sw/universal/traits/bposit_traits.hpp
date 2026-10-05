#pragma once
// bposit_traits.hpp : traits for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/traits/integral_constant.hpp>
// This traits header relies on being included after the bposit class is defined.

namespace sw { namespace universal {

template<typename _Ty>
struct is_bposit_trait
	: false_type
{
};
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
struct is_bposit_trait< sw::universal::bposit<nbits, rs, es, bt> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_bposit = is_bposit_trait<_Ty>::value;

template<typename _Ty, typename Type = void>
using enable_if_bposit = std::enable_if_t<is_bposit<_Ty>, Type>;

}} // namespace sw::universal
