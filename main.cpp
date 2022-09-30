#include <iostream>
#include <cctype>
#include <fstream>
#include <vector>

using namespace std;

enum State {
	StartST, WordST, ConstST, OperST, ErrST,
	EndOfWordST, EndOfConstST, EndOfOperST, EndOfErrST,
	FinalST, FinalWordST, FinalConstST, FinalOperST, FinalErrST,
	ConstToOperST, OperToWordST, OperToConstST, WordToOperST, LongOperST
};
enum Symbol {spaceSym, alphaSym, digitSym, specialSym, finalSym};
enum LexemType {keyWord, specialSymbol, idendificator, constant, error};

struct Lexem
{
	char* lexemText;
	LexemType lType;
	Lexem* next;
};

// create structure to operate with text from file (my string class)
struct DynamicText
{
	char* text = new char('\0');
	int len = 1; //length of the text
	int mem = 1; //memory for text

	int addSym(int pos, char sym)
	{
		if (pos + 1 >= mem)
		{
			mem += 8;
			char* newText = new char[mem];

			int i = -1;
			while (len - (++i)) newText[i] = text[i];
			char* buff = text;  // for delete without memory lost
			text = newText;
			delete[] buff;
		}
		if (text[pos] == '\0')
		{
			text[pos] = sym;
			text[pos + 1] = '\0';
			++len;
		}
		else text[pos] = sym;
		return 0;
	}
};

// overloading "<<" for work with my text class
ostream& operator << (ofstream& outFile, const DynamicText& dymText)
{
	if (dymText.len > 0) return outFile << dymText.text;
	return outFile << "";
}

// functions-identifiers of some symbols
bool isSpace(char sym){if(sym == ' ' || sym == '\t' || sym == '\n') return true; return false;}
bool isSpec(char sym){if(sym == '=' || sym == '<' || sym == '>' || sym == '+' || sym == '-') return true; return false;}
bool isFinal(char sym){if(sym == '\0') return true;	return false;}

// I hope it is clear from the name of function
Symbol whatSym(char sym)
{
	if (isSpace(sym)) return spaceSym;        // my
	if (isalpha(sym)) return alphaSym;    // not my
	if (isdigit(sym)) return digitSym;   // not my
	if (isSpec(sym)) return specialSym;    // my
	if (isFinal(sym)) return finalSym;    // my
	return spaceSym;
}

// This must Work!
vector <Lexem> lexemsFromFile(char filename[])
{
	ifstream file(filename);
	int n = 0;                      // Number of characters
	DynamicText text;               // text from file

    // writing from file to text
	char sym = file.get();
	while (sym != EOF) { text.addSym(n, sym); sym = file.get(); ++n; }

    // Automate's states table (not in an external function for reliability)
	State tableOfStates[14][5]{};

	tableOfStates[StartST][spaceSym] = StartST;
	tableOfStates[StartST][alphaSym] = WordST;
	tableOfStates[StartST][digitSym] = ConstST;
	tableOfStates[StartST][specialSym] = OperST;
	tableOfStates[StartST][finalSym] = FinalST;

	tableOfStates[WordST][spaceSym] = EndOfWordST;
	tableOfStates[WordST][alphaSym] = WordST;
	tableOfStates[WordST][digitSym] = WordST;
	tableOfStates[WordST][specialSym] = WordToOperST;
	tableOfStates[WordST][finalSym] = FinalWordST;

	tableOfStates[ConstST][spaceSym] = EndOfConstST;
	tableOfStates[ConstST][alphaSym] = ErrST;
	tableOfStates[ConstST][digitSym] = ConstST;
	tableOfStates[ConstST][specialSym] = ConstToOperST;
	tableOfStates[ConstST][finalSym] = FinalConstST;

	tableOfStates[OperST][spaceSym] = EndOfOperST;
	tableOfStates[OperST][alphaSym] = OperToWordST;
	tableOfStates[OperST][digitSym] = OperToConstST;
	tableOfStates[OperST][specialSym] = LongOperST;
	tableOfStates[OperST][finalSym] = FinalOperST;

	tableOfStates[ErrST][spaceSym] = EndOfErrST;
	tableOfStates[ErrST][alphaSym] = ErrST;
	tableOfStates[ErrST][digitSym] = ErrST;
	tableOfStates[ErrST][specialSym] = ErrST;
	tableOfStates[ErrST][finalSym] = FinalErrST;


	tableOfStates[WordToOperST][spaceSym] = EndOfOperST;
	tableOfStates[WordToOperST][alphaSym] = OperToWordST;
	tableOfStates[WordToOperST][digitSym] = OperToConstST;
	tableOfStates[WordToOperST][specialSym] = LongOperST;
	tableOfStates[WordToOperST][finalSym] = FinalOperST;

	tableOfStates[ConstToOperST][spaceSym] = EndOfOperST;
	tableOfStates[ConstToOperST][alphaSym] = OperToWordST;
	tableOfStates[ConstToOperST][digitSym] = OperToConstST;
	tableOfStates[ConstToOperST][specialSym] = LongOperST;
	tableOfStates[ConstToOperST][finalSym] = FinalOperST;

	tableOfStates[OperToWordST][spaceSym] = EndOfWordST;
	tableOfStates[OperToWordST][alphaSym] = WordST;
	tableOfStates[OperToWordST][digitSym] = WordST;
	tableOfStates[OperToWordST][specialSym] = OperST;
	tableOfStates[OperToWordST][finalSym] = FinalWordST;

	tableOfStates[OperToConstST][spaceSym] = EndOfConstST;
	tableOfStates[OperToConstST][alphaSym] = ErrST;
	tableOfStates[OperToConstST][digitSym] = ConstST;
	tableOfStates[OperToConstST][specialSym] = ConstToOperST;
	tableOfStates[OperToConstST][finalSym] = FinalConstST;

	tableOfStates[LongOperST][spaceSym] = EndOfOperST;
	tableOfStates[LongOperST][alphaSym] = OperToWordST;
	tableOfStates[LongOperST][digitSym] = OperToConstST;
	tableOfStates[LongOperST][specialSym] = ErrST;
	tableOfStates[LongOperST][finalSym] = FinalOperST;


	tableOfStates[EndOfWordST][spaceSym] = StartST;
	tableOfStates[EndOfWordST][alphaSym] = WordST;
	tableOfStates[EndOfWordST][digitSym] = ConstST;
	tableOfStates[EndOfWordST][specialSym] = OperST;
	tableOfStates[EndOfWordST][finalSym] = FinalST;

	tableOfStates[EndOfConstST][spaceSym] = StartST;
	tableOfStates[EndOfConstST][alphaSym] = WordST;
	tableOfStates[EndOfConstST][digitSym] = ConstST;
	tableOfStates[EndOfConstST][specialSym] = OperST;
	tableOfStates[EndOfConstST][finalSym] = FinalST;

	tableOfStates[EndOfOperST][spaceSym] = StartST;
	tableOfStates[EndOfOperST][alphaSym] = WordST;
	tableOfStates[EndOfOperST][digitSym] = ConstST;
	tableOfStates[EndOfOperST][specialSym] = OperST;
	tableOfStates[EndOfOperST][finalSym] = FinalST;

	tableOfStates[EndOfErrST][spaceSym] = StartST;
	tableOfStates[EndOfErrST][alphaSym] = WordST;
	tableOfStates[EndOfErrST][digitSym] = ConstST;
	tableOfStates[EndOfErrST][specialSym] = OperST;
	tableOfStates[EndOfErrST][finalSym] = FinalST;


    // The True Beginning of this function
	vector <Lexem> resultLexems;

	State currentState = StartST;
	int pos = 0, lexemLen = 0;

	while (currentState != FinalST)
	{
		currentState = tableOfStates[currentState][whatSym(text.text[pos])];

		if (currentState == WordST) ++lexemLen;
		if (currentState == ConstST) ++lexemLen;

		if (currentState == EndOfWordST)
		{
			Lexem currentLexem{};

			currentLexem.lexemText = new char[lexemLen + 1];
			int lexPos = -1;

			do currentLexem.lexemText[++lexPos] = text.text[pos - lexemLen]; while (--lexemLen);
			currentLexem.lexemText[++lexPos] = '\0';

			resultLexems.push_back(currentLexem);
			lexemLen = 0;
		}
		if (currentState == EndOfWordST)
		{
			Lexem currentLexem{};

			currentLexem.lexemText = new char[lexemLen + 1];
			int lexPos = -1;

			do currentLexem.lexemText[++lexPos] = text.text[pos - lexemLen]; while (--lexemLen);
			currentLexem.lexemText[++lexPos] = '\0';

			resultLexems.push_back(currentLexem);
			lexemLen = 0;
		}
		if (currentState == FinalST)
		{
			if (lexemLen > 0)
			{
				Lexem currentLexem{};

				currentLexem.lexemText = new char[lexemLen + 1];
				int lexPos = -1;

				do currentLexem.lexemText[++lexPos] = text.text[pos - lexemLen]; while (--lexemLen);
				currentLexem.lexemText[++lexPos] = '\0';

				resultLexems.push_back(currentLexem);
			}
		}
		++pos;
	}
	return resultLexems;
}

/*
vector <char*> lexemAnalyzator()
{
	return 0;
}
*/

//int main()
{
	char inputFilename[] = "input.txt";
	char outputFilename[] = "output.txt";

	vector <Lexem> lexems;
	lexems = lexemsFromFile(inputFilename);

	int i = 0;
	return 0;
}
