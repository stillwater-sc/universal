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

#include <iomanip>
#include <string>

// BISECTION_THROW_ARITHMETIC_EXCEPTION now defaults in bisection_impl.hpp, so that a
// translation unit which includes core.hpp directly gets the same default (#1334,
// #1436). Defining it before this header still wins, as before.

// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation unit
// that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/bisection/core.hpp>
// layer 2a: the string producers -- to_binary, type_tag, color_print, components
#include <universal/number/bisection/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<<
#include <universal/number/bisection/iostream.hpp>
#include <universal/number/bisection/attributes.hpp>
