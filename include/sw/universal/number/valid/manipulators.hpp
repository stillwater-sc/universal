#pragma once

#include <sstream>
#include <string>
#include <universal/number/posit/manipulators.hpp>
#include <universal/number/valid/core.hpp>

namespace sw {
namespace universal {

template<unsigned nbits, unsigned es, typename bt>
std::string type_tag(const valid<nbits, es, bt>& = {}) {
	std::ostringstream stream;
	stream << "valid<" << nbits << ',' << es << '>';
	return stream.str();
}

template<unsigned nbits, unsigned es, typename bt>
std::string to_binary(const valid<nbits, es, bt>& value, bool nibbleMarker = false) {
	std::ostringstream stream;
	stream << (value.lower_open() ? '(' : '[') << to_binary(value.lb(), nibbleMarker) << ", "
	       << to_binary(value.ub(), nibbleMarker) << (value.upper_open() ? ')' : ']');
	return stream.str();
}

}
}  // namespace sw
