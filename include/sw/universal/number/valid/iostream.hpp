#pragma once

#include <iostream>
#include <universal/number/posit/iostream.hpp>
#include <universal/number/valid/core.hpp>

namespace sw {
namespace universal {

template<unsigned nbits, unsigned es, typename bt>
std::ostream& operator<<(std::ostream& stream, const valid<nbits, es, bt>& value) {
	if (value.isnar())
		return stream << "[nar]";
	stream << (value.lower_open() ? '(' : '[') << value.lb() << ", " << value.ub() << (value.upper_open() ? ')' : ']');
	return stream;
}

}
}  // namespace sw
