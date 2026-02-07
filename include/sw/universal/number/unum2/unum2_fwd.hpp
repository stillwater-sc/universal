#pragma once
// Operation matrix/table for unum2 operations.
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT


namespace sw { namespace universal {
    /* forward declarations */
    // lattice
    template<int... exacts> class lattice;
    // unum2
    template<typename T> class unum2;
    // Operation matrix
    template<typename T> class op_matrix;


    // Unum2 specialized math functions
    template<typename T> unum2<T> pow(unum2<T> u, int n);
    template<typename T> unum2<T> abs(unum2<T> u);
}}
