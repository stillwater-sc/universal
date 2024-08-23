#pragma once
// occurrence.hpp: utility object to track arithmetic operation counts during execution of a specific number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <vector>

namespace sw { namespace universal {

struct scaleTracker {
public:
	// smallest and biggest scale must be strictly ordered with operator < (== less then) 
	scaleTracker(int smallestScale, int biggestScale) : scales(1ull + biggestScale - smallestScale), minScale(smallestScale), maxScale(biggestScale), underflows(0), overflows(0)
	{
	}

	// clear the occurrence counts, but keep the configuration of the scale tracker 
	void clear() {
		scales.clear();
	}

	void incr(int scale) {
		if (scale < minScale) {
			++underflows;
		}
		else {
			if (scale > maxScale) {
				++overflows;
			}
			else {
				size_t index = static_cast<int64_t>(scale) - static_cast<int64_t>(minScale);
				scales[index] += 1;
			}
		}
	}

	void report(std::ostream& ostr) {
		int i = minScale;
		for (auto f : scales) {
			ostr << std::setw(4) << i++ << " : " << f << '\n';
		}
		ostr << "underflows : " << underflows << '\n';
		ostr << "overflows  : " << overflows << '\n';
	}

private:
	std::vector<size_t> scales;
	int minScale;
	int maxScale;
	size_t underflows;
	size_t overflows;
};

}} // namespace sw::universal
