#pragma once
//  sorn_traits.hpp : traits for SORN number system
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/integral_constant.hpp>

namespace sw { namespace universal {

// define a trait for sorn types
template<typename _Ty>
struct is_sorn_trait
	: false_type
{
};

template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
struct is_sorn_trait< sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_sorn = is_sorn_trait<_Ty>::value;

template<typename _Ty>
using enable_if_sorn = std::enable_if_t<is_sorn<_Ty>, _Ty>;

}} // namespace sw::universal
