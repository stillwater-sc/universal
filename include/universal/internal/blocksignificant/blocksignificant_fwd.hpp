#pragma once
// blocksignificant_fwd.hpp: forward definitions for blocksignificant
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

/*
   The fraction bits in a floating-point representation benefit from different
   representations for different operators:
   - for addition and subtraction, a 2's complement encoding is best, 
   - for multiplication, a simple 1's complement encoding is best
   - for division
   - for square root
   a blocksignificant type will be marked by its encoding to enable direct code paths.
   By encoding it in the type, we won't be able to dynamically go between types,
   but that is ok as the blocksignificant is a composition type that gets used
   by the ephemeral blocktriple type, which is set up for each floating-point
   operation, used, and then discarded. 

   The last piece of information we need to manage for blocksignificants is where
   the radix point is. For add/sub it is at a fixed location, nbits - 3, and
   for multiplication and division is transforms from the input values to the
   output values. The blocksignificant operators, add, sub, mul, div, sqrt manage
   this radix point transformation. Fundamentally, the actual bits of the 
   blocksignificant are used as a binary encoded integer. The encoding interpretation
   and the placement of the radix point, are directed by the aggregating class,
   such as blocktriple.
 */
namespace sw { namespace universal {

// forward references
template<unsigned nbits, typename bt> class blocksignificant;
template<unsigned nbits, typename bt> constexpr blocksignificant<nbits, bt> twosComplementFree(const blocksignificant<nbits, bt>&) noexcept;
template<unsigned nbits, typename bt> struct bsquorem;
template<unsigned nbits, typename bt> bsquorem<nbits, bt> longdivision(const blocksignificant<nbits, bt>&, const blocksignificant<nbits, bt>&);

}} // namespace sw::universal
