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
    constexpr unsigned wes   = 2;

    // Low Precision
    constexpr unsigned lbits = 8;
    constexpr unsigned les   = 2;
    
    // High Precision (same as Working for posits - uses quire)
    constexpr unsigned hbits = 64;  
    constexpr unsigned hes   = 2;


    // Table Column Width
    constexpr unsigned COLWIDTH = 25;