// unum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../../posit/posit.hpp"

using namespace std;

constexpr unsigned int MAX_ES = 5;
constexpr unsigned int MAX_K = 10;
uint64_t GENERATED_SCALE_FACTORS[MAX_ES][MAX_K];

void generateScaleFactorLookupTable() {
	uint64_t useed, useed_power_k;
	for (int es = 0; es < MAX_ES; es++) {
		useed = two_to_the_power(two_to_the_power(uint64_t(es)));
		useed_power_k = useed; 
		GENERATED_SCALE_FACTORS[es][0] = 1; // for k = 0
		for (int k = 1; k < MAX_K; k++) {
			useed_power_k *= useed;
			GENERATED_SCALE_FACTORS[es][k] = useed_power_k;
		}
	}

}

void printScaleFactors(uint64_t scale_factors[MAX_ES][MAX_K]) {
	cout << "      ";
	for (int k = 0; k < MAX_K; k++) {
		cout << "     k = " << k << "   ";
	}
	cout << endl;
	for (int es = 0; es < MAX_ES; es++) {
		cout << "es = " << es << " ";
		for (int k = 0; k < MAX_K; k++) {
			cout << setw(12) << scale_factors[es][k] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

int main()
{
	generateScaleFactorLookupTable();
	printScaleFactors(GENERATED_SCALE_FACTORS);
    
	return 0;
}



