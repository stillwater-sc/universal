#pragma once
// sorn_impl.hpp: implementation of SORN arithmetic number format, based on unum type-II format
//
// Copyright (C) 2022 ITEM, University of Bremen.
//
// This file is aimed to provide an addition and to be included in the universal numbers project.
#include <cassert>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include <bitset>

#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal {

// struct sornInterval: a struct defining a SORN interval with two interval bound values and open/closed conditions.
template<typename Real> 
struct sornInterval {
	Real lowerBound;
	Real upperBound;
	bool lowerIsOpen;
	bool upperIsOpen;

	std::string getInt() {
		std::stringstream configStream;
		if ((this->lowerBound == this->upperBound) && (not this->lowerIsOpen && not this->upperIsOpen)) {
			configStream << this->lowerBound;
		}
		else {
			configStream << (this->lowerIsOpen ? '(' : '[') << this->lowerBound << ',' << this->upperBound << (this->upperIsOpen ? ')' : ']');
		}
		return configStream.str();
	}

	bool isZero() const noexcept {
		if (this->lowerBound == 0 && this->upperBound == 0 && not this->lowerIsOpen && not this->upperIsOpen) {
			return true;
		} 
		else {
			return false;
		}
	}

};

// setSornDT: function to create a SORN datatype
template<typename Real>
inline std::vector<sornInterval<Real>> setSornDT(signed int start, signed int stop, unsigned int steps, float stepSize,
									bool flagNeg, bool flagInf, bool flagZero, bool flagLin,
									bool flagLog, bool flagHalfopen, bool flagOpen, size_t sornBits ) {
	using SORN_INTERVAL = sornInterval<Real>;
	using std::pow;
	using std::fmax;

	std::vector<SORN_INTERVAL> sornDT;
	// 1. halfopen config
	if (flagHalfopen) {
		// 1.1. zero
		if (flagZero) {
			SORN_INTERVAL structInt;
			structInt.lowerBound = 0;
			structInt.upperBound = 0;
			structInt.lowerIsOpen = false;
			structInt.upperIsOpen = false;
			sornDT.push_back(structInt);
		}
		// 1.2. positive part
		if (flagLin) {
			// 1.2.1 linear config
			assert(start == 0 && "%% ERROR %% Start value has to be set to 0 for linear halfopen configuration.");
			for (int b = 0; b < (int)steps; b++) {
				SORN_INTERVAL structInt;
				structInt.lowerBound = b * stepSize;
				structInt.upperBound = (b + 1) * stepSize;
				structInt.lowerIsOpen = (b == 0 && not flagZero ? false : true);
				structInt.upperIsOpen = false;
				sornDT.push_back(structInt);
			}
		}
		else if (flagLog) {
			// 1.2.2 logarithmic config (Note: "steps" value is ignored for logarithmic configuration)
			for (int b = start; b < (stop + 1); b++) {
				SORN_INTERVAL structInt;
				structInt.lowerBound = (b == start ? 0 : (float)pow(2, (b - 1)));
				structInt.upperBound = (float)pow(2, b);
				structInt.lowerIsOpen = (b == 0 && not flagZero ? false : true);
				structInt.upperIsOpen = false;
				sornDT.push_back(structInt);
			}
		}
		// 1.3. infinity
		if (flagInf) {
			SORN_INTERVAL structInt;
			structInt.lowerBound = sornDT[sornDT.size() - 1].upperBound;
			structInt.upperBound = std::numeric_limits<float>::infinity();
			structInt.lowerIsOpen = true;
			structInt.upperIsOpen = false;
			sornDT.push_back(structInt);
		}
		// 1.4. negative intervals
		if (flagNeg) {
			std::vector<SORN_INTERVAL> negDT;
			for (size_t b = (flagZero ? 1 : 0); b < sornDT.size(); b++) {
				SORN_INTERVAL structInt;
				structInt.lowerBound = -sornDT[b].upperBound;
				structInt.upperBound = (sornDT[b].lowerBound == 0 ? sornDT[b].lowerBound : -sornDT[b].lowerBound);
				structInt.lowerIsOpen = false;
				structInt.upperIsOpen = true;
				negDT.insert(negDT.begin(), structInt);
			}
			sornDT.insert(sornDT.begin(), negDT.begin(), negDT.end());
		}
	}
	// 2. open config (TODO)
	else if (flagOpen) {
		assert(not flagOpen && "%% WARNING %% Open interval datatypes not implemented.");
	}
	// 3. check config
	assert(sornDT.size() == sornBits && "Something is wrong with the Datatype size. Check sornDT and sornBits.");
	// 4. return DT
	return sornDT;
}

// float2SORN: a function converting a single float input to a SORN interval
template<typename Real>
sornInterval<Real> float2SORN(float operand, std::vector<sornInterval<Real>> sornDT, sornInterval<Real>& sornIntVal) {
	for (size_t b = 0; b < sornDT.size(); b++) {
		if (	( (operand > sornDT[b].lowerBound) || (operand == sornDT[b].lowerBound && not sornDT[b].lowerIsOpen) ) &&
				( (operand < sornDT[b].upperBound) || (operand == sornDT[b].upperBound && not sornDT[b].upperIsOpen) )		) {
			sornIntVal = sornDT[b];
		}
	}
	return sornIntVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// class sorn: a class for defining a SORN format:	sorn<start,stop,steps,lin,halfopen,neg,inf,zero>					 
// 
// -- Mandatory configuration parameters:	
//		start:	lowest value in the SORN lattice // for "lin" start=0 // for "log" -inf<start<inf, lattice begins with 2^start 
//		stop:	highest non-infinity value in the SORN lattice // for "lin" start<stop // for "log" start<stop, lattice ends with 2^stop
//		steps:	number of intervals/steps within the SORN representation between "start" and "stop" for "lin" (positive part),
//				not required for "log" distribution (any positve value allowed)
// 
// -- Optional configuration parameters: (all "true" by default)
//		lin:		set the SORN interval distribution to "linear" (true) or "logarithmic" (false)
//		halfopen:	set the SORN interval distribution to "halfopen bounds, no exact values" (true) or "open bounds,
//					intermediate exact values" (false)
//		neg:		include negative values/intervals in the SORN datatype, symmetric to positive part
//		inf:		inlcude infinity value/interval bounds to the SORN datatype
//		zero:		inlcude the exact zero value in the SORN datatype
//////////////////////////////////////////////////////////////////////////////////////////////////
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg=1, bool _inf=1, bool _zero=1>
class sorn {
public:
	using Real = float;
	using SORN_INTERVAL = sornInterval<Real>;
	typedef Real value_type;

private:

	// input configuration parameters
	static constexpr signed int start	= _start;	// lowest non-zero value in the SORN lattice 
	static constexpr signed int stop	= _stop;	// highest non-infinity value in the SORN lattice 
	static constexpr unsigned int steps	= _steps;	// number of intervals/steps within the SORN representation between start and stop (only for positive part, only for linear distribution)
	static constexpr float stepSize = (float)(stop - start) / (float)steps;

	// input configuration flags
	static constexpr bool flagNeg		= _neg;			// include negative values/intervals in the SORN datatype, symmetric to positive part	(default: true)
	static constexpr bool flagInf		= _inf;			// inlcude infinity bounds to the SORN datatype											(default: true)
	static constexpr bool flagZero		= _zero;		// inlcude the exact zero value in the SORN datatype									(default: true)
	static constexpr bool flagLin		= _lin;			// set the SORN interval distribution to "linear"										(default: true)
	static constexpr bool flagLog		= not _lin;		// set the SORN interval distribution to "logarithmic"									(default: false)
	static constexpr bool flagHalfopen	= _halfopen;	// set the SORN interval distribution to halfopen without exacts						(default: true)
	static constexpr bool flagOpen		= not _halfopen;// set the SORN interval distribution to open with intermediate exacts					(default: false) 

public:

	// SORN value with NaN initialisation
	SORN_INTERVAL sornIntVal = { std::numeric_limits<Real>::quiet_NaN() , std::numeric_limits<Real>::quiet_NaN() , false , false };

	// SORN bitwidth (redundant with sornDT.size())
	static constexpr size_t sornBits =	( 
											( (flagLin ? steps : stop - start + 1) + (flagInf ? 1 : 0) ) *		// determine number of intervals (either halfopen or open)
											(flagOpen ? 2 : 1)													// double if open intervals with intermediate exact values are used
										) *
										(flagNeg ? 2 : 1) +														// double if negative values/intervals are included
										(flagZero ? 1 : 0) -													// consider the exact zero value
										((flagOpen & flagInf & flagNeg) ? 1 : 0);									// for open intervals only one value for +-inf is used
	static constexpr size_t nbits = sornBits;

	// SORN datatype
	std::vector<SORN_INTERVAL> sornDT = setSornDT<Real>(start, stop, steps, stepSize, flagNeg, flagInf, flagZero, flagLin, flagLog, flagHalfopen, flagOpen, sornBits);

	// constructors
	sorn() {}

	// specific value constructor
	constexpr sorn(const SpecificValue code) noexcept {
		switch (code) {
		case SpecificValue::maxpos:
			//maxpos();
			break;
		case SpecificValue::minpos:
			//minpos();
			break;
		case SpecificValue::zero:
		default:
			//zero();
			break;
		case SpecificValue::minneg:
			//minneg();
			break;
		case SpecificValue::maxneg:
			//maxneg();
			break;
		case SpecificValue::infpos:
			//setinf(false);
			break;
		case SpecificValue::infneg:
			//setinf(true);
			break;
		case SpecificValue::nar: // approximation as IEEE-754 SORNs don't have a NaR
		case SpecificValue::qnan:
			//setnan(NAN_TYPE_QUIET);
			break;
		case SpecificValue::snan:
			//setnan(NAN_TYPE_SIGNALLING);
			break;
		}
	}

	// converting constructors
	sorn(signed char initial_value)			{ *this = initial_value; }
	sorn(short initial_value)				{ *this = initial_value; }
	sorn(int initial_value)					{ *this = initial_value; }
	sorn(long initial_value)				{ *this = initial_value; }
	sorn(long long initial_value)			{ *this = initial_value; }
	sorn(char initial_value)				{ *this = initial_value; }
	sorn(unsigned short initial_value)		{ *this = initial_value; }
	sorn(unsigned int initial_value)		{ *this = initial_value; }
	sorn(unsigned long initial_value)		{ *this = initial_value; }
	sorn(unsigned long long initial_value)	{ *this = initial_value; }
	sorn(float initial_value)				{ *this = initial_value; }
	sorn(double initial_value)				{ *this = initial_value; }
	sorn(long double initial_value)			{ *this = initial_value; }

	// assignment operators for native types
	sorn operator=(signed char rhs)			{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(short rhs)				{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(int rhs)					{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(long rhs)				{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(long long rhs)			{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(char rhs)				{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(unsigned short rhs)		{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(unsigned int rhs)		{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(unsigned long rhs)		{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(unsigned long long rhs)	{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(float rhs)				{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(double rhs)				{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }
	sorn operator=(long double rhs)			{ float2SORN( (float)rhs, sornDT, this->sornIntVal); return *this; }

	///////////////////////////////
	////////// operators //////////
	///////////////////////////////

	// write to output
	friend std::ostream& operator<< (std::ostream& ostr, sorn& s) {
		return ostr << s.sornIntVal.getInt();
	}

	// arithmetics	(TODO: div, comparison?)

	// negation operator
	sorn operator-() {
		if (iszero()) {
			return *this;
		}
		else {
			sorn negated;
			negated.sornIntVal.lowerBound = (this->sornIntVal.upperBound == 0 ? this->sornIntVal.upperBound : -this->sornIntVal.upperBound);
			negated.sornIntVal.upperBound = (this->sornIntVal.lowerBound == 0 ? this->sornIntVal.lowerBound : -this->sornIntVal.lowerBound);
			negated.sornIntVal.lowerIsOpen = this->sornIntVal.upperIsOpen;
			negated.sornIntVal.upperIsOpen = this->sornIntVal.lowerIsOpen;
			return negated;
		}
	}

	// single operand addition 
	
	// sorn + sorn
	sorn& operator+=(sorn& rhs) {
		if (rhs.iszero()) {
			return *this;
		}
		else if (this->iszero()) {
			*this = rhs;
			return *this;
		}
		else {
			this->sornIntVal.lowerBound += rhs.sornIntVal.lowerBound;
			this->sornIntVal.upperBound += rhs.sornIntVal.upperBound;
			this->sornIntVal.lowerIsOpen = (this->sornIntVal.lowerIsOpen || rhs.sornIntVal.lowerIsOpen);
			this->sornIntVal.upperIsOpen = (this->sornIntVal.upperIsOpen || rhs.sornIntVal.upperIsOpen);
			adaptToDT();
			return *this;
		}
	}
	// sorn + int
	sorn& operator+=(int rhs) {
		this->sornIntVal.lowerBound += rhs;
		this->sornIntVal.upperBound += rhs;
		adaptToDT();
		return *this;
	}
	// sorn + float
	sorn& operator+=(float rhs) {
		this->sornIntVal.lowerBound += rhs;
		this->sornIntVal.upperBound += rhs;
		adaptToDT();
		return *this;
	}
	// sorn + double
	sorn& operator+=(double rhs) {
		this->sornIntVal.lowerBound += (float)rhs;
		this->sornIntVal.upperBound += (float)rhs;
		adaptToDT();
		return *this;
	}

	// single operand subtraction

	// sorn - sorn
	sorn& operator-=(sorn& rhs) {
		if (rhs.iszero()) {
			return *this;
		}
		else if (this->iszero()) {
			*this = -rhs;
			return *this;
		}
		else {
			this->sornIntVal.lowerBound -= rhs.sornIntVal.upperBound;
			this->sornIntVal.upperBound -= rhs.sornIntVal.lowerBound;
			this->sornIntVal.lowerIsOpen = (this->sornIntVal.lowerIsOpen || rhs.sornIntVal.upperIsOpen);
			this->sornIntVal.upperIsOpen = (this->sornIntVal.upperIsOpen || rhs.sornIntVal.lowerIsOpen);
			adaptToDT();
			return *this;
		}
	}
	// sorn - int
	sorn& operator-=(int rhs) {
		this->sornIntVal.lowerBound -= rhs;
		this->sornIntVal.upperBound -= rhs;
		adaptToDT();
		return *this;
	}
	// sorn - float
	sorn& operator-=(float rhs) {
		this->sornIntVal.lowerBound -= rhs;
		this->sornIntVal.upperBound -= rhs;
		adaptToDT();
		return *this;
	}
	// sorn - double
	sorn& operator-=(double rhs) {
		this->sornIntVal.lowerBound -= (float)rhs;
		this->sornIntVal.upperBound -= (float)rhs;
		adaptToDT();
		return *this;
	}

	// single operand multiplication

	// sorn * sorn
	sorn& operator*=(sorn& rhs) {
		// 1. create references and intermediate values for the bounds
		float &lhsLOW	= this->sornIntVal.lowerBound;
		float &lhsUP	= this->sornIntVal.upperBound;
		float &rhsLOW	= rhs.sornIntVal.lowerBound;
		float &rhsUP	= rhs.sornIntVal.upperBound;
		float resLOW;
		float resUP;
		// 2. create references and intermediate values for the conditions
		bool &lhsLOWc	= this->sornIntVal.lowerIsOpen;
		bool &lhsUPc	= this->sornIntVal.upperIsOpen;
		bool &rhsLOWc	= rhs.sornIntVal.lowerIsOpen;
		bool &rhsUPc	= rhs.sornIntVal.upperIsOpen;
		bool resLOWc;
		bool resUPc;
		// 3. check zero case
		if (rhs.iszero() || this->iszero()) {
			setzero();
			return *this;
		}
		else { // 4. follow interval arithmetic rules for multiplication
			if (lhsLOW >= 0) { //					  lower bound					upper bound					lower condition					upper condition
				if		(rhsLOW >= 0)				{ resLOW = lhsLOW * rhsLOW;		resUP = lhsUP  * rhsUP;		resLOWc = lhsLOWc || rhsLOWc;	resUPc = lhsUPc  || rhsUPc; }
				else if (rhsLOW < 0 && rhsUP >= 0)	{ resLOW = lhsUP  * rhsLOW;		resUP = lhsUP  * rhsUP;		resLOWc = lhsUPc  || rhsLOWc;	resUPc = lhsUPc  || rhsUPc; }
				else if (rhsUP < 0)					{ resLOW = lhsUP  * rhsLOW;		resUP = lhsLOW * rhsUP;		resLOWc = lhsUPc  || rhsLOWc;	resUPc = lhsLOWc || rhsUPc; }
				else                                { resLOW = 0              ;		resUP = 0             ;		resLOWc = 0                 ;	resUPc = 0                ; } // ETLO added
			}
			else if (lhsLOW < 0 && lhsUP >= 0) {
				if		(rhsLOW >= 0)				{ resLOW = lhsLOW * rhsUP;		resUP = lhsUP  * rhsUP;		resLOWc = lhsLOWc || rhsUPc;	resUPc = lhsUPc  || rhsUPc; }
				else if (rhsLOW < 0 && rhsUP >= 0)	{ resLOW = fmin(lhsLOW * rhsUP  , lhsUP * rhsLOW);			resLOWc = (lhsLOW * rhsUP  < lhsUP*rhsLOW ? lhsLOWc || rhsUPc  : lhsUPc || rhsLOWc);	// lower bound & condition
													  resUP  = fmax(lhsLOW * rhsLOW , lhsUP * rhsUP );			resUPc  = (lhsLOW * rhsLOW > lhsUP*rhsUP  ? lhsLOWc || rhsLOWc : lhsUPc || rhsUPc ); }	// upper bound & condition
				else if (rhsUP < 0)					{ resLOW = lhsUP  * rhsLOW;		resUP = lhsLOW * rhsLOW;	resLOWc = lhsUPc  || rhsLOWc;	resUPc = lhsLOWc || rhsLOWc;}
				else                                { resLOW = 0              ;		resUP = 0             ;		resLOWc = 0                 ;	resUPc = 0                ; } // ETLO added 
			}
			else if (lhsUP < 0) {
				if		(rhsLOW >= 0)				{ resLOW = lhsLOW * rhsUP;		resUP = lhsUP  * rhsLOW;	resLOWc = lhsLOWc || rhsUPc;	resUPc = lhsUPc  || rhsLOWc;}
				else if (rhsLOW < 0 && rhsUP >= 0)	{ resLOW = lhsLOW * rhsUP;		resUP = lhsLOW * rhsLOW;	resLOWc = lhsLOWc || rhsUPc;	resUPc = lhsLOWc || rhsLOWc;}
				else if (rhsUP < 0)					{ resLOW = lhsUP  * rhsUP;		resUP = lhsLOW * rhsLOW;	resLOWc = lhsUPc  || rhsUPc;	resUPc = lhsLOWc || rhsLOWc;}
				else                                { resLOW = 0              ;		resUP = 0             ;		resLOWc = 0                 ;	resUPc = 0                ; } // ETLO added
			}
			else {
				{ resLOW = 0              ;		resUP = 0             ;		resLOWc = 0                 ;	resUPc = 0                ; } // ETLO added
			}
			// 5. set results
			if (resLOW == -0) { resLOW = 0; } // remove -0
			if (resUP  == -0) { resUP  = 0; } // remove -0
			this->sornIntVal.lowerBound	 = resLOW;
			this->sornIntVal.upperBound	 = resUP;
			this->sornIntVal.lowerIsOpen = resLOWc;
			this->sornIntVal.upperIsOpen = resUPc;
			// 6. return
			adaptToDT();
			return *this;
		}
	}
	// sorn * int
	sorn& operator*=(int rhs) {
		this->sornIntVal.lowerBound *= rhs;
		this->sornIntVal.upperBound *= rhs;
		if (rhs < 0) { switchBounds(); }
		adaptToDT();
		return *this;
	}
	// sorn * float
	sorn& operator*=(float rhs) {
		this->sornIntVal.lowerBound *= rhs;
		this->sornIntVal.upperBound *= rhs;
		if (rhs < 0) { switchBounds(); }
		adaptToDT();
		return *this;
	}
	// sorn * double
	sorn& operator*=(double rhs) {
		this->sornIntVal.lowerBound *= (float)rhs;
		this->sornIntVal.upperBound *= (float)rhs;
		if (rhs < 0) { switchBounds(); }
		adaptToDT();
		return *this;
	}

	//////////////////////////////////////////
	////////// arithmetic functions //////////
	//////////////////////////////////////////

	// absolute value
	sorn abs() {
		sorn absVal;
		if (this->sornIntVal.upperBound <= 0) {
			absVal = -*this;
		}
		else if (this->sornIntVal.lowerBound < 0) {
			absVal.sornIntVal.lowerBound = 0;
			absVal.sornIntVal.lowerIsOpen = false;
			if (std::abs(this->sornIntVal.lowerBound) > std::abs(this->sornIntVal.upperBound)) {
				absVal.sornIntVal.upperBound = std::abs(this->sornIntVal.lowerBound);
				absVal.sornIntVal.upperIsOpen = this->sornIntVal.lowerIsOpen;
			}
			else {
				absVal.sornIntVal.upperBound = this->sornIntVal.upperBound;
				absVal.sornIntVal.upperIsOpen = this->sornIntVal.upperIsOpen;
			}
		}
		else {
			absVal = *this;
		};
		absVal.adaptToDT();
		return absVal;
	}

	template<typename Real>
	Real to_native() const noexcept {
		return Real(0.0f);
	}
	// make conversions to native types explicit
	//explicit operator int()       const noexcept { return to_int(); }
	//explicit operator long()      const noexcept { return to_long(); }
	//explicit operator long long() const noexcept { return to_long_long(); }
	explicit operator float()     const noexcept { return to_native<float>(); }
	explicit operator double()    const noexcept { return to_native<double>(); }

	///////////////////////////////////////
	////////// helper functions //////////
	///////////////////////////////////////

	// special value functions
	bool iszero() const noexcept { return this->sornIntVal.isZero(); }
	bool isnan() const noexcept { return false; }

	sorn& setzero() { this->sornIntVal = { 0,0,0,0 }; return *this; }

	Real minVal() const noexcept { return sornDT[0].lowerBound; }
	Real maxVal() const noexcept { return sornDT[sornDT.size() - 1].upperBound; }

	// adaptToDT: adapt the variable sornIntVal to the defined datatype
	SORN_INTERVAL& adaptToDT() {
		// 1. lower bound
		bool lowerExists = false;
		// 1.1. check if lower bound already exisits in DT
		for (size_t b = 0; b < sornDT.size(); b++) {
			if (this->sornIntVal.lowerBound == sornDT[b].lowerBound && this->sornIntVal.lowerIsOpen == sornDT[b].lowerIsOpen) {
				lowerExists = true;
			}
		}
		// 1.2. adapt lower bound to DT
		if (not lowerExists) {
			// 1.2.1. lower bound < min value
			if (this->sornIntVal.lowerBound < minVal()) {
				this->sornIntVal.lowerBound = minVal();
				this->sornIntVal.lowerIsOpen = this->sornDT[0].lowerIsOpen;
			}
			else {
				for (size_t b = 0; b < sornDT.size(); b++) {
					// 1.2.2. lower bound value between two possible values (from DT)
					if (this->sornIntVal.lowerBound > sornDT[b].lowerBound && this->sornIntVal.lowerBound < sornDT[b].upperBound) {
						this->sornIntVal.lowerBound = sornDT[b].lowerBound;
						this->sornIntVal.lowerIsOpen = sornDT[b].lowerIsOpen;
					} // 1.2.3. lower bound in DT but open/close condition does not match
					else if (this->sornIntVal.lowerBound == sornDT[b].lowerBound && this->sornIntVal.lowerIsOpen != sornDT[b].lowerIsOpen) {
						// cond. is "open" but has to be "closed"
						if (this->sornIntVal.lowerIsOpen) {
							this->sornIntVal.lowerBound = sornDT[b].lowerBound;
							this->sornIntVal.lowerIsOpen = sornDT[b].lowerIsOpen;
						}
						// cond. is "closed" but has to be "open" --> utilize prevoius interval
						else if (not this->sornIntVal.lowerIsOpen) {
							this->sornIntVal.lowerBound = sornDT[b - 1].lowerBound;
							this->sornIntVal.lowerIsOpen = sornDT[b - 1].lowerIsOpen;
						}
					}
				}
			}
		}
		// 2. upper bound
		bool upperExists = false;
		// 2.1.check if upper bound already exisits in DT
		for (size_t b = 0; b < sornDT.size(); b++) {
			if (this->sornIntVal.upperBound == sornDT[b].upperBound && this->sornIntVal.upperIsOpen == sornDT[b].upperIsOpen) {
				upperExists = true;
			}
		}
		// 2.2. adapt upper bound to DT
		if (not upperExists) {
			// 2.2.1 upper bound > max value
			if (this->sornIntVal.upperBound > maxVal()) {
				this->sornIntVal.upperBound = maxVal();
				this->sornIntVal.upperIsOpen = this->sornDT[sornDT.size() - 1].upperIsOpen;
			}
			else {
				for (size_t b = 0; b < sornDT.size(); b++) {
					// 2.2.2. upper bound value between two possible values (from DT)
					if (this->sornIntVal.upperBound > sornDT[b].lowerBound && this->sornIntVal.upperBound < sornDT[b].upperBound) {
						this->sornIntVal.upperBound = sornDT[b].upperBound;
						this->sornIntVal.upperIsOpen = sornDT[b].upperIsOpen;
						break; // when the upper bound it set to open zero the next iteration would set it to closed zero
					} // 2.2.3. upper bound in DT but open/close condition does not match
					else if (this->sornIntVal.upperBound == sornDT[b].upperBound && this->sornIntVal.upperIsOpen != sornDT[b].upperIsOpen) {
						// cond. is "open" but has to be "closed"
						if (this->sornIntVal.upperIsOpen) {
							this->sornIntVal.upperBound = sornDT[b].upperBound;
							this->sornIntVal.upperIsOpen = sornDT[b].upperIsOpen;
						} // cond. is "closed" but has to be "open" --> utilize next interval
						else if (not this->sornIntVal.upperIsOpen) {
							this->sornIntVal.upperBound = sornDT[b+1].upperBound;
							this->sornIntVal.upperIsOpen = sornDT[b+1].upperIsOpen;
						}
						break;
					}
				}
			}
		}
		// 3. return value
		return this->sornIntVal;
	}

	// switchBounds: switch Boundaries of sornIntVal
	SORN_INTERVAL& switchBounds() {
		float newLow  = this->sornIntVal.upperBound;
		float newUp   = this->sornIntVal.lowerBound;
		bool  newLowC = this->sornIntVal.upperIsOpen;
		bool  newUpC  = this->sornIntVal.lowerIsOpen;
		this->sornIntVal = { newLow,newUp,newLowC,newUpC };
		return this->sornIntVal;
	}

	void setbits(std::uint64_t v) noexcept {}

	// setBits: set the sornIntVal parameter via binary input (input type: bitset)
	SORN_INTERVAL& setBits(std::bitset<sornBits> bin) {
		bool lowerSet = false;
		for (size_t b = 0; b < sornBits; b++) {
			if (bin[b] == 1 && not lowerSet) {
				this->sornIntVal = this->sornDT[b];
				lowerSet = true;
			}
			else if (bin[b] == 1 && lowerSet) {
				this->sornIntVal.upperBound = this->sornDT[b].upperBound;
				this->sornIntVal.upperIsOpen = this->sornDT[b].upperIsOpen;
			}
		}
		return this->sornIntVal;
	}

	//////////////////////////////////////
	////////// getter functions //////////
	//////////////////////////////////////

	// getConfig: writes all configuration parameters and flags to a string
	std::string getConfig() {
		std::stringstream configStream;
		configStream << "-- configuration parameters:" << '\t' << "start: " << start << ", stop: " << stop << ", steps: " << steps << ", stepSize: " << stepSize << '\n';
		configStream << "-- configuration flags:" << "\t\t";
		if (flagLin) configStream << "Lin, "; else if (flagLog) configStream << "Log, ";
		if (flagHalfopen) configStream << "Halfopen, "; else if (flagOpen) configStream << "Open, ";
		if (flagNeg) configStream << "Neg, ";
		if (flagInf) configStream << "Inf, ";
		if (flagZero) configStream << "Zero";
		configStream << '\n';
		return configStream.str();
	}

	// getDT: writes the SORN datatype configuration to a string
	std::string getDT() {
		std::stringstream DTstream;
		DTstream << "-- SORN datatype:" << "\t\t";
		for (size_t b = 0; b < sornDT.size(); b++) {
			DTstream << sornDT[b].getInt() << ' ';
		}
		DTstream << '\n';
		return DTstream.str();
	}

	// getBits: returns the binary representation of a SORN value using bitset class (note: displayed from max downto 0 when using << operator)
	std::bitset<sornBits> getBits() {
		std::bitset<sornBits> raw_bits;
		for (size_t b = 0; b < sornBits; b++) {
			if ((sornIntVal.lowerBound == sornDT[b].lowerBound && sornIntVal.lowerIsOpen == sornDT[b].lowerIsOpen) ||
				(sornIntVal.upperBound == sornDT[b].upperBound && sornIntVal.upperIsOpen == sornDT[b].upperIsOpen)) {
				raw_bits[b] = 0b1;
			}
			else {
				raw_bits[b] = 0b0;
			}
		}
		return raw_bits;
	}

private:
	template<signed int _sstart, signed int _sstop, unsigned int _ssteps, bool _llin, bool _hhalfopen, bool _nneg, bool _iinf, bool _zzero>
	friend bool operator!=(const sorn< _sstart, _sstop, _ssteps, _llin, _hhalfopen, _nneg, _iinf, _zzero>& lhs, const sorn< _sstart, _sstop, _ssteps, _llin, _hhalfopen, _nneg, _iinf, _zzero>&);

	template<signed int _sstart, signed int _sstop, unsigned int _ssteps, bool _llin, bool _hhalfopen, bool _nneg, bool _iinf, bool _zzero>
	friend std::ostream& operator<<(std::ostream&, const sorn< _sstart, _sstop, _ssteps, _llin, _hhalfopen, _nneg, _iinf, _zzero>&);

}; // end class sorn

//////////////////////////////////////////////////////////////////////////////////////////////////

template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
inline bool operator!=(const sorn< _start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, const sorn< _start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) { 
	return false; 
}

template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
inline std::ostream& operator<<(std::ostream& ostr, const sorn< _start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs) { 
	return ostr << "[ " << lhs.minVal() << ", " << lhs.maxVal() << "]";
}


//////////////////////////////////////////
////////// two operand addition //////////
//////////////////////////////////////////

// sorn + sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator+ (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs,
																		   sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> sum = lhs;
	return sum += rhs;
}
// sorn + int
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator+ (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, int rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> sum = lhs;
	return sum += rhs;
}
// sorn + float
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator+ (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, float rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> sum = lhs;
	return sum += rhs;
}
// sorn + double
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator+ (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, double rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> sum = lhs;
	return sum += rhs;
}
// int + sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator+ (int lhs, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> sum = rhs;
	return sum += lhs;
}
// float + sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator+ (float lhs, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> sum = rhs;
	return sum += lhs;
}
// double + sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator+ (double lhs, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> sum = rhs;
	return sum += lhs;
}

/////////////////////////////////////////////
////////// two operand subtraction //////////
/////////////////////////////////////////////

// sorn - sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator- (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs,
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> dif = lhs;
	return dif -= rhs;
}
// sorn - int
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator- (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, int rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> dif = lhs;
	return dif -= rhs;
}
// sorn - float
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator- (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, float rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> dif = lhs;
	return dif -= rhs;
}
// sorn - double
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator- (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, double rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> dif = lhs;
	return dif -= rhs;
}
// int - sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator- (int lhs, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> dif = -rhs;
	return dif += lhs;
}
// float - sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator- (float lhs, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> dif = -rhs;
	return dif += lhs;
}
// double - sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator- (double lhs, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> dif = -rhs;
	return dif += lhs;
}

////////////////////////////////////////////////
////////// two operand multiplication //////////
////////////////////////////////////////////////

// sorn * sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator* (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs,
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> prod = lhs;
	return prod *= rhs;
}
// sorn * int
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator* (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, int rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> prod = lhs;
	return prod *= rhs;
}
// sorn * float
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator* (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, float rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> prod = lhs;
	return prod *= rhs;
}
// sorn * double
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator* (sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs, double rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> prod = lhs;
	return prod *= rhs;
}
// int * sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator* (int lhs, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> prod = rhs;
	return prod *= lhs;
}
// float * sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator* (float lhs, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> prod = rhs;
	return prod *= lhs;
}
// double * sorn
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> operator* (double lhs, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& rhs) {
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> prod = rhs;
	return prod *= lhs;
}

///////////////////////////////////////////
////////// arithmetic operations //////////
///////////////////////////////////////////

// absolute value
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> abs(sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> op) {
	return op.abs();
}

// hypot(sorn,sorn)
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg = 1, bool _inf = 1, bool _zero = 1>
sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> hypot(sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> lhs,
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> rhs) {
	using std::sqrt;
	sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> res, lhsAbs, rhsAbs;
	// take abs value of inputs
	lhsAbs = lhs.abs();
	rhsAbs = rhs.abs();
	// carry out hypot on the abs values of the inputs
	res.sornIntVal.lowerBound = sqrt(lhsAbs.sornIntVal.lowerBound * lhsAbs.sornIntVal.lowerBound + rhsAbs.sornIntVal.lowerBound * rhsAbs.sornIntVal.lowerBound);
	res.sornIntVal.upperBound = sqrt(lhsAbs.sornIntVal.upperBound * lhsAbs.sornIntVal.upperBound + rhsAbs.sornIntVal.upperBound * rhsAbs.sornIntVal.upperBound);
	res.sornIntVal.lowerIsOpen = lhsAbs.sornIntVal.lowerIsOpen || rhsAbs.sornIntVal.lowerIsOpen;
	res.sornIntVal.upperIsOpen = lhsAbs.sornIntVal.upperIsOpen || rhsAbs.sornIntVal.upperIsOpen;
	res.adaptToDT();
	return res;
}

}} // end namespace sw::universal

