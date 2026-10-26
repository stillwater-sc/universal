#pragma once

#include <iostream>
#include <universal/number/valid/core.hpp>

namespace sw {
namespace universal {

template<unsigned nbits, unsigned es, typename bt>
std::ostream& operator<<(std::ostream& stream, const valid<nbits, es, bt>& value) {
	if (value.isnar())
		return stream << "[nar]";

    if (value.isexact()) {
        return stream << value.template lower<double>();
    }

	return stream << (value.lower_open() ? '(' : '[') << value.template lower<double>() << ", "
	              << value.template upper<double>() << (value.upper_open() ? ')' : ']');
}

}
}  // namespace sw
