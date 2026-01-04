// Common implementations
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>


// No of operations supported by the op matrix
#define OP_MATRIX_TOTAL_SUPPORTED_OPS    2


static inline uint64_t _horizontal_invert(uint64_t i, uint64_t MASK) {
    // Invert index according to 2's compliment.
    return ((~i & MASK) + 1) & MASK;
}


namespace sw { namespace universal {

enum op_matrix_type {
    OP_MATRIX_TYPE_ADD = 0,
    OP_MATRIX_TYPE_MUL = 1
};

}}
