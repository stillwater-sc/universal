#pragma once
// einteger_fwd.hpp: type forwards of adaptive arbitrary precision integer numbers
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// forward references
template<typename BlockType> class einteger;
template<typename BlockType> bool parse(const std::string& number, einteger<BlockType>& v);

}} // namespace sw::universal
