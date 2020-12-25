#pragma once
// tribonacci.hpp: definition of the Tribonacci sequence: T(n) = T(n-1) + T(n-2) + T(n-3)
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <tuple>

/** Background
 * https://en.wikipedia.org/wiki/List_of_integer_sequences
 *
 * https://en.wikipedia.org/wiki/Generalizations_of_Fibonacci_numbers#Tribonacci_numbers
 *
 * https://oeis.org/A000073
 *
 * 0, 0, 1, 1, 2, 4, 7, 13, 24, 44, 81, 149, 274, 504, 927, 1705, 3136, 5768, 10609, 19513, 35890, 66012, … (sequence A000073 in the OEIS) 
 *
 * The series was first described formally by Agronomof in 1914, but its first unintentional 
 * use is in the Origin of species by Charles R. Darwin. In the example of illustrating the 
 * growth of elephant population, he relied on the calculations made by his son, George H. Darwin. 
 * The term tribonacci was suggested by Feinberg in 1963.
 */

namespace sw { namespace sequences {

// generate the Tribonacci sequence of number of terms
// limited argument to be an unsigned int as we are returning
// the full vector of values. There is no practical use case
// that could support an argument of type size_t (unsigned long long)
template<typename Ty>
std::vector<Ty> Tribonacci(unsigned terms) {
    Ty nminus2 = 0, nminus1 = 0, n = 1;
    std::vector<Ty> v;
    if (terms == 0) return v;
    v.push_back(nminus2);
    if (terms == 1) return v;
    v.push_back(nminus1);
    if (terms == 2) return v;
    v.push_back(n);
    if (terms == 3) return v;
    for (unsigned c = 3; c < terms; ++c) {
	Ty next = nminus2 + nminus1 + n;
	v.push_back(next);
	nminus2 = nminus1;
	nminus1 = n;
        n = next;
    }
    return v;
}

// generate the n'th Tribonacci number
template<typename Ty>
Ty TribonacciNumber(unsigned n) {
    Ty nminus2 = 0, nminus1 = 0, n0 = 1;

    if (n <= 0) return Ty(0);
    if (n == 1) return Ty(0);
    if (n == 2) return Ty(0);
    if (n == 3) return Ty(1);
    for (unsigned c = 4; c <= n; ++c) {
        Ty next = nminus2 + nminus1 + n0;
        nminus2 = nminus1;
        nminus1 = n0;
        n0 = next;
    }
    return n0;
}

/*
 * this begs the question: should the argument and the return value
 * be different types? Clearly, the Tribonacci number will not be
 * representable by the type of the argument as it will be greater
 * than the maximum value associated with the argument type.
 * 
 * To support consistency the answer is yes, as we could write a
 * statement that will yield a consistent answer. Problem is that
 * it would also require a computational relationship between the
 * types that would be rediculously expensive to compute without
 * an analytical formula.
 * 
 * I have settled on a projection from the unsigned ints to a target
 * number system to side step all this complexity.
 */
}}  // namespace sw::sequences
