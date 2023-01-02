/** **********************************************************************
 * Configurations for multi-precision iterative refinement experiements
 *
 * @author:     James Quinlan
 * @date:       2022-12-13
 * @copyright:  Copyright (c) 2022 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license 
 * ***********************************************************************
 */
#pragma once

    /** 
     * Numerical Precisions
     *  - adjust according to type (float/posit)
     */

    // Working Precision
    constexpr unsigned wbits = 64;
    constexpr unsigned wes   = 2;

    // Low Precision
    constexpr unsigned lbits = 16;
    constexpr unsigned les   = 2;
    
    // High Precision (same as Working for posits - uses quire)
    constexpr unsigned hbits = 64;  
    constexpr unsigned hes   = 2;

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
    bool randsol = true;

    /** 
     * Reporting Options
     *  switchyard for displaying/printing 
     *  results to screen or file
     */

    constexpr bool print          = false;
    constexpr bool showCondest    = false; // bug if true in condest.hpp.
    constexpr bool showCond       = true;
    constexpr bool showAmax       = true;
    constexpr bool showSize       = true;
    constexpr bool showSol        = false;
    constexpr bool showAlgo       = false;
    constexpr bool showNumProps   = true;
    constexpr bool showProcesses  = false;

    /** 
     * Table Column Width
     *  - format table width for results
     */
    constexpr unsigned COLWIDTH = 25;