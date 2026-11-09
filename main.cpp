#include "MPInt.h"

int main() {
	// Basic values around decimal group boundaries.
	MPInt<Unlimited> zero;
	MPInt<Unlimited> smallNumber("42");
	MPInt<Unlimited> decimalBaseMinusOne("999999999");
	MPInt<Unlimited> decimalBase("1000000000");
	MPInt<Unlimited> decimalBasePlusOne("1000000001");

	// Values around the 32-bit block boundary.
	MPInt<Unlimited> largestOneBlock("4294967295");
	MPInt<Unlimited> firstTwoBlockValue("4294967296");

	// Multiple decimal groups and a larger value.
	MPInt<Unlimited> threeGroups("1000000001000000001");
	MPInt<Unlimited> largeNumber("123456789012345678901234567890");

	// Largest value that fits in four bytes.
	MPInt<4> fourByteMaximum("4294967295");

	std::cout << "Zero: " << zero << '\n';
	std::cout << "Small number: " << smallNumber << '\n';
	std::cout << "Decimal base - 1: " << decimalBaseMinusOne << '\n';
	std::cout << "Decimal base: " << decimalBase << '\n';
	std::cout << "Decimal base + 1: " << decimalBasePlusOne << '\n';
	std::cout << "Largest value in one 32-bit block: " << largestOneBlock << '\n';
	std::cout << "First value requiring two 32-bit blocks: " << firstTwoBlockValue << '\n';
	std::cout << "Three decimal groups: " << threeGroups << '\n';
	std::cout << "Large number: " << largeNumber << '\n';
	std::cout << "Four-byte maximum: " << fourByteMaximum << '\n';
}
