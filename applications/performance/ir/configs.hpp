/** **********************************************************************
 * Configurations for multi-precision iterative refinement experiements
 *      - Global Configurations (Constants used in multiple files)
 * 
 * @author:     James Quinlan
 * @date:       2024-03-17
 * @copyright:  Copyright (c) 2022 James Quinlan
 * @license:    MIT Open Source license 
 * ***********************************************************************
 */
#pragma once

    /** 
     * Numerical Precisions
     *  - adjust according to type (float/posit)
     */

    // Low Precision
    constexpr unsigned lbits = 16;
    constexpr unsigned les   = 5;

    // Working Precision
    constexpr unsigned wbits = 32;
    constexpr unsigned wes   = 8;
    
    // High Precision (same as Working for posits - uses quire)
    // Not same works better than quire alone
    constexpr unsigned hbits = 64;  
    constexpr unsigned hes   = 11;

    /**
     * Floats or Posits ?
     * - Used during squeezing preconditioning 
    */
    bool usePosits = true; 
    // currently not used - intention is to use as global var
    // for both luir and squeeze.hpp (at least squeeze)


    /**
     * Squeeze Selection 0, 21, 22, 24
     * 0  No rounding
     * 21 Round then replace infinities
     * 22 Scale, then Round
     * 24 Two-sided Equilibration
     * size_t algo = 24; // See Higham 2019 Squeeze
    */

    /**
     * Solution Vector = [1, 1,...,1]' or RANDOM 
    */
    bool randsol = false; // true = random solution vector

    /** 
     * Reporting Options
     *  switchyard for displaying/printing 
     *  results to screen or file.
     */

    constexpr bool write2file     = false;
    constexpr bool printMat       = false;
    constexpr bool printLU        = false;
    constexpr bool printPA        = false;
    constexpr bool printPerm      = false;
    constexpr bool showCondest    = false; // bug if true in condest.hpp.
    constexpr bool showCond       = true;
    constexpr bool showAmax       = true;  // Maximum element of A
    constexpr bool showSize       = true;  // Size of A
    constexpr bool showSol        = false; // Solution vector
    constexpr bool showAlgo       = true;  // Squeeze Algorithm
    constexpr bool showNumProps   = true;  // Numeric Properties
    constexpr bool showProcesses  = false; // Show the processes  
    constexpr bool showTolerance  = true; 
    constexpr bool showScalingMat = false;

    /** 
     * Table Column Width
     *  - format table width for results
     */
    constexpr unsigned COLWIDTH = 25;


    /** 
     * Maximum Iterations
     */
    constexpr unsigned MAXIT = 10;