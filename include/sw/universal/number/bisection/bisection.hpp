#pragma once
// bisection.hpp: umbrella header for the bisection number system
//
// A number system framework based on recursive interval bisection.
// Define any number system with just two functions: a generator g(x)
// for bracketing and a refinement f(a,b) for bisection.
//
// Reference: Peter Lindstrom, "Universal Coding of the Reals using
//            Bisection", CoNGA'19 (2019).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <universal/utility/directives.hpp>
#include <universal/utility/compiler.hpp>

#include <iostream>
#include <iomanip>
#include <string>

// Exception behavior
#if !defined(BISECTION_THROW_ARITHMETIC_EXCEPTION)
#define BISECTION_THROW_ARITHMETIC_EXCEPTION 0
#endif

#include <universal/number/bisection/exceptions.hpp>
#include <universal/number/bisection/bisection_fwd.hpp>
#include <universal/number/bisection/bisection_impl.hpp>
#include <universal/number/bisection/numeric_limits.hpp>
#include <universal/number/bisection/manipulators.hpp>
#include <universal/number/bisection/attributes.hpp>
#include <universal/number/bisection/generators.hpp>
