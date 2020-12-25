#ifndef COMPILE_LEXICAL_H
#define COMPILE_LEXICAL_H

#include "Parsing.h"
#include "Generator.h"

int stringToInt(const std::string &target) {
	int res = 0;
	for (auto item : target) {
		res = res * 10 + item - '0';
	}
	return res;
}

class Node {
public:
	std::string name;
	int id;
	std::vector<Node> son;
	int fr, to;

	Node() {
		name = "";
		id = 0;
		fr = to = -1;
	}

	Node(const std::string &name_, const int &id_, const int &fr_, const int &to_) {
		name = name_; id = id_;
		fr = fr_;
		to = to_;
	}

	void insertSon(const Node &b) {
		son.emplace_back(b);
	}

	bool operator == (const Node &b) const {
		return name == b.name and id == b.id and fr == b.fr and to == b.to;
	}

	bool operator != (const Node&b) const {
		return name != b.name or id != b.id or fr != b.fr or to != b.to;
	}
};

const Node nullNode = Node("", -1, -1, -1);

inline int countConst(const Node &now) {
	return (now.son.size() - 1) / 3;
}

inline int countVar(const Node &now) {
	return (now.son.size() - 1) / 2;
}

class Description {
public:
	std::string name;
	int kind; // 0 const 1 variable 2 process
	int adr;
	int val;

	Description() {
		name = "";
		kind = adr = 0;
		val = 0;
	}

	Description(const std::string &name_, const int &kind_,
	            const int &adr_, const int &val_) {
		name = name_;
		kind = kind_;
		adr = adr_;
		val = val_;
	}

	bool operator < (const Description & b) const {
		return name < b.name;
	}

	bool operator == (const Description &b) const {
		return name == b.name and kind == b.kind and adr == b.adr and val == b.val;
	}
};

class Procedure {
public:
	int fa, level, nowDx;
	std::string pName;
	std::vector<Description> nameTable;

	Procedure() {
		fa = nowDx = level = -1;
	}

	Procedure(const int &fa_, const std::string &name_) {
		pName = name_;
		fa = fa_;
		nowDx = 3;
		level = -1;
	}

	int findDes(const std::string &name, const int &kind=-1) {
		for (auto i = 0; i < nameTable.size(); i++) {
			if (nameTable[i].name == name) {
				if (kind != -1 and nameTable[i].kind != kind) continue;
				return i;
			}
		}
		return -1;
	}

	// get const value
	int getValue(const std::string &name, int &val) {
		for (auto i = 0; i < nameTable.size(); i++) {
			if (nameTable[i].name == name) {
				if (nameTable[i].kind != 0) continue;
				val = nameTable[i].val;
				return 0;
			}
		}
		return -1;
	}

	// get variable value
	int getAdr(const std::string &name, int &adr) {
		for (auto i = 0; i < nameTable.size(); i++) {
			if (nameTable[i].name == name) {
				if (nameTable[i].kind != 1) continue;
				adr = nameTable[i].adr;
				return 0;
			}
		}
		return -1;
	}

	// get procedure offset
	int getOff(const std::string &name, int &off) {
		for (auto i = 0; i < nameTable.size(); i++) {
			if (nameTable[i].name == name) {
				if (nameTable[i].kind != 2) continue;
				off = nameTable[i].adr;
				return 0;
			}
		}
		return -1;
	}

	int modifyVal(const std::string &name, const int &val, const int &kind=-1, const int &type=0) {
		for (auto & i : nameTable) {
			if (i.name == name) {
				if (kind != -1 and i.kind != kind) continue;
				if (type == 0) i.val = val;
				else i.adr = val;
				return 0;
			}
		}
		return -1;
	}

	void insertDes(const std::string &name, const int &kind, const int &adr, const int &val) {
		nameTable.emplace_back(Description(name, kind, adr, val));
	}
};

class Lexical {
public:
	Parsing parsing;
	std::vector<Word> words;
	int wordIndex;
	int nodeCnt;
	int lastErrorIndex;
	int lastErrorCode;
	std::vector<std::string> relationOperators;
	std::vector<Procedure> procedureTable;
	Node root;
	int procedureCnt = 0;
	Generator generator;
	int mainAdr;

	int findDescription(const std::string &name, const int &nowProcedure, const int &kind) {
		int procedureIndex = nowProcedure;
		while (true) {
			if (procedureTable[procedureIndex].findDes(name, kind) != -1) return 0;
			procedureIndex = procedureTable[procedureIndex].fa;
			if (procedureIndex == -1) break;
		}
		return -1;
	}

	int checkDescription(const std::string &name, const int &nowProcedure, const int &kind) {
		if (procedureTable[nowProcedure].findDes(name, kind) != -1) return 0;
		return -1;
	}

	int findProcedure(const std::string &name, const int &nowProcedure, int &level, int &off) {
		int procedureIndex = nowProcedure;
		level = 0;
		while (true) {
			int ret = procedureTable[procedureIndex].getOff(name, off);
			if (ret != -1) return 0;
			procedureIndex = procedureTable[procedureIndex].fa;
			level += 1;
			if (procedureIndex == -1) break;
		}
		return -1;
	}

	int findLevelAdr(const std::string &name, const int &nowProcedure, int &level, int &adr) {
		int procedureIndex = nowProcedure;
		level = 0;
		while (true) {
			int ret = procedureTable[procedureIndex].getAdr(name, adr);
			if (ret != -1) return 0;
			procedureIndex = procedureTable[procedureIndex].fa;
			level += 1;
			if (procedureIndex == -1) break;
		}
		return -1;
	}

	int findConst(const std::string &name, const int &nowProcedure, int &val) {
		int procedureIndex = nowProcedure;
		while (true) {
			int ret = procedureTable[procedureIndex].getValue(name, val);
			if (ret != -1) return 0;
			procedureIndex = procedureTable[procedureIndex].fa;
			if (procedureIndex == -1) break;
		}
		return -1;
	}

	void addProcedure(const int &nowProcedure, const std::string &name) {
		if (nowProcedure != -1) procedureTable[nowProcedure].insertDes(name, 2, -1, -1);
		procedureTable.emplace_back(Procedure(nowProcedure, name));
		if (nowProcedure != -1) {
			procedureTable[procedureTable.size() - 1].level = procedureTable[nowProcedure].level + 1;
		} else {
			procedureTable[procedureTable.size() - 1].level = 0;
		}
		procedureCnt++;
	}

	void searchConst(const Node &now, const int &nowProcedure) {
		auto len = now.son.size();
		for (auto i = 1; i < len; i += 2) {
			auto constDef = now.son[i];
			procedureTable[nowProcedure].insertDes(constDef.son[0].name, 0, -1,
						            stringToInt(constDef.son[2].son[0].name));
			// std::cout << now.son[i].name << std::endl;
		}
	}

	void searchVariable(const Node &now, const int &nowProcedure) {
		auto len = now.son.size();
		for (auto i = 1; i < len; i += 2) {
			procedureTable[nowProcedure].insertDes(now.son[i].name, 1,
										  procedureTable[nowProcedure].nowDx++, 0);
		}
	}

	Lexical(const Parsing &parsing_) {
		parsing = parsing_;
		words = parsing_.words;
		words.emplace_back(Word("", 0, 0, 0, 0));
		wordIndex = -1;
		nodeCnt = 0;
		lastErrorIndex = lastErrorCode = -1;
		relationOperators.emplace_back("=");
		relationOperators.emplace_back("#");
		relationOperators.emplace_back("<");
		relationOperators.emplace_back("<=");
		relationOperators.emplace_back(">");
		relationOperators.emplace_back(">=");

		root = mainProgram();
	}

	Node errorParsing(const int &returnIndex, const int errIndex=-1, const int errCode=-1) {
		generator.recovery();
		wordIndex = returnIndex;
		if (errIndex != -1) {
			if (lastErrorIndex <= errIndex) {
				lastErrorIndex = errIndex;
				lastErrorCode = errCode;
			}
		}
		return nullNode;
	}

	Node relationProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;
		std::string tmp = words[++wordIndex].token;

		for (const auto& item : relationOperators) {
			if (tmp == item) {
				generator.popIndex();
				return Node(tmp, nodeCnt++, wordIndex, wordIndex);
			}
		}

		return errorParsing(beginIndex, wordIndex, 1);
	}

	Node conditionProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].token == "odd") {
			int oddIndex = wordIndex;
			Node expression = expressionProgram(nowProcedure);
			if (expression == nullNode) return errorParsing(beginIndex);

			Node now = Node("condition", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(Node("odd", nodeCnt++, oddIndex, oddIndex));
			now.insertSon(expression);

			generator.genOPR("odd");

			generator.popIndex();
			return now;
		} else {
			wordIndex--;
			Node expressionA = expressionProgram(nowProcedure);
			if (expressionA == nullNode) return errorParsing(beginIndex);

			Node relation = relationProgram(nowProcedure);
			if (relation == nullNode) return errorParsing(beginIndex);

			Node expressionB = expressionProgram(nowProcedure);
			if (expressionB == nullNode) return errorParsing(beginIndex);

			generator.genOPR(relation.name);

			Node now = Node("condition", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(expressionA);
			now.insertSon(relation);
			now.insertSon(expressionB);
			generator.popIndex();
			return now;
		}
	}

	Node mulDivProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;
		auto tmpToken = words[++wordIndex].token;

		if (tmpToken == "*" or tmpToken == "/") {
			generator.popIndex();
			return Node(tmpToken, nodeCnt++, wordIndex, wordIndex);
		}

		return errorParsing(beginIndex, wordIndex, 2);
	}

	Node factorProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		Node identifier = identifierProgram(nowProcedure, 1, 1);
		if (identifier == nullNode) {
			Node unsignedInt = unsignedIntProgram(nowProcedure);
			if (unsignedInt == nullNode) {
				if (words[++wordIndex].token != "(") return errorParsing(beginIndex, wordIndex, 3); // BASE ERROR
				int lBraIndex = wordIndex;

				Node expression = expressionProgram(nowProcedure);
				if (expression == nullNode) return errorParsing(beginIndex);

				if (words[++wordIndex].token != ")") return errorParsing(beginIndex, wordIndex, 4); // BASE ERROR

				Node now = Node("factor", nodeCnt++, beginIndex, wordIndex);
				now.insertSon(Node("(", nodeCnt++, lBraIndex, lBraIndex));
				now.insertSon(expression);
				now.insertSon(Node(")", nodeCnt++, wordIndex, wordIndex));

				generator.popIndex();
				return now;
			} else {
				Node now = Node("factor", nodeCnt++, beginIndex, wordIndex);
				now.insertSon(unsignedInt);

				generator.genLIT(stringToInt(unsignedInt.son[0].name));

				generator.popIndex();
				return now;
			}
		} else {
			Node now = Node("factor", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(identifier);

			int identifierLevel, identifierAdr;
			if (findLevelAdr(identifier.name, nowProcedure, identifierLevel, identifierAdr) == -1) {
				int val = 0;
				if (findConst(identifier.name, nowProcedure, val) == -1) {
					return errorParsing(beginIndex, wordIndex, 34);
				} else {
					generator.genLIT(val);
				}
			} else {
				generator.genLOD(identifierLevel, identifierAdr);
			}

			generator.popIndex();
			return now;
		}
	}

	Node itemProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		Node factor = factorProgram(nowProcedure);
		if (factor == nullNode) return errorParsing(beginIndex);

		Node now = Node("item", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(factor);

		while (true) {
			int loopIndex = wordIndex;

			Node operation = mulDivProgram(nowProcedure);
			if (operation == nullNode) {
				wordIndex = loopIndex;
				break;
			}

			factor = factorProgram(nowProcedure);
			if (factor == nullNode) return errorParsing(beginIndex);
			generator.genOPR(operation.name);

			now.insertSon(operation);
			now.insertSon(factor);
			now.to = wordIndex;
		}

		generator.popIndex();
		return now;
	}

	Node addSubProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;
		auto tmpToken = words[++wordIndex].token;

		if (tmpToken == "+" or tmpToken == "-") {
			generator.popIndex();
			return Node(tmpToken, nodeCnt++, wordIndex, wordIndex);
		}

		return errorParsing(beginIndex, wordIndex, 5);
	}

	Node expressionProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		Node flag = addSubProgram(nowProcedure);

		Node item = itemProgram(nowProcedure);
		if (item == nullNode) return errorParsing(beginIndex);

		Node now = Node("expression", nodeCnt++, beginIndex, wordIndex);
		if (flag != nullNode) now.insertSon(flag);
		now.insertSon(item);

		if (flag != nullNode) {
			generator.genOPR("flag");
		}

		while (true) {
			int loopIndex = wordIndex;
			flag = addSubProgram(nowProcedure);
			if (flag == nullNode) {
				wordIndex = loopIndex;
				break;
			}

			item = itemProgram(nowProcedure);
			if (item == nullNode) return errorParsing(beginIndex);
			generator.genOPR(flag.name);

			now.insertSon(flag);
			now.insertSon(item);
			now.to = wordIndex;
		}

		generator.popIndex();
		return now;
	}

	Node emptyProgram() {
		return Node("empty", nodeCnt++, 0, 0);
	}

	Node complexProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].token != "begin") return errorParsing(beginIndex, wordIndex, 6); // BASE ERROR
		int beginChIndex = wordIndex;

		Node sentence = sentenceProgram(nowProcedure);
		if (sentence == nullNode) return errorParsing(beginIndex);

		Node now = Node("complex", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(Node("begin", nodeCnt++, beginChIndex, beginChIndex)); // BASE ERROR
		now.insertSon(sentence);

		int loopIndex;
		while (true) {
			loopIndex = wordIndex;

			if (words[++wordIndex].token != ";") {
				wordIndex = loopIndex;
				break;
			} else {
				sentence = sentenceProgram(nowProcedure);
				if (sentence == nullNode) return errorParsing(beginIndex);
				now.insertSon(Node(";", nodeCnt++, loopIndex + 1, loopIndex + 1));
				now.insertSon(sentence);
				now.to = wordIndex;
			}
		}

		if (words[++wordIndex].token != "end") return errorParsing(beginIndex, wordIndex, 7); // BASE ERROR
		now.insertSon(Node("end", nodeCnt++, wordIndex, wordIndex));

		generator.popIndex();
		return now;
	}

	Node writeProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].token != "write") return errorParsing(beginIndex, wordIndex, 8); // BASE ERROR
		int writeIndex = wordIndex;

		if (words[++wordIndex].token != "(") return errorParsing(beginIndex, wordIndex, 9); // BASE ERROR
		int lBraIndex = wordIndex;

		Node expression = expressionProgram(nowProcedure);
		if (expression == nullNode) return errorParsing(beginIndex);

		Node now = Node("writeSentence", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(Node("write", nodeCnt++, writeIndex, writeIndex));
		now.insertSon(Node("(", nodeCnt++, lBraIndex, lBraIndex));
		now.insertSon(expression);

		generator.genOPT();

		int loopIndex;
		while (true) {
			loopIndex = wordIndex;
			if (words[++wordIndex].token != ",") {
				wordIndex = loopIndex;
				break;
			} else {
				expression = expressionProgram(nowProcedure);
				if (expression == nullNode) return errorParsing(beginIndex);
				now.insertSon(Node(",", nodeCnt++, loopIndex + 1, loopIndex + 1));
				now.insertSon(expression);

				generator.genOPT();
			}
		}

		if (words[++wordIndex].token != ")") return errorParsing(beginIndex, wordIndex, 10); // BASE ERROR
		now.insertSon(Node(")", nodeCnt++, wordIndex, wordIndex));

		generator.popIndex();
		return now;
	}

	Node readProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].token != "read") return errorParsing(beginIndex, wordIndex, 11); // BASE ERROR
		int readIndex = wordIndex;

		if (words[++wordIndex].token != "(") return errorParsing(beginIndex, wordIndex, 12); // BASE ERROR
		int lBraIndex = wordIndex;

		Node identifier = identifierProgram(nowProcedure);
		if (identifier == nullNode) return errorParsing(beginIndex);

		Node now = Node("readSentence", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(Node("read", nodeCnt++, readIndex, readIndex));
		now.insertSon(Node("(", nodeCnt++, lBraIndex, lBraIndex));
		now.insertSon(identifier);

		generator.genIPT();
		int identifierLevel, identifierAdr;
		if (findLevelAdr(identifier.name, nowProcedure, identifierLevel, identifierAdr) == -1) {
			return errorParsing(beginIndex, wordIndex, 34);
		}
		generator.genSTO(identifierLevel, identifierAdr);


		int loopIndex;
		while (true) {
			loopIndex = wordIndex;
			if (words[++wordIndex].token != ",") {
				wordIndex = loopIndex;
				break;
			} else {
				identifier = expressionProgram(nowProcedure);
				if (identifier == nullNode) return errorParsing(beginIndex);
				now.insertSon(Node(",", nodeCnt++, loopIndex + 1, loopIndex + 1));
				now.insertSon(identifier);

				generator.genIPT();
				int identifierLevel, identifierAdr;
				if (findLevelAdr(identifier.name, nowProcedure, identifierLevel, identifierAdr) == -1) {
					return errorParsing(beginIndex, wordIndex, 34);
				}
				generator.genSTO(identifierLevel, identifierAdr);
			}
		}

		if (words[++wordIndex].token != ")") return errorParsing(beginIndex, wordIndex, 13); // BASE ERROR
		now.insertSon(Node(")", nodeCnt++, wordIndex, wordIndex));

		generator.popIndex();
		return now;
	}

	Node callProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].token != "call") return errorParsing(beginIndex, wordIndex, 14); // BASE ERROR
		int callIndex = wordIndex;

		Node identifier = identifierProgram(nowProcedure, 1, 2);
		if (identifier == nullNode) return errorParsing(beginIndex);

		Node now = Node("callSentence", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(Node("call", nodeCnt++, callIndex, callIndex));
		now.insertSon(identifier);

		int identifierLevel, identifierOff;
		if (findProcedure(identifier.name, nowProcedure, identifierLevel, identifierOff) == -1) {
			return errorParsing(beginIndex, wordIndex, 34);
		}
		generator.genCAL(identifierLevel, identifierOff);

		generator.popIndex();
		return now;
	}

	Node whileProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].token != "while") return errorParsing(beginIndex, wordIndex, 15); // BASE ERROR
		int whileIndex = wordIndex;

		int conditionGeneration = generator.getIndex();

		Node condition = conditionProgram(nowProcedure);
		if (condition == nullNode) return errorParsing(beginIndex);

		int judgeGeneration = generator.genJPC();

		if (words[++wordIndex].token != "do") return errorParsing(beginIndex, wordIndex, 16); //BASE ERROR
		int doIndex = wordIndex;

		Node sentence = sentenceProgram(nowProcedure);
		if (sentence == nullNode) return errorParsing(beginIndex);

		Node now = Node("whileSentence", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(Node("while", nodeCnt++, whileIndex, whileIndex));
		now.insertSon(condition);
		now.insertSon(Node("do", nodeCnt++, doIndex, wordIndex));
		now.insertSon(sentence);

		generator.genJMP(conditionGeneration + 1);
		generator.modifyA(judgeGeneration, generator.getIndex() + 1);

		generator.popIndex();
		return now;
	}

	Node conditionSentenceProgram(const int &nowProcedure) {
		generator.pushIndex();

		int beginIndex = wordIndex;

		if (words[++wordIndex].token != "if") return errorParsing(beginIndex, wordIndex, 17); // BASE ERROR
		int ifIndex = wordIndex;

		Node condition = conditionProgram(nowProcedure);
		if (condition == nullNode) return errorParsing(beginIndex);

		int judgeGeneration = generator.genJPC();

		if (words[++wordIndex].token != "then") return errorParsing(beginIndex, wordIndex, 18); // BASE ERROR
		int thenIndex = wordIndex;

		Node sentence = sentenceProgram(nowProcedure);
		if (sentence == nullNode) return errorParsing(beginIndex);

		Node now = Node("conditionSentence", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(Node("if", nodeCnt++, ifIndex, ifIndex));
		now.insertSon(condition);
		now.insertSon(Node("then", nodeCnt++, thenIndex, thenIndex));
		now.insertSon(sentence);

		generator.modifyA(judgeGeneration, generator.getIndex() + 1);
		generator.popIndex();
		return now;
	}

	Node assignProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		Node identifier = identifierProgram(nowProcedure, 1, 1);
		if (identifier == nullNode) return errorParsing(beginIndex);

		if (words[++wordIndex].token != ":=") return errorParsing(beginIndex, wordIndex, 19); // BASE ERROR
		int assignIndex = wordIndex;

		Node expression = expressionProgram(nowProcedure);
		if (expression == nullNode) return errorParsing(beginIndex);

		Node now = Node("assignSentence", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(identifier);
		now.insertSon(Node(":=", nodeCnt++, assignIndex, assignIndex));
		now.insertSon(expression);

		int identifierLevel, identifierAdr;
		if (findLevelAdr(identifier.name, nowProcedure, identifierLevel, identifierAdr) == -1) {
			return errorParsing(beginIndex, wordIndex, 34);
		}
		generator.genSTO(identifierLevel, identifierAdr);

		generator.popIndex();
		return now;
	}

	Node sentenceProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		Node assignSentence = assignProgram(nowProcedure);
		if (assignSentence != nullNode) {
			Node now = Node("sentence", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(assignSentence);
			generator.popIndex();
			return now;
		} else {
			wordIndex = beginIndex;
		}

		Node conditionSentence = conditionSentenceProgram(nowProcedure);
		if (conditionSentence != nullNode) {
			Node now = Node("sentence", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(conditionSentence);
			generator.popIndex();
			return now;
		} else {
			wordIndex = beginIndex;
		}

		Node whileSentence = whileProgram(nowProcedure);
		if (whileSentence != nullNode) {
			Node now = Node("sentence", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(whileSentence);
			generator.popIndex();
			return now;
		} else {
			wordIndex = beginIndex;
		}

		Node callSentence = callProgram(nowProcedure);
		if (callSentence != nullNode) {
			Node now = Node("sentence", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(callSentence);
			generator.popIndex();
			return now;
		} else {
			wordIndex = beginIndex;
		}

		Node readSentence = readProgram(nowProcedure);
		if (readSentence != nullNode) {
			Node now = Node("sentence", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(readSentence);
			generator.popIndex();
			return now;
		} else {
			wordIndex = beginIndex;
		}

		Node writeSentence = writeProgram(nowProcedure);
		if (writeSentence != nullNode) {
			Node now = Node("sentence", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(writeSentence);
			generator.popIndex();
			return now;
		} else {
			wordIndex = beginIndex;
		}

		Node complexSentence = complexProgram(nowProcedure);
		if (complexSentence != nullNode) {
			Node now = Node("sentence", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(complexSentence);
			generator.popIndex();
			return now;
		} else {
			wordIndex = beginIndex;
		}

		Node emptySentence = emptyProgram();
		if (emptySentence != nullNode) {
			Node now = Node("sentence", nodeCnt++, beginIndex, wordIndex);
			now.insertSon(emptySentence);
			generator.popIndex();
			return now;
		} else {
			wordIndex = beginIndex;
		}

		return errorParsing(beginIndex);
	}

	Node processHeadProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].token != "procedure") return errorParsing(beginIndex, wordIndex, 20); // BASE ERROR
		int procedureIndex = beginIndex;

		Node identifier = identifierProgram(nowProcedure, 2);
		if (identifier == nullNode) return errorParsing(beginIndex);

		if (words[++wordIndex].token != ";") return errorParsing(beginIndex, wordIndex, 21); // BASE ERROR

		Node now = Node("processHead", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(Node("procedure", nodeCnt++, procedureIndex, procedureIndex));
		now.insertSon(identifier);
		now.insertSon(Node(";", nodeCnt++, wordIndex, wordIndex));

		generator.popIndex();
		return now;
	}

	Node processDesProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (procedureTable[nowProcedure].level == 3) return errorParsing(beginIndex, wordIndex, 30); // BASE ERROR

		Node processHead = processHeadProgram(nowProcedure);
		if (processHead == nullNode) return errorParsing(beginIndex);

		int beginProcedureCnt = procedureCnt;
		addProcedure(nowProcedure, processHead.son[1].name);
		Node subProcess = subProgram(procedureCnt - 1, processHead.son[1].name);
		if (subProcess == nullNode) {
			procedureCnt = beginProcedureCnt;
			return errorParsing(beginIndex);
		}

		if (words[++wordIndex].token != ";") return errorParsing(beginIndex, wordIndex, 22); // BASE ERROR

		Node now = Node("processDes", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(processHead);
		now.insertSon(subProcess);
		now.insertSon(Node(";", nodeCnt++, wordIndex, wordIndex));

		while (true) {
			Node nextProcessDes = processDesProgram(nowProcedure);
			if (nextProcessDes == nullNode) {
				generator.popIndex();
				return now;
			}

			now.insertSon(nextProcessDes);
			now.to = wordIndex;
		}
	}

	Node variableDesProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].token != "var") return errorParsing(beginIndex, wordIndex, 23);
		int varIndex = wordIndex;

		Node varIdentifier = identifierProgram(nowProcedure, 2);
		if (varIdentifier == nullNode) return errorParsing(beginIndex);

		Node now = Node("varDescription", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(Node("var", nodeCnt++, varIndex, varIndex));
		while (true) {
			wordIndex++;
			if (words[wordIndex].token == ";") {
				now.to = wordIndex;
				now.insertSon(varIdentifier);
				now.insertSon(Node(";", nodeCnt++, wordIndex, wordIndex));
				searchVariable(now, nowProcedure);

				generator.popIndex();
				return now;
			} else if (words[wordIndex].token == ",") {
				now.to = wordIndex;
				now.insertSon(varIdentifier);
				now.insertSon(Node(",", nodeCnt++, wordIndex, wordIndex));

				varIdentifier = identifierProgram(nowProcedure, 2);
				if (varIdentifier == nullNode) return errorParsing(beginIndex);
			} else return errorParsing(beginIndex, wordIndex, 24); // BASE ERROR
		}
	}

	Node identifierProgram(const int &nowProcedure, const int &checkTable=0, const int &checkKind=1) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].symbol == parsing.identifierBase) {
			if (checkTable == 1) {
				if (checkKind == 1) {
					if (findDescription(words[wordIndex].token, nowProcedure, 0) == -1 and
					    findDescription(words[wordIndex].token, nowProcedure, 1) == -1)
						return errorParsing(beginIndex, wordIndex, 31);
				} else {
					if (findDescription(words[wordIndex].token, nowProcedure, 2) == -1)
						return errorParsing(beginIndex, wordIndex, 32);
				}
			} else if (checkTable == 2) {
				if (checkDescription(words[wordIndex].token, nowProcedure, 0) != -1)
					return errorParsing(beginIndex, wordIndex, 33);
				if (checkDescription(words[wordIndex].token, nowProcedure, 1) != -1)
					return errorParsing(beginIndex, wordIndex, 33);
				if (checkDescription(words[wordIndex].token, nowProcedure, 2) != -1)
					return errorParsing(beginIndex, wordIndex, 33);
			}

			generator.popIndex();
			return Node(words[wordIndex].token, nodeCnt++, wordIndex, wordIndex);
		}
		return errorParsing(beginIndex, wordIndex, 25); // BASE ERROR
	}

	Node numberProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].symbol == parsing.constBase) {
			generator.popIndex();
			return Node(words[wordIndex].token, nodeCnt++, wordIndex, wordIndex);
		}

		return errorParsing(beginIndex, wordIndex, 26); // BASE ERROR
	}

	Node unsignedIntProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		Node number = numberProgram(nowProcedure);
		if (number == nullNode) return errorParsing(beginIndex);

		Node now = Node("unsignedInteger", nodeCnt++, beginIndex, wordIndex);
		while (true) {
			now.insertSon(number);
			now.to = wordIndex;
			int lastIndex = wordIndex;

			number = numberProgram(nowProcedure);
			if (number == nullNode) {
				wordIndex = lastIndex;
				generator.popIndex();
				return now;
			}
		}
	}

	Node constDefProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		Node identifier = identifierProgram(nowProcedure, 2);
		if (identifier == nullNode) return errorParsing(beginIndex);

		if (words[++wordIndex].token != "=") return errorParsing(beginIndex, wordIndex, 27);

		Node unsignedInt = unsignedIntProgram(nowProcedure);
		if (unsignedInt == nullNode) return errorParsing(beginIndex);

		Node now = Node("constDefine", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(identifier);
		now.insertSon(Node("=", nodeCnt++, wordIndex, wordIndex));
		now.insertSon(unsignedInt);
		generator.popIndex();
		return now;
	}

	Node constDesProgram(const int &nowProcedure) {
		generator.pushIndex();
		int beginIndex = wordIndex;

		if (words[++wordIndex].token != "const") return errorParsing(beginIndex, wordIndex, 28); // BASE ERROR
		int constIndex = wordIndex;

		Node constDef = constDefProgram(nowProcedure);
		if (constDef == nullNode) return errorParsing(beginIndex);

		Node now = Node("constDes", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(Node("const", nodeCnt++, constIndex, constIndex));
		while (true) {
			wordIndex++;
			if (words[wordIndex].token == ";") {
				now.to = wordIndex;
				now.insertSon(constDef);
				now.insertSon(Node(";", nodeCnt++, wordIndex, wordIndex));

				searchConst(now, nowProcedure);
				generator.popIndex();
				return now;
			} else if (words[wordIndex].token == ",") {
				now.to = wordIndex;
				now.insertSon(constDef);
				now.insertSon(Node(",", nodeCnt++, wordIndex, wordIndex));

				constDef = constDefProgram(nowProcedure);
				if (constDef == nullNode) return errorParsing(beginIndex);
			} else return errorParsing(beginIndex, wordIndex, 29); // BASE ERROR
		}
	}

	Node subProgram(const int &nowProcedure, const std::string &name) {
		generator.pushIndex();
		int beginIndex = wordIndex;
		Node constDes = nullNode;
		Node variableDes = nullNode;
		Node processDes = nullNode;
		Node sentence = nullNode;

		int dataSize = 0;

		constDes = constDesProgram(nowProcedure);
		if (constDes != nullNode) dataSize += countConst(constDes);
		variableDes = variableDesProgram(nowProcedure);
		if (variableDes != nullNode) dataSize += countVar(variableDes);

		processDes = processDesProgram(nowProcedure);

		int intIndex = generator.genINT();
		generator.modifyA(intIndex, dataSize);
		if (nowProcedure != 0) procedureTable[procedureTable[nowProcedure].fa].modifyVal(name, intIndex, 2, 1);

		sentence = sentenceProgram(nowProcedure);
		if (sentence == nullNode) return errorParsing(beginIndex);

		Node now = Node("subProgram", nodeCnt++, beginIndex, wordIndex);
		if (constDes != nullNode) now.insertSon(constDes);
		if (variableDes != nullNode) now.insertSon(variableDes);
		if (processDes != nullNode) now.insertSon(processDes);
		now.insertSon(sentence);

		generator.genRET();
		generator.popIndex();

		mainAdr = intIndex;
		return now;
	}

	Node mainProgram() {
		generator.pushIndex();
		int beginIndex = wordIndex;
		addProcedure(-1, "main");

		Node subSon = subProgram(0, "main");
		if (subSon == nullNode) return errorParsing(beginIndex);

		if (words[++wordIndex].token != ".") return errorParsing(beginIndex); // BASE ERROR

		Node now = Node("main", nodeCnt++, beginIndex, wordIndex);
		now.insertSon(subSon);
		now.insertSon(Node(".", nodeCnt++, wordIndex, wordIndex));

		lastErrorIndex = -1;
		lastErrorCode = -1;
		generator.popIndex();
		return now;
	}

	void dfsGraph(Node &now, bool gv=false) {
		if (not gv and now.name == ":=") {
			now.name = "_=";
		}
		std::string tmp;
		if (not gv) {
			tmp += "dot.node('";
			tmp += now.name + "_" + std::to_string(now.id);
			tmp += "', '";
			tmp += now.name + "_" + std::to_string(now.id);
			tmp += "')\n";
		} else {
			tmp += "\t";
			tmp += "\"" + std::to_string(now.id) + "\"";
			tmp += " ";
			tmp += "[label=\"" + now.name + "\"]\n";
		}
		std::cout << tmp;
		for (auto &item : now.son) {
			dfsGraph(item, gv);
		}
	}

	void dfsEdge(const Node &now, bool gv=false) {
		for (const auto &item : now.son) {
			std::string tmp;
			if (not gv) {
				tmp += "dot.edge('";
				tmp += now.name + "_" + std::to_string(now.id);
				tmp += "', '";
				tmp += item.name + "_" + std::to_string(item.id);
				tmp += "')\n";
			} else {
				tmp += "\t";
				tmp += "\"" + std::to_string(now.id) + "\"";
				tmp += " -> ";
				tmp += "\"" + std::to_string(item.id) + "\"\n";
			}

			std::cout << tmp;
			dfsEdge(item, gv);
		}
	}

	void outputGraph(bool gv=false) {
		dfsGraph(root, gv);
		dfsEdge(root, gv);
	}

	void drawTreePython() {
		if (lastErrorIndex != lastErrorCode or lastErrorIndex != -1) return;

		auto outputStream = freopen("draw.py", "w", stdout);

		std::cout << "from graphviz import Digraph" << std::endl;
		std::cout << "dot = Digraph(comment='Lexical', format=\"png\")" << std::endl;
		outputGraph();
		std::cout << "dot.render('Lexical', view=True)\n";

		fflush(outputStream);
		freopen("CON", "w", stdout);
		std::cout << "draw.py ready" << std::endl;

		// system("python draw.py");
	}

	void drawTree() {
		if (lastErrorIndex != lastErrorCode or lastErrorIndex != -1) return;

		auto outputStream = freopen("Lexical.gv", "w", stdout);

		std::cout << "// Lexical" << std::endl;
		std::cout << "digraph {" << std::endl;
		outputGraph(true);
		std::cout << "}\n";

		fflush(outputStream);
		freopen("CON", "w", stdout);
		std::cout << "Lexical.gv ready" << std::endl;

		system(R"(.\bin\dot.exe .\Lexical.gv -Gdpi=100 -Tpng -o .\Lexical.png )");
		system(R"(start .\Lexical.png)");
	}

	void dfs(const Node &now, int de=0) {
		for (int i = 0; i < de; i++) {
			std::cout << "-";
		}
		std::cout << now.name << ":" << now.id << std::endl;

		for (const auto &item : now.son) {
			dfs(item, de + 1);
		}
	}

	void output() {
		if (lastErrorIndex != -1) {
			std::cout << lastErrorCode << ":" << lastErrorIndex << "---" << words[lastErrorIndex].line << std::endl;
		}
		dfs(root);
	}

	void outputNameTable() {
		for (auto & nowProcedure : procedureTable) {
			std::cout << nowProcedure.pName << std::endl;
			for (auto & nowDescription : nowProcedure.nameTable) {
					std::cout << "NAME:" << nowDescription.name << "\t";
				if (nowDescription.kind == 0) {
					std::cout << "KIND:" << "CONSTANT" << "\t";
					std::cout << "VAL:" << nowDescription.val << std::endl;
				} else if (nowDescription.kind == 1) {
					std::cout << "KIND:" << "VARIABLE" << "\t";
					std::cout << "LEVEL:" << nowProcedure.level << "\t";
					std::cout << "ADR:" << nowDescription.adr << std::endl;
				} else if (nowDescription.kind == 2) {
					std::cout << "KIND:" << "PROCEDURE" << "\t";
					std::cout << "LEVEL:" << nowProcedure.level << "\t";
					std::cout << "ADR:" << nowDescription.adr << std::endl;
				}
			}
		}
	}

	void outputCode() {
		generator.outputCode();
	}

	std::vector<InterCode> getInterCode() {
		return generator.interCode;
	}
};


#endif //COMPILE_LEXICAL_H
