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
    // Working Precision
    constexpr unsigned wbits = 64;
    constexpr unsigned wes   = 11;

    // Low Precision
    constexpr unsigned lbits = 16;
    constexpr unsigned les   = 5;
    
    // High Precision (same as Working for posits - uses quire)
    constexpr unsigned hbits = 128;  
    constexpr unsigned hes   = 15;


    // Table Column Width
    constexpr unsigned COLWIDTH = 25;

    // Reporting Options
    constexpr bool print          = false;
    constexpr bool showCondest    = false;
    constexpr bool showCond       = true;
    constexpr bool showAmax       = true;
    constexpr bool showSize       = false;
    constexpr bool showAlgo       = true;
    constexpr bool showNumProps   = true;

    // Squeeze Selection 0, 21, 22, 24
    // 0  No rounding
    // 21 Round then replace infinities
    // 22 Scale, then Round
    // 24 Two-sided Equilibration
    // size_t algo = 24; // See Higham 2019 Squeeze