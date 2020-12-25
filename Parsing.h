#ifndef COMPILE_PARSING_H
#define COMPILE_PARSING_H
#include <string>
#include <vector>
#include <cstring>
#include <set>

#define ISNUM(X) (X <= '9' && X >= '0')
#define ISSCH(X) (X <= 'z' && X >= 'a')
#define ISBCH(X) (X <= 'Z' && X >= 'A')
#define ISLET(X) (ISSCH(X) || ISBCH(X))
#define ISSPC(X) (X == ' ' or X == '\t')
#define ISENT(X) (X == '\r')
#define ISNEL(X) (X == '\n')
#define ISDIV(X) (X == '=' || X == '<' || X == '>' || X == ';' || X == '.' || X == '#' || X == ':' || X == '+' || \
				  X == '-' || X == '*' || X == '/' || X == '(' || X == ')' || X == ',')

class Word {
public:
	std::string token;
	int symbol, number;
	int line, row;

	Word() {
		token = "";
		symbol = number = 0;
		line = row = 0;
	}

	Word(const std::string &token_, const int &symbol_, const int &number_, const int &line_, const int &row_) {
		token = token_; symbol = symbol_; number = number_;
		line = line_; row = row_;
	}
};

class Parsing {
public:
    std::string content;
    std::vector<int> contentLine;
    std::vector<int> contentRow;
    std::vector<std::string> reservationWords;
    std::vector<std::string> divisionWords;
	std::vector<std::string> operationWords;
	int reservationBase, divisionBase, operationBase;
	int constBase, identifierBase;
	std::vector<Word> words;

	void init() {
		reservationBase = 0;
		reservationWords = std::vector<std::string>{"begin", "end", "if", "then", "else", "while", "do", "call", "odd",
											  "const", "var", "procedure", "read", "write"};
		divisionBase = reservationWords.size();
		divisionWords = std::vector<std::string>{"(", ")", ";", ".", ","};
		operationBase = divisionBase + (int)divisionWords.size();
		operationWords = std::vector<std::string>{":=", "+", "-", "*", "/", "<", ">", "<=", ">=", "=", "#"};
		constBase = operationBase + (int)operationWords.size();
		identifierBase = constBase + 1;
	}

    Parsing() {
	    reservationBase = divisionBase = operationBase = \
	    constBase = identifierBase = 0;
		init();
    }

    explicit Parsing(const std::string &content_) {
	    reservationBase = divisionBase = operationBase = \
	    constBase = identifierBase = 0;
    	init();
		content = content_;

		getSymbol();
    }

    void filter() {
    	std::string filterResult;
    	int continueSpace = 0;
    	int nowLine = 1, nowRow = 0;
    	for (auto &item : content) {
    		if (ISNEL(item)) {
				item = ' ';
				nowLine++;
				nowRow = -1;
    		} else if (ISENT(item)) {
			    item = ' ';
			    nowRow = -1;
    		}
    		if (ISSPC(item)) continueSpace++;
    		else continueSpace = 0;

    		nowRow++;
    		if (continueSpace > 1) {
    			continue;
    		}
		    filterResult += item;
		    contentLine.push_back(nowLine);
		    contentRow.push_back(nowRow);
    	}
    	content = filterResult;
    }

	int getReservation(const std::string& target) {
		for (int i = 0; i < reservationWords.size(); i++) {
			if(reservationWords[i] == target) return reservationBase + i;
		}
		for (int i = 0; i < divisionWords.size(); i++){
			if(divisionWords[i] == target) return divisionBase + i;
		}
		for (int i = 0; i < operationWords.size(); i++) {
			if (operationWords[i] == target) return operationBase + i;
		}
		return -1;
	}

	bool isNumber(const std::string& target) {
		for (auto item : target) if (! ISNUM(item)) return false;
		return true;
	}

	bool isIdentifier(std::string target) {
		if (! ISLET(*(target.begin()))) return false;
		for (auto item : target) if ((! ISLET(item)) && (! ISNUM(item))) return false;
		return true;
	}

	bool isDivision(const std::string& target) {
		for (const auto& item : divisionWords) {
			if (target == item) return true;
		}
		for (const auto& item : operationWords) {
			if (target == item) return true;
		}
		return false;
	}

	static int getNumber(const std::string& target) {
		int res = 0;
		for (auto item : target) {
			res = res * 10 + item - '0';
		}
		return res;
	}

	int getSymbol() {
		filter();
		auto now = content.begin();
		while (now != content.end()) {
			std::string target;

			while (now != content.end() && ISSPC(*now)) now++;
			auto nowPosition = now;
			if (now == content.end()) break;

			bool isDiv = ISDIV(*now);
			bool isLet = ISLET(*now);
			bool isNum = ISNUM(*now);
			char doubleDiv = *now;
			while (now != content.end() && ! ISSPC(*now)) {
				if (isLet && ISDIV(*now)) break;
				if (isDiv && (ISLET(*now) || ISNUM(*now))) break;
				if (isNum && ISDIV(*now)) break;
				if (ISDIV(*now)) {
					if (target.length() == 1) {
						if (*now == '='){
							if (doubleDiv != '<' && doubleDiv != '>' && doubleDiv != ':') break;
						} else break;
					} else if (target.length() >= 2) break;
				}
				target += *now;
				now++;
			}

			int symbol = getReservation(target);
			int number = 0;
			if (symbol == -1) {
				if (isNumber(target)) {
					symbol = constBase;
					number = getNumber(target);
				}
				if (isIdentifier(target)) symbol = identifierBase;
			}

			int targetPosition = nowPosition - content.begin();
			words.emplace_back(target, symbol, number, contentLine[targetPosition], contentRow[targetPosition]);

			if (symbol == -1) {
				std::cout << "parsing error in " << contentLine[targetPosition] <<
											 ":" << contentRow[targetPosition] << std::endl;
				return -1;
			}
		}
		return 0;
	}

	void printWords() {
		for (const auto& item : words) {
			std::cout << "<" << item.symbol << ",\t" << item.number << ">\t" << item.token
					  << "\t" << item.line << ":\t" << item.row << std::endl;
		}
	}
};


#endif //COMPILE_PARSING_H
