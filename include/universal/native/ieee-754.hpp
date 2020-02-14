#pragma once
// ieee-754.hpp: simple math functions
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
namespace native {

union float_decoder {
  float f;
  struct {
    uint32_t fraction : 23;
    uint8_t  exponent : 8;
    uint8_t  sign : 1;
  } parts;
};

union double_decoder {
  double d;
  struct {
    uint64_t fraction : 52;
    uint16_t exponent : 11;
    uint8_t  sign : 1;
  } parts;
};

} // namespace native
} // namespace sw
