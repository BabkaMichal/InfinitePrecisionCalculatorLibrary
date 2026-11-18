#include "MPTerm.h"
#include "MPInt.h"

#include <iostream>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

void runShowcase() {
	std::cout << "Ukazka knihovny MPInt" << std::endl;
	auto printOperation = [](
		const auto& left,
		const char* operation,
		const auto& right,
		const auto& result,
		const char* resultPrecision
	) {
		std::cout << left << ' ' << operation << ' ' << right << " = " << result
			<< " (vysledek " << resultPrecision << ')' << std::endl;
	};

	MPInt<4> smallNumber("4294967295");
	MPInt<8> mediumNumber("4294967296");

	std::cout << "Maximum MPInt<4>: " << smallNumber << std::endl;
	std::cout << "Priklad MPInt<8>: " << mediumNumber << std::endl;
	std::cout << "Maximum MPInt<8>: "
		<< MPInt<8>("18446744073709551615") << "\n" << std::endl;


	MPInt<8> mixedPrecisionSum = smallNumber + mediumNumber;
	printOperation(smallNumber, "+", mediumNumber, mixedPrecisionSum, "MPInt<8>");

	MPInt<8> mixedPrecisionDifference = smallNumber - mediumNumber;
	printOperation(smallNumber, "-", mediumNumber, mixedPrecisionDifference, "MPInt<8>");

	MPInt<8> mixedPrecisionProduct = smallNumber * mediumNumber;
	printOperation(smallNumber, "*", mediumNumber, mixedPrecisionProduct, "MPInt<8>");

	MPInt<8> mixedPrecisionQuotient = mediumNumber / smallNumber;
	printOperation(mediumNumber, "/", smallNumber, mixedPrecisionQuotient, "MPInt<8>");
	std::cout << std::endl;

	std::cout << "100! = " << MPInt<Unlimited>("100").factorial() << "\n" << std::endl;

	MPInt<8> compoundValue("7");
	MPInt<4> compoundOperand("5");
	std::cout << compoundValue << " += " << compoundOperand << std::endl;
	compoundValue += compoundOperand;
	std::cout << "Vysledek: " << compoundValue << "\n" << std::endl;

	MPInt<Unlimited> original("123456789");
	MPInt<Unlimited> copied(original);
	MPInt<Unlimited> moved(std::move(copied));
	std::cout << "Zkopirovana hodnota: " << original << std::endl;
	std::cout << "Presunuta hodnota: " << moved << std::endl;
	std::cout << "Zdroj po presunu: " << copied << "\n" << std::endl;

	try {
		MPInt<4> maximum("4294967295");
		MPInt<4> one("1");
		std::cout << maximum << " + " << one
			<< " = ocekava se preteceni MPInt<4>" << std::endl;
		auto overflowResult = maximum + one;
		std::cout << "Vysledek: " << overflowResult << std::endl;
	}
	catch (const MPIntOverflowException& error) {
		const MPIntOverflowValue& value = error.overflowValue();

		std::cout << "Zachycene preteceni: " << error.what() << std::endl;
		std::cout << "Znamenko vysledku: " << value.sign << std::endl;
		std::cout << "Bloky vysledku od nejmensiho: ";

		for (uint32_t block : value.blocks) {
			std::cout << block << ' ';
		}

		std::cout << std::endl;
	}
}


int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Pouzijte ./mpcalc.exe <mode> (mode: 1-3)" << std::endl;
		return 1;
	}

	const std::string mode = argv[1];

	if (mode == "1") {
		MPTerm<Unlimited>().run();
	}
	else if (mode == "2") {
		MPTerm<32>().run();
	}
	else if (mode == "3") {
		runShowcase();
	}
	else {
		std::cout << "Neplatny mode. Zadejte 1, 2 nebo 3." << std::endl;
		return 1;
	}

	return 0;
}
