// fibonacci.cpp: experiments with representing Fibonacci sequences
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/integer/integer.hpp>
#include <universal/sequences/tribonacci.hpp>

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;
	using namespace sw::sequences;

	int nrOfFailedTestCases = 0;

	{
		using Scalar = sw::universal::integer<64>;
		constexpr unsigned N = 10;
		auto v = Tribonacci<Scalar>(N);
		cout << "Tribonacci Sequence: " << v.size() << endl;
		// for (auto e: v) { cout << e << '\n'; }

		for (unsigned n = 1; n <= N; ++n) {
			cout << setw(3) << n << " : " << TribonacciNumber<Scalar>(n) << endl;
		}
	}

	// enumerate till we exhaust the number system
	{
		constexpr size_t N = 256;
		using Scalar = sw::universal::integer<N,uint32_t>;

		unsigned next = 20;
		Scalar tri_n_minus_1 = TribonacciNumber<Scalar>(next++);
		Scalar tri_n         = TribonacciNumber<Scalar>(next);
		while (tri_n > tri_n_minus_1) {
			tri_n_minus_1 = tri_n;
			tri_n = TribonacciNumber<Scalar>(++next);
		}
		cout << "Largest Tribonacci number that can be represented by " << typeid(Scalar).name() << " is\n";
		cout << "T(" << next << ") = " << tri_n_minus_1 << endl;
		std::string rep = to_string(tri_n_minus_1);
		cout << "Number of digits: " << rep.size() << "    binary size relates to decimal size as " << N << "/3.3 ~ " << int(float(N)/3.3f) << " digits\n";
		cout << tri_n << endl;
		rep = to_string(tri_n);
		cout << "Number of digits: " << rep.size() << endl;
	}

	{
		// approximate the tribonacci constant with Tribonacci numbers

/*
* http://www.plouffe.fr/simon/constants/tribo.txt
		
                  1/2 1/3                 1/2 1/3       n              1/2 1/3
   (1/3 (19 + 3 33   )    + 1/3 (19 - 3 33   )    + 1/3)  (586 + 102 33   )
 3 ---------------------------------------------------------------------------
                             1/2 2/3                      1/2 1/3
                (586 + 102 33   )    + 4 - 2 (586 + 102 33   )


To get the actual n'th Tribonacci number just round the result to the
nearest integer.

Here is the formula 'lprinted'...

3*(1/3*(19+3*33^(1/2))^(1/3)+1/3*(19-3*33^(1/2))^(1/3)+1/3)^n/((586+102*33^(1
/2))^(2/3)+4-2*(586+102*33^(1/2))^(1/3))*(586+102*33^(1/2))^(1/3);

The Tribonacci constant is the number,


             / 19          1/2\1/3              4
             |---- + 1/9 33   |    + ----------------------- + 1/3
             \ 27             /        / 19          1/2\1/3
                                     9 |---- + 1/9 33   |
                                       \ 27             /

That is, to 2000 digits,

1.8392867552141611325518525646532866004241787460975922467787586394042032220819\
664257384354194283070141419798268592409741641784507465074369438315458204995137\
962496555396446136661215402779726781189410412116092232821559560718167121823659\
866522733785378156969892521173957914132287210618789840852549569311453491349853\
459576175035965221323814247272722417358187700069790551025490449657107425265477\
228110065989375556363093330528262357538519719942991453008254663977472900587005\
974481391931672825848839626332970700687236831127837750250557122275153259578946\
560570686422283918659698294691356239220443192476147068811451726766712743964146\
212571843342662340390218352494591033227231061513286997030808036302223324997105\
243107472354231399744381826565607351940357874911762680524537079221110849710806\
876410050156541475662235008885665949715821834184868714802901255436993480513679\
165025853053878276666126224317766358200942985505387325991651787730184472388604\
262223248578207927210491601817837256132034398143022745339976212315504033867429\
329211621461871005431394971720877066783628535843161868720076270556066286615872\
583604894548715974517957319994623277097882394431315993156954991902982990489811\
919162577227821911741801990454404999470440432292859815087349182932451478281234\
011480030070735745986560988334384935663011291738932340632674811917649501573719\
740158804774047763482793990163956499470645515174942095990467587524349090141939\
486129791076083227246286714743514591895896961639473879820311021550411824865031\
569338304874203198108804658533318308514303663024494607359312956920585419684618\
243975811650388247397223670172954759783519181934736394975136472916687566443612\
683155587831906926554378903136489971524686054164113260296410779699661802650907\
758291861964381973767049357163573551440417861877682790630246446436644681984810\
216684354867161274190793389271846906122690289889541937481380042108236458264769\
135451306315598582881579145267928147361235964430962001482193710376258244980500\
206455704970548795335106944401651706671178932997839


This formula has 2 parts, first the numerator is the root of (x^3-x^2-x-1)
no surprise here, but the denominator was obtained using LLL (Pari-Gp)
algorithm. The thing is, if you try to get a closed formula by doing
the Z-transform or anything classical, it won't work very well since
the actual symbolic expression will be huge and won't simplify.

The numerical values of Tribonacci numbers are c**n essentially and
the c here is one of the roots of (x^3-x^2-x-1), then there is another
constant c2. So the exact formula is c**n/c2.

Another way of doing 'exact formulas' are given by using [ ] function
the n'th term of the series expansion of 1/(1-x-x**2) is

        1-2*[(n+2)/3)]+[(n+1)/3]+[n/3].

*/
		constexpr size_t N = 256;
		using Scalar = sw::universal::integer<N, uint32_t>;
		constexpr unsigned MaxT = 80;
		auto v = Tribonacci<Scalar>(MaxT);  // T(294) is biggest Tribonacci number for int256
		for (const Scalar& e : v) { cout << e << '\n'; }
		streamsize precision = cout.precision();
		cout << setprecision(30);
		cout << "oracle : 1.8392867552141611325518525646532866004241787460975922467787586394042032220819\n";
		for (unsigned i = MaxT-10; i < MaxT; ++i) {
			double Tn = double(v[i]);
			double Tn1 = double(v[i - 1]);
			cout << Tn << " : double(" << v[i] << ")\n";
			//cout << Tn1 << endl;
			double phi = Tn / Tn1;
			cout << setw(6) << i << " : " << setw(30) << phi << endl;
		}
		// TODO: there is a bug in the conversion of intteger to double
		// 72 : 1.83928675521416118421313967701
		// 73 : 1.83928675521416118421313967701
		// 74 : 1.83928675521416140625774460204
		// 75 : -0.790232284311144894672906957567   <--- double conversion is incorrect
		// 76 : -0.953462346881624567274116088811
		// 77 : 1.2784071172071358457600354086
		// 78 : 0.961820366834929441068879896193
		// 79 : 0.0146916049436009929496371739788
		cout << setprecision(precision);
	}


	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
