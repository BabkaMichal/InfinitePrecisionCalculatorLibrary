#include "MPInt.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using BoundedDivisionResult = decltype(
	std::declval<MPInt<4>>() / std::declval<MPInt<8>>()
);

using UnlimitedDivisionResult = decltype(
	std::declval<MPInt<8>>() / std::declval<MPInt<Unlimited>>()
);

static_assert(std::is_same_v<BoundedDivisionResult, MPInt<8>>);
static_assert(std::is_same_v<UnlimitedDivisionResult, MPInt<Unlimited>>);

template <typename Number>
void expectValue(
	const std::string& testName,
	const Number& actual,
	const std::string& expected
) {
	std::ostringstream output;
	output << actual;

	if (output.str() != expected) {
		std::cerr << "FAIL: " << testName
			<< ", expected " << expected
			<< ", got " << output.str() << '\n';
		throw std::runtime_error("MPInt division test failed");
	}

	std::cout << "PASS: " << testName << '\n';
}

int main() {
	expectValue(
		"exact division",
		MPInt<Unlimited>("100") / MPInt<Unlimited>("10"),
		"10"
	);

	expectValue(
		"division with remainder",
		MPInt<Unlimited>("100") / MPInt<Unlimited>("3"),
		"33"
	);

	expectValue(
		"dividend smaller than divisor",
		MPInt<Unlimited>("3") / MPInt<Unlimited>("10"),
		"0"
	);

	expectValue(
		"zero dividend",
		MPInt<Unlimited>("0") / MPInt<Unlimited>("987"),
		"0"
	);

	expectValue(
		"division across a 32-bit block boundary",
		MPInt<Unlimited>("4294967296") / MPInt<Unlimited>("2"),
		"2147483648"
	);

	expectValue(
		"division of multi-block values",
		MPInt<Unlimited>("18446744073709551616")
			/ MPInt<Unlimited>("4294967296"),
		"4294967296"
	);

	expectValue(
		"negative dividend truncates toward zero",
		(MPInt<Unlimited>("0") - MPInt<Unlimited>("7"))
			/ MPInt<Unlimited>("3"),
		"-2"
	);

	expectValue(
		"negative divisor",
		MPInt<Unlimited>("7")
			/ (MPInt<Unlimited>("0") - MPInt<Unlimited>("3")),
		"-2"
	);

	expectValue(
		"both operands negative",
		(MPInt<Unlimited>("0") - MPInt<Unlimited>("7"))
			/ (MPInt<Unlimited>("0") - MPInt<Unlimited>("3")),
		"2"
	);

	expectValue(
		"mixed precision division",
		MPInt<4>("100") / MPInt<8>("3"),
		"33"
	);

	bool divisionByZeroThrown = false;

	try {
		auto result = MPInt<Unlimited>("10") / MPInt<Unlimited>("0");
		(void)result;
	}
	catch (const std::domain_error&) {
		divisionByZeroThrown = true;
	}

	if (!divisionByZeroThrown) {
		throw std::runtime_error("Division by zero did not throw");
	}

	std::cout << "PASS: division by zero throws\n";
}
