#pragma once 
// attributes.hpp: functions to query number system attributes 
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc. 
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.  
#include <string>
#include <sstream>

namespace sw { namespace universal {  

// functions to provide details about properties of a bfloat configuration

inline int scale(bfloat16 bf) {
	return bf.scale();
}
	
inline std::string dynamic_range(const bfloat16& a) {
	std::stringstream s;
	bfloat16 b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);

	s << type_tag(a) << ": ";
	s << "minpos scale " << std::setw(10) << d.scale() << "    ";
	s << "maxpos scale " << std::setw(10) << e.scale() << '\n';
	s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
	s << "[" << to_binary(b) << " ... " << to_binary(c) << ", 0, " << to_binary(d) << " ... " << to_binary(e) << "]\n";
	return s.str();
}

}}  // namespace sw::universal
