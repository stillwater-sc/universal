#pragma once
// poxel_traits.hpp : traits for the poxel
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/traits/integral_constant.hpp>
// This traits header relies on being included after the poxel class is defined.

namespace sw { namespace universal {

template<typename _Ty>
struct is_poxel_trait
	: false_type
{
};
template<unsigned nbits, unsigned es, typename bt>
struct is_poxel_trait< sw::universal::poxel<nbits, es, bt> >
	: true_type
{
};

template<typename _Ty>
constexpr bool is_poxel = is_poxel_trait<_Ty>::value;

template<typename _Ty, typename Type = void>
using enable_if_poxel = std::enable_if_t<is_poxel<_Ty>, Type>;

}} // namespace sw::universal
