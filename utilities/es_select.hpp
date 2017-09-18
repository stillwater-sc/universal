// es_select.hpp: select es value at run-time
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#pragma once

#include <type_traits>



template <std::size_t ES>
using es_tag = std::integral_constant<std::size_t, ES>;