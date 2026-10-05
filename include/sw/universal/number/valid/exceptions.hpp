#pragma once

#include <stdexcept>

namespace sw {
namespace universal {

struct valid_arithmetic_exception : public std::runtime_error {
	explicit valid_arithmetic_exception(const char* message) : std::runtime_error(message) {}
};

struct valid_divide_by_zero : public valid_arithmetic_exception {
	valid_divide_by_zero() : valid_arithmetic_exception("valid division by an interval containing zero") {}
};

}
}  // namespace sw
