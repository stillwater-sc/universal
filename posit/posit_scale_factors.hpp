#pragma once

#include <cstdint>

constexpr unsigned int MAX_ES = 5;
constexpr unsigned int MAX_K = 10;

/*
LOOKUP table for different useed^k values
useed = 2^2^es
The table is just positive k's as columns, and es as rows
         k = 0  k = 1  k = 2 ...
es = 0   
es = 1          useed^k
es = 2
...

*/
constexpr uint64_t SCALE_FACTORS[MAX_ES][MAX_K] = {
/*               k = 0         k = 1            k = 2         k = 3          k = 4            k = 5              k = 6         k = 7         k = 8          k = 9  */
	/* es = 0 */ {   1,            4,               8,           16,            32,              64,               128,          256,          512,          1024 },
	/* es = 1 */ {   1,           16,              64,          256,          1024,            4096,             16384,        65536,       262144,       1048576 },
	/* es = 2 */ {   1,          256,            4096,        65536,       1048576,        16777216,         268435456,   4294967296,  68719476736, 1099511627776 },
	/* es = 3 */ {   1,        65536,        16777216,   4294967296, 1099511627776, 281474976710656, 72057594037927936,            0,            0,             0 },
	/* es = 4 */ {   1,   4294967296, 281474976710656,            0,             0,               0,                 0,            0,            0,             0 },
};

