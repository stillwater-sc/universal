//  distinct_powers.cpp : algorithm to find ll integer combinations of a^b for some range
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
// integer number system
#include <universal/integer/integer.hpp>
#include <universal/integer/integer_functions.hpp>

/*
 * Consider all integer combinations of a^b for 2 <= a <= upperbound, 2 <= b <= upperbound
 *
 * Sorted, with any repeats removed, we get some sequence. What is the cardinality of that sequence?
 */
template<size_t nbits>
sw::unum::integer<nbits> IntegerPowerCombinations(const sw::unum::integer<nbits>& min, const sw::unum::integer<nbits>& max) {
	using namespace std;
	using namespace sw::unum;
	using Integer = integer<nbits>;

	vector<Integer> combinations;
	for (Integer a = min; a <= max; ++a) {
		for (Integer b = min; b <= max; ++b) {
//			cout << a << '^' << b << " = " << ipow(a, b) << endl;
			combinations.push_back(ipow(a, b));
		}
	}
	sort(combinations.begin(), combinations.end());
	// remove duplicates
	combinations.erase(unique(combinations.begin(), combinations.end()), combinations.end());
	return Integer(combinations.size());
}
	
template<size_t nbits>
sw::unum::integer<nbits> IntegerPowerCombinationsUsingSet(const sw::unum::integer<nbits>& min, const sw::unum::integer<nbits>& max) {
	using namespace std;
	using namespace sw::unum;
	using Integer = integer<nbits>;

	set<Integer> combinations;
	for (Integer a = min; a <= max; ++a) {
		for (Integer b = min; b <= max; ++b) {
//			cout << a << '^' << b << " = " << ipow(a, b) << endl;
			combinations.insert(ipow(a, b));
		}
	}
	return Integer(combinations.size());
}

int main()
try {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 1024;
	using Integer = integer<nbits>;

	cout << "100^100 = " << ipow(Integer(100), Integer(100)) << endl;

	Integer min = 90;
	for (Integer max = min + 1; max <= 100; ++max) {
		Integer cardinality = IntegerPowerCombinations(min, max);

		cout << "cardinality of integer power combinations in the range "
			 << "[" << min << ',' << max << "] = " << cardinality << endl;
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

/*
cardinality of integer power combinations in the range [2,3] = 4
cardinality of integer power combinations in the range [2,4] = 8
cardinality of integer power combinations in the range [2,5] = 15
cardinality of integer power combinations in the range [2,6] = 23
cardinality of integer power combinations in the range [2,7] = 34
cardinality of integer power combinations in the range [2,8] = 44
cardinality of integer power combinations in the range [2,9] = 54
cardinality of integer power combinations in the range [2,10] = 69
cardinality of integer power combinations in the range [2,11] = 88
cardinality of integer power combinations in the range [2,12] = 106
cardinality of integer power combinations in the range [2,13] = 129
cardinality of integer power combinations in the range [2,14] = 152
cardinality of integer power combinations in the range [2,15] = 177
cardinality of integer power combinations in the range [2,16] = 195
cardinality of integer power combinations in the range [2,17] = 226
cardinality of integer power combinations in the range [2,18] = 256
cardinality of integer power combinations in the range [2,19] = 291
cardinality of integer power combinations in the range [2,20] = 324
cardinality of integer power combinations in the range [2,21] = 361
cardinality of integer power combinations in the range [2,22] = 399
cardinality of integer power combinations in the range [2,23] = 442
cardinality of integer power combinations in the range [2,24] = 483
cardinality of integer power combinations in the range [2,25] = 519
cardinality of integer power combinations in the range [2,26] = 564
cardinality of integer power combinations in the range [2,27] = 600
cardinality of integer power combinations in the range [2,28] = 648
cardinality of integer power combinations in the range [2,29] = 703
cardinality of integer power combinations in the range [2,30] = 755
cardinality of integer power combinations in the range [2,31] = 814
cardinality of integer power combinations in the range [2,32] = 849
cardinality of integer power combinations in the range [2,33] = 906
cardinality of integer power combinations in the range [2,34] = 965
cardinality of integer power combinations in the range [2,35] = 1027
cardinality of integer power combinations in the range [2,36] = 1071
cardinality of integer power combinations in the range [2,37] = 1140
cardinality of integer power combinations in the range [2,38] = 1206
cardinality of integer power combinations in the range [2,39] = 1275
cardinality of integer power combinations in the range [2,40] = 1344
cardinality of integer power combinations in the range [2,41] = 1421
cardinality of integer power combinations in the range [2,42] = 1492
cardinality of integer power combinations in the range [2,43] = 1570
cardinality of integer power combinations in the range [2,44] = 1645
cardinality of integer power combinations in the range [2,45] = 1723
cardinality of integer power combinations in the range [2,46] = 1802
cardinality of integer power combinations in the range [2,47] = 1888
cardinality of integer power combinations in the range [2,48] = 1953
cardinality of integer power combinations in the range [2,49] = 2019
cardinality of integer power combinations in the range [2,50] = 2104
cardinality of integer power combinations in the range [2,51] = 2193
cardinality of integer power combinations in the range [2,52] = 2282
cardinality of integer power combinations in the range [2,53] = 2379
cardinality of integer power combinations in the range [2,54] = 2471
cardinality of integer power combinations in the range [2,55] = 2570
cardinality of integer power combinations in the range [2,56] = 2652
cardinality of integer power combinations in the range [2,57] = 2752
cardinality of integer power combinations in the range [2,58] = 2852
cardinality of integer power combinations in the range [2,59] = 2960
cardinality of integer power combinations in the range [2,60] = 3062
cardinality of integer power combinations in the range [2,61] = 3174
cardinality of integer power combinations in the range [2,62] = 3276
cardinality of integer power combinations in the range [2,63] = 3382
cardinality of integer power combinations in the range [2,64] = 3424
cardinality of integer power combinations in the range [2,65] = 3534
cardinality of integer power combinations in the range [2,66] = 3641
cardinality of integer power combinations in the range [2,67] = 3756
cardinality of integer power combinations in the range [2,68] = 3860
cardinality of integer power combinations in the range [2,69] = 3975
cardinality of integer power combinations in the range [2,70] = 4090
cardinality of integer power combinations in the range [2,71] = 4212
cardinality of integer power combinations in the range [2,72] = 4298
cardinality of integer power combinations in the range [2,73] = 4423
cardinality of integer power combinations in the range [2,74] = 4545
cardinality of integer power combinations in the range [2,75] = 4671
cardinality of integer power combinations in the range [2,76] = 4782
cardinality of integer power combinations in the range [2,77] = 4914
cardinality of integer power combinations in the range [2,78] = 5042
cardinality of integer power combinations in the range [2,79] = 5178
cardinality of integer power combinations in the range [2,80] = 5262
cardinality of integer power combinations in the range [2,81] = 5352
cardinality of integer power combinations in the range [2,82] = 5487
cardinality of integer power combinations in the range [2,83] = 5630
cardinality of integer power combinations in the range [2,84] = 5745
cardinality of integer power combinations in the range [2,85] = 5890
cardinality of integer power combinations in the range [2,86] = 6032
cardinality of integer power combinations in the range [2,87] = 6179
cardinality of integer power combinations in the range [2,88] = 6277
cardinality of integer power combinations in the range [2,89] = 6430
cardinality of integer power combinations in the range [2,90] = 6579
cardinality of integer power combinations in the range [2,91] = 6736
cardinality of integer power combinations in the range [2,92] = 6856
cardinality of integer power combinations in the range [2,93] = 7013
cardinality of integer power combinations in the range [2,94] = 7169
cardinality of integer power combinations in the range [2,95] = 7332
cardinality of integer power combinations in the range [2,96] = 7419
cardinality of integer power combinations in the range [2,97] = 7586
cardinality of integer power combinations in the range [2,98] = 7749
cardinality of integer power combinations in the range [2,99] = 7917
cardinality of integer power combinations in the range [2,100] = 7994
 */
