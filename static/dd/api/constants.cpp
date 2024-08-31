// constants.cpp: test suite runner for creating and verifying double-double constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>

namespace sw {
	namespace universal {

		sw::universal::dd GenerateDoubleDouble(const std::string& str) {
			using namespace sw::universal;
			dd v(str);
			auto defaultPrecision = std::cout.precision();
			// 53 bits = 16 decimal digits, 17 to include last, 15 typical valid digits
			std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
			std::cout << to_pair(v) << '\n';
			std::cout << std::setprecision(defaultPrecision);
			return v;
		}

		void report(const sw::universal::dd& v, int precision = 17) {
			auto defaultPrecision = std::cout.precision();
			std::cout << std::setprecision(precision) << to_pair(v) << " : " << v << '\n';
			std::cout << std::setprecision(defaultPrecision);
		}

		const dd dd_inv_int[] = {
			dd(std::numeric_limits< dd >::infinity()),			//	1/0
			dd("1.0"),											//	1/1
			dd("0.5"),											//	1/2
			dd("0.3333333333333333333333333333333333333"),		//	1/3
			dd("0.25"),											//	1/4
			dd("0.2"),											//	1/5
			dd("0.1666666666666666666666666666666666667"),		//	1/6
			dd("0.1428571428571428571428571428571428571"),		//	1/7
			dd("0.125"),										//	1/8
			dd("0.1111111111111111111111111111111111111"),		//	1/9
			dd("0.1"),											//	1/10
			dd("0.0909090909090909090909090909090909091"),		//	1/11
			dd("0.0833333333333333333333333333333333333"),		//	1/12
			dd("0.0769230769230769230769230769230769231"),		//	1/13
			dd("0.0714285714285714285714285714285714286"),		//	1/14
			dd("0.0666666666666666666666666666666666667"),		//	1/15
			dd("0.0625"),										//	1/16
			dd("0.0588235294117647058823529411764705882"),		//	1/17
			dd("0.0555555555555555555555555555555555556"),		//	1/18
			dd("0.0526315789473684210526315789473684211"),		//	1/19
			dd("0.05"),											//	1/20
			dd("0.0476190476190476190476190476190476190"),		//	1/21
			dd("0.0454545454545454545454545454545454545"),		//	1/22
			dd("0.0434782608695652173913043478260869565"),		//	1/23
			dd("0.0416666666666666666666666666666666667"),		//	1/24
			dd("0.04"),											//	1/25
			dd("0.0384615384615384615384615384615384615"),		//	1/26
			dd("0.0370370370370370370370370370370370370"),		//	1/27
			dd("0.0357142857142857142857142857142857143"),		//	1/28
			dd("0.0344827586206896551724137931034482759"),		//	1/29
			dd("0.0333333333333333333333333333333333333"),		//	1/30
			dd("0.0322580645161290322580645161290322581"),		//	1/31
			dd("0.03125"),										//	1/32
			dd("0.0303030303030303030303030303030303030"),		//	1/33
			dd("0.0294117647058823529411764705882352941"),		//	1/34
			dd("0.0285714285714285714285714285714285714"),		//	1/35
			dd("0.0277777777777777777777777777777777778"),		//	1/36
			dd("0.0270270270270270270270270270270270270"),		//	1/37
			dd("0.0263157894736842105263157894736842105"),		//	1/38
			dd("0.0256410256410256410256410256410256410"),		//	1/39
			dd("0.025"),										//	1/40
			dd("0.0243902439024390243902439024390243902")		//	1/41
		};

		const dd dd_inv_fact[] = {
			dd("1.0"),											//	1/0!
			dd("1.0"),											//	1/1!
			dd("0.5"),											//	1/2!
			dd("1.66666666666666666666666666666666667E-1"),		//	1/3!
			dd("4.16666666666666666666666666666666667E-2"),		//	1/4!
			dd("8.33333333333333333333333333333333333E-3"),		//	1/5!
			dd("1.38888888888888888888888888888888889E-3"),		//	1/6!
			dd("1.98412698412698412698412698412698413E-4"),		//	1/7!
			dd("2.48015873015873015873015873015873016E-5"),		//	1/8!
			dd("2.75573192239858906525573192239858907E-6"),		//	1/9!
			dd("2.75573192239858906525573192239858907E-7"),		//	1/10!
			dd("2.50521083854417187750521083854417188E-8"),		//	1/11!
			dd("2.08767569878680989792100903212014323E-9"),		//	1/12!
			dd("1.60590438368216145993923771701549479E-10"),	//	1/13!
			dd("1.14707455977297247138516979786821057E-11"),	//	1/14!
			dd("7.64716373181981647590113198578807044E-13"),	//	1/15!
			dd("4.77947733238738529743820749111754403E-14"),	//	1/16!
			dd("2.81145725434552076319894558301032002E-15"),	//	1/17!
			dd("1.56192069685862264622163643500573334E-16"),	//	1/18!
			dd("8.22063524662432971695598123687228075E-18"),	//	1/19!
			dd("4.11031762331216485847799061843614037E-19"),	//	1/20!
			dd("1.95729410633912612308475743735054304E-20"),	//	1/21!
			dd("8.89679139245057328674889744250246834E-22"),	//	1/22!
			dd("3.86817017063068403771691193152281232E-23"),	//	1/23!
			dd("1.61173757109611834904871330480117180E-24"),	//	1/24!
			dd("6.44695028438447339619485321920468721E-26"),	//	1/25!
			dd("2.47959626322479746007494354584795662E-27"),	//	1/26!
			dd("9.18368986379554614842571683647391340E-29"),	//	1/27!
			dd("3.27988923706983791015204172731211193E-30"),	//	1/28!
			dd("1.13099628864477169315587645769383170E-31"),	//	1/29!
			dd("3.76998762881590564385292152564610566E-33"),	//	1/30!
			dd("1.21612504155351794962997468569229215E-34"),	//	1/31!
			dd("3.80039075485474359259367089278841297E-36"),	//	1/32!
			dd("1.15163356207719502805868814932982211E-37"),	//	1/33!
		};

		void EnumerateConstants() {
			dd _zero("0.0"); report(_zero);
			dd _one("1.0"); report(_one);
			dd _ten("10.0"); report(_ten);

			dd _tenth("0.1"); report(_tenth);
			dd _third("0.333333333333333333333333333333333333"); report(_third);

			dd _2pi("6.283185307179586476925286766559005768"); report(_2pi);
			dd _pi("3.141592653589793238462643383279502884"); report(_pi);
			dd _pi2("1.570796326794896619231321691639751442"); report(_pi2);
			dd _pi4("0.785398163397448309615660845819875721"); report(_pi4);
			dd _3pi4 = _pi2 + _pi4;	report(_3pi4);

			dd _e("2.718281828459045235360287471352662498"); report(_e);

			dd _ln2("0.693147180559945309417232121458176568"); report(_ln2);
			dd _ln10("2.302585092994045684017991454684364208"); report(_ln10);

			dd _lge("1.442695040888963407359924681001892137"); report(_lge);
			dd _lg10("3.321928094887362347870319429489390176"); report(_lg10);

			dd _log2("0.301029995663981195213738894724493027"); report(_log2);
			dd _loge("0.434294481903251827651128918916605082"); report(_loge);

			dd _sqrt2("1.414213562373095048801688724209698079"); report(_sqrt2);

			dd _inv_pi("0.318309886183790671537767526745028724"); report(_inv_pi);
			dd _inv_pi2("0.636619772367581343075535053490057448"); report(_inv_pi2);
			dd _inv_e("0.367879441171442321595523770161460867"); report(_inv_e);
			dd _inv_sqrt2("0.707106781186547524400844362104849039"); report(_inv_sqrt2);
		}

		int VerifyParse(const std::string& str) {
			int nrFailedTestCases{ 0 };
			dd v{};
			if (!parse(str, v)) {
				std::cerr << "failed to parse " << str << '\n';
				++nrFailedTestCases;
			}
			else {
				ReportValue(v, str);
				std::cout << "PASS\n";
			}
			return nrFailedTestCases;
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

	std::string test_suite  = "double-double constants";
	std::string test_tag    = "dd constants";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		dd a, b, c;
		double _third = 0.3333333333333333333333333333333;
		double _third2 = _third * pow(2.0, -53.0);
		double _short = 0.3333333333333333;
		ReportValue(_short, "0.3333333333333333", 35, 32);
		ReportValue(_third, "0.3333333333333333333333333333333", 35, 32);

		a = _third;
		b = _third2;
		ReportValue(a, "0.3333....", 35, 32);
		ReportValue(b, "0.3333....*2^-53", 35, 32);
		c = a + b;
		ReportValue(c, "0.3333....", 35, 32);
		std::cout << to_pair(c) << '\n';

		dd d(_third, _third2);
		ReportValue(d, "0.3333....", 35, 32);
		std::cout << to_pair(d) << '\n';

		dd e("0.3333333333333333333333333333333333333333333333333");
		ReportValue(e, "0.3333....", 35, 32);
		std::cout << to_pair(e) << '\n';

		dd f(0.3333333333333333, 1.8503717077085935e-17);
		ReportValue(f, "0.3333....", 35, 32);
		std::cout << to_pair(f) << '\n';

		dd g = reciprocal(dd(3.0));
		ReportValue(g, "1/3", 35, 32);
		std::cout << to_pair(g) << '\n';
	}


	// parsing scientific formats
	VerifyParse("12.5e-2");
	VerifyParse("12.5e-1");
	VerifyParse("12.5e-0");
	VerifyParse("12.5e+1");
	VerifyParse("12.5e2");
	VerifyParse("12.5e-02");
	VerifyParse("12.5e-01");
	VerifyParse("12.5e00");
	VerifyParse("12.5e+01");
	VerifyParse("12.5e02");
	VerifyParse("12.5e-002");
	VerifyParse("12.5e-001");
	VerifyParse("12.5e000");
	VerifyParse("12.5e+001");
	VerifyParse("12.5e002");
	VerifyParse("12.5e-200");
	VerifyParse("12.5e-100");
	VerifyParse("12.5e000");
	VerifyParse("12.5e+100");
	VerifyParse("12.5e200");

	std::cout << "verifying constants\n";
	struct constant_kv {
		std::string name;
		std::string digits;
		dd value;
	} constant_symbol_table[] = {
		{ "dd_2pi", "6.283185307179586476925286766559005768", dd_2pi },
		{ "dd_pi" , "3.141592653589793238462643383279502884", dd_pi },
		{ "dd_pi2", "1.570796326794896619231321691639751442", dd_pi_2 },
		{ "dd_pi4", "0.785398163397448309615660845819875721", dd_pi_4 },

		{ "dd_e"  , "2.718281828459045235360287471352662498", dd_e },

		{ "dd_ln2", "0.693147180559945309417232121458176568", dd_ln2 },
		{ "dd_ln10", "2.302585092994045684017991454684364208", dd_ln10 },

		{ "dd_lge", "1.442695040888963407359924681001892137", dd_lge },
		{ "dd_lg10", "3.321928094887362347870319429489390176", dd_lg10 },

		{ "dd_log2", "0.301029995663981195213738894724493027", dd_log2 },
		{ "dd_loge", "0.434294481903251827651128918916605082", dd_loge },

		{ "dd_sqrt2", "1.414213562373095048801688724209698079", dd_sqrt2 },

		{ "dd_1_pi", "0.318309886183790671537767526745028724", dd_2_pi },
		{ "dd_2_pi", "0.636619772367581343075535053490057448", dd_2_pi },
		{ "dd_1_e", "0.367879441171442321595523770161460867", dd_1_e },
		{ "dd_1_sqrt2", "0.707106781186547524400844362104849039", dd_1_sqrt2 },
	};

	{
		// calculate 1 / sqrt(pi / 4)
		dd sqrt_pi_4 = sqrt(dd_pi_4);
		dd result = reciprocal(sqrt_pi_4);
		std::cout << "1 / sqrt(pi / 4) : " << result << '\n';
		std::cout << to_pair(result) << '\n';
	}
	/*
	* 
	* ETLO August 6, 2024
	* Need to verify if these are the most accurate double-double approximations available.
	* 
verifying constants
dd_2pi          : 6.28318530717958647692528676655896e+00 vs 6.28318530717958647692528676655901e+00 : ( 6.28318530717958620,  2.4492935982947059e-16) : -4.93038065763132378382330353301741e-32
dd_pi           : 3.14159265358979323846264338327948e+00 vs 3.14159265358979323846264338327951e+00 : ( 3.14159265358979310,  1.2246467991473530e-16) : -2.46519032881566189191165176650871e-32
dd_pi2          : 1.57079632679489661923132169163974e+00 vs 1.57079632679489661923132169163976e+00 : ( 1.57079632679489660,  6.1232339957367648e-17) : -1.23259516440783094595582588325435e-32
dd_pi4          : 7.85398163397448309615660845819878e-01 vs 7.85398163397448309615660845819878e-01 : ( 0.78539816339744828,  3.0616169978683830e-17) : 0.00000000000000000000000000000000e+00
dd_e            : 2.71828182845904523536028747135264e+00 vs 2.71828182845904523536028747135266e+00 : ( 2.71828182845904510,  1.4456468917292499e-16) : -2.46519032881566189191165176650871e-32
dd_ln2          : 6.93147180559945309417232121458176e-01 vs 6.93147180559945309417232121458176e-01 : ( 0.69314718055994529,  2.3190468138462996e-17) : 0.00000000000000000000000000000000e+00
dd_ln10         : 2.30258509299404568401799145468437e+00 vs 2.30258509299404568401799145468437e+00 : ( 2.30258509299404590, -2.1707562233822494e-16) : 0.00000000000000000000000000000000e+00
dd_lge          : 1.44269504088896340735992468100189e+00 vs 1.44269504088896340735992468100189e+00 : ( 1.44269504088896340,  2.0355273740931027e-17) : 0.00000000000000000000000000000000e+00
dd_lg10         : 3.32192809488736234787031942948935e+00 vs 3.32192809488736234787031942948935e+00 : ( 3.32192809488736220,  1.6616175169735918e-16) : 0.00000000000000000000000000000000e+00
dd_log2         : 3.01029995663981195213738894724493e-01 vs 6.93147180559945309417232121458176e-01 : ( 0.30102999566398120, -2.8037281277851700e-18) : -3.92117184895964114203493226733683e-01
dd_loge         : 4.34294481903251827651128918916605e-01 vs 4.34294481903251827651128918916605e-01 : ( 0.43429448190325182,  1.0983196502167652e-17) : 0.00000000000000000000000000000000e+00
dd_sqrt2        : 1.41421356237309504880168872420971e+00 vs 1.41421356237309504880168872420971e+00 : ( 1.41421356237309510, -9.6672933134529122e-17) : 0.00000000000000000000000000000000e+00
dd_inv_pi       : 3.18309886183790671537767526745029e-01 vs 3.18309886183790671537767526745029e-01 : ( 0.31830988618379069, -1.9678676675182486e-17) : 0.00000000000000000000000000000000e+00
dd_inv_pi2      : 6.36619772367581343075535053490057e-01 vs 6.36619772367581343075535053490057e-01 : ( 0.63661977236758138, -3.9357353350364972e-17) : 0.00000000000000000000000000000000e+00
dd_inv_e        : 3.67879441171442321595523770161459e-01 vs 3.67879441171442321595523770161459e-01 : ( 0.36787944117144233, -1.2428753672788364e-17) : 0.00000000000000000000000000000000e+00
dd_inv_sqrt2    : 7.07106781186547524400844362104854e-01 vs 7.07106781186547524400844362104854e-01 : ( 0.70710678118654757, -4.8336466567264561e-17) : 0.00000000000000000000000000000000e+00
	 */
	auto defaultPrecision = std::cout.precision();
	std::cout << std::setprecision(32);
	for (auto record : constant_symbol_table) {
		dd a(record.digits);
		dd error = (a - record.value);
		std::cout << std::left << std::setw(15) << record.name << " : " << a << " vs " << record.value << " : " << to_pair(a) << " : " << error << '\n';
	}
	std::cout << std::setprecision(defaultPrecision);

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
