#pragma once
//  boolean_logic_operators.hpp : full set of boolean logic operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// boolean operators to unify the lack of a bit xor operator in C++
inline bool bnot(bool a) { return !a; }
// AND/NAND
inline bool band(bool a, bool b) { return (a && b); }
inline bool bnand(bool a, bool b) { return !(a && b); }
// OR/NOR
inline bool bor(bool a, bool b) { return (a || b); }
inline bool bnor(bool a, bool b) { return !(a || b); }
// XOR/XNOR
inline bool bxor(bool a, bool b) { return (a || b) && !(a && b); }
inline bool bxnor(bool a, bool b) { return !((a || b) && !(a && b)); }
inline bool bxor(bool a, bool b, bool c) { return bxor(bxor(a, b), c); }

}} // namespace sw::universal
