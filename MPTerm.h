#pragma once

#include "MPInt.h"

#include <deque>
#include <string>
#include <regex>
#include <iostream>
#include <exception>

template <std::size_t MaxBytes>
class MPTerm {
public:
	void run() {
		std::cout << "Zadejte jednoduchy matematicky vyraz s nejvyse jednou operaci +, -, *, / nebo !" << std::endl;

		while (running) {
			std::string line;

			std::cout << "> ";
			if (!std::getline(std::cin, line)) {
				running = false;
				break;
			}

			try {
				processLine(line);
			}
			catch (const std::exception& error) {
				std::cout << "Chyba: " << error.what() << std::endl;
			}
		}
	}

private:
    using Number = MPInt<MaxBytes>;
    std::deque<Number> history;
    bool running = true;

	void printHistory() const {	
		for (std::size_t i = 0; i < history.size(); i++) {
			std::cout << '$' << i + 1 << " = " << history[i] << std::endl;
		}
	}

	Number parseOperand(const std::string& operand) const {
		//history
		if (operand[0] == '$') {
			std::size_t index =
				static_cast<std::size_t>(operand[1] - '1');

			if (index >= history.size()) {
				throw std::out_of_range(operand + "nemá aktuální hodnotu v historii");
			}

			return history[index];
		}

		//number
		return Number(operand);
	}

    void processLine(const std::string& line) {

		//ignore empty lines
		if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
			return;
		}
		
		if (line == "exit") {
			std::cout << "Koncim." << std::endl;
			running = false;
			return;
		}

		if (line == "bank") {
			printHistory();
			return;
		}

		//first operand required
		//after either operator and second operand or factorial
		//lone operand is also valid
		static const std::regex expressionPattern(
			R"(^(\d+|[$][1-5])(?:([+*/-])(\d+|[$][1-5])|(!))?$)"
		);
		std::smatch match;

		if (!std::regex_match(line, match, expressionPattern)) {
			std::cout << "Neplatne zadani" << std::endl;
			return;
		}

		Number left = parseOperand(match[1].str());
		if (match[2].matched) {
			Number right = parseOperand(match[3].str());
			char op = match[2].str()[0];
			switch (op) {
				case '+':
					left += right;
					break;
				case '-':
					left -= right;
					break;
				case '*':
					left *= right;
					break;
				case '/':
					left /= right;
					break;
			}
		} else if (match[4].matched) {
			left = left.factorial();
		}

		//add to history
		history.push_front(left);

		if (history.size() > 5) {
			history.pop_back();
		}

		std::cout << "$1 = " << history.front() << std::endl;
	}
};