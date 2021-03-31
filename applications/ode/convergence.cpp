// convergence.cpp: convergence analysis of ODE solvers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// Author: Jacob Todd  jtodd1@une.edu
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license

#include <iostream>
#include <fstream>
#include "ode_solvers.hpp"
#include <universal/number/posit/posit>

template<typename Scalar>
Scalar my_ode_func(const Scalar& t, const Scalar& u) {
	return (-5*u);
}

template<typename Scalar>
Scalar my_true_func(const Scalar& t) {
    using namespace std;
	return (exp(-5*t));
}

template<typename Scalar>
void Convergence(std::vector<std::vector<Scalar>> results, Scalar (*true_func)(const Scalar&), char write_to[]) {
    using namespace std;
    std::ofstream ofs;
    ofs.open(write_to);
    ofs << "t,approximation,true,error\n";
    for (unsigned int i = 0; i < results[0].size(); i++) {
        Scalar true_value = true_func(results[0][i]);
        Scalar error = true_value - results[1][i];
        ofs << results[0][i] << "," << results[1][i] << "," << true_value << "," << error << "\n";
    };
    ofs.close();
}

int main() {
    using namespace std;
    using namespace sw::universal;
    using Scalar = double;
    Scalar butcher[5][5] = {
    {0, 0, 0, 0, 0},
    {0.5, 0.5, 0, 0, 0},
    {0.5, 0, 0.5, 0, 0},
    {1, 0, 0, 1, 0},
    {0, Scalar(1)/Scalar(6), Scalar(1)/Scalar(3), Scalar(1)/Scalar(3), Scalar(1)/Scalar(6)}
    };
    int steps[3] = {10, 100, 5000};
    Scalar u0 = 1;
    Scalar tspan[2] = {0, 1};

    auto solution = GRKSpan(butcher, my_ode_func, u0, tspan, steps[1]);
    char out_path[] = "/mypath";
    Convergence(solution, my_true_func, out_path);
}