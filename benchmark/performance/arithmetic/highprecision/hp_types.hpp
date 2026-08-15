#pragma once
//  hp_types.hpp : the multi-component types under comparison, and their component counts
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// issue #1315: the benchmarks compare each classic multi-component type against its floatcascade<N>
// counterpart, with double as the reference floor:
//
//     double        1 component,  53 bits of significand
//     dd            2 components, 106 bits, classic Bailey/Hida double-double
//     dd_cascade    2 components, 106 bits, floatcascade<2> with Priest renormalization
//     td_cascade    3 components, 159 bits, floatcascade<3>, no classic counterpart
//     qd            4 components, 212 bits, classic Bailey/Hida quad-double
//     qd_cascade    4 components, 212 bits, floatcascade<4> with Priest renormalization
//
// The component counts are what the optimizer barrier needs: a workload that only consumes the
// leading limb lets the compiler delete the work that produced the trailing ones.
#include <universal/number/dd/dd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>

#include "hp_bench.hpp"

namespace hpbench {

	// every limb goes through the sink, so no part of a result can be proven dead and deleted
	template<typename Scalar, int NR_COMPONENTS>
	struct limbSink {
		static void store(const Scalar& v) {
			for (int i = 0; i < NR_COMPONENTS; ++i) g_sink = v[i];
		}
	};

	template<> struct componentSink<sw::universal::dd> : limbSink<sw::universal::dd, 2> {};
	template<> struct componentSink<sw::universal::dd_cascade> : limbSink<sw::universal::dd_cascade, 2> {};
	template<> struct componentSink<sw::universal::td_cascade> : limbSink<sw::universal::td_cascade, 3> {};
	template<> struct componentSink<sw::universal::qd> : limbSink<sw::universal::qd, 4> {};
	template<> struct componentSink<sw::universal::qd_cascade> : limbSink<sw::universal::qd_cascade, 4> {};

} // namespace hpbench
