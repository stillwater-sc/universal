#pragma once
// pop.hpp: umbrella header for POP (Precision-Optimized Programs) precision tuning
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// POP is a static analysis method for bit-level precision tuning. Given accuracy
// requirements on program outputs, it determines the minimum bits needed at each
// variable/intermediate using forward and backward error transfer functions,
// solved as a Linear Program (LP).
//
// Usage:
//   #include <universal/mixedprecision/pop.hpp>
//
//   using namespace sw::universal;
//
//   // Step 1: Build expression graph
//   ExprGraph g;
//   int a = g.variable("a", 1.0, 10.0);
//   int b = g.variable("b", 1.0, 10.0);
//   int c = g.mul(a, b);
//   g.require_nsb(c, 16);  // require 16 significant bits at output
//
//   // Step 2a: Iterative fixpoint analysis (no LP solver)
//   g.analyze();
//
//   // Step 2b: OR use LP solver for optimal assignment
//   PopSolver solver;
//   solver.solve(g);
//
//   // Step 3: Optional carry refinement (10-30% reduction)
//   CarryAnalyzer ca;
//   ca.refine(g);
//
//   // Step 4: Generate code
//   PopCodeGenerator gen(g);
//   std::cout << gen.generateHeader();
//
//   // Step 5: Get type recommendations
//   TypeAdvisor advisor;
//   g.report(std::cout, advisor);
//
// Reference: Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning,"
//            PhD thesis, Universite de Perpignan, 2021.

// Phase 1: Transfer functions and UFP
#include <universal/mixedprecision/ufp.hpp>
#include <universal/mixedprecision/transfer.hpp>

// Phase 2: Expression graph and iterative fixpoint analysis
#include <universal/mixedprecision/expression_graph.hpp>

// Phase 3: LP solver and optimal bit assignment
#include <universal/mixedprecision/simplex.hpp>
#include <universal/mixedprecision/pop_solver.hpp>

// Phase 4: Carry-bit refinement
#include <universal/mixedprecision/carry_analysis.hpp>

// Phase 5: Code generation
#include <universal/mixedprecision/codegen.hpp>
