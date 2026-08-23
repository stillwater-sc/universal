#pragma once
// blocktriple_debug.hpp: introspection and tracing for blocktriple
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// blocktriple.hpp declares constexprClassParameters() and the two trace helpers
// but does not define them, so it needs no <iostream>. Their definitions live
// here. Include this header to call constexprClassParameters(), or to enable any
// of the BLOCKTRIPLE_TRACE_* switches: with tracing on, blocktriple's add/mul/div
// instantiate the helpers below, and a translation unit that has not included
// this header will not link.
#include <iostream>
#include <typeinfo>
#include <universal/internal/blocktriple/blocktriple.hpp>

namespace sw { namespace universal {

template<unsigned fbits, BlockTripleOperator op, typename bt>
void blocktriple<fbits, op, bt>::constexprClassParameters() const {
	std::cout << "-------------------------------------------------------------\n";
	std::cout << "type              : " << typeid(*this).name() << '\n';
	std::cout << "fbits             : " << fbits << '\n';
	std::cout << "operator          : " << op << '\n';
	std::cout << "bitsInByte        : " << bitsInByte << '\n';
	std::cout << "bitsInBlock       : " << bitsInBlock << '\n';
	std::cout << "nrBlocks          : " << nrBlocks << '\n';
	std::cout << "storageMask       : " << to_binary(storageMask) << '\n';

	std::cout << "MSU               : " << MSU << '\n';

	std::cout << "fhbits            : " << fhbits << '\n';
	std::cout << "rbits             : " << rbits << "      rounding bits for addition/subtraction\n";
	std::cout << "abits             : " << abits << "      size of the addend = fbits + rbits\n";
	std::cout << "mbits             : " << mbits << "      size of the multiplier output\n";
	std::cout << "divbits           : " << divbits << "      size of the divider output\n";
	std::cout << "sqrtbits          : " << sqrtbits << "      size of the square root output\n";
	// we transform input operands into the operation's target output size
	// so that everything is aligned correctly before the operation starts.
	std::cout << "bfbits            : " << bfbits << "      bits in the blocksignificand representation\n";
	std::cout << "radix             : " << radix << "      position of the radix point of the ALU operator result\n";
//		std::cout << "encoding          : " << encoding << '\n';
	std::cout << "normalBits        : " << normalBits << "      normal bits to track: metaprogramming trick to remove warnings\n";
	std::cout << "normalFormMask    : " << to_binary(normalFormMask) << "   normalFormMask for small configurations\n";
	std::cout << "significand type  : " << typeid(significand_t).name() << '\n';

	std::cout << "ALL_ONES          : " << to_binary(ALL_ONES) << '\n';
	std::cout << "maxbits           : " << maxbits << "        bit to check for overflow: metaprogramming trick\n";
	std::cout << "overflowPattern   : " << to_binary(overflowPattern) << '\n';

	std::cout << std::endl;
}

// operand-level trace: the significands as the ALU sees them, before normalization
template<unsigned fbits, BlockTripleOperator op, typename bt>
void blocktriple<fbits, op, bt>::traceSignificandOp(const char* header, const char* label, const blocktriple& lhs, const blocktriple& rhs) const {
	std::cout << header << '\n';
	std::cout << typeid(_significand).name() << '\n';
	std::cout << "lhs significand : " << to_binary(lhs._significand) << " : " << lhs._significand << '\n';
	std::cout << "rhs significand : " << to_binary(rhs._significand) << " : " << rhs._significand << '\n';
	std::cout << label << " significand : " << to_binary(_significand) << " : " << _significand << '\n';
}

// result-level trace: the normalized blocktriple
template<unsigned fbits, BlockTripleOperator op, typename bt>
void blocktriple<fbits, op, bt>::traceNormalizedOp(const char* header, const char* label, const blocktriple& lhs, const blocktriple& rhs) const {
	std::cout << header << '\n';
	std::cout << typeid(*this).name() << '\n';
	std::cout << "lhs : " << to_binary(lhs) << " : " << lhs << '\n';
	std::cout << "rhs : " << to_binary(rhs) << " : " << rhs << '\n';
	std::cout << label << " : " << to_binary(*this) << " : " << *this << '\n';
}

}} // namespace sw::universal
