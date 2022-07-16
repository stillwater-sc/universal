#pragma once
// lns_fwd.hpp :  forward declarations of the logarithmic number system environment
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for size_t

namespace sw { namespace universal {

// core lns types
template<size_t nbits, size_t rbits, typename bt> class lns;
template<size_t nbits, size_t rbits, typename bt> lns<nbits, rbits, bt> abs(const lns<nbits, rbits, bt>&);
template<size_t nbits, size_t rbits, typename bt> lns<nbits, rbits, bt> sqrt(const lns<nbits, rbits, bt>&);

}} // namespace sw::universal

