#pragma once
// exceptions.hpp: exceptions for problems in posit calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <exception>
#include <string>

struct shift_too_large
  : std::runtime_error
{
    shift_too_large(const std::string& error = "Shift value too large for given bitset") : std::runtime_error(error) {}
};