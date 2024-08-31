// constants.cpp: test suite runner for creating and verifying quad-double constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>

namespace sw {
	namespace universal {

		sw::universal::qd GenerateQuadDouble(const std::string& str) {
			using namespace sw::universal;
			qd v(str);
			auto oldPrec = std::cout.precision();
			// 53 bits = 16 decimal digits, 17 to include last, 15 typical valid digits
			std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
			std::cout << to_quad(v) << '\n';
			std::cout << std::setprecision(oldPrec);
			return v;
		}

		void report(const sw::universal::qd& v, int precision = 17) {
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(precision) << to_quad(v) << " : " << v << '\n';
			std::cout << std::setprecision(oldPrec);
		}

		void EnumerateConstants() {
			qd _zero("0.0"); report(_zero);
			qd _one("1.0"); report(_one);
			qd _ten("10.0"); report(_ten);

			qd _tenth("0.1"); report(_tenth);
			qd _third("0.333333333333333333333333333333333333"); report(_third);

			qd _2pi("6.283185307179586476925286766559005768"); report(_2pi);
			qd _pi("3.141592653589793238462643383279502884"); report(_pi);
			qd _pi2("1.570796326794896619231321691639751442"); report(_pi2);
			qd _pi4("0.785398163397448309615660845819875721"); report(_pi4);
			qd _3pi4 = _pi2 + _pi4;	report(_3pi4);

			qd _e("2.718281828459045235360287471352662498"); report(_e);

			qd _ln2("0.693147180559945309417232121458176568"); report(_ln2);
			qd _ln10("2.302585092994045684017991454684364208"); report(_ln10);

			qd _lge("1.442695040888963407359924681001892137"); report(_lge);
			qd _lg10("3.321928094887362347870319429489390176"); report(_lg10);

			qd _log2("0.301029995663981195213738894724493027"); report(_log2);
			qd _loge("0.434294481903251827651128918916605082"); report(_loge);

			qd _sqrt2("1.414213562373095048801688724209698079"); report(_sqrt2);

			qd _inv_pi("0.318309886183790671537767526745028724"); report(_inv_pi);
			qd _inv_pi2("0.636619772367581343075535053490057448"); report(_inv_pi2);
			qd _inv_e("0.367879441171442321595523770161460867"); report(_inv_e);
			qd _inv_sqrt2("0.707106781186547524400844362104849039"); report(_inv_sqrt2);
		}

		int VerifyParse(const std::string& str) {
			int nrFailedTestCases{ 0 };
			qd v{};
			if (!parse(str, v)) {
				std::cerr << "failed to parse " << str << '\n';
				++nrFailedTestCases;
			}
			else {
				std::cout << std::setw(20) << str << " : " << v << '\n';
				std::cout << to_binary(v) << '\n';
				std::cout << "PASS\n";
			}
			return nrFailedTestCases;
		}

		int TestScientifiFormatParsing() {
			// parsing scientific formats
			int nrOfFailedTests{ 0 };
			nrOfFailedTests += VerifyParse("12.5e-2");
			nrOfFailedTests += VerifyParse("12.5e-1");
			nrOfFailedTests += VerifyParse("12.5e-0");
			nrOfFailedTests += VerifyParse("12.5e+1");
			nrOfFailedTests += VerifyParse("12.5e2");
			nrOfFailedTests += VerifyParse("12.5e-02");
			nrOfFailedTests += VerifyParse("12.5e-01");
			nrOfFailedTests += VerifyParse("12.5e00");
			nrOfFailedTests += VerifyParse("12.5e+01");
			nrOfFailedTests += VerifyParse("12.5e02");
			nrOfFailedTests += VerifyParse("12.5e-002");
			nrOfFailedTests += VerifyParse("12.5e-001");
			nrOfFailedTests += VerifyParse("12.5e000");
			nrOfFailedTests += VerifyParse("12.5e+001");
			nrOfFailedTests += VerifyParse("12.5e002");
			nrOfFailedTests += VerifyParse("12.5e-200");
			nrOfFailedTests += VerifyParse("12.5e-100");
			nrOfFailedTests += VerifyParse("12.5e000");
			nrOfFailedTests += VerifyParse("12.5e+100");
			nrOfFailedTests += VerifyParse("12.5e200");

			return nrOfFailedTests;
		}


		void FindRepresentationForOneThird() {
			double _third = 0.333'333'333'333'333'333'333'333'333'333'3;
			double _third2 = _third * std::pow(2.0, -53.0);
			double _short = 0.333'333'333'333'333'3;
			ReportValue(_short, "0.333'333'333'333'333'3", 35, 32);
			ReportValue(_third, "0.333'333'333'333'333'333'333'333'333'333'3", 35, 32);

			qd a{ 0 }, b, c;
			std::cout << std::setprecision(64);

			a = _third;
			b = _third2;
			std::cout << std::setw(35) << "0.3333...." << " : " << a << '\n';
			std::cout << std::setw(35) << "0.3333...." << " : " << b << '\n';
			c = a + b;
			std::cout << std::setw(35) << "0.3333...." << " : " << c << '\n';
			std::cout << to_quad(c) << '\n';

			qd d(_third, _third2);
			std::cout << std::setw(35) << "0.3333...." << " : " << d << '\n';
			std::cout << to_quad(d) << '\n';

			// 212 bits represent 10log(2) * 212 = 63.8 digits of accuracy 
			//                            1        10        20        30        40        50        60        70        80        90        100
			//                            '        '         '         '         '         '         '         '         '         '         '
			std::string ten3s = "0.3333333333";
			std::string twenty3s = "0.33333333333333333333";
			std::string thirty3s = "0.333333333333333333333333333333";
			std::string fourty3s = "0.3333333333333333333333333333333333333333";
			std::string fifty3s = "0.33333333333333333333333333333333333333333333333333";
			std::string sixty3s = "0.333333333333333333333333333333333333333333333333333333333333";
			std::string seventy3s = "0.3333333333333333333333333333333333333333333333333333333333333333333333";
			std::string eighty3s = "0.33333333333333333333333333333333333333333333333333333333333333333333333333333333";
			std::string ninety3s = "0.333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333";
			std::string onehundred3s = "0.3333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333";
			// qd                 e("0.333'333'333'333'333'333'333'333'333'333'333'333'333'333'333'333'33");  // 50 digits
			std::string sixty4_3s = "0.3333333333333333333333333333333333333333333333333333333333333333";
			qd e(seventy3s);
			std::cout << std::setw(35) << "0.3333...." << " : " << e << '\n';
			std::cout << to_quad(e) << '\n';   // NOTE: this yields a better representation than sixty4_3s or even onehundred3s

			qd f(0.33333333333333331, 1.8503717077085941e-17, 1.0271626370065257e-33, 5.7018980481966837e-50);
			std::cout << std::setw(35) << "0.3333...." << " : " << f << '\n';
			std::cout << to_quad(f) << '\n';

			qd h(sixty4_3s);
			std::cout << std::setw(35) << "0.3333...." << " : " << h << '\n';
			std::cout << to_quad(h) << '\n';

			qd g(onehundred3s);
			std::cout << std::setprecision(100);
			std::cout << std::setw(35) << "0.3333...." << " : " << g << '\n';
			std::cout << to_quad(g) << '\n';
		}

		void GenerateConstants() {
			// phi to one hundred digits
			std::string hundred_digits_of_phi = "1.6180339887498948482045868343656381177203091798057628621354486227052604628189024497072072041893911374";

			qd qd_phi_origin(hundred_digits_of_phi);
			std::cout << "phi     " << std::setprecision(64) << qd_phi_origin << '\n';
			std::cout << to_quad(qd_phi_origin) << '\n';
			qd qd_inv_phi_ = 1.0 / qd_phi_origin;
			std::cout << "inv_phi " << std::setprecision(64) << qd_inv_phi_ << '\n';
			std::cout << to_quad(qd_inv_phi_) << '\n';

			// e to one hundred digits
			std::string hundred_digits_of_e = "2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274";

			qd qd_e_origin(hundred_digits_of_e);
			std::cout << "e       " << std::setprecision(64) << qd_e_origin << '\n';
			std::cout << to_quad(qd_e_origin) << '\n';
			qd qd_inv_e_ = 1.0 / qd_e_origin;
			std::cout << "inv_e   " << std::setprecision(64) << qd_inv_e_ << '\n';
			std::cout << to_quad(qd_inv_e_) << '\n';

			// pi to one hundred digits
			std::string hundred_digits_of_pi = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679";

			qd qd_pi_origin(hundred_digits_of_pi);
			std::cout << "pi      " << std::setprecision(64) << qd_pi_origin << '\n';
			std::cout << to_quad(qd_pi_origin) << '\n';
			qd qd_2pi_ = qd_pi_origin * 2.0;
			std::cout << "2pi     " << std::setprecision(64) << qd_2pi_ << '\n';
			std::cout << to_quad(qd_2pi_) << '\n';
			qd qd_pi2_ = qd_pi_origin * 0.5;
			std::cout << "pi2     " << std::setprecision(64) << qd_pi2_ << '\n';
			std::cout << to_quad(qd_pi2_) << '\n';
			qd qd_pi4_ = qd_pi_origin * 0.25;
			std::cout << "pi4     " << std::setprecision(64) << qd_pi4_ << '\n';
			std::cout << to_quad(qd_pi4_) << '\n';
			qd qd_3pi4_ = qd_pi_origin * 0.75;
			std::cout << "3pi4    " << std::setprecision(64) << qd_3pi4_ << '\n';
			std::cout << to_quad(qd_3pi4_) << '\n';

			qd qd_inv_pi_ = 1.0 / qd_pi_origin;
			std::cout << "1/pi    " << std::setprecision(64) << qd_inv_pi_ << '\n';
			std::cout << to_quad(qd_inv_pi_) << '\n';
			qd qd_inv_pi2_ = 1.0 / qd_pi2_;
			std::cout << "1/pi2   " << std::setprecision(64) << qd_inv_pi2_ << '\n';
			std::cout << to_quad(qd_inv_pi2_) << '\n';

			// natural logarithm (base = e)
			qd qd_ln2_("0.6931471805599453094172321214581765680755001343602552541206800094933936219696947156058633269964186875");
			std::cout << "ln(2)   " << std::setprecision(64) << qd_ln2_ << '\n';
			std::cout << to_quad(qd_ln2_) << '\n';
			qd qd_ln10_("2.302585092994045684017991454684364207601101488628772976033327900967572609677352480235997205089598298");
			std::cout << "ln(10)  " << std::setprecision(64) << qd_ln10_ << '\n';
			std::cout << to_quad(qd_ln10_) << '\n';

			// binary logarithm (base = 2)
			qd qd_lge_("1.442695040888963407359924681001892137426645954152985934135449406931109219181185079885526622893506344");
			std::cout << "lg(e)  " << std::setprecision(64) << qd_lge_ << '\n';
			std::cout << to_quad(qd_lge_) << '\n';
			qd qd_lg10_("3.321928094887362347870319429489390175864831393024580612054756395815934776608625215850139743359370155");
			std::cout << "lg(10)  " << std::setprecision(64) << qd_lg10_ << '\n';
			std::cout << to_quad(qd_lg10_) << '\n';

			// common logarithm (base = 10)
			qd qd_log2_("0.301029995663981195213738894724493026768189881462108541310427461127108189274424509486927252118186172");
			std::cout << "log(2)  " << std::setprecision(64) << qd_log2_ << '\n';
			std::cout << to_quad(qd_log2_) << '\n';
			qd qd_loge_("0.4342944819032518276511289189166050822943970058036665661144537831658646492088707747292249493384317483");
			std::cout << "log(e)  " << std::setprecision(64) << qd_loge_ << '\n';
			std::cout << to_quad(qd_loge_) << '\n';
			qd qd_log10_("1.0");
			std::cout << "log(10)  " << std::setprecision(64) << qd_log10_ << '\n';
			std::cout << to_quad(qd_log10_) << '\n';

			qd qd_sqrt2_("1.414213562373095048801688724209698078569671875376948073176679737990732478462107038850387534327641573");
			std::cout << "sqrt(2) " << std::setprecision(64) << qd_sqrt2_ << '\n';
			std::cout << to_quad(qd_sqrt2_) << '\n';
			qd qd_inv_sqrt2_ = 1.0 / qd_sqrt2_;
			std::cout << "inv_sqrt(2) " << std::setprecision(64) << qd_inv_sqrt2_ << '\n';
			std::cout << to_quad(qd_inv_sqrt2_) << '\n';
		}
	}
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "quad-double constants";
	std::string test_tag    = "qd constants";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// we need 64 digits of precision in the strings

	std::cout << "verifying constants\n"; 
	struct constant_kv {
		std::string name;
		std::string digits;
		qd value;
	} constant_symbol_table[] = {
		{ "qd_phi"    , "1.6180339887498948482045868343656381177203091798057628621354486228", qd_phi },
		{ "qd_1_phi", "0.6180339887498948482045868343656381177203091798057628621354486227", qd_1_phi },

		{ "qd_e"      , "2.7182818284590452353602874713526624977572470936999595749669676277", qd_e },
		{ "qd_1_e"  , "0.3678794411714423215955237701614608674458111310317678345078368017", qd_1_e },

		{ "qd_2pi"    , "6.2831853071795864769252867665590057683943387987502116419498891847", qd_2pi },
		{ "qd_pi"     , "3.1415926535897932384626433832795028841971693993751058209749445923", qd_pi },
		{ "qd_pi2"    , "1.5707963267948966192313216916397514420985846996875529104874722962", qd_pi_2 },
		{ "qd_pi4"    , "0.7853981633974483096156608458198757210492923498437764552437361481", qd_pi_4 },
		{ "qd_3pi4"   , "2.3561944901923449288469825374596271631478770495313293657312084443", qd_3pi_4 },

		{ "qd_1_pi"   , "0.31830988618379067153776752674502872406891929148091289749533468812", qd_1_pi },
		{ "qd_2_pi"   , "0.63661977236758134307553505349005744813783858296182579499066937624", qd_2_pi },

		{ "qd_ln2"    , "0.69314718055994530941723212145817656807550013436025525412068000950", qd_ln2 },
		{ "qd_lne"    , "1.00000000000000000000000000000000000000000000000000000000000000000", qd(1.0)},
		{ "qd_ln10"   , "2.30258509299404568401799145468436420760110148862877297603332790097", qd_ln10 },

		{ "qd_lg2"    , "1.0000000000000000000000000000000000000000000000000000000000000000", qd(1.0)},
		{ "qd_lge"    , "1.4426950408889634073599246810018921374266459541529859341354494069", qd_lge },
		{ "qd_lg10"   , "3.3219280948873623478703194294893901758648313930245806120547563956", qd_lg10 },

		{ "qd_log2"   , "3.0102999566398119521373889472449302676818988146210854131042746113e-01", qd_log2 },
		{ "qd_loge"   , "4.3429448190325182765112891891660508229439700580366656611445378316e-01", qd_loge },
		{ "qd_log10"  , "1.0000000000000000000000000000000000000000000000000000000000000000", qd(1.0)},

		{ "qd_sqrt2"    , "1.4142135623730950488016887242096980785696718753769480731766797380", qd_sqrt2 },
		{ "qd_1_sqrt2", "7.0710678118654752440084436210484903928483593768847403658833986899e-01", qd_1_sqrt2 },
	};

	/*
	 * 
	 * ETLO August 31, 2024
	 * Need to verify if these are the most accurate quad-double approximations available.
	 * This is Debug, Release cuts the precision in half
verifying constants
qd_phi          : 1.61803398874989484820458683436564e+00 vs 1.61803398874989484820458683436564e+00 : ( 1.6180339887498949, -5.4321152036825061e-17, 2.6543252083815655e-33, -3.3049919975020983e-50) : 4.74778387287989937373662113478098e-66
qd_1_phi        : 6.18033988749894848204586834365638e-01 vs 6.18033988749894848204586834365638e-01 : ( 0.6180339887498949, -5.4321152036825061e-17, 2.6543252083815655e-33, -3.3049919975021083e-50) : 2.84867032372793962424197268086859e-65
qd_e            : 2.71828182845904523536028747135266e+00 vs 2.71828182845904523536028747135266e+00 : ( 2.7182818284590451, 1.4456468917292502e-16, -2.1277171080381768e-33, 1.5156301598412188e-49) : -1.89911354915195974949464845391239e-65
qd_1_e          : 3.67879441171442321595523770161461e-01 vs 3.67879441171442321595523770161461e-01 : ( 0.36787944117144233, -1.2428753672788363e-17, -5.830044851072742e-34, -2.8267977849017436e-50) : 0.00000000000000000000000000000000e+00
qd_2pi          : 6.28318530717958647692528676655901e+00 vs 6.28318530717958647692528676655901e+00 : ( 6.2831853071795862, 2.4492935982947064e-16, -5.9895396194366793e-33, 2.2249084417267317e-49) : 3.79822709830391949898929690782478e-65
qd_pi           : 3.14159265358979323846264338327950e+00 vs 3.14159265358979323846264338327950e+00 : ( 3.1415926535897931, 1.2246467991473532e-16, -2.9947698097183397e-33, 1.1124542208633653e-49) : -3.79822709830391949898929690782478e-65
qd_pi2          : 1.57079632679489661923132169163975e+00 vs 1.57079632679489661923132169163975e+00 : ( 1.5707963267948966, 6.123233995736766e-17, -1.4973849048591698e-33, 5.5622711043168312e-50) : 2.84867032372793962424197268086859e-65
qd_pi4          : 7.85398163397448309615660845819876e-01 vs 7.85398163397448309615660845819876e-01 : ( 0.78539816339744828, 3.061616997868383e-17, -7.4869245242958492e-34, 2.7811355521584156e-50) : 1.42433516186396981212098634043429e-65
qd_3pi4         : 2.35619449019234492884698253745963e+00 vs 2.35619449019234492884698253745963e+00 : ( 2.3561944901923448, 9.1848509936051484e-17, 3.9168984647504003e-33, -2.5867981632704857e-49) : 3.79822709830391949898929690782478e-65
qd_1_pi         : 3.18309886183790671537767526745029e-01 vs 3.18309886183790671537767526745029e-01 : ( 0.31830988618379069, -1.9678676675182486e-17, -1.0721436282893004e-33, 8.053563926594112e-50) : 0.00000000000000000000000000000000e+00
qd_2_pi         : 6.36619772367581343075535053490057e-01 vs 6.36619772367581343075535053490057e-01 : ( 0.63661977236758138, -3.9357353350364972e-17, -2.1442872565786008e-33, 1.6107127853188224e-49) : 0.00000000000000000000000000000000e+00
qd_ln2          : 6.93147180559945309417232121458177e-01 vs 6.93147180559945309417232121458177e-01 : ( 0.69314718055994529, 2.3190468138462996e-17, 5.7077084384162121e-34, -3.5824322106018109e-50) : -4.74778387287989937373662113478098e-66
qd_lne          : 1.00000000000000000000000000000000e+00 vs 1.00000000000000000000000000000000e+00 : ( 1, 0, 0, 0) : 0.00000000000000000000000000000000e+00
qd_ln10         : 2.30258509299404568401799145468436e+00 vs 2.30258509299404568401799145468436e+00 : ( 2.3025850929940459, -2.1707562233822494e-16, -9.9842624544657766e-33, -4.0233574544502064e-49) : 7.59645419660783899797859381564957e-65
qd_lg2          : 1.00000000000000000000000000000000e+00 vs 1.00000000000000000000000000000000e+00 : ( 1, 0, 0, 0) : 0.00000000000000000000000000000000e+00
qd_lge          : 1.44269504088896340735992468100189e+00 vs 1.44269504088896340735992468100189e+00 : ( 1.4426950408889634, 2.0355273740931033e-17, -1.0614659956117258e-33, -1.3836716780181433e-50) : -3.79822709830391949898929690782478e-65
qd_lg10         : 3.32192809488736234787031942948939e+00 vs 3.32192809488736234787031942948939e+00 : ( 3.3219280948873622, 1.661617516973592e-16, 1.2215512178458181e-32, 5.9551189702782473e-49) : -7.59645419660783899797859381564957e-65
qd_log2         : 3.01029995663981195213738894724493e-01 vs 3.01029995663981195213738894724493e-01 : ( 0.3010299956639812, -2.8037281277851704e-18, 5.4719484023146385e-35, 5.1051389831070954e-51) : -4.15431088876991195201954349293336e-66
qd_loge         : 4.34294481903251827651128918916605e-01 vs 4.34294481903251827651128918916605e-01 : ( 0.43429448190325182, 1.0983196502167651e-17, 3.717181233110959e-34, 7.7344843465042927e-51) : 0.00000000000000000000000000000000e+00
qd_log10        : 1.00000000000000000000000000000000e+00 vs 1.00000000000000000000000000000000e+00 : ( 1, 0, 0, 0) : 0.00000000000000000000000000000000e+00
qd_sqrt2        : 1.41421356237309504880168872420970e+00 vs 1.41421356237309504880168872420970e+00 : ( 1.4142135623730951, -9.6672933134529135e-17, 4.1386753086994136e-33, 4.9355469914683519e-50) : 9.49556774575979874747324226956196e-66
qd_1_sqrt2      : 7.07106781186547524400844362104849e-01 vs 7.07106781186547524400844362104849e-01 : ( 0.70710678118654757, -4.8336466567264567e-17, 2.0693376543497068e-33, 2.4677734957341759e-50) : 4.74778387287989937373662113478098e-66
	 */
	auto oldPrec = std::cout.precision();
	std::cout << std::setprecision(32);
	for (auto e : constant_symbol_table) {
		qd c(e.digits);
		qd error = (c - e.value);
		std::cout << std::left << std::setw(15) << e.name << " : " << c << " vs " << e.value << " : " << to_quad(c) << " : " << error << '\n';
	}

	{

		qd a{ 2.0 };
		qd sqrt2 = sqrt(a);
		std::cout << "sqrt(2.0) " << to_quad(sqrt2) << '\n';

		a = 3.0;
		std::cout << "sqrt(3.0) " << to_quad(sqrt(a)) << '\n';
		a = 5.0;
		std::cout << "sqrt(5.0) " << to_quad(sqrt(a)) << '\n';

		std::cout << "1/sqrt(2.0) " << to_quad(reciprocal(sqrt2)) << '\n';

		a = sqrt(qd_pi);
		std::cout << "2 / sqrtpi " << to_quad(2.0 / a) << '\n';
	}
	/* 
	
		Debug build
		sqrt(2.0) ( 1.4142135623730951, -9.6672933134529135e-17, 4.1386753086994136e-33, 4.9355469914683509e-50)
		sqrt(3.0) ( 1.7320508075688772, 1.0035084221806903e-16, -1.4959542475733896e-33, 5.3061475632961675e-50)
		sqrt(5.0) ( 2.2360679774997898, -1.0864230407365012e-16, 5.3086504167631309e-33, -6.6099839950042175e-50)
		1/sqrt(2.0) ( 0.70710678118654757, -4.8336466567264567e-17, 2.0693376543497068e-33, 2.4677734957341755e-50)
		2 / sqrtpi ( 1.1283791670955126, 1.5335459613165881e-17, -4.7656845966936863e-34, -2.0077946616552625e-50)

		Release build
		sqrt(2.0) ( 1.4142135623730951, -9.6672933134529135e-17, 4.1386753203466335e-33, -3.3032885712977947e-49)
		sqrt(3.0) ( 1.7320508075688772, 1.0035084221806903e-16, -1.4959542883445281e-33, 5.0676801879243325e-50)
		sqrt(5.0) ( 2.2360679774997898, -1.0864230407365012e-16, 5.3086504310320564e-33, -2.7103246582355688e-49)
		1/sqrt(2.0) ( 0.70710678118654757, -4.8336466312625432e-17, -3.039266735626984e-34, -1.350504809842679e-50)
		2 / sqrtpi ( 1.1283791670955126, 1.5335458971746789e-17, 2.6579683555126638e-34, -1.683757146154259e-50)

		difference       sqrt2    : -1.16472195516512003859185071422508e-41
		difference       sqrt3    : +4.07711385546630871610406778813869e-41
		difference       sqrt5    : -1.42689253079290274645876254577245e-41
		difference       1_sqrt2  : -2.54639133258339414062837196323682e-25
		difference       2_sqrtpi : +6.41419091181394536188494115009207e-25

	*/

	{
		qd debug_sqrt2(1.4142135623730951, -9.6672933134529135e-17, 4.1386753086994136e-33, 4.9355469914683509e-50);
		qd debug_sqrt3 (1.7320508075688772, 1.0035084221806903e-16, -1.4959542475733896e-33, 5.3061475632961675e-50);
		qd debug_sqrt5(2.2360679774997898, -1.0864230407365012e-16, 5.3086504167631309e-33, -6.6099839950042175e-50);
		qd debug_1_sqrt2(0.70710678118654757, -4.8336466567264567e-17, 2.0693376543497068e-33, 2.4677734957341755e-50);
		qd debug_2_sqrtpi(1.1283791670955126, 1.5335459613165881e-17, -4.7656845966936863e-34, -2.0077946616552625e-50);

		qd release_sqrt2(1.4142135623730951, -9.6672933134529135e-17, 4.1386753203466335e-33, -3.3032885712977947e-49);
		qd release_sqrt3(1.7320508075688772, 1.0035084221806903e-16, -1.4959542883445281e-33, 5.0676801879243325e-50);
		qd release_sqrt5(2.2360679774997898, -1.0864230407365012e-16, 5.3086504310320564e-33, -2.7103246582355688e-49);
		qd release_1_sqrt2(0.70710678118654757, -4.8336466312625432e-17, -3.039266735626984e-34, -1.350504809842679e-50);
		qd release_2_sqrtpi(1.1283791670955126, 1.5335458971746789e-17, 2.6579683555126638e-34, -1.683757146154259e-50);

		std::cout << "difference       sqrt2    : " << (debug_sqrt2 - release_sqrt2) << '\n';
		std::cout << "difference       sqrt3    : " << (debug_sqrt3 - release_sqrt3) << '\n';
		std::cout << "difference       sqrt5    : " << (debug_sqrt5 - release_sqrt5) << '\n';
		std::cout << "difference       1_sqrt2  : " << (debug_1_sqrt2 - release_1_sqrt2) << '\n';
		std::cout << "difference       2_sqrtpi : " << (debug_2_sqrtpi - release_2_sqrtpi) << '\n';
	}

	std::cout << std::setprecision(oldPrec);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
