#pragma once
// multidouble_constant_tables.hpp: the entries of the multi-double constant tables, for
// multidouble_constants.hpp (#1849)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// One table per type: each entry's name, its value, and the constant it holds.
// dd's table names ln(2) and ln(10) twice, as dd_ln2/dd_ln10 and dd_log2/dd_log10, where the other
// types' log2 is log10(2).
#include <array>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/multidouble_constants.hpp>

namespace sw { namespace universal { namespace multidouble_verification {


	inline std::array<table_entry<dd>, 24> dd_constant_table() {
		return { {
			{ "dd_pi_4", dd_pi_4, constant_id::pi_4 },
			{ "dd_pi_3", dd_pi_3, constant_id::pi_3 },
			{ "dd_pi_2", dd_pi_2, constant_id::pi_2 },
			{ "dd_pi", dd_pi, constant_id::pi },
			{ "dd_3pi_4", dd_3pi_4, constant_id::three_pi_4 },
			{ "dd_2pi", dd_2pi, constant_id::two_pi },
			{ "dd_phi", dd_phi, constant_id::phi },
			{ "dd_e", dd_e, constant_id::e },
			{ "dd_log2", dd_log2, constant_id::ln2 },
			{ "dd_log10", dd_log10, constant_id::ln10 },
			{ "dd_ln2", dd_ln2, constant_id::ln2 },
			{ "dd_ln10", dd_ln10, constant_id::ln10 },
			{ "dd_lge", dd_lge, constant_id::log2e },
			{ "dd_lg10", dd_lg10, constant_id::log2_10 },
			{ "dd_loge", dd_loge, constant_id::log10e },
			{ "dd_sqrt2", dd_sqrt2, constant_id::sqrt2 },
			{ "dd_sqrt3", dd_sqrt3, constant_id::sqrt3 },
			{ "dd_sqrt5", dd_sqrt5, constant_id::sqrt5 },
			{ "dd_1_phi", dd_1_phi, constant_id::inv_phi },
			{ "dd_1_e", dd_1_e, constant_id::inv_e },
			{ "dd_1_pi", dd_1_pi, constant_id::inv_pi },
			{ "dd_2_pi", dd_2_pi, constant_id::two_inv_pi },
			{ "dd_1_sqrt2", dd_1_sqrt2, constant_id::inv_sqrt2 },
			{ "dd_2_sqrtpi", dd_2_sqrtpi, constant_id::two_over_sqrt_pi },
		} };
	}

	inline std::array<table_entry<qd>, 23> qd_constant_table() {
		return { {
			{ "qd_pi_4", qd_pi_4, constant_id::pi_4 },
			{ "qd_pi_3", qd_pi_3, constant_id::pi_3 },
			{ "qd_pi_2", qd_pi_2, constant_id::pi_2 },
			{ "qd_pi", qd_pi, constant_id::pi },
			{ "qd_3pi_4", qd_3pi_4, constant_id::three_pi_4 },
			{ "qd_2pi", qd_2pi, constant_id::two_pi },
			{ "qd_phi", qd_phi, constant_id::phi },
			{ "qd_e", qd_e, constant_id::e },
			{ "qd_ln2", qd_ln2, constant_id::ln2 },
			{ "qd_ln10", qd_ln10, constant_id::ln10 },
			{ "qd_lge", qd_lge, constant_id::log2e },
			{ "qd_lg10", qd_lg10, constant_id::log2_10 },
			{ "qd_log2", qd_log2, constant_id::log10_2 },
			{ "qd_loge", qd_loge, constant_id::log10e },
			{ "qd_sqrt2", qd_sqrt2, constant_id::sqrt2 },
			{ "qd_sqrt3", qd_sqrt3, constant_id::sqrt3 },
			{ "qd_sqrt5", qd_sqrt5, constant_id::sqrt5 },
			{ "qd_1_phi", qd_1_phi, constant_id::inv_phi },
			{ "qd_1_e", qd_1_e, constant_id::inv_e },
			{ "qd_1_pi", qd_1_pi, constant_id::inv_pi },
			{ "qd_2_pi", qd_2_pi, constant_id::two_inv_pi },
			{ "qd_1_sqrt2", qd_1_sqrt2, constant_id::inv_sqrt2 },
			{ "qd_2_sqrtpi", qd_2_sqrtpi, constant_id::two_over_sqrt_pi },
		} };
	}

	inline std::array<table_entry<dd_cascade>, 23> dd_cascade_constant_table() {
		return { {
			{ "ddc_pi_4", ddc_pi_4, constant_id::pi_4 },
			{ "ddc_pi_3", ddc_pi_3, constant_id::pi_3 },
			{ "ddc_pi_2", ddc_pi_2, constant_id::pi_2 },
			{ "ddc_pi", ddc_pi, constant_id::pi },
			{ "ddc_3pi_4", ddc_3pi_4, constant_id::three_pi_4 },
			{ "ddc_2pi", ddc_2pi, constant_id::two_pi },
			{ "ddc_phi", ddc_phi, constant_id::phi },
			{ "ddc_e", ddc_e, constant_id::e },
			{ "ddc_ln2", ddc_ln2, constant_id::ln2 },
			{ "ddc_ln10", ddc_ln10, constant_id::ln10 },
			{ "ddc_lge", ddc_lge, constant_id::log2e },
			{ "ddc_lg10", ddc_lg10, constant_id::log2_10 },
			{ "ddc_log2", ddc_log2, constant_id::log10_2 },
			{ "ddc_loge", ddc_loge, constant_id::log10e },
			{ "ddc_sqrt2", ddc_sqrt2, constant_id::sqrt2 },
			{ "ddc_sqrt3", ddc_sqrt3, constant_id::sqrt3 },
			{ "ddc_sqrt5", ddc_sqrt5, constant_id::sqrt5 },
			{ "ddc_1_phi", ddc_1_phi, constant_id::inv_phi },
			{ "ddc_1_e", ddc_1_e, constant_id::inv_e },
			{ "ddc_1_pi", ddc_1_pi, constant_id::inv_pi },
			{ "ddc_2_pi", ddc_2_pi, constant_id::two_inv_pi },
			{ "ddc_1_sqrt2", ddc_1_sqrt2, constant_id::inv_sqrt2 },
			{ "ddc_2_sqrtpi", ddc_2_sqrtpi, constant_id::two_over_sqrt_pi },
		} };
	}

	inline std::array<table_entry<td_cascade>, 23> td_cascade_constant_table() {
		return { {
			{ "tdc_pi_4", tdc_pi_4, constant_id::pi_4 },
			{ "tdc_pi_3", tdc_pi_3, constant_id::pi_3 },
			{ "tdc_pi_2", tdc_pi_2, constant_id::pi_2 },
			{ "tdc_pi", tdc_pi, constant_id::pi },
			{ "tdc_3pi_4", tdc_3pi_4, constant_id::three_pi_4 },
			{ "tdc_2pi", tdc_2pi, constant_id::two_pi },
			{ "tdc_phi", tdc_phi, constant_id::phi },
			{ "tdc_e", tdc_e, constant_id::e },
			{ "tdc_ln2", tdc_ln2, constant_id::ln2 },
			{ "tdc_ln10", tdc_ln10, constant_id::ln10 },
			{ "tdc_lge", tdc_lge, constant_id::log2e },
			{ "tdc_lg10", tdc_lg10, constant_id::log2_10 },
			{ "tdc_log2", tdc_log2, constant_id::log10_2 },
			{ "tdc_loge", tdc_loge, constant_id::log10e },
			{ "tdc_sqrt2", tdc_sqrt2, constant_id::sqrt2 },
			{ "tdc_sqrt3", tdc_sqrt3, constant_id::sqrt3 },
			{ "tdc_sqrt5", tdc_sqrt5, constant_id::sqrt5 },
			{ "tdc_1_phi", tdc_1_phi, constant_id::inv_phi },
			{ "tdc_1_e", tdc_1_e, constant_id::inv_e },
			{ "tdc_1_pi", tdc_1_pi, constant_id::inv_pi },
			{ "tdc_2_pi", tdc_2_pi, constant_id::two_inv_pi },
			{ "tdc_1_sqrt2", tdc_1_sqrt2, constant_id::inv_sqrt2 },
			{ "tdc_2_sqrtpi", tdc_2_sqrtpi, constant_id::two_over_sqrt_pi },
		} };
	}

	inline std::array<table_entry<qd_cascade>, 23> qd_cascade_constant_table() {
		return { {
			{ "qdc_pi_4", qdc_pi_4, constant_id::pi_4 },
			{ "qdc_pi_3", qdc_pi_3, constant_id::pi_3 },
			{ "qdc_pi_2", qdc_pi_2, constant_id::pi_2 },
			{ "qdc_pi", qdc_pi, constant_id::pi },
			{ "qdc_3pi_4", qdc_3pi_4, constant_id::three_pi_4 },
			{ "qdc_2pi", qdc_2pi, constant_id::two_pi },
			{ "qdc_phi", qdc_phi, constant_id::phi },
			{ "qdc_e", qdc_e, constant_id::e },
			{ "qdc_ln2", qdc_ln2, constant_id::ln2 },
			{ "qdc_ln10", qdc_ln10, constant_id::ln10 },
			{ "qdc_lge", qdc_lge, constant_id::log2e },
			{ "qdc_lg10", qdc_lg10, constant_id::log2_10 },
			{ "qdc_log2", qdc_log2, constant_id::log10_2 },
			{ "qdc_loge", qdc_loge, constant_id::log10e },
			{ "qdc_sqrt2", qdc_sqrt2, constant_id::sqrt2 },
			{ "qdc_sqrt3", qdc_sqrt3, constant_id::sqrt3 },
			{ "qdc_sqrt5", qdc_sqrt5, constant_id::sqrt5 },
			{ "qdc_1_phi", qdc_1_phi, constant_id::inv_phi },
			{ "qdc_1_e", qdc_1_e, constant_id::inv_e },
			{ "qdc_1_pi", qdc_1_pi, constant_id::inv_pi },
			{ "qdc_2_pi", qdc_2_pi, constant_id::two_inv_pi },
			{ "qdc_1_sqrt2", qdc_1_sqrt2, constant_id::inv_sqrt2 },
			{ "qdc_2_sqrtpi", qdc_2_sqrtpi, constant_id::two_over_sqrt_pi },
		} };
	}

}}}  // namespace sw::universal::multidouble_verification
