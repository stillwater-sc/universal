#pragma once
// efloat_test_suite.hpp: verification functions for efloat arithmetic tests
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/number/efloat/efloat.hpp>

namespace sw { namespace universal {

// Verify addition of two efloats
template<unsigned nlimbs>
int VerifyEfloatAddition(bool reportTestCases, const efloat<nlimbs>& a, const efloat<nlimbs>& b, const efloat<nlimbs>& expected) {
    int nrOfFailedTestCases = 0;
    efloat<nlimbs> result = a + b;
    if (result != expected) {
        nrOfFailedTestCases++;
        if (reportTestCases) {
            ReportBinaryArithmeticError("FAIL", "+", a, b, result, expected);
        }
    } else {
        if (reportTestCases) {
            ReportBinaryArithmeticSuccess("PASS", "+", a, b, result, expected);
        }
    }
    return nrOfFailedTestCases;
}

}} // namespace sw::universal
