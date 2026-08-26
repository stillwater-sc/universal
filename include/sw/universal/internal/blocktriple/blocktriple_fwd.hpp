#pragma once
// blocktriple_fwd.hpp: type forwards for blocktriple
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// blocktriple.hpp is ~70,000 preprocessed lines. A header that only needs to NAME the
// type -- to declare a parameter or a return type -- should include this instead and
// let the definition arrive at the point of instantiation (#1334).
//
// integer<> is the case that motivated it: its only use of blocktriple is
// integer::normalize(blocktriple<nbits-1, REP, TargetBlockType>&), a member TEMPLATE
// whose body is instantiated only when it is called. Every caller of normalize() (the
// quire) has blocktriple complete already, so the arithmetic core never needed it.
//
// The default template arguments live HERE, not on the definition in blocktriple.hpp:
// a template's default arguments may be given only once across all its declarations.
#include <cstdint>

namespace sw { namespace universal {

// operator specialization tag for blocktriple
enum class BlockTripleOperator { REP, ADD, MUL, DIV, SQRT };

template<unsigned _fbits, BlockTripleOperator _op = BlockTripleOperator::ADD, typename bt = std::uint32_t>
class blocktriple;

}} // namespace sw::universal
