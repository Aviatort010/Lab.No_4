#include <iostream>
#include <cctype>
#include <cstring>
#include <fstream>
#include <vector>

using namespace std;


enum State {
    StartST, WordST, ConstST, OperST, ErrST,
    WordToOperST, ConstToOperST, OperToWordST, OperToConstST,
    LongOperST,
    EndOfWordST, EndOfConstST, EndOfOperST, EndOfErrST,
    FinalST, FinalWordST, FinalConstST, FinalOperST, FinalErrST,
};

enum LT { id, ao, co, vl, eq, Doj, lp, un, wl, Er };
enum  Ast { S, A, B, C, D, E, G, H, N, K, J, idA, aoA, coA, vlA, eqA, doA, lpA, unA, wlA };
const int parser_matr[9][9]{
        {Er, B,	 Er, D,	 B, Er, H,	 Er, J },
        {Er, Er, Er, Er, C,	 Er, Er, Er, Er},
        {Er, Er, Er, Er, Er, Er, Er, N,	 Er},
        {Er, Er, Er, D,	 Er, Er, H,	 Er, J },
        {Er, Er, C,	 Er, Er, Er, Er, Er, Er},
        {A,	 Er, Er, Er, Er, Er, Er, Er, Er},
        {Er, Er, Er, Er, E,	 Er, Er, Er, Er},
        {Er, Er, Er, Er, Er, G,	 Er, Er, Er},
        {Er, Er, Er, Er, Er, Er, Er, Er, Er},
};
enum Symbol {spaceSym, alphaSym, digitSym, specialSym, finalSym};
enum LexemType {Do, Lp, Un, En, Co, Eq, Id, Vl, Ao, Lo, No, Sc, Wl};
char lex_name[9][3]{ "id", "ao", "co", "vl", "eq", "do", "lp", "un", "wl" };

struct Lexem
{
    char* lexemText;
    LexemType lType;
    size_t index;
};

void file_read(char*& line, char inputfile[]) {
    ifstream fin(inputfile);
    streamoff size = 0;
    fin.seekg(0, ios::end);
    size = fin.tellg();
    fin.seekg(0, ios::beg);
    line = new char[(size_t)size + 1];
    fin.getline(line, size + 1, '\0');
    fin.close();
}

// functions-identifiers of some symbols
bool isSpace(char sym){if(sym == ' ' || sym == '\t' || sym == '\n') return true; return false;}
bool isSpec(char sym){if(sym == '=' || sym == '<' || sym == '>' || sym == '+' || sym == '-') return true; return false;}
bool isFinal(char sym){if(sym == '\0') return true;	return false;}

// I hope it is clear from the name of function
Symbol whatSym(char sym)
{
    if (isSpace(sym)) return spaceSym;        // my
    if (isalpha(sym)) return alphaSym;       // not my
    if (isdigit(sym)) return digitSym;      // not my
    if (isSpec(sym))  return specialSym;   // my
    if (isFinal(sym)) return finalSym;    // my
    return spaceSym;
}

LexemType whatLexemWord(const Lexem& lex)
{
    if (!strcmp(lex.lexemText, "do")) return Do;
    if (!strcmp(lex.lexemText, "loop")) return Lp;
    if (!strcmp(lex.lexemText, "until")) return Un;
    return Id;
}

LexemType whatLexemOper(const Lexem& lex)
{
    if (!strcmp(lex.lexemText, "=")) return Eq;
    if (!strcmp(lex.lexemText, "+")) return Ao;
    if (!strcmp(lex.lexemText, "-")) return Ao;
    if (!strcmp(lex.lexemText, "*")) return Ao;
    if (!strcmp(lex.lexemText, "/")) return Ao;
    if (!strcmp(lex.lexemText, ">")) return Co;
    if (!strcmp(lex.lexemText, "<")) return Co;
    if (!strcmp(lex.lexemText, ">=")) return Co;
    if (!strcmp(lex.lexemText, "<=")) return Co;
    if (!strcmp(lex.lexemText, "<>")) return Co;
    return Wl;
}

// This must Work!
vector <Lexem> lexemsFromFile(char filename[])
{
    int n = 0;                      // Number of characters
    char* text;                     // text from file
    // writing from file to text
    file_read(text, filename);
    cout << text;

    // Automate's states table (not in an external function for reliability)
    int tableOfStates[14][5] = {
            {StartST,       WordST,         ConstST,        OperST,         FinalST},       // 0.StartST
            {EndOfWordST,   WordST,         WordST,         WordToOperST,   FinalWordST},   // 1.WordST
            {EndOfConstST,  ErrST,          ConstST,        ConstToOperST,  FinalConstST},  // 2.ConstST
            {EndOfOperST,   OperToWordST,   OperToConstST,  LongOperST,     FinalOperST},   // 3.OperST
            {EndOfErrST,    ErrST,          ErrST,          ErrST,          FinalErrST},    // 4.ErrST
            {EndOfOperST,   OperToWordST,   OperToConstST,  LongOperST,     FinalOperST},   // 5.WordToOperST
            {EndOfOperST,   OperToWordST,   OperToConstST,  LongOperST,     FinalOperST},   // 6.ConstToOperST
            {EndOfWordST,   WordST,         WordST,         WordToOperST,   FinalWordST},   // 7.OperToWordST
            {EndOfConstST,  ErrST,          ConstST,        ConstToOperST,  FinalConstST},  // 8.OperToConstST
            {EndOfOperST,   OperToWordST,   OperToConstST,  ErrST,          FinalOperST},   // 9.LongOperST
            {StartST,       WordST,         ConstST,        OperST,         FinalST},       // 10.EndOfWordST
            {StartST,       WordST,         ConstST,        OperST,         FinalST},       // 11.EndOfConstST
            {StartST,       WordST,         ConstST,        OperST,         FinalST},       // 12.EndOfOperST
            {StartST,       WordST,         ConstST,        OperST,         FinalST}        // 13.EndOfErrST
    };//{spaceSym,      alphaSym,       digitSym,       specialSym,     finalSym};

    // The True Beginning of this function
    vector <Lexem> resultLexems;

    int currentState = StartST;
    int pos = 0, lexemLen = 0, ind = 0;
    bool run = true;

    while (run)
    {
        Symbol sym = whatSym(text[pos]);
        currentState = tableOfStates[currentState][sym];

        if (currentState == ErrST) ++lexemLen;
        if (currentState == WordST) ++lexemLen;
        if (currentState == OperST) ++lexemLen;
        if (currentState == ConstST) ++lexemLen;
        if (currentState == LongOperST) ++lexemLen;

        if (currentState == WordToOperST)
        {
            Lexem currentLexem{};

            currentLexem.lexemText = new char[lexemLen + 1];
            int lexPos = -1;

            do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
            currentLexem.lexemText[++lexPos] = '\0';
            currentLexem.lType = whatLexemWord(currentLexem);
            currentLexem.index == ind;
            ++ind;

            resultLexems.push_back(currentLexem);
            lexemLen = 1;
        }
        if (currentState == ConstToOperST)
        {
            Lexem currentLexem{};

            currentLexem.lexemText = new char[lexemLen + 1];
            int lexPos = -1;

            do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
            currentLexem.lexemText[++lexPos] = '\0';
            currentLexem.lType = Vl;
            currentLexem.index == ind;
            ++ind;

            resultLexems.push_back(currentLexem);
            lexemLen = 1;
        }
        if (currentState == OperToWordST)
        {
            Lexem currentLexem{};

            currentLexem.lexemText = new char[lexemLen + 1];
            int lexPos = -1;

            do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
            currentLexem.lexemText[++lexPos] = '\0';
            currentLexem.lType = whatLexemOper(currentLexem);
            currentLexem.index == ind;
            ++ind;

            resultLexems.push_back(currentLexem);
            lexemLen = 1;
        }
        if (currentState == OperToConstST)
        {
            Lexem currentLexem{};

            currentLexem.lexemText = new char[lexemLen + 1];
            int lexPos = -1;

            do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
            currentLexem.lexemText[++lexPos] = '\0';
            currentLexem.lType = whatLexemOper(currentLexem);
            currentLexem.index == ind;
            ++ind;

            resultLexems.push_back(currentLexem);
            lexemLen = 1;
        }

        if (currentState == EndOfWordST)
        {
            Lexem currentLexem{};
            currentLexem.lexemText = new char[lexemLen + 1];
            int lexPos = -1;

            do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
            currentLexem.lexemText[++lexPos] = '\0';
            currentLexem.lType = whatLexemWord(currentLexem);
            currentLexem.index == ind;
            ++ind;

            resultLexems.push_back(currentLexem);
            lexemLen = 0;
        }
        if (currentState == EndOfConstST)
        {
            Lexem currentLexem{};
            currentLexem.lexemText = new char[lexemLen + 1];
            int lexPos = -1;

            do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
            currentLexem.lexemText[++lexPos] = '\0';
            currentLexem.lType = Vl;
            currentLexem.index == ind;
            ++ind;

            resultLexems.push_back(currentLexem);
            lexemLen = 0;
        }
        if (currentState == EndOfOperST)
        {
            Lexem currentLexem{};
            currentLexem.lexemText = new char[lexemLen + 1];
            int lexPos = -1;

            do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
            currentLexem.lexemText[++lexPos] = '\0';
            currentLexem.lType = whatLexemOper(currentLexem);
            currentLexem.index == ind;
            ++ind;

            resultLexems.push_back(currentLexem);
            lexemLen = 0;
        }
        if (currentState == EndOfErrST)
        {
            Lexem currentLexem{};
            currentLexem.lexemText = new char[lexemLen + 1];
            int lexPos = -1;

            do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
            currentLexem.lexemText[++lexPos] = '\0';
            currentLexem.lType = Wl;
            currentLexem.index == ind;
            ++ind;

            resultLexems.push_back(currentLexem);
            lexemLen = 0;
        }
        if (currentState == FinalST)
        {
            run = false;
            if (lexemLen > 0)
            {
                Lexem currentLexem{};

                currentLexem.lexemText = new char[lexemLen + 1];
                int lexPos = -1;

                do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
                currentLexem.lexemText[++lexPos] = '\0';

                resultLexems.push_back(currentLexem);
            }
        }
        if (currentState == FinalWordST)
        {
            run = false;
            if (lexemLen > 0)
            {
                Lexem currentLexem{};

                currentLexem.lexemText = new char[lexemLen + 1];
                int lexPos = -1;

                do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
                currentLexem.lexemText[++lexPos] = '\0';
                currentLexem.lType = whatLexemWord(currentLexem);
                currentLexem.index == ind;
                ++ind;

                resultLexems.push_back(currentLexem);
            }
        }
        if (currentState == FinalConstST)
        {
            run = false;
            if (lexemLen > 0)
            {
                Lexem currentLexem{};

                currentLexem.lexemText = new char[lexemLen + 1];
                int lexPos = -1;

                do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
                currentLexem.lexemText[++lexPos] = '\0';
                currentLexem.lType = Vl;
                currentLexem.index == ind;
                ++ind;

                resultLexems.push_back(currentLexem);
            }
        }
        if (currentState == FinalOperST)
        {
            run = false;
            if (lexemLen > 0)
            {
                Lexem currentLexem{};

                currentLexem.lexemText = new char[lexemLen + 1];
                int lexPos = -1;

                do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
                currentLexem.lexemText[++lexPos] = '\0';
                currentLexem.lType = whatLexemOper(currentLexem);
                currentLexem.index == ind;
                ++ind;

                resultLexems.push_back(currentLexem);
            }
        }
        if (currentState == FinalErrST)
        {
            run = false;
            if (lexemLen > 0)
            {
                Lexem currentLexem{};

                currentLexem.lexemText = new char[lexemLen + 1];
                int lexPos = -1;

                do currentLexem.lexemText[++lexPos] = text[pos - lexemLen]; while (--lexemLen);
                currentLexem.lexemText[++lexPos] = '\0';
                currentLexem.lType = Wl;
                currentLexem.index == ind;
                ++ind;

                resultLexems.push_back(currentLexem);
            }
        }
        ++pos;
    }
    return resultLexems;
}


void print_vec(vector<Lexem>& v, ofstream& fout) {
    for (size_t i = 0; i < v.size(); ++i) {
        fout << v[i].lexemText << '[' << lex_name[v[i].lType];
        (i == v.size() - 1) ? fout << ']' : fout << "] ";
    }
}

void expected(int state, Lexem& lex, ofstream& fout, int save) {

    int* arr;
    int size = 0;
    if (state != Er) {
        for (int i = 0; i < 9; ++i) if (parser_matr[i][state] != Er) ++size;
        arr = new int[size];
        for (int i = 0; i < 9; ++i) if (parser_matr[i][state] != Er)*arr++ = i;
        fout << endl << lex.index + 1 << ' ';
    }
    else {
        for (int i = 0; i < 9; ++i) if (parser_matr[i][save] != Er) ++size;
        arr = new int[size];
        for (int i = 0; i < 9; ++i)if (parser_matr[i][save] != Er) *arr++ = i;
        fout << endl << lex.index << ' ';
    }
    arr -= size;
    if (size == 1) fout << lex_name[arr[0]];
    else lex_name[arr[0]][0] < lex_name[arr[1]][0] ? fout << lex_name[arr[0]] << ' ' << lex_name[arr[1]] : fout << lex_name[arr[1]] << ' ' << lex_name[arr[0]];
    delete[] arr;
}

void is_ok(int state, size_t vsize, Lexem& lex, ofstream& fout) {
    if (lex.index == vsize - 1)	fout << endl << "OK";
}

vector <char*> lexemAnalyzator(vector<Lexem>& v, ofstream& fout)
{
    if (!v.size()){fout << "\n0 do";}
    else {
        int State = S, save;

        for (size_t i = 0; i < v.size(); ++i) {
            save = State;
            State = parser_matr[v[i].lType][State];
            if (State == Er) {
                expected(State, v[i], fout, save);
                fout.close();
                exit(0);

            } else if (State == J) {
                is_ok(State, v.size(), v[i], fout);
                if (i != v.size() - 1) State = S;
            }

            if (State == H && (i == v.size() - 1 || Er == parser_matr[v[i + 1].lType][State])) {
                is_ok(State, v.size(), v[i], fout);
                if (i != v.size() - 1) State = S;
            }

            if (i == v.size() - 1 && State != J && State != H) expected(State, v[i], fout, State);
        }
    }
}


int main()
{
    char inputFilename[] = "input.txt";

    ofstream outfile("output.txt");

    vector <Lexem> lexems;
    lexems = lexemsFromFile(inputFilename);

    print_vec(lexems, outfile);
    lexemAnalyzator(lexems, outfile);

    int i = 0;
    return 0;
}
