#pragma once
// ereal_constants.hpp: high-precision math constants for ereal adaptive precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <string>
#include <cmath>
#include <cstddef>
#include <limits>

namespace sw { namespace universal {

// Mathematical constants for ereal adaptive-precision arithmetic.
//
// These are genuine high-precision constants, NOT double-precision placeholders:
// a `double` literal can carry only ~16 digits, so initializing an ereal from
// one (e.g. ereal<m>(3.14159...long...)) silently truncates to double precision.
// That capped every transcendental at ~16 digits (issue #1002).
//
//   - pi, ln2, ln10 are stored as precomputed non-overlapping double-component
//     expansions (the dd/qd approach), generated offline with MPFR and verified
//     value-accurate to ~313 decimal digits. They are reconstructed by summing
//     the components into an ereal<maxlimbs, FpType> (exact: the components are a valid
//     Priest expansion, so the running sum loses no precision). This bypasses
//     parse() entirely -- parse() degrades past ~130 digits and cannot represent
//     a 300-digit constant (it mangled the 230-digit ln10 string down to ~86
//     correct digits), which left ln10/pi/ln2-dependent transcendentals capped
//     well below full precision (issue #1002, deeper root cause).
//   - the remaining constants are DERIVED from those via exact operations
//     (multiply/divide by powers of two, small integers) or extended-precision
//     algorithms (the ereal sqrt/exp/divide are themselves extended-precision),
//     so their correctness follows from the algorithm rather than from
//     hand-transcribed tail digits.
//
// Each accessor caches its value in a function-local static (computed once per
// ereal<maxlimbs, FpType> instantiation).

namespace ereal_detail {
	// parse a decimal string into an expansion. Retained for short ad-hoc
	// constants (e.g. trigonometry's atan(1/2) seed); NOT used for the base
	// constants below, which are stored as limb expansions to bypass parse's
	// long-string precision loss.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> parse_constant(const char* digits) {
		ereal<maxlimbs, FpType> v;
		v.parse(std::string(digits));
		return v;
	}

	// A stored constant is a sum of (mantissa, exponent) pairs: a double mantissa in
	// [1, 2) and a binary exponent. The pairs are what lets one table serve every limb
	// type. A plain double-component table cannot go below 2^-1074, so it ends after ~19
	// components -- the ~313-digit ceiling these constants had (#1567). With explicit
	// exponents the table continues to 2^-4298, and a limb type reconstructs as many
	// entries as it can hold: double limbs stop at 19 (the next term would be subnormal,
	// which an expansion's components must not be), x87 and binary128 take all 79.
	struct scaled_limb { double m; int e; };

	// Reconstruct an ereal from a stored table. Each term is ldexp(FpType(m), e), which
	// is exact, and the terms are non-overlapping and decreasing, so summing them through
	// two_sum growth is exact too: the result carries the table's precision, bounded only
	// by what ereal<maxlimbs, FpType> can hold.
	//
	// The sum stops where the limb type's range does: a term that is not normal in FpType
	// cannot be a component of an expansion (Shewchuk), and everything below it is smaller
	// still. For double limbs that is entry 19 -- exactly the table these constants had, so
	// their values are unchanged -- while x87 and binary128 take the whole table.
	//
	// It does NOT stop at maxlimbs limbs: the arithmetic does not either (#1572), and a
	// narrow configuration has always carried the constant to the full precision of the
	// table. Capping here instead would silently change every ereal<n> below 19.
	template<unsigned maxlimbs, typename FpType, std::size_t N>
	inline ereal<maxlimbs, FpType> from_limbs(const scaled_limb (&components)[N]) {
		ereal<maxlimbs, FpType> v(0.0);
		if (N == 0) return v;
		for (std::size_t i = 0; i < N; ++i) {
			if constexpr (std::numeric_limits<FpType>::digits >= std::numeric_limits<double>::digits) {
				// the mantissa is exact in the limb type, and so is the scaling
				const FpType term = std::ldexp(static_cast<FpType>(components[i].m), components[i].e);
				if (!std::isnormal(term)) break;
				v += ereal<maxlimbs, FpType>(term);
			}
			else {
				// A narrower limb cannot hold a 53-bit mantissa, and casting it would throw
				// away 29 of its bits -- which left ereal<5, float>'s pi at 7 digits. The
				// term is built as a double and split into limbs by the conversion, which
				// is exact (#1564).
				const double term = std::ldexp(components[i].m, components[i].e);
				if (term == 0.0) break;
				const ereal<maxlimbs, FpType> t(term);
				if (t.iszero()) break;
				// EVERY limb of the split must be normal, not just the leading one: a
				// subnormal component is not a valid part of an expansion, and one slipped
				// in makes the constant inexact under scaling -- pi/2 * 2 stopped agreeing
				// with pi after 45 digits for float limbs.
				bool normal = true;
				for (FpType c : t.limbs()) normal = normal && std::isnormal(c);
				if (!normal) break;                                      // past the limb type's range
				v += t;
			}
		}
		return v;
	}

	// pi (Archimedes' constant)
	// 79 components, value-accurate to ~1300 decimal digits
	static constexpr scaled_limb ereal_pi_limbs[] = {
		{ 1.5707963267948966, 1 }, { 1.1030637736600981, -53 },
		{ -1.9437167343794348, -109 }, { 1.3006829321979858, -163 },
		{ 1.194711497303197, -217 }, { 1.0593532623276969, -275 },
		{ 1.3187749878599735, -330 }, { 1.5052539966008873, -385 },
		{ 1.728500942564475, -441 }, { -1.9289681805133119, -497 },
		{ 1.4764878640863075, -552 }, { 1.6492908846230399, -606 },
		{ -1.6426694775082384, -661 }, { -1.8945489743710715, -716 },
		{ -1.0861472432109665, -770 }, { -1.6071392062305625, -826 },
		{ 1.4256443981893288, -880 }, { 1.3896335123229455, -934 },
		{ 1.5276711524004638, -988 }, { -1.3595493292686542, -1048 },
		{ 1.363828458748309, -1102 }, { -1.8067372403432622, -1157 },
		{ -1.0948899831017598, -1212 }, { 1.4017096020587754, -1266 },
		{ -1.0532214777034024, -1320 }, { 1.5788614136155301, -1375 },
		{ -1.7258573290347257, -1430 }, { -1.5344834293882612, -1486 },
		{ -1.2114119094313458, -1540 }, { 1.963755353832232, -1595 },
		{ -1.4128579277606097, -1649 }, { -1.3117952078659307, -1703 },
		{ -1.541828784499639, -1757 }, { 1.5240210595693688, -1812 },
		{ -1.6804913679729705, -1867 }, { 1.4124865774379167, -1921 },
		{ 1.614298406376267, -1977 }, { 1.4562050083277736, -2031 },
		{ 1.718232457952477, -2087 }, { -1.273252905485442, -2143 },
		{ -1.9116893068142538, -2200 }, { 1.68183678602858, -2254 },
		{ -1.4508533305148545, -2310 }, { -1.0959181145177246, -2365 },
		{ 1.8027061222245322, -2419 }, { 1.2540985976232206, -2475 },
		{ 1.961964794100799, -2529 }, { -1.8117834180361863, -2583 },
		{ 1.0911785590846408, -2638 }, { 1.7593107332271023, -2692 },
		{ -1.8728310479716073, -2746 }, { -1.3251180990046567, -2800 },
		{ 1.9770677441316673, -2855 }, { 1.3213205342662082, -2911 },
		{ -1.9635655909262217, -2965 }, { -1.7997727563384671, -3019 },
		{ 1.61014470613673, -3074 }, { 1.8782445941230346, -3129 },
		{ -1.907405195798147, -3183 }, { 1.4146492737035083, -3237 },
		{ -1.245244756737848, -3292 }, { -1.9606953027368226, -3349 },
		{ -1.8256278508632178, -3405 }, { -1.5755984838722805, -3460 },
		{ -1.8663982096140126, -3514 }, { -1.5046087460149915, -3570 },
		{ -1.8223421867607237, -3626 }, { 1.0449562609430407, -3681 },
		{ -1.6887912328064394, -3735 }, { -1.5418048631555594, -3789 },
		{ 1.7497248137748902, -3844 }, { 1.9130634500529606, -3899 },
		{ 1.142787038971806, -3956 }, { -1.7809543063791822, -4013 },
		{ 1.1961511549679489, -4073 }, { 1.9289816580203358, -4129 },
		{ 1.6695492723315184, -4188 }, { -1.9538528918240696, -4244 },
		{ -1.354509770290395, -4298 },
	};
	// ln(2) (natural log of 2)
	// 79 components, value-accurate to ~1300 decimal digits
	static constexpr scaled_limb ereal_ln2_limbs[] = {
		{ 1.3862943611198906, -1 }, { 1.671049338670869, -56 },
		{ 1.4818058297110612, -111 }, { -1.6754337732548843, -165 },
		{ -1.1392006982648661, -219 }, { 1.8457293321138242, -274 },
		{ 1.583297448925901, -328 }, { 1.8529870760416438, -385 },
		{ 1.2662252921518922, -440 }, { -1.5701704235520795, -494 },
		{ -1.8567116070364675, -548 }, { 1.0745813567006006, -604 },
		{ -1.5661808938012436, -658 }, { 1.4360492505001876, -712 },
		{ -1.1378840253679765, -766 }, { 1.0991017792472597, -821 },
		{ 1.5061380367233577, -875 }, { -1.6318088011572798, -929 },
		{ -1.8418885063872508, -987 }, { 1.5786737631925716, -1041 },
		{ 1.0515284873302506, -1096 }, { 1.4655045696982238, -1151 },
		{ -1.479014662800619, -1206 }, { 1.40301791114227, -1265 },
		{ 1.7686836059746853, -1319 }, { -1.661877056938829, -1373 },
		{ -1.5501005289748861, -1427 }, { 1.632335490867795, -1484 },
		{ 1.1687751770179102, -1539 }, { -1.5670589806789075, -1593 },
		{ -1.7216642110479345, -1647 }, { -1.5029706046588942, -1703 },
		{ -1.4886683436173638, -1757 }, { -1.4538946720159152, -1811 },
		{ 1.5386788479549038, -1867 }, { -1.9409347339038896, -1921 },
		{ 1.5534615727484897, -1975 }, { 1.1890079541002938, -2030 },
		{ -1.5754294584739192, -2085 }, { 1.3190636185221047, -2140 },
		{ -1.144130804953582, -2194 }, { -1.1683528906000595, -2248 },
		{ -1.1829828421789188, -2302 }, { 1.784821937149055, -2356 },
		{ -1.8389297598648473, -2410 }, { -1.0478235907770888, -2470 },
		{ 1.9739310499627258, -2524 }, { -1.3663703012501311, -2583 },
		{ -1.7330464392776126, -2637 }, { -1.9334087711061974, -2691 },
		{ -1.9293295038024507, -2745 }, { 1.288852147797232, -2800 },
		{ -1.0916942940665109, -2856 }, { 1.30961541792978, -2910 },
		{ -1.8062944821575684, -2964 }, { 1.513952501325918, -3018 },
		{ 1.362524899416634, -3074 }, { 1.0201839036569884, -3128 },
		{ -1.7182734134228472, -3182 }, { 1.4509824247708223, -3236 },
		{ 1.383796308577375, -3290 }, { 1.0684119001544883, -3346 },
		{ 1.4298730576591292, -3400 }, { 1.6909287401732815, -3455 },
		{ 1.842420996370223, -3509 }, { -1.9305758277109324, -3563 },
		{ -1.0405408995314245, -3618 }, { 1.5468100079473723, -3672 },
		{ 1.7818256524091682, -3726 }, { 1.9647187429454376, -3780 },
		{ -1.268628251791196, -3834 }, { 1.6129834207687581, -3891 },
		{ -1.3415232282859533, -3948 }, { -1.3660756814374693, -4002 },
		{ 1.3733170842033775, -4057 }, { 1.7083757605564887, -4111 },
		{ 1.388278667840751, -4169 }, { 1.3865162876870025, -4223 },
		{ 1.2191320306203925, -4278 },
	};
	// ln(10) (natural log of 10)
	// 79 components, value-accurate to ~1300 decimal digits
	static constexpr scaled_limb ereal_ln10_limbs[] = {
		{ 1.151292546497023, 1 }, { -1.9552433837472967, -53 },
		{ -1.6200392055346837, -107 }, { -1.176028701449294, -161 },
		{ 1.015684148970807, -215 }, { -1.977792542398015, -271 },
		{ -1.779695505698759, -325 }, { 1.021671564092804, -379 },
		{ -1.8215374370263355, -434 }, { -1.6819243722570356, -490 },
		{ -1.8351704790607832, -544 }, { -1.2355828168838934, -598 },
		{ 1.701844374869092, -652 }, { 1.0863602527257274, -706 },
		{ -1.065771174285553, -760 }, { 1.2581516683223593, -816 },
		{ 1.8845404042247131, -872 }, { -1.3147115422862428, -929 },
		{ -1.7722764224676797, -984 }, { -1.7532545971385984, -1040 },
		{ 1.6695611971210134, -1095 }, { 1.6425597161501613, -1150 },
		{ -1.3440314185671272, -1205 }, { -1.3760969337322653, -1259 },
		{ 1.8799003745292773, -1314 }, { -1.057798754686118, -1369 },
		{ -1.352666103221499, -1424 }, { -1.8705790846461223, -1480 },
		{ -1.7578546811232114, -1534 }, { -1.284726834760307, -1589 },
		{ 1.888570610441996, -1643 }, { -1.2367122074770311, -1700 },
		{ -1.3204556381392938, -1754 }, { 1.7305062193873053, -1809 },
		{ -1.838439703110713, -1864 }, { 1.677428960763911, -1918 },
		{ -1.4899418313040356, -1972 }, { -1.4017984926593048, -2028 },
		{ -1.1761184467028836, -2083 }, { 1.5802571757241435, -2139 },
		{ -1.908179214305265, -2194 }, { 1.765432876953771, -2248 },
		{ -1.157916597320229, -2304 }, { 1.04166257489596, -2358 },
		{ -1.2349130872366898, -2414 }, { -1.9003082441366017, -2470 },
		{ -1.9367201267389786, -2525 }, { 1.0616470004658916, -2580 },
		{ -1.823838921038496, -2634 }, { -1.7531108387830272, -2688 },
		{ -1.3481602459160793, -2742 }, { 1.549050362720181, -2796 },
		{ -1.9980151606476102, -2851 }, { -1.9703878483274482, -2906 },
		{ 1.6950302336714445, -2961 }, { 1.3858485084767034, -3017 },
		{ -1.161696203960661, -3074 }, { 1.5222560423538904, -3129 },
		{ 1.9769653890473171, -3184 }, { -1.1289494149323478, -3238 },
		{ 1.1856245125453, -3292 }, { 1.4624492946493017, -3347 },
		{ 1.1488730456455116, -3402 }, { 1.659595203305197, -3456 },
		{ -1.6424293841267077, -3510 }, { -1.1979982402198737, -3568 },
		{ 1.976037038442269, -3623 }, { 1.4062173570898444, -3678 },
		{ 1.5019563418328716, -3733 }, { 1.8552959891054401, -3791 },
		{ 1.419910944131797, -3845 }, { -1.7161674873512018, -3902 },
		{ -1.2940656965699169, -3958 }, { 1.1549322366797508, -4012 },
		{ -1.3713074776244352, -4068 }, { 1.673191530743959, -4125 },
		{ 1.5437247102675804, -4179 }, { -1.3376030304623399, -4235 },
		{ 1.1277488038201542, -4292 },
	};
}

// ---- base constants, reconstructed from stored limb expansions ----

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_pi() {
	static const ereal<maxlimbs, FpType> v = ereal_detail::from_limbs<maxlimbs, FpType>(ereal_detail::ereal_pi_limbs);
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_ln2() {
	static const ereal<maxlimbs, FpType> v = ereal_detail::from_limbs<maxlimbs, FpType>(ereal_detail::ereal_ln2_limbs);
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_ln10() {
	static const ereal<maxlimbs, FpType> v = ereal_detail::from_limbs<maxlimbs, FpType>(ereal_detail::ereal_ln10_limbs);
	return v;
}

// ---- pi multiples / fractions: exact scaling by powers of two and small ints ----

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_pi_2() {  // pi/2  (exact: pi * 2^-1)
	static const ereal<maxlimbs, FpType> v = ereal_pi<maxlimbs, FpType>() * ereal<maxlimbs, FpType>(0.5);
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_pi_4() {  // pi/4  (exact)
	static const ereal<maxlimbs, FpType> v = ereal_pi<maxlimbs, FpType>() * ereal<maxlimbs, FpType>(0.25);
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_3pi_4() {  // 3*pi/4  (exact: 3/4 is exact in binary)
	static const ereal<maxlimbs, FpType> v = ereal_pi<maxlimbs, FpType>() * ereal<maxlimbs, FpType>(0.75);
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_2pi() {  // 2*pi  (exact)
	static const ereal<maxlimbs, FpType> v = ereal_pi<maxlimbs, FpType>() * ereal<maxlimbs, FpType>(2.0);
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_pi_3() {  // pi/3  (extended-precision divide)
	static const ereal<maxlimbs, FpType> v = ereal_pi<maxlimbs, FpType>() / ereal<maxlimbs, FpType>(3.0);
	return v;
}

// ---- square roots: extended-precision ereal Newton sqrt ----

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_sqrt2() {
	static const ereal<maxlimbs, FpType> v = sqrt(ereal<maxlimbs, FpType>(2.0));
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_sqrt3() {
	static const ereal<maxlimbs, FpType> v = sqrt(ereal<maxlimbs, FpType>(3.0));
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_sqrt5() {
	static const ereal<maxlimbs, FpType> v = sqrt(ereal<maxlimbs, FpType>(5.0));
	return v;
}

// ---- e and golden ratio: derived ----

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_e() {  // e = exp(1)  (extended-precision Taylor)
	static const ereal<maxlimbs, FpType> v = exp(ereal<maxlimbs, FpType>(1.0));
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_phi() {  // phi = (1 + sqrt(5)) / 2
	static const ereal<maxlimbs, FpType> v = (ereal<maxlimbs, FpType>(1.0) + ereal_sqrt5<maxlimbs, FpType>()) * ereal<maxlimbs, FpType>(0.5);
	return v;
}

// ---- logarithm bases: derived from ln2 / ln10 (extended-precision divide) ----

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_lge() {  // log2(e) = 1/ln(2)
	static const ereal<maxlimbs, FpType> v = ereal<maxlimbs, FpType>(1.0) / ereal_ln2<maxlimbs, FpType>();
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_lg10() {  // log2(10) = ln(10)/ln(2)
	static const ereal<maxlimbs, FpType> v = ereal_ln10<maxlimbs, FpType>() / ereal_ln2<maxlimbs, FpType>();
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_log2() {  // log10(2) = ln(2)/ln(10)
	static const ereal<maxlimbs, FpType> v = ereal_ln2<maxlimbs, FpType>() / ereal_ln10<maxlimbs, FpType>();
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_loge() {  // log10(e) = 1/ln(10)
	static const ereal<maxlimbs, FpType> v = ereal<maxlimbs, FpType>(1.0) / ereal_ln10<maxlimbs, FpType>();
	return v;
}

// ---- reciprocals / other derived constants ----

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_1_phi() {  // 1/phi = phi - 1  (exact identity)
	static const ereal<maxlimbs, FpType> v = ereal_phi<maxlimbs, FpType>() - ereal<maxlimbs, FpType>(1.0);
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_1_e() {  // 1/e
	static const ereal<maxlimbs, FpType> v = ereal<maxlimbs, FpType>(1.0) / ereal_e<maxlimbs, FpType>();
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_1_pi() {  // 1/pi
	static const ereal<maxlimbs, FpType> v = ereal<maxlimbs, FpType>(1.0) / ereal_pi<maxlimbs, FpType>();
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_2_pi() {  // 2/pi
	static const ereal<maxlimbs, FpType> v = ereal<maxlimbs, FpType>(2.0) / ereal_pi<maxlimbs, FpType>();
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_1_sqrt2() {  // 1/sqrt(2) = sqrt(2)/2  (exact scaling)
	static const ereal<maxlimbs, FpType> v = ereal_sqrt2<maxlimbs, FpType>() * ereal<maxlimbs, FpType>(0.5);
	return v;
}

template<unsigned maxlimbs = 19, typename FpType = double>
inline ereal<maxlimbs, FpType> ereal_2_sqrtpi() {  // 2/sqrt(pi)
	static const ereal<maxlimbs, FpType> v = ereal<maxlimbs, FpType>(2.0) / sqrt(ereal_pi<maxlimbs, FpType>());
	return v;
}

}} // namespace sw::universal
