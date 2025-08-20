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

}}  // namespace sw::universal
