#include <iostream>
#include <cstdio>
#include <fstream>
#include "Parsing.h"
#include "Lexical.h"
#include "Interpreter.h"

std::string readAll() {
	std::ifstream iFile;
	iFile.open("test4.pas", std::ios::in);
	std::string res = "";
	char tmp;
	while (iFile.good()) {
		tmp = iFile.get();
		if (tmp >= 'A' and tmp <= 'Z') tmp = tmp - 'A' + 'a';
		res += tmp;
		if (tmp == '.') break;
	}
	return res;
}

int main() {
	auto text = readAll();
	Parsing parsing(text);
	parsing.printWords();
	Lexical lexical(parsing);
	// lexical.output();
	// lexical.outputGraph();
	// lexical.drawTree();
	lexical.outputNameTable();
	lexical.outputCode();
	Interpreter interpreter(lexical.getInterCode(), lexical.mainAdr);
	interpreter.run();

    return 0;
}