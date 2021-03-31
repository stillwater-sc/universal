// ode_solvers.hpp: functions to solve ODE problems
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// Author: Jacob Todd  jtodd1@une.edu
//
// This file is part of the universal numbers project, which is released under an MIT Open Source licens

template <typename Scalar>
Scalar GRKValue(Scalar b_table[5][5], Scalar (*f)(const Scalar&, const Scalar&), const Scalar h, Scalar t0, Scalar u0) {
	using namespace std;
	int s = sizeof(b_table[0])/sizeof(b_table[0][0]) - 1; // number of steps
    Scalar ks[4];
    fill(ks, ks + s, Scalar(0));

    for(int i = 0; i < s; i++){
        Scalar sum = 0;
        for(int j = 1; j <= s; j++){
            sum = sum + b_table[i][j] * ks[j - 1];
        }
        sum = h * sum;
        ks[i] = f(t0 + h * b_table[0][i], u0 + sum);
    }

    Scalar out = 0;
    for(int i = 1; i <= s; i++){
        out = out + b_table[s][i] * ks[i - 1];
    }
    out = out + u0;
	return out;
}

template <typename Scalar>
std::vector<std::vector<Scalar>> GRKSpan(Scalar b_table[5][5], Scalar (*f)(const Scalar&, const Scalar&), Scalar u0, Scalar tspan[2], int n) {
	using namespace std;
    std::vector<std::vector<Scalar>> approximations(2, std::vector<Scalar> (n));
	int s = sizeof(b_table[0])/sizeof(b_table[0][0]) - 1; // number of steps
    Scalar h = (tspan[1] - tspan[0])/n;
    Scalar ui = u0;
    int row = 0;
    for (Scalar t = tspan[0]; t <= tspan[1]; t = t + h) {
        Scalar ks[4];
        fill(ks, ks + s, Scalar(0));

        for(int i = 0; i < s; i++){
            Scalar sum = 0;
            for(int j = 1; j <= s; j++){
                sum = sum + b_table[i][j] * ks[j - 1];
            }
            sum = h * sum;
            ks[i] = f(t + h * b_table[0][i], ui + sum);
        }

        Scalar out = 0;
        for(int i = 1; i <= s; i++){
            out = out + b_table[s][i] * ks[i - 1];
        }
        ui = ui + out;
    
        approximations[0][row] = t;
        approximations[1][row] = ui;
        row = row + 1;
    }
    return(approximations);
}