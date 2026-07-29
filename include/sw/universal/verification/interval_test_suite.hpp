#pragma once
//  interval_test_suite.hpp : shared verification helpers for interval<Scalar>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/interval/interval.hpp>

namespace sw { namespace universal {

	// enclosureOK: the acceptance policy for an interval arithmetic result against the
	// expected (true) enclosure. EFT (native float) types must produce the EXACT enclosure
	// for exactly-representable operands (Stage 2, #1247); Universal fixed-size types keep
	// Stage-1 outward rounding and are only required to CONTAIN the true result (#1234).
	template<typename Scalar>
	inline bool enclosureOK(const interval<Scalar>& expected, const interval<Scalar>& computed) {
		if constexpr (interval_detail::interval_eft_exact<Scalar>::value) return expected == computed;
		else return expected.subset_of(computed);
	}

}} // namespace sw::universal
