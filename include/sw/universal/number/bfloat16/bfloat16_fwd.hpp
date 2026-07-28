#pragma once
// bfloat16_fwd.hpp: forward definitions of the Google Brain Float number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// forward references
class bfloat16;
inline bool parse(const std::string& number, bfloat16& v);
// fused multiply-add: a*b + c with a single round-to-nearest-even (see math/functions/fma.hpp)
inline bfloat16 fma(bfloat16 a, bfloat16 b, bfloat16 c);

}}  // namespace sw::universal
