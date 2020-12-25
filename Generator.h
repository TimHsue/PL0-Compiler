#ifndef COMPILE_GENERATOR_H
#define COMPILE_GENERATOR_H
#include <vector>
#include <string>

const std::vector<std::string> codeToString{"LIT", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "OPR"};
const std::vector<std::string> operationToString{"ret", "odd", "+", "-", "*", "/", "flag", "",
                                                 "=", "#", "<", ">=", ">", "<=", "output", "enter", "input", "pop"};
/*
 * 0 LIT
 * 1 LOD
 * 2 STO
 * 3 CAL
 * 4 INT
 * 5 JMP
 * 6 JPC
 * 7 OPR
 *
 */
class InterCode {
public:
	int code;
	int level, addr;

	InterCode() {
		level = addr = code = 0;
	}

	InterCode(const int &code_, const int &level_, const int &addr_) {
		code = code_;
		level = level_;
		addr = addr_;
	}

	InterCode(const std::string &code_) {
		code = 7;
		level = addr = 0;
		for (int i = 0; i < operationToString.size(); i++) {
			if (operationToString[i] == code_) {
				addr = i;
				break;
			}
		}
	}

	std::string genString() {
		if (code == 7)
			return codeToString[code] + "\t" + std::to_string(level) + "\t" + operationToString[addr];
		else
			return codeToString[code] + "\t" + std::to_string(level) + "\t" + std::to_string(addr);
	}
};

class Generator {
public:
	std::vector<InterCode> interCode;
	std::vector<int> sizeStack;

	int getIndex() {
		return interCode.size() - 1;
	}

	void pushIndex() {
		sizeStack.push_back(interCode.size());
	}

	void recovery() {
		int lastSize = sizeStack[sizeStack.size() - 1];
		while (lastSize != interCode.size()) interCode.pop_back();
		sizeStack.pop_back();
	}

	void popIndex() {
		sizeStack.pop_back();
	}

	void modifyA(const int &index, const int &val) {
		interCode[index].addr = val;
	}

	int genINT() {
		interCode.emplace_back(InterCode(4, 0, 0));
		return getIndex();
	}

	int genJPC() {
		interCode.emplace_back(InterCode(6, 0, 0));
		return getIndex();
	}

	int genRET() {
		interCode.emplace_back(InterCode(7, 0, 0));
		return getIndex();
	}

	int genOPR(const std::string &type) {
		for (int i = 0; i < operationToString.size(); i ++) {
			if (operationToString[i] == type) {
				interCode.emplace_back(InterCode(7, 0, i));
				return getIndex();
			}
		}
		return 0;
	}

	int genJMP(const int &adr) {
		interCode.emplace_back(InterCode(5, 0, adr));
		return getIndex();
	}

	int genLOD(const int &level, const int &adr) {
		interCode.emplace_back(InterCode(1, level, adr));
		return getIndex();
	}

	int genSTO(const int &level, const int &adr) {
		interCode.emplace_back(InterCode(2, level, adr));
		return getIndex();
	}

	int genLIT(const int &val) {
		interCode.emplace_back(InterCode(0, 0, val));
		return getIndex();
	}

	int genCAL(const int &level, const int &adr) {
		interCode.emplace_back(InterCode(3, level, adr));
		return getIndex();
	}

	int genOPT() {
		interCode.emplace_back(InterCode(7, 0, 14));
		interCode.emplace_back(InterCode(7, 0, 15));
		return getIndex();
	}

	int genIPT() {
		interCode.emplace_back(InterCode(7, 0, 16));
		return getIndex();
	}

	void outputCode() {
		int lineNumber = 0;
		for (auto item : interCode) {
			if (item.code == 7) {
				std::cout << lineNumber++ << "\t"
				          << codeToString[item.code] << "\t"
				          << item.level << "\t" << operationToString[item.addr] << std::endl;
			} else {
				std::cout << lineNumber++ << "\t"
				          << codeToString[item.code] << "\t"
				          << item.level << "\t" << item.addr << std::endl;
			}
		}
	}
};

#endif //COMPILE_GENERATOR_H
