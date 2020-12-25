#ifndef COMPILE_INTERPRETER_H
#define COMPILE_INTERPRETER_H

#include "Generator.h"
#include <iostream>

class Interpreter {
public:
	std::vector<InterCode> interCode;
	int PC;
	InterCode IP;
	std::vector<int> SS;
	std::vector<int> BPStack;
	std::vector<int> SLStack;
	int BP;

	Interpreter() {
		PC = BP = 0;
	}

	Interpreter(const std::vector<InterCode> &interCode_, const int &mainAdr) {
		interCode = interCode_;
		PC = mainAdr;
		IP = interCode[PC];
		BP = 0;
		SS.emplace_back(-2);
		SS.emplace_back(0);
		SS.emplace_back(0);
		BPStack.emplace_back(1);
		SLStack.emplace_back(2);
	}

	void push(const int &val) {
		SS.emplace_back(val);
	}

	void outputStack(const std::string& type) {
		std::cout << "--------" + type + "--------" << std::endl;
		for (int i = SS.size() - 1; i >= 0; i--) {
			auto item = SS[i];
			auto tmp = std::to_string(item);
			while (tmp.length() < 5) {
				tmp += " ";
			}
			std::cout << "|" + tmp + "|";

			for (int j = 0; j < BPStack.size(); j++) {
				if (BPStack[j] - 1 == i) std::cout << "PC";
				if (BPStack[j] == i) std::cout << "BP" << std::endl << " ----- ";
				if (SLStack[j] == i) std::cout << "SL";
			}
			std::cout << std::endl;
		}
		std::cout << "-----------------" << std::endl;
	}

	void ret() {
		PC = SS[BP];
		int DL = SS[BP + 1];
		while (BP != SS.size()) SS.pop_back();
		BP = DL;
		outputStack("ret");
		BPStack.pop_back();
		SLStack.pop_back();
	}

	void getIP() {
		IP = interCode[PC];
	}

	int getSL(int level) {
		if (level == 0) return BP;
		level--;
		int nowSL = SS[BP + 2];
		while (level) {
			level--;
			nowSL = SS[nowSL + 2];
		}
		return nowSL;
	}

	void store(int level, const int &adr) {
		int val = SS.back();
		SS.pop_back();

		int pos = getSL(level) + adr;
		SS[pos] = val;
	}

	void load(int level, const int &adr) {
		int pos = getSL(level) + adr;
		SS.emplace_back(SS[pos]);
	}

	void entry(const int &level, const int &adr) {
		int SL = getSL(level);
		int nowBP = SS.size();
		SS.emplace_back(PC);
		SS.emplace_back(BP);
		SS.emplace_back(SL);
		BP = nowBP;
		PC = adr;

		BPStack.emplace_back(SS.size() - 2);
		SLStack.emplace_back(SS.size() - 1);
		outputStack("ent");
	}

	void init(const int &cnt) {
		for (int i = 0; i < cnt; i++) {
			SS.emplace_back(0);
		}
	}

	void nextI() {
		IP = interCode[++PC];
	}

	void jump(const int &adr) {
		PC = adr;
		IP = interCode[PC];
	}

	void jumpC(const int &adr) {
		int val = SS.back();
		SS.pop_back();

		if (val == 0) jump(adr);
		else nextI();
	}

	void read() {
		int tmp;
		std::cin >> tmp;
		SS.emplace_back(tmp);
	}

	void write() {
		int tmp;
		tmp = SS.back();
		SS.pop_back();

		std::cout << tmp;
	}

	void enter() {
		std::cout << std::endl;
	}

	//const std::vector<std::string> operationToString{0"ret", 1"odd", 2"+", 3"-", 4"*", 5"/", 6"flag", 7"",
	//                                                 8"=", 9"#", 10"<", 11">=", 12">", 13"<=", 14"output", 15"enter",
	//                                                 16"input", 17"pop"};
	void handleOPR() {
		int tmpA, tmpB;
#ifdef TDEBUG
		std::cout << std::endl << SS[SS.size() - 2] << operationToString[IP.addr] << SS[SS.size() - 1] << std::endl;
#endif
		switch (IP.addr) {
			case 0:
				ret();
				break;
			case 1:
				tmpA = SS.back(); SS.pop_back();
				SS.emplace_back(!tmpA);
				nextI();
				break;
			case 2:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpA + tmpB);
				break;
			case 3:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpB - tmpA);
				break;
			case 4:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpA * tmpB);
				break;
			case 5:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpB / tmpA);
				break;
			case 6:
				tmpA = SS.back(); SS.pop_back();
				SS.emplace_back(-tmpA);
				break;
			case 8:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpA == tmpB);
				break;
			case 9:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpA != tmpB);
				break;
			case 10:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpA > tmpB);
				break;
			case 11:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpA <= tmpB);
				break;
			case 12:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpA < tmpB);
				break;
			case 13:
				tmpA = SS.back(); SS.pop_back();
				tmpB = SS.back(); SS.pop_back();
				SS.emplace_back(tmpA >= tmpB);
				break;
			case 14:
				write();
				break;
			case 15:
				enter();
				break;
			case 16:
				read();
				break;
		}
	}

	//const std::vector<std::string> codeToString{0"LIT", 1"LOD", 2"STO", 3"CAL", 4"INT", 5"JMP", 6"JPC", 7"OPR"};
	void explain() {
		switch (IP.code) {
			case 0:
				push(IP.addr);
				nextI();
				break;
			case 1:
				load(IP.level, IP.addr);
				nextI();
				break;
			case 2:
				store(IP.level, IP.addr);
				nextI();
				break;
			case 3:
				entry(IP.level, IP.addr);
				break;
			case 4:
				init(IP.addr);
				nextI();
				break;
			case 5:
				jump(IP.addr);
				break;
			case 6:
				jumpC(IP.addr);
				break;
			case 7:
				handleOPR();
				nextI();
				break;
		}
	}

	void run() {
		while (true) {
			if (PC < 0) break;
			getIP();
#ifdef TDEBUG
			std::cout << PC << "\t" << IP.genString() << "\t" << SS.size() << std::endl;
#endif
			explain();
		}
		std::cout << std::endl << "yeah!" << std::endl;
	}
};

#endif