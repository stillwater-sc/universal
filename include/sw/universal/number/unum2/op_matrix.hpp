// Operation matrix/table for unum2 operations.
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#pragma once

#include <iostream>
#include <cstdint>

#include <universal/number/unum2/common.hpp>
#include <universal/number/unum2/unum2_impl.hpp>

namespace sw { namespace universal {

template <typename T>
class op_matrix final {
private:
    // Buffer of op matrices.
    unum2<T> *_matrices[OP_MATRIX_TOTAL_SUPPORTED_OPS];
    bool *_present_flags[OP_MATRIX_TOTAL_SUPPORTED_OPS];
    size_t _N;

public:
    op_matrix(size_t N) : _N { N } {
        for(int i = 0; i < OP_MATRIX_TOTAL_SUPPORTED_OPS; i++) {
            _matrices[i] = nullptr;
            _present_flags[i] = nullptr;
        }

        try {
            for(int i = 0; i < OP_MATRIX_TOTAL_SUPPORTED_OPS; i++) {
                _matrices[i] = new unum2<T>[N * N];
                _present_flags[i] = new bool[N * N];
            }
        } catch(const std::bad_alloc& e) {
            cleanup();
            throw;  // rethrow
        }
        
        for(int i = 0; i < OP_MATRIX_TOTAL_SUPPORTED_OPS; i++) {
            for(int j = 0; j < N * N; j++) 
                _present_flags[i][j] = false;
        }
    }

    op_matrix(const op_matrix&) = delete;
    op_matrix& operator=(const op_matrix&) = delete;
    op_matrix(op_matrix&&) = delete;
    op_matrix& operator=(op_matrix&&) = delete;

    ~op_matrix() {
        cleanup();
    }

    // Check if op_matrix has an operation
    bool has(uint64_t i, uint64_t j, op_matrix_type type) {
        return _present_flags[type][i * _N + j];
    }

    // Set a op_matrix operation
    void set(uint64_t i, uint64_t j, op_matrix_type type, unum2<T> num) {
        if(has(i, j, type)) 
            return;

        _matrices[type][i * _N + j] = num;
        _present_flags[type][i * _N + j] = true;
    }

    unum2<T> get(uint64_t i, uint64_t j, op_matrix_type type) {
        if(!has(i, j, type)) 
            throw std::runtime_error("The given operation has to been set/interned yet!");

        return _matrices[type][i * _N + j];
    }

private:
    void cleanup() {
        for(int i = 0; i < OP_MATRIX_TOTAL_SUPPORTED_OPS; i++) {
            delete[] _matrices[i];
            delete[] _present_flags[i];

            _matrices[i] = nullptr;
            _present_flags[i] = nullptr;
        }
    }
};

}}
