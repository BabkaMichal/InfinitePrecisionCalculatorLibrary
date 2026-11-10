#include <vector>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cstddef>
#include <iomanip>

/*TODO:
math functions
other operators multiple types
exception
*/

//unlimited size constant
inline constexpr std::size_t Unlimited = std::numeric_limits<std::size_t>::max();

//max precision for multiple MPInt types
consteval std::size_t maxPrecision(std::size_t left, std::size_t right) {
	if (left == Unlimited || right == Unlimited) {
		return Unlimited;
	}

	if (left > right) {
		return left;
	}

	return right;
}

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
		//copy move schematic, everything default because of vector
		MPInt(const MPInt&) = default;
		MPInt(MPInt&&) = default;

		MPInt& operator=(const MPInt&) = default;
		MPInt& operator=(MPInt&&) = default;

		//default constructor
		MPInt() : sign(1), blocks(1, 0) {}

		//constructor from string
		MPInt(const std::string& value) : sign(1), blocks(1, 0) {
			//sanity checks
			if (value.empty()) {
				//TODO: handle error
				throw std::invalid_argument("empty input");
			}

			//digit check, pushing to blocks
			for (char character : value) {
				if (character < '0' || character > '9') {
					//TODO: handle error
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

			result.checkSize();
			return result;
		}

		//'-' overload for substraction
		MPInt operator-(const MPInt& other) const {
			MPInt result;

			//same sign
			if (sign == other.sign) {
				int comparison = compareMagnitude(blocks, other.blocks);

				//same number
				if (comparison == 0) {
					return MPInt();
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

			result.checkSize();
			return result;
		}

		//'*' overload for multiplication
		MPInt operator*(const MPInt& other) const {
			if (isZero() || other.isZero()) {
				return MPInt();
			}

			//multiply magnitudes
			MPInt result;
			result.blocks = multiplyMagnitude(blocks, other.blocks);
			result.sign = sign * other.sign;

			result.checkSize();
			return result;
		}

		//'/' overload for division
		MPInt operator/(const MPInt& other) const {
			if (other.isZero()) {
				//TODO: custom exception class
				throw std::domain_error("Division by zero");
			}
			
			//divide magnitudes
			MPInt result;
			result.blocks = divideMagnitude(blocks, other.blocks);
			result.sign = sign * other.sign;

			result.checkSize();
			return result;
		}

		//'+=' overload for addition assignment
		MPInt& operator+=(const MPInt& other) {
			*this = *this + other;
			return *this;
		}

		//'-=' overload for subtraction assignment
		MPInt& operator-=(const MPInt& other) {
			*this = *this - other;
			return *this;
		}

		//'*=' overload for multiplication assignment
		MPInt& operator*=(const MPInt& other) {
			*this = *this * other;
			return *this;
		}

		//'/=' overload for division assignment
		MPInt& operator/=(const MPInt& other) {
			*this = *this / other;
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

				//remove leading zeros blocks
				while (!remaining.empty() && remaining.back() == 0) {
					remaining.pop_back();
				}
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
					//TODO: Change to custom exception class
					throw std::overflow_error("Value exceeds maximum size");
				}
			}
		}

		//MATH helpers
		bool isZero() const {
			return blocks.size() == 1 && blocks[0] == 0;
		}

		addMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {

		}

		compareMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {

		}

		subtractMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {

		}

		multiplyMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		
		}

		divideMagnitude(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {

		}

};