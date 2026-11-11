#include "MPInt.h"

#include <iostream>
#include <type_traits>
#include <utility>

using BoundedResult = decltype(
	std::declval<MPInt<4>>() + std::declval<MPInt<8>>()
);

using UnlimitedResult = decltype(
	std::declval<MPInt<8>>() + std::declval<MPInt<Unlimited>>()
);

static_assert(std::is_same_v<BoundedResult, MPInt<8>>);
static_assert(std::is_same_v<UnlimitedResult, MPInt<Unlimited>>);

static_assert(std::is_same_v<
	decltype(std::declval<MPInt<4>>() - std::declval<MPInt<8>>()),
	MPInt<8>>);
static_assert(std::is_same_v<
	decltype(std::declval<MPInt<4>>() * std::declval<MPInt<8>>()),
	MPInt<8>>);
static_assert(std::is_same_v<
	decltype(std::declval<MPInt<4>>() / std::declval<MPInt<8>>()),
	MPInt<8>>);

int main() {
	MPInt<4> smallNumber("100");
	MPInt<8> mediumNumber("250");
	MPInt<Unlimited> largeNumber("1000000000000");

	std::cout << "MPInt<4> + MPInt<8> -> MPInt<8>: "
		<< smallNumber + mediumNumber << '\n';
	std::cout << "MPInt<8> - MPInt<4> -> MPInt<8>: "
		<< mediumNumber - smallNumber << '\n';
	std::cout << "MPInt<4> * MPInt<8> -> MPInt<8>: "
		<< smallNumber * mediumNumber << '\n';
	std::cout << "MPInt<Unlimited> / MPInt<4> -> MPInt<Unlimited>: "
		<< largeNumber / smallNumber << '\n';

	MPInt<4> compoundTarget("100");
	compoundTarget += mediumNumber;
	std::cout << "MPInt<4> after += MPInt<8> (left precision stays 4): "
		<< compoundTarget << '\n';
}
