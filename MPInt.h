#include <vector>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <stdexcept>
#include <cstddef>

inline constexpr std::size_t Unlimited = std::numeric_limits<std::size_t>::max();

template <std::size_t MaxBytes>
class MPInt {
	public:
		//constructor from string
		MPInt(const std::string& value) : sign(1), blocks(1, 0) {
			//sanity checks
			if (value.empty()) {
				throw std::invalid_argument("empty input");
			}

			//digit check and addition to blocks
			for (char character : value) {
				if (character < '0' || character > '9') {
					throw std::invalid_argument("Input must contain only digits");
				}
				uint32_t digit = static_cast<uint32_t>(character - '0');
				addDigit(digit);
			}

			checkSize();
		}
		//TODO vyzkouset print
		void print() {
			if (sign == -1)
				std::cout << "-";

			for (size_t i = 0; i < blocks.size(); i++)
				std::cout << blocks[i] << "2^" << (i * 32) << ", ";
		}

	private:
		int sign;
		std::vector<uint32_t> blocks;

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

		std::size_t usedBytes() const {
			std::size_t bytes = (blocks.size() - 1) * sizeof(uint32_t);
			uint32_t highestBlock = blocks.back();

			while (highestBlock != 0) {
				++bytes;
				highestBlock >>= 8;
			}

			return bytes == 0 ? 1 : bytes;
		}

		void checkSize() const {
			if constexpr (MaxBytes != Unlimited || MaxBytes >= 4) {
				if (usedBytes() > MaxBytes) {
					throw std::overflow_error("Value exceeds maximum size");
				}
			}
		}


};