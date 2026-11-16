#pragma once

#include <vector>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <utility>


/*TODO:
edge cases
*/

//unlimited size constant
inline constexpr std::size_t Unlimited = std::numeric_limits<std::size_t>::max();

//max precision for multiple MPInt types
consteval std::size_t maxPrecision(std::size_t left, std::size_t right) {
	if (left == Unlimited || right == Unlimited)
		return Unlimited;

	if (left > right)
		return left;

	return right;
}

//custom exception
//stores the value, not operands
struct MPIntOverflowValue {
	int sign;
	std::vector<uint32_t> blocks;
};

class MPIntOverflowException : public std::overflow_error {
	public:
		explicit MPIntOverflowException(MPIntOverflowValue value)
			: std::overflow_error("MPInt result exceeded its precision limit"),
			overflowValue_(std::move(value)) {}

		const MPIntOverflowValue& overflowValue() const noexcept {
			return overflowValue_;
		}

	private:
		MPIntOverflowValue overflowValue_;
};

template <std::size_t MaxBytes>
class MPInt {
	//can access private members of other MPInt types
	template <std::size_t>
	friend class MPInt;

	//asert of template parameters
	static_assert(
		MaxBytes == Unlimited || MaxBytes >= 4,
		"MaxBytes must be Unlimited or at least 4"
		);

public:
	//copy default because of vector
	//move also resets the other object to a valid state
	MPInt(const MPInt&) = default;
	MPInt& operator=(const MPInt&) = default;

	MPInt(MPInt&& other) : sign(other.sign), blocks(1, 0) {
		blocks.swap(other.blocks);
		other.sign = 1;
		normalize();
	}

	MPInt& operator=(MPInt&& other) {
		if (this != &other) {
			std::vector<uint32_t> resetBlocks{ 0 };

			blocks = std::move(other.blocks);
			sign = other.sign;
			other.blocks.swap(resetBlocks);
			other.sign = 1;
			normalize();
		}

		return *this;
	}

	//default constructor
	MPInt() : sign(1), blocks(1, 0) {}

	//constructor from string
	MPInt(const std::string& value) : sign(1), blocks(1, 0) {
		//sanity checks
		if (value.empty()) {
			throw std::invalid_argument("empty input");
		}

		//digit check, pushing to blocks
		for (char character : value) {
			if (character < '0' || character > '9') {
				throw std::invalid_argument("Input must contain only digits");
			}
			uint32_t digit = static_cast<uint32_t>(character - '0');
			addDigit(digit);
		}

		checkSize();
	}


	//'+' overload for addition
	template <std::size_t OtherBytes>
	auto operator+(const MPInt<OtherBytes>& other) const
		-> MPInt<maxPrecision(MaxBytes, OtherBytes)> {

		//takes the bigger precision
		using ResultType = MPInt<maxPrecision(MaxBytes, OtherBytes)>;
		ResultType result;

		//same sign
		if (sign == other.sign) {
			result.blocks = addMagnitude(blocks, other.blocks);
			result.sign = sign;
		}

		//different sign
		else {
			int comparison = compareMagnitude(blocks, other.blocks);

			//same number
			if (comparison == 0) {
				return ResultType();
			}
			//this bigger
			if (comparison > 0) {
				result.blocks = subtractMagnitude(blocks, other.blocks);
				result.sign = sign;
			}
			//other bigger
			else {
				result.blocks = subtractMagnitude(other.blocks, blocks);
				result.sign = other.sign;
			}
		}

		result.normalize();
		result.checkSize();
		return result;
	}

	// '-' overload for subtraction
	template <std::size_t OtherBytes>
	auto operator-(const MPInt<OtherBytes>& other) const
		-> MPInt<maxPrecision(MaxBytes, OtherBytes)> {

		using ResultType = MPInt<maxPrecision(MaxBytes, OtherBytes)>;
		ResultType result;

		//same sign
		if (sign == other.sign) {
			int comparison = compareMagnitude(blocks, other.blocks);

			//same number
			if (comparison == 0) {
				return ResultType();
			}
			//this bigger
			if (comparison > 0) {
				result.blocks = subtractMagnitude(blocks, other.blocks);
				result.sign = sign;
			}
			//other bigger
			else {
				result.blocks = subtractMagnitude(other.blocks, blocks);
				result.sign = -sign;
			}
		}

		//different sign
		else {
			result.blocks = addMagnitude(blocks, other.blocks);
			result.sign = sign;
		}

		result.normalize();
		result.checkSize();
		return result;
	}

	//'*' overload for multiplication
	template <std::size_t OtherBytes>
	auto operator*(const MPInt<OtherBytes>& other) const
		-> MPInt<maxPrecision(MaxBytes, OtherBytes)> {

		if (isZero() || other.isZero()) {
			return MPInt<maxPrecision(MaxBytes, OtherBytes)>();
		}

		//multiply magnitudes
		using ResultType = MPInt<maxPrecision(MaxBytes, OtherBytes)>;
		ResultType result;
		result.blocks = multiplyMagnitude(blocks, other.blocks);
		result.sign = sign * other.sign;

		result.normalize();
		result.checkSize();
		return result;
	}

	//'/' overload for division
	template <std::size_t OtherBytes>
	auto operator/(const MPInt<OtherBytes>& other) const
		-> MPInt<maxPrecision(MaxBytes, OtherBytes)> {

		if (other.isZero()) {
			throw std::domain_error("Division by zero");
		}

		//divide magnitudes
		using ResultType = MPInt<maxPrecision(MaxBytes, OtherBytes)>;
		ResultType result;
		result.blocks = divideMagnitude(blocks, other.blocks);
		result.sign = sign * other.sign;

		result.normalize();
		result.checkSize();
		return result;
	}

	//'+=' overload for addition assignment
	template <std::size_t OtherBytes>
	MPInt& operator+=(const MPInt<OtherBytes>& other) {
		auto result = *this + other;
		assignResult(result);
		return *this;
	}

	//'-=' overload for subtraction assignment
	template <std::size_t OtherBytes>
	MPInt& operator-=(const MPInt<OtherBytes>& other) {
		auto result = *this - other;
		assignResult(result);
		return *this;
	}

	//'*=' overload for multiplication assignment
	template <std::size_t OtherBytes>
	MPInt& operator*=(const MPInt<OtherBytes>& other) {
		auto result = *this * other;
		assignResult(result);
		return *this;
	}

	//'/=' overload for division assignment
	template <std::size_t OtherBytes>
	MPInt& operator/=(const MPInt<OtherBytes>& other) {
		auto result = *this / other;
		assignResult(result);
		return *this;
	}

	//factorial logic
	MPInt factorial() const {
		if (sign < 0) {
			throw std::domain_error("Factorial is not defined for negative numbers");
		}

		MPInt result("1");
		MPInt counter(*this);
		const MPInt one("1");

		//multiply result by counter and decrement counter until it reaches 1
		while (compareMagnitude(counter.blocks, one.blocks) > 0) {
			result *= counter;
			counter -= one;
		}

		return result;
	}

	//'<<' operator overload for printing MPInt
	friend std::ostream& operator<<(std::ostream& output, const MPInt& value) {
		//decimal base for conversion
		static constexpr uint64_t DecimalBase = 1'000'000'000;

		//copy blocks
		std::vector<uint32_t> remaining = value.blocks;
		std::vector<uint32_t> groups;

		while (!remaining.empty()) {
			uint64_t remainder = 0;

			//divide the number by DecimalBase(1 000 000 000) and get the remainder
			for (std::size_t i = remaining.size(); i > 0; --i) {

				uint64_t current = (remainder << 32) + remaining[i - 1];
				remaining[i - 1] = static_cast<uint32_t>(current / DecimalBase);
				remainder = current % DecimalBase;
			}

			//store the remainder as a group of digits
			groups.push_back(static_cast<uint32_t>(remainder));
			normalizeMagnitude(remaining);

			//stop if number is zero
			if (remaining.size() == 1 && remaining[0] == 0)
				break;
		}

		//add negative sign
		if (value.sign < 0) {
			output << '-';
		}

		//print the first group without leading zeroes
		output << groups.back();

		//change fill character to '0' in ostream
		char previousFill = output.fill('0');

		//print the remaining groups with leading zeroes
		for (std::size_t i = groups.size() - 1; i > 0; --i) {
			output << std::setw(9) << groups[i - 1];
		}

		//change stream back to normal
		output.fill(previousFill);

		return output;
	}


private:
	//variables
	int sign;
	std::vector<uint32_t> blocks;

	//handles adding digit to blocks and managing carry
	void addDigit(uint32_t digit) {
		uint64_t carry = digit;

		for (std::size_t i = 0; i < blocks.size(); ++i) {
			uint64_t current = static_cast<uint64_t>(blocks[i]) * 10 + carry;

			blocks[i] = static_cast<uint32_t>(current);
			carry = current >> 32;
		}

		if (carry != 0) {
			blocks.push_back(static_cast<uint32_t>(carry));
		}
	}

	//counts the number of bytes used in every block
	std::size_t usedBytes() const {
		//use number of full blocks for initial byte count
		std::size_t bytes = (blocks.size() - 1) * sizeof(uint32_t);
		uint32_t highestBlock = blocks.back();

		//count the number of bytes in the highest block
		while (highestBlock != 0) {
			++bytes;
			highestBlock >>= 8;
		}

		//if the number is zero, return 1 byte
		return bytes == 0 ? 1 : bytes;
	}

	//checks if the number of bytes used exceeds the maximum allowed size
	void checkSize() const {
		if constexpr (MaxBytes != Unlimited) {
			if (usedBytes() > MaxBytes) {
				throw MPIntOverflowException({ sign, blocks });
			}
		}
	}

	//on asignment narrow the result to the current types precision
	template <std::size_t ResultBytes>
	void assignResult(const MPInt<ResultBytes>& result) {
		MPInt narrowedResult;

		narrowedResult.blocks = result.blocks;
		narrowedResult.sign = result.sign;
		narrowedResult.normalize();
		narrowedResult.checkSize();

		*this = std::move(narrowedResult);
	}

	//MATH helpers
	bool isZero() const {
		return blocks.size() == 1 && blocks[0] == 0;
	}

	//normalize by removing leading zeroes and setting sign to positive if the number is zero
	void normalize() {
		normalizeMagnitude(blocks);

		if (isZero()) {
			sign = 1;
		}
	}

	//remove leading zeroes
	static void normalizeMagnitude(std::vector<uint32_t>& magnitude) {
		if (magnitude.empty()) {
			magnitude.push_back(0);
			return;
		}

		while (magnitude.size() > 1 && magnitude.back() == 0) {
			magnitude.pop_back();
		}
	}

	//Shift the value left and append one input bit.
	static void shiftLeftAndAddBit(std::vector<uint32_t>& value, uint32_t bit) {
		uint64_t carry = bit;

		for (std::size_t i = 0; i < value.size(); ++i) {
			uint64_t shiftedBlock = (static_cast<uint64_t>(value[i]) << 1) + carry;
			value[i] = static_cast<uint32_t>(shiftedBlock);
			carry = shiftedBlock >> 32;
		}

		if (carry != 0) {
			value.push_back(static_cast<uint32_t>(carry));
		}
	}

	//MATH functions
	static int compareMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		//A bigger
		if (a.size() > b.size())
			return 1;

		//B bigger
		if (a.size() < b.size())
			return -1;

		//same size, compare block by block from the most significant
		for (std::size_t i = a.size(); i > 0; --i) {
			if (a[i - 1] > b[i - 1])
				return 1;

			if (a[i - 1] < b[i - 1])
				return -1;
		}
		//equal
		return 0;
	}

	static std::vector<uint32_t> addMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		//use the longer input size
		std::size_t resultSize = std::max(a.size(), b.size());
		std::vector<uint32_t> result;
		result.reserve(resultSize + 1);

		uint64_t carry = 0;

		//add blocks with carry
		for (std::size_t i = 0; i < resultSize; i++) {
			uint64_t sum = carry;

			//add the current blocks when they exist
			if (i < a.size())
				sum += a[i];

			if (i < b.size())
				sum += b[i];

			//store the low 32 bits and carry the rest to the next block
			result.push_back(static_cast<uint32_t>(sum));
			carry = sum >> 32;
		}

		//append a new block if a carry remains
		if (carry != 0) {
			result.push_back(static_cast<uint32_t>(carry));
		}

		return result;
	}

	//asumes a >= b
	static std::vector<uint32_t> subtractMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		std::vector<uint32_t> result;
		result.reserve(a.size());

		uint64_t borrow = 0;
		const uint64_t blockBase = uint64_t{ 1 } << 32;

		//subtract blocks
		for (std::size_t i = 0; i < a.size(); ++i) {
			uint64_t subtractAmount = borrow;

			//treat missing block in b as zero
			if (i < b.size()) {
				subtractAmount += b[i];
			}

			if (a[i] < subtractAmount) {
				//borrow one unit from next block
				result.push_back(static_cast<uint32_t>(blockBase + a[i] - subtractAmount));
				borrow = 1;
			}
			else {
				result.push_back(static_cast<uint32_t>(a[i] - subtractAmount));
				borrow = 0;
			}
		}

		normalizeMagnitude(result);
		return result;
	}

	static std::vector<uint32_t> multiplyMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		std::vector<uint32_t> result(a.size() + b.size(), 0);
		
		//multiply each block of a by every block of b.
		for (std::size_t i = 0; i < a.size(); i++) {
			uint64_t carry = 0;

			for (std::size_t j = 0; j < b.size(); j++) {
				uint64_t product = static_cast<uint64_t>(a[i]) * b[j] + result[i + j] + carry;

				//store the low 32 bits and carry the rest forward.
				result[i + j] = static_cast<uint32_t>(product);
				carry = product >> 32;
			}

			//store the carry in the next result block.
			result[i + b.size()] = static_cast<uint32_t>(carry);
		}

		normalizeMagnitude(result);
		return result;
	}

	static std::vector<uint32_t> divideMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		//build the quotient while keeping the partial remainder.
		std::vector<uint32_t> quotient(a.size(), 0);
		std::vector<uint32_t> remainder{ 0 };

		//count the significant bits in the dividend.
		std::size_t bitCount = (a.size() - 1) * 32;
		uint32_t highestBlock = a.back();

		while (highestBlock != 0) {
			++bitCount;
			highestBlock >>= 1;
		}

		//process the dividend bits from most significant to least significant.
		for (std::size_t i = bitCount; i > 0; --i) {
			std::size_t bitIndex = i - 1;
			uint32_t currentBit =
				(a[bitIndex / 32] >> (bitIndex % 32)) & 1U;

			//append this bit to the partial remainder.
			shiftLeftAndAddBit(remainder, currentBit);

			//subtract the divisor and set this quotient bit when possible.
			if (compareMagnitude(remainder, b) >= 0) {
				remainder = subtractMagnitude(remainder, b);
				quotient[bitIndex / 32] |= uint32_t{ 1 } << (bitIndex % 32);
			}
		}

		normalizeMagnitude(quotient);
		return quotient;
	}
};