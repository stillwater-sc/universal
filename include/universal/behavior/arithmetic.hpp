#pragma once
// arithmetic.hpp: enum types used to classify arithmetic behavior, such as Modular vs Saturating arithmetic, Projective vs Real arithmetic, etc.
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

enum class InfiniteLimit : bool{Finite, Infinite};
enum class Arithmetic : bool{Modular, Saturating};

struct ArithmeticBehavior {
	constexpr ArithmeticBehavior(Arithmetic a, InfiniteLimit l) : arith(a), limit(l) {};
	Arithmetic    arith;
	InfiniteLimit limit;
};

constexpr ArithmeticBehavior Modular(Arithmetic::Modular, InfiniteLimit::Finite);
constexpr ArithmeticBehavior Saturating(Arithmetic::Saturating, InfiniteLimit::Finite);
constexpr ArithmeticBehavior Projective(Arithmetic::Modular, InfiniteLimit::Infinite);
constexpr ArithmeticBehavior Real(Arithmetic::Saturating, InfiniteLimit::Infinite);

}} // namespace sw::universal
