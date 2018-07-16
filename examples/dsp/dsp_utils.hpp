// dsp_utils.cpp: functions to aid in inspecting DSP data structures
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

template<typename Ty>
std::ostream& DisplaySample(std::ostream& ostr, const Ty& value, const Ty& min, const Ty& max) {
	// min is 0 stars
	// 0   is 40 stars
	// max is 80 stars
	int maxStars = 80;
	float sample = float(value);
	float range = float(max - min);
	float midPoint = range/2.0f;
	float portion = (sample + midPoint)/range;
	int ub = int(maxStars * portion);
	for (int i = 0; i < ub; i++) {
		ostr << "*";
	}
	return ostr << std::endl;
}

template<typename Ty>
void DisplaySignal(std::ostream& ostr, const std::vector<Ty>& samples) {
	Ty min = minValue(samples);
	Ty max = maxValue(samples);
	// create a horizontal display
	int cnt = 0;
	ostr << std::fixed << std::setprecision(3);
	for (typename std::vector<Ty>::const_iterator it = samples.begin(); it != samples.end(); it++) {
		ostr << std::setw(3) << cnt++ << " " << std::setw(6) << float(*it) << " ";
		DisplaySample(ostr, *it, min, max);
	}
	ostr << std::setprecision(5);
}