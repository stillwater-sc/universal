// convergence.cpp: convergence analysis of ODE solvers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
// Author: Jacob Todd  jtodd1@une.edu
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>

template<typename Scalar>
Scalar my_ode_func(const Scalar& t, const Scalar& u) {
	return (-5*u);
}

template<typename Scalar>
Scalar golden_reference(const Scalar& t) {
	return (exp(-5*t));
}

/// <summary>
/// GRKValue is ...
/// </summary>
/// <typeparam name="Scalar"></typeparam>
/// <param name="b_table"></param>
/// <param name="f"></param>
/// <param name="h"></param>
/// <param name="t0"></param>
/// <param name="u0"></param>
/// <returns></returns>
template <typename Scalar>
Scalar GRKValue(Scalar b_table[5][5], Scalar(*f)(const Scalar&, const Scalar&), Scalar h, Scalar t0, Scalar u0) {
    int s = sizeof(b_table[0]) / sizeof(b_table[0][0]) - 1; // number of steps
    Scalar ks[4];
    std::fill(ks, ks + s, Scalar(0));

    for (int i = 0; i < s; i++) {
        Scalar sum = 0;
        for (int j = 1; j <= s; j++) {
            sum = sum + b_table[i][j] * ks[j - 1];
        }
        sum = h * sum;
        ks[i] = f(t0 + h * b_table[0][i], u0 + sum);
    }

    Scalar out = 0;
    for (int i = 1; i <= s; i++) {
        out = out + b_table[s][i] * ks[i - 1];
    }
    out = out + u0;
    return out;
}

/// <summary>
/// GRKSpan is ...
/// </summary>
/// <typeparam name="Scalar"></typeparam>
/// <param name="b_table"></param>
/// <param name="f"></param>
/// <param name="u0"></param>
/// <param name="tspan"></param>
/// <param name="n"></param>
/// <returns>is a vector of vectors the best return value?</returns>
template <typename Scalar>
std::vector<std::vector<Scalar>> GRKSpan(Scalar b_table[5][5], Scalar(*f)(const Scalar&, const Scalar&), Scalar u0, Scalar tspan[2], int n) {
    std::vector<std::vector<Scalar>> approximations(2, std::vector<Scalar>(n));
    int s = sizeof(b_table[0]) / sizeof(b_table[0][0]) - 1; // number of steps    : if b_table would be a dynamic matrix then number of rows and columns would be known: TODO
    Scalar h = (tspan[1] - tspan[0]) / n;
    Scalar ui = u0;
    int row = 0;
    for (Scalar t = tspan[0]; t <= tspan[1]; t = t + h) {
        Scalar ks[4];
        std::fill(ks, ks + s, Scalar(0));

        for (int i = 0; i < s; i++) {
            Scalar sum = 0;
            for (int j = 1; j <= s; j++) {
                sum = sum + b_table[i][j] * ks[j - 1];
            }
            sum = h * sum;
            ks[i] = f(t + h * b_table[0][i], ui + sum);
        }

        Scalar out = 0;
        for (int i = 1; i <= s; i++) {
            out = out + b_table[s][i] * ks[i - 1];
        }
        ui = ui + out;

        approximations[0][row] = t;    // TODO: can we make this cleaner/simpler?
        approximations[1][row] = ui;
        row = row + 1;
    }
    return(approximations);
}

template <typename Scalar, typename Vector>
void GRKSpanDemo(Scalar b_table[5][5], 
                Scalar(*f)(const Scalar&, const Scalar&), 
                Scalar u0, Scalar tspan[2], int n,
                Vector& t_s, Vector& ui_s) {

    int s = sizeof(b_table[0]) / sizeof(b_table[0][0]) - 1; // number of steps    : if b_table would be a dynamic matrix then number of rows and columns would be known: TODO
    Scalar h = (tspan[1] - tspan[0]) / n;
    Scalar ui = u0;
    int row = 0;
    for (Scalar t = tspan[0]; t <= tspan[1]; t = t + h) {
        Scalar ks[4];
        std::fill(ks, ks + s, Scalar(0));

        for (int i = 0; i < s; i++) {
            Scalar sum = 0;
            for (int j = 1; j <= s; j++) {
                sum = sum + b_table[i][j] * ks[j - 1];
            }
            sum = h * sum;
            ks[i] = f(t + h * b_table[0][i], ui + sum);
        }

        Scalar out = 0;
        for (int i = 1; i <= s; i++) {
            out = out + b_table[s][i] * ks[i - 1];
        }
        ui = ui + out;

        t_s[row] = t;    // TODO: can we make this cleaner/simpler?
        ui_s[row] = ui;
        row = row + 1;
    }


}

int main() 
try {
    using namespace sw::universal;
    using Scalar = double;

    // TODO: we should make this a dynamic matrix so number of rows and columns is part of the type
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

    {
        auto solution = GRKSpan(butcher, my_ode_func, u0, tspan, steps[1]);
        std::string outputFile("ode_convergence.csv");
        std::cout << "Record the ODE solver convergence steps for offline graphing\nWriting to file: " << outputFile << '\n';
        std::ofstream ofs;
        ofs.open(outputFile);
        ofs << "t,approximation,true,error\n";
        for (unsigned int i = 0; i < solution[0].size(); i++) {
            Scalar true_value = golden_reference(solution[0][i]);
            Scalar error = true_value - solution[1][i];
            ofs << solution[0][i] << "," << solution[1][i] << "," << true_value << "," << error << "\n";
        };
        ofs.close();  
    }

    {
        std::string outputFile("ode_convergence2.csv");
        int N = steps[1];
        std::vector<Scalar> t_s(N);
        std::vector<Scalar> ui_s(N);
        GRKSpanDemo(butcher, my_ode_func, u0, tspan, steps[1], t_s, ui_s);
        std::ofstream ofs;
        ofs.open(outputFile);
        ofs << "t,approximation,true,error\n";
        for (unsigned int i = 0; i < t_s.size(); i++) {
            Scalar true_value = golden_reference(t_s[i]);
            Scalar error = true_value - ui_s[i];
            ofs << t_s[i] << "," << ui_s[i] << "," << true_value << "," << error << "\n";
        };
        ofs.close();  
    }
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
catch (std::runtime_error& err) {
    std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return EXIT_FAILURE;
}