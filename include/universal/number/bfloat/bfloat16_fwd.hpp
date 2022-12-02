#pragma once
// bfloat16_fwd.hpp: forward definitions of the Google Brain Float number system
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// forward references
class bfloat16;
bool parse(const std::string& number, bfloat16& v);

}}  // namespace sw::universal
