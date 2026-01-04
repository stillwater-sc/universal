// Common implementations
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

static inline uint64_t _horizontal_invert(uint64_t i, uint64_t MASK) {
    // Invert index according to 2's compliment.
    return ((~i & MASK) + 1) & MASK;
}
