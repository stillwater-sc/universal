#pragma once
// constants.hpp: definition of base math constants in long double precision for cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// best practice for C++11
// inject mathematical constants in our namespace

// a_b reads a over b, as in 1_pi being 1 over pi

// long double values
constexpr long double cf_pi_4     =  0.78539816339744830961566084581987572104929234984378L; // pi/4
constexpr long double cf_pi_3     =  1.0471975511965977461542144610931676280657231331250L;  // pi/3
constexpr long double cf_pi_2     =  1.5707963267948966192313216916397514420985846996876L;  // pi/2
constexpr long double cf_3pi_4    =  2.35619449019234492884698253745962716314787704953133L; // 3*pi/4
constexpr long double cf_pi       =  3.14159265358979323846264338327950288419716939937510L; //   pi
constexpr long double cf_2pi      =  6.28318530717958647692528676655900576839433879875020L; // 2*pi
constexpr long double cf_3pi      =  9.42477796076937943077094761183850865259150819812530L; // 3*pi
constexpr long double cf_4pi      = 12.56637061435917295385057353311801153678867759750040L; // 4*pi
constexpr long double cf_4_pi     =  1.2732395447351626861510701069801148962756771659237L;  // 4/pi
constexpr long double cf_3_pi     =  0.95492965855137201461330258098310123649020193853168L; // 3/pi
constexpr long double cf_2_pi     =  0.63661977236758134307553505349005744813783858296183L; // 2/pi
constexpr long double cf_1_pi     =  0.31830988618379067153776752674502872406891929148091L; // 1/pi

constexpr long double cf_2_sqrtpi =  1.1283791670955125738961589031215451716881012585786L;  // 2/sqrt(pi)
constexpr long double cf_sqrt2    =  1.4142135623730950488016887242096980785696718753769L;  // sqrt(2)
constexpr long double cf_1_sqrt2  =  0.70710678118654752440084436210484903928483593768847L; // 1/sqrt(2)

constexpr long double cf_e        =  2.71828182845904523536028747135266249775724709369996L; // e
constexpr long double cf_e_gamma  =  0.57721566490153286060651209008240243104215933593992L; // euler gamma
constexpr long double cf_log2e    =  1.44269504088896340735992468100189213742664595415299L; // log2(e)
constexpr long double cf_log10e   =  0.43429448190325182765112891891660508229439700580366L; // log10(e)
constexpr long double cf_ln2      =  0.69314718055994530941723212145817656807550013436025L; // ln(2)
constexpr long double cf_ln3      =  1.09861228866810969139524523692252570464749055782275L; // ln(3)
constexpr long double cf_ln4      =  1.38629436111989061883446424291635313615100026872050L; // ln(4) = 2*ln(2)
constexpr long double cf_ln10     =  2.30258509299404568401799145468436420760110148862877L; // ln(10) -> essential for log10(x) = ln(x)/ln(10)

}} // namespace sw::universal
