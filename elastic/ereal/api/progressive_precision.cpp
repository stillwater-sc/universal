// progressive_precision.cpp: demonstrate precision scaling with maxlimbs for ereal mathlib functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// OVERVIEW:
// ---------
// This test demonstrates that ereal adaptive-precision arithmetic delivers more
// correct digits as maxlimbs increases. Each limb carries one IEEE-754 double,
// contributing ~53 bits == ~15.95 decimal digits, so the expected accuracy is:
//
//   ereal<4>  : 212 bits  -> ~ 64 decimal digits
//   ereal<8>  : 424 bits  -> ~128 decimal digits
//   ereal<12> : 636 bits  -> ~192 decimal digits
//   ereal<16> : 848 bits  -> ~256 decimal digits
//   ereal<19> : 1007 bits -> ~304 decimal digits   (Shewchuk's limit, static_assert maxlimbs<=19)
//
// METHODOLOGY:
// ------------
// Each function is evaluated at every level and compared against a ground-truth
// reference that is MORE precise than the level being measured. The references
// are stored as precomputed non-overlapping double-component expansions
// (generated offline with MPFR, ~300 decimal digits) and reconstructed by
// summing the components into an ereal<19> -- this is exact and, crucially,
// bypasses ereal::parse(), which loses precision past ~130 digits and cannot
// represent a 300-digit reference. (An earlier revision of this test parsed
// 300-digit reference strings and was silently capped at the *reference's* ~24
// digits, masking the functions' true accuracy.) The computed value is widened
// to ereal<19> (exact: its non-overlapping limbs are re-summed) and the relative
// error is taken in ereal<19> arithmetic; correct digits == -log10 of that error.
//
// CRITICAL -- references match the COMPUTED input, not the exact decimal:
// inputs flow through std::stod, so e.g. cos(0.3) evaluates cos(double(0.3))
// where double(0.3) = 0.299999999999999988... . Every reference below was
// generated from the exact double value of its input (mpmath.mpf(python_float)),
// NOT from the exact decimal; a decimal-built reference would diverge from the
// computation after ~16 digits and produce false precision-loss reports (the
// original failure mode behind issue #1002 / the cos(0.3) discrepancy).
//
// REFERENCE GENERATION (reproducible):
// ------------------------------------
//   from mpmath import mp, mpf, cos
//   mp.dps = 400
//   v = cos(mpf(0.3))                 # exact double value of the input
//   r = v; limbs = []
//   for _ in range(20):               # extract the non-overlapping expansion
//       d = float(r); limbs.append(d); r = r - mpf(d)
//

#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>

namespace sw { namespace universal {

// Highest maxlimbs level (index into the ladder) that is computed and measured.
// Lower regression levels stop early so CI stays fast; the reference is always
// the full ~300-digit expansion, so every measured level is an honest comparison.
//   L1 -> ereal<8> (2 honest points: 64, 128 digits)
//   L2 -> ereal<12>, L3 -> ereal<16>, L4 -> ereal<19> (full ladder)
static int g_max_level = 1;

constexpr int  LEVELS = 5;
const char*    MAXLIMBS_LABELS[LEVELS] = { "ereal<4> ", "ereal<8> ", "ereal<12>", "ereal<16>", "ereal<19>" };

// Expected correct digits == ~15.95 digits/limb, with a ~6% safety margin to
// absorb last-ULP wobble of the transcendental algorithms and the ~304-digit
// representable ceiling of the ereal<19> reference at the top level.
const double PRECISION_THRESHOLDS[LEVELS] = {
	60.0,    // ereal<4>   (~ 64 expected)
	120.0,   // ereal<8>   (~128 expected)
	180.0,   // ereal<12>  (~192 expected)
	240.0,   // ereal<16>  (~256 expected)
	290.0    // ereal<19>  (~304 expected, bounded by reference precision)
};

// Correct decimal digits implied by a relative error, capped at 320 (just above
// the ~304-digit reference ceiling); an exact match reports the cap.
double decimal_digits_precision(double relative_error) {
	if (relative_error <= 0.0) return 320.0;
	double d = -std::log10(relative_error);
	if (d > 320.0) d = 320.0;
	return d;
}

// Reconstruct the ground-truth reference from a stored expansion (exact sum).
template<std::size_t N>
ereal<19> make_ref(const double (&components)[N]) {
	ereal<19> r(0.0);
	for (std::size_t i = 0; i < N; ++i) r += components[i];
	return r;
}

// Relative error of an ereal<M> result against the ground-truth ereal<19>
// reference. computed is widened to ereal<19> by re-summing its non-overlapping
// limbs (exact, no precision loss).
template<unsigned M>
double rel_error_vs_ref(const ereal<M>& computed, const ereal<19>& ref) {
	ereal<19> w(0.0);
	for (double v : computed.limbs()) w += v;
	if (ref.iszero()) return std::abs(double(abs(w)));
	return std::abs(double(abs(w - ref) / abs(ref)));
}

struct TestResult {
	std::string function_name;
	double      ref_value;        // leading double, for display
	double      digits[LEVELS];
	bool        passed[LEVELS];
	bool        measured[LEVELS];

	TestResult(const std::string& name, double rv) : function_name(name), ref_value(rv) {
		for (int i = 0; i < LEVELS; ++i) { digits[i] = 0.0; passed[i] = false; measured[i] = false; }
	}
};

// Evaluate func at each maxlimbs level (up to g_max_level) and measure the
// correct digits against the injected ground-truth expansion.
template<typename Func>
TestResult test_function_progressive(const std::string& name, const std::string& input,
                                     const ereal<19>& ref, Func func) {
	TestResult result(name, double(ref));
	double x = std::stod(input);  // the value the computation actually sees

	auto measure = [&](auto tag, int idx) {
		using Real = decltype(tag);
		if (idx > g_max_level) return;
		Real computed = func(Real(x));
		result.digits[idx]   = decimal_digits_precision(rel_error_vs_ref(computed, ref));
		result.passed[idx]   = (result.digits[idx] >= PRECISION_THRESHOLDS[idx]);
		result.measured[idx] = true;
	};
	measure(ereal<4>{},  0); measure(ereal<8>{},  1); measure(ereal<12>{}, 2);
	measure(ereal<16>{}, 3); measure(ereal<19>{}, 4);
	return result;
}

void print_result(const TestResult& result) {
	std::cout << "\n" << result.function_name << " = "
	          << std::setprecision(16) << result.ref_value << " ...\n";
	for (int i = 0; i < LEVELS; ++i) {
		std::cout << "  " << MAXLIMBS_LABELS[i] << " : ";
		if (!result.measured[i]) { std::cout << "(not measured at this regression level)\n"; continue; }
		std::cout << std::fixed << std::setprecision(1) << std::setw(6) << result.digits[i]
		          << " digits  [" << (result.passed[i] ? "PASS" : "FAIL")
		          << ": >=" << std::setw(5) << PRECISION_THRESHOLDS[i] << " expected]";
		if (!result.passed[i]) std::cout << " *** PRECISION LOSS DETECTED ***";
		std::cout << "\n";
	}
}

// ----------------------------------------------------------------------------
// Ground-truth references: precomputed non-overlapping expansions (MPFR, ~300
// digits) of each function at the exact double value of its input. See header.
// ----------------------------------------------------------------------------
	static const double REF_sin[] = {
		0.479425538604203, -5.103969860556013e-18, 3.7134329111577535e-34,
		-1.3517949849042625e-50, 7.35267291924973e-67, -4.1932026326953344e-83,
		1.8184716862532242e-99, -1.0022716896765329e-115, 1.2091026171598097e-132,
		7.194016312890547e-149, 2.371100248723222e-165, -4.079768820777539e-182,
		1.39862843312719e-198, 1.5853220140166796e-215, 7.671397408495679e-232,
		1.003539715567143e-248, 1.8843902586321277e-265, 1.0315309003492216e-281,
		-1.0020296291234945e-298
	};
	static const double REF_cos[] = {
		0.955336489125606, 4.1935600297907467e-17, 7.260829633530445e-34,
		-2.312485065541358e-50, -1.528138951760694e-66, -1.0135373490917977e-82,
		-3.0209764410814384e-99, -9.997553626897338e-116, -2.668197431222967e-132,
		1.2537178544126246e-148, 5.930625612829037e-165, -6.779876908478516e-182,
		4.246646229462259e-198, -4.20186398184701e-215, 1.0073853884132444e-231,
		-6.844651365696203e-248, -2.7087000699913784e-264, -2.030294940582396e-280,
		1.2071794591340463e-296
	};
	static const double REF_tan[] = {
		0.4227932187381618, 8.031615517522239e-18, -7.4482009636882465e-34,
		-1.025319131785375e-50, -1.984136515534763e-67, 3.629864273350765e-84,
		2.040034399268119e-100, 1.1142715265988389e-116, -6.193608488193079e-134,
		-4.116419010798719e-150, -1.4067925724556799e-167, -3.462808403136352e-184,
		2.9762516495284718e-201, 1.7871206040792382e-217, -1.0036198396573264e-233,
		-4.450308656467498e-250, -8.092278632228702e-268, 4.283522788275548e-284,
		1.6071207007881876e-300
	};
	static const double REF_atan[] = {
		0.7853981633974483, 3.061616997868383e-17, -7.486924524295849e-34,
		2.781135552158413e-50, 1.418057994910079e-66, 4.3624655403381215e-84,
		1.507343183062385e-100, 4.775308867199975e-117, 7.609945413360733e-134,
		-1.1785750077367572e-150, 2.5038729235888473e-167, 1.5526011196039737e-183,
		-4.292033097902901e-200, -1.3739437397422748e-216, -4.372522487006022e-233,
		-8.978774946446448e-250, 4.421351432452312e-266, 2.392347908934279e-282,
		1.459937185353973e-298
	};
	static const double REF_asin[] = {
		0.5235987755982989, -5.360408832255455e-17, -4.991283016197232e-34,
		-3.8478076800910752e-50, 9.453719966067193e-67, -4.10175144032351e-83,
		-5.091037298130543e-100, 2.0103151493570977e-116, 9.899601237381739e-133,
		5.731480903641318e-150, 1.0713677121117652e-166, 1.0350674130693159e-183,
		-9.828929673963323e-200, 2.95181819998456e-216, 2.9290728856648317e-232,
		1.1319934451974795e-248, 3.602810439928853e-265, -1.6768488192798997e-281,
		-1.5751403973568078e-298
	};
	static const double REF_acos[] = {
		1.0471975511965979, -1.072081766451091e-16, -9.982566032394464e-34,
		-7.6956153601821505e-50, 1.8907439932134387e-66, -8.20350288064702e-83,
		-1.0182074596261087e-99, 4.0206302987141954e-116, 1.9799202474763478e-132,
		1.1462961807282637e-149, 2.1427354242235305e-166, 2.0701348261386317e-183,
		-1.9657859347926646e-199, 5.90363639996912e-216, 5.8581457713296635e-232,
		2.263986890394959e-248, 7.205620879857706e-265, -3.3536976385597995e-281,
		-3.1502807947136156e-298
	};
	static const double REF_exp[] = {
		2.718281828459045, 1.4456468917292502e-16, -2.1277171080381768e-33,
		1.5156301598412191e-49, -9.335381378820847e-66, -2.021852603357621e-82,
		8.683510531926606e-99, -7.143883456983458e-115, -4.2998107368448233e-131,
		-5.091208330963075e-149, -2.6451469583258087e-166, -1.038562802070212e-182,
		-6.600489053715511e-199, -3.5499991859875913e-215, -2.9538161202476976e-232,
		1.5938733441162375e-248, 9.443610167112715e-265, -5.3517225248129615e-281,
		2.5800799215277924e-298
	};
	static const double REF_exp2[] = {
		11.313708498984761, -7.733834650762331e-16, 3.310940246959531e-32,
		3.948437593174681e-49, 3.2715234288959166e-66, 2.4844345030599467e-83,
		-1.600880213684865e-99, 2.4516032918006868e-116, -1.1783891884949429e-132,
		1.203933810062895e-149, 7.424442288338671e-166, 1.0434647408407364e-183,
		-9.056103289375168e-200, -3.666315735359697e-216, 2.751419012661063e-232,
		1.6086434378975392e-248, -4.318210062060695e-265, -1.5672239720151072e-281,
		5.976666304413818e-298
	};
	static const double REF_exp10[] = {
		31.622776601683793, 7.566535620287155e-16, 9.224089669804404e-33,
		-5.238503247488964e-49, 2.7852592225697304e-65, 1.5927449443072191e-81,
		-1.1265574261037848e-97, -1.8812073350141274e-114, 1.6162132830153628e-130,
		8.228293029818381e-147, -4.342707813046673e-163, 2.5260133896519782e-179,
		-8.394329481128868e-196, -1.0146456311016577e-212, -3.9601293293878443e-230,
		8.832648570481983e-248, 7.202179859572962e-264, -3.9728814402589827e-280,
		-1.23082216703803e-296
	};
	static const double REF_log[] = {
		0.6931471805599453, 2.3190468138462996e-17, 5.707708438416212e-34,
		-3.5824322106018114e-50, -1.352169675798863e-66, 6.080638740240814e-83,
		2.8955024332347147e-99, 2.351386712145641e-116, 4.459774417014281e-133,
		-3.069933263232527e-149, -2.0151474461966832e-165, 1.618534348863741e-182,
		-1.3094978047454462e-198, 6.665188278589824e-215, -2.93171211597727e-231,
		7.859799559040711e-248, 5.978862565926012e-264, -3.5958643436716937e-280,
		-1.4081782025014926e-297
	};
	static const double REF_log2[] = {
		3.321928094887362, 1.661617516973592e-16, 1.2215512178458181e-32,
		5.9551189702782496e-49, 1.9067002888523684e-65, -1.5486538280959164e-81,
		-1.358026745405091e-98, 6.171206499684013e-115, 1.4555153370895912e-131,
		-6.217171513008069e-148, 3.4081276760873518e-164, -1.522915951041203e-180,
		-2.1761671158889576e-197, -2.6726931325194733e-214, -1.1530191873195246e-230,
		-6.635465239775069e-247, -3.834673300744754e-263, -6.879401464954534e-280,
		-5.921887039368661e-297
	};
	static const double REF_log10[] = {
		2.0
	};
	static const double REF_sinh[] = {
		0.5210953054937474, -2.3328183476404597e-17, 5.906772043637963e-34,
		2.856709312099801e-51, -1.4819429595899448e-67, -7.357929318554909e-84,
		2.5704950770170603e-101, -6.377557707074133e-119, 8.450833712708591e-136,
		3.6354816563268436e-153, -2.391631196758738e-169, 9.876651317789843e-186,
		3.957972620313449e-202, -2.993091372541462e-219, -2.0083007454085867e-235,
		-1.1138222498888702e-251, 9.117206432174923e-268, 2.700103075134856e-284,
		1.4983117549127203e-300
	};
	static const double REF_cosh[] = {
		1.1276259652063807, 8.703480114456192e-17, 1.749515425656136e-34,
		5.279340081345056e-51, 1.6259732434520283e-67, -1.6076205089633587e-83,
		-1.4815285655861184e-100, 4.220363945318802e-117, -1.062877864994621e-133,
		9.592262207522915e-150, -2.6583229077305805e-166, -1.2306168152681951e-182,
		-5.118795605804455e-199, 2.1046171183515733e-215, 9.294802535730868e-232,
		-5.356848213196078e-248, -3.157017759950481e-264, -1.41197262667007e-280,
		1.2208755952778851e-296
	};
	static const double REF_tanh[] = {
		0.46211715726000974, 2.1916603238260928e-17, 4.249939575546907e-34,
		-2.1814327151954593e-50, 1.2001414320710812e-66, 2.2748355215638215e-83,
		-7.301962780263138e-100, -1.407990201883062e-116, -2.6758888028547294e-133,
		-1.6252304284326456e-149, 3.655773608160261e-166, 1.6158203792801627e-182,
		-3.8993656558382707e-199, 2.051103063255818e-215, 3.8928984966366596e-232,
		-2.987926573666305e-248, -1.8687007184556486e-264, -1.657697302797394e-281,
		-6.532921840331151e-298
	};
	static const double REF_asinh[] = {
		0.881373587019543, -2.250545892825866e-17, 2.9665892654081693e-34,
		-2.0404041210835052e-50, -1.0772888647467253e-66, -2.550748057872085e-83,
		-1.8697081584197546e-100, -6.331881121513197e-117, -2.6987922632532484e-134,
		2.1836359743304957e-150, -1.0120768990517773e-166, 2.6430741476509008e-183,
		-8.081625776554416e-200, -3.944550308804798e-216, -2.445053489932753e-233,
		-2.0817741454407362e-249, -5.87705168176328e-266, 1.2363638826681037e-283,
		2.2167295769257996e-300
	};
	static const double REF_acosh[] = {
		1.3169578969248168, -8.682250844852022e-17, 3.6222524942066425e-33,
		1.0898928386895417e-49, 8.876401433774435e-66, 3.364777899943897e-83,
		-2.865705406866918e-100, 2.450462923748029e-117, 4.446801927548534e-134,
		4.063590963764956e-150, 1.7927350233642353e-167, 5.494806627154501e-184,
		3.952203313974374e-200, -1.8115168876924536e-217, 1.9913759822110004e-234,
		-1.1836337393672488e-250, 3.3644356620855353e-267, -7.66459307873617e-284,
		5.131902499775132e-300
	};
	static const double REF_atanh[] = {
		0.5493061443340549, -4.535648617500765e-17, -4.345718236585198e-34,
		-1.0855212286200538e-50, -4.021238790694613e-67, -3.221843234796674e-83,
		4.070664661624898e-100, 1.7565318312086985e-116, -5.93109946722663e-133,
		1.1669787047044236e-149, 1.0329746489663094e-165, -5.1781931895382495e-182,
		-1.3642976336562586e-198, 3.4939809790435853e-215, 1.5506496253596917e-231,
		-1.1931504210030821e-247, -5.714364515801568e-264, -2.1686170252017533e-280,
		-7.546936605320084e-297
	};
	static const double REF_sqrt[] = {
		1.4142135623730951, -9.667293313452913e-17, 4.1386753086994136e-33,
		4.935546991468351e-50, 4.089404286119896e-67, 3.1055431288249333e-84,
		-2.0011002671060812e-100, 3.0645041147508584e-117, -1.4729864856186786e-133,
		1.504917262578619e-150, 9.280552860423338e-167, 1.3043309260509205e-184,
		-1.132012911171896e-200, -4.5828946691996216e-217, 3.4392737658263287e-233,
		2.010804297371924e-249, -5.397762577575869e-266, -1.959029965018884e-282,
		7.470832880517273e-299
	};
	static const double REF_pow[] = {
		11.313708498984761, -7.733834650762331e-16, 3.310940246959531e-32,
		3.948437593174681e-49, 3.2715234288959166e-66, 2.4844345030599467e-83,
		-1.600880213684865e-99, 2.4516032918006868e-116, -1.1783891884949429e-132,
		1.203933810062895e-149, 7.424442288338671e-166, 1.0434647408407364e-183,
		-9.056103289375168e-200, -3.666315735359697e-216, 2.751419012661063e-232,
		1.6086434378975392e-248, -4.318210062060695e-265, -1.5672239720151072e-281,
		5.976666304413818e-298
	};

}} // namespace sw::universal

// Regression guards: the highest maxlimbs level measured scales with the
// regression level so CI/level-1 stays fast. The reference is always the full
// ~300-digit expansion, so every measured level is an honest comparison.
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

#if REGRESSION_LEVEL_4
	g_max_level = 4;   // through ereal<19>
#elif REGRESSION_LEVEL_3
	g_max_level = 3;   // through ereal<16>
#elif REGRESSION_LEVEL_2
	g_max_level = 2;   // through ereal<12>
#else
	g_max_level = 1;   // through ereal<8> (fast, level-1 / CI default)
#endif

	std::cout << "Progressive Precision Validation - ereal mathlib\n";
	std::cout << "=================================================\n";
	std::cout << "(measured against precomputed ~300-digit MPFR expansions; ground truth is\n";
	std::cout << " more precise than every level below, so each row is an honest comparison)\n";
	std::cout << "(highest level measured at this regression level = " << MAXLIMBS_LABELS[g_max_level] << ")\n";
	std::cout << "\nExpected accuracy (~15.95 decimal digits per 53-bit limb):\n";
	std::cout << "  ereal<4>  : 212 bits  -> ~ 64 digits  (threshold >= 60)\n";
	std::cout << "  ereal<8>  : 424 bits  -> ~128 digits  (threshold >= 120)\n";
	std::cout << "  ereal<12> : 636 bits  -> ~192 digits  (threshold >= 180)\n";
	std::cout << "  ereal<16> : 848 bits  -> ~256 digits  (threshold >= 240)\n";
	std::cout << "  ereal<19> : 1007 bits -> ~304 digits  (threshold >= 290, ref-bounded)\n";

	std::vector<TestResult> results;
	auto run = [&](const std::string& name, const std::string& input,
	               const ereal<19>& ref, auto func) {
		auto r = test_function_progressive(name, input, ref, func);
		print_result(r);
		results.push_back(r);
	};

	std::cout << "\n\n" << std::string(80, '=') << "\nTRIGONOMETRIC FUNCTIONS\n" << std::string(80, '=') << "\n";
	run("sin(0.5)",         "0.5", make_ref(REF_sin),  [](auto x) { return sin(x); });
	run("cos(0.3)",         "0.3", make_ref(REF_cos),  [](auto x) { return cos(x); });
	run("tan(0.4)",         "0.4", make_ref(REF_tan),  [](auto x) { return tan(x); });
	run("atan(1.0) [pi/4]", "1.0", make_ref(REF_atan), [](auto x) { return atan(x); });
	run("asin(0.5) [pi/6]", "0.5", make_ref(REF_asin), [](auto x) { return asin(x); });
	run("acos(0.5) [pi/3]", "0.5", make_ref(REF_acos), [](auto x) { return acos(x); });

	std::cout << "\n\n" << std::string(80, '=') << "\nEXPONENTIAL FUNCTIONS\n" << std::string(80, '=') << "\n";
	run("exp(1.0) [e]", "1.0", make_ref(REF_exp),   [](auto x) { return exp(x); });
	run("exp2(3.5)",    "3.5", make_ref(REF_exp2),  [](auto x) { return exp2(x); });
	run("exp10(1.5)",   "1.5", make_ref(REF_exp10), [](auto x) { return exp10(x); });

	std::cout << "\n\n" << std::string(80, '=') << "\nLOGARITHM FUNCTIONS\n" << std::string(80, '=') << "\n";
	run("log(2.0) [ln(2)]", "2.0",   make_ref(REF_log),   [](auto x) { return log(x); });
	run("log2(10.0)",       "10.0",  make_ref(REF_log2),  [](auto x) { return log2(x); });
	run("log10(100.0)",     "100.0", make_ref(REF_log10), [](auto x) { return log10(x); });

	std::cout << "\n\n" << std::string(80, '=') << "\nHYPERBOLIC FUNCTIONS\n" << std::string(80, '=') << "\n";
	run("sinh(0.5)",  "0.5", make_ref(REF_sinh),  [](auto x) { return sinh(x); });
	run("cosh(0.5)",  "0.5", make_ref(REF_cosh),  [](auto x) { return cosh(x); });
	run("tanh(0.5)",  "0.5", make_ref(REF_tanh),  [](auto x) { return tanh(x); });
	run("asinh(1.0)", "1.0", make_ref(REF_asinh), [](auto x) { return asinh(x); });
	run("acosh(2.0)", "2.0", make_ref(REF_acosh), [](auto x) { return acosh(x); });
	run("atanh(0.5)", "0.5", make_ref(REF_atanh), [](auto x) { return atanh(x); });

	std::cout << "\n\n" << std::string(80, '=') << "\nPOWER AND ROOT FUNCTIONS\n" << std::string(80, '=') << "\n";
	run("sqrt(2.0)",     "2.0", make_ref(REF_sqrt), [](auto x) { return sqrt(x); });
	run("pow(2.0, 3.5)", "2.0", make_ref(REF_pow),  [](auto x) { using Real = decltype(x); return pow(x, Real(3.5)); });

	std::cout << "\n\n" << std::string(80, '=') << "\nSUMMARY\n" << std::string(80, '=') << "\n\n";
	int passed_by_level[LEVELS] = {0,0,0,0,0};
	int total_by_level[LEVELS]  = {0,0,0,0,0};
	for (const auto& r : results) {
		for (int i = 0; i < LEVELS; ++i) {
			if (!r.measured[i]) continue;
			total_by_level[i]++;
			if (r.passed[i]) passed_by_level[i]++;
		}
	}

	std::cout << "Functions tested: " << results.size() << "\n\n";
	bool all_passed = true;
	for (int i = 0; i < LEVELS; ++i) {
		std::cout << MAXLIMBS_LABELS[i] << " : ";
		if (total_by_level[i] == 0) { std::cout << "(not measured at this regression level)\n"; continue; }
		std::cout << passed_by_level[i] << "/" << total_by_level[i] << " passed";
		if (passed_by_level[i] == total_by_level[i]) {
			std::cout << " [ok]\n";
		} else {
			std::cout << " [x] FAILURES DETECTED\n";
			all_passed = false;
		}
	}

	std::cout << "\n";
	if (all_passed) {
		std::cout << "Progressive precision validation: PASS\n";
		std::cout << "All functions achieve expected precision scaling with maxlimbs.\n";
		return EXIT_SUCCESS;
	}
	std::cout << "Progressive precision validation: FAIL\n";
	std::cout << "Some functions do not achieve expected precision scaling.\n";
	return EXIT_FAILURE;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
