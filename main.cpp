#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

using namespace std;


char *getTextFromFile(const char *inFilename) {
    ifstream inFile(inFilename);
    char *textFromFile;
    long int sizeOfText;

    inFile.seekg(0, ios::end);              // перемещает текущую позицию указателя в файле в конец
    sizeOfText = (long int) inFile.tellg(); // возвращает текущую позицию в файле в типе streampos
    inFile.seekg(0, ios::beg);              // переместили указатель обратно в начало

    textFromFile = new char[sizeOfText + 1];// +1 т.к. есть ещё "невидимый" символ завершения файла EOF
    inFile.getline(textFromFile, sizeOfText + 1, '\0');  // getline(куда, сколько, до куда)
    inFile.close();

    return textFromFile;
}

/*====================================================================================================================*/

enum State {
    StartST, WordST, ConstST, OperST, ErrST,
    WordToOperST, ConstToOperST, OperToWordST, OperToConstST,
    LongOperST,
    EndOfWordST, EndOfConstST, EndOfOperST, EndOfErrST,
    FinalST, FinalWordST, FinalConstST, FinalOperST, FinalErrST,
};

enum Symbol {
    spaceSym, alphaSym, digitSym, specialSym, finalSym
};

enum LexemType {
    Do, Lp, Un, Co, Eq, Id, Vl, Ao, Wl
};

struct Lexem {
    char *textLexem;
    LexemType typeLexem;
    int index;

    friend ostream& operator<< (ostream&, const Lexem&);
};


// functions-identifiers of some symbols
bool isSpace(char sym) {
    if (sym == ' ' || sym == '\t' || sym == '\n') return true;
    return false;
}

bool isSpec(char sym) {
    if (sym == '=' || sym == '<' || sym == '>' || sym == '+' || sym == '-') return true;
    return false;
}

bool isFinal(char sym) {
    if (sym == '\0') return true;
    return false;
}


Symbol whatSym(char sym) {
    if (isSpace(sym)) return spaceSym;
    if (isalpha(sym)) return alphaSym;
    if (isdigit(sym)) return digitSym;
    if (isSpec(sym)) return specialSym;
    if (isFinal(sym)) return finalSym;
    return spaceSym;
}


LexemType whatLexemWord(const Lexem &lex) {
    if (!strcmp(lex.textLexem, "do")) return Do;
    if (!strcmp(lex.textLexem, "loop")) return Lp;
    if (!strcmp(lex.textLexem, "until")) return Un;
    return Id;
}


LexemType whatLexemOper(const Lexem &lex) {
    if (!strcmp(lex.textLexem, "=")) return Eq;
    if (!strcmp(lex.textLexem, "+")) return Ao;
    if (!strcmp(lex.textLexem, "-")) return Ao;
    if (!strcmp(lex.textLexem, "*")) return Ao;
    if (!strcmp(lex.textLexem, "/")) return Ao;
    if (!strcmp(lex.textLexem, ">")) return Co;
    if (!strcmp(lex.textLexem, "<")) return Co;
    if (!strcmp(lex.textLexem, ">=")) return Co;
    if (!strcmp(lex.textLexem, "<=")) return Co;
    if (!strcmp(lex.textLexem, "<>")) return Co;
    return Wl;
}


void getLexemFromText(Lexem &currentLexem,
                      const char *&text, const int &textPos, int &lexemLen,
                      int curState, int &ind) {
    currentLexem.textLexem = new char[lexemLen + 1];
    int lexPos = -1;
    do currentLexem.textLexem[++lexPos] = text[textPos - lexemLen]; while (--lexemLen);
    currentLexem.textLexem[++lexPos] = '\0';

    if (curState == WordToOperST) {
        currentLexem.typeLexem = whatLexemWord(currentLexem);
        lexemLen = 1;
    } else if (curState == ConstToOperST) {
        currentLexem.typeLexem = Vl;
        lexemLen = 1;
    } else if ((curState == OperToWordST) || (curState == OperToConstST)) {
        currentLexem.typeLexem = whatLexemOper(currentLexem);
        lexemLen = 1;
    } else if (curState == EndOfWordST) {
        currentLexem.typeLexem = whatLexemWord(currentLexem);
        lexemLen = 0;
    } else if (curState == EndOfConstST) {
        currentLexem.typeLexem = Vl;
        lexemLen = 0;
    } else if (curState == EndOfOperST) {
        currentLexem.typeLexem = whatLexemOper(currentLexem);
        lexemLen = 0;
    } else if (curState == EndOfErrST) {
        currentLexem.typeLexem = Wl;
        lexemLen = 0;
    } else if (curState == FinalWordST) {
        currentLexem.typeLexem = whatLexemWord(currentLexem);
    } else if (curState == FinalConstST) {
        currentLexem.typeLexem = Vl;
    } else if (curState == FinalOperST) {
        currentLexem.typeLexem = whatLexemOper(currentLexem);
    } else if (curState == FinalErrST) {
        currentLexem.typeLexem = Wl;
    }
    currentLexem.index = ind;
    ++ind;
}


vector<Lexem> getLexemsFromText(const char *textFromFile) {
    // таблица состояний автомата (для надёжности не в отдельной функции)
    int tableOfStates[14][5] = {
            {StartST,      WordST,       ConstST,       OperST,        FinalST},       // 0.StartST
            {EndOfWordST,  WordST,       WordST,        WordToOperST,  FinalWordST},   // 1.WordST
            {EndOfConstST, ErrST,        ConstST,       ConstToOperST, FinalConstST},  // 2.ConstST
            {EndOfOperST,  OperToWordST, OperToConstST, LongOperST,    FinalOperST},   // 3.OperST
            {EndOfErrST,   ErrST,        ErrST,         ErrST,         FinalErrST},    // 4.ErrST
            {EndOfOperST,  OperToWordST, OperToConstST, LongOperST,    FinalOperST},   // 5.WordToOperST
            {EndOfOperST,  OperToWordST, OperToConstST, LongOperST,    FinalOperST},   // 6.ConstToOperST
            {EndOfWordST,  WordST,       WordST,        WordToOperST,  FinalWordST},   // 7.OperToWordST
            {EndOfConstST, ErrST,        ConstST,       ConstToOperST, FinalConstST},  // 8.OperToConstST
            {EndOfOperST,  OperToWordST, OperToConstST, ErrST,         FinalOperST},   // 9.LongOperST
            {StartST,      WordST,       ConstST,       OperST,        FinalST},       // 10.EndOfWordST
            {StartST,      WordST,       ConstST,       OperST,        FinalST},       // 11.EndOfConstST
            {StartST,      WordST,       ConstST,       OperST,        FinalST},       // 12.EndOfOperST
            {StartST,      WordST,       ConstST,       OperST,        FinalST}        // 13.EndOfErrST
    };//        {    spaceSym,         alphaSym,           digitSym,           specialSym,         finalSym};

    // само Начало работы функции
    vector<Lexem> lexemsFromText;

    // Позиция указателя в тексте, длина лексемы, идендификатор (номер) лексемы
    int pos = 0, lexemLen = 0, ind = 0;
    int currentState = StartST;

    bool run = true;
    while (run) {
        // определяем тип символа
        Symbol sym = whatSym(textFromFile[pos]);
        // определяем состояние исходя из символа по таблице
        currentState = tableOfStates[currentState][sym];

        // пробегаемся по состояниям
        if ((currentState == ErrST) || (currentState == WordST) ||
            (currentState == OperST) || (currentState == ConstST) ||
            (currentState == LongOperST))
            ++lexemLen;

        else if ((currentState == WordToOperST) || (currentState == ConstToOperST) ||
                 (currentState == OperToWordST) || (currentState == OperToConstST) ||
                 (currentState == EndOfWordST) || (currentState == EndOfConstST) ||
                 (currentState == EndOfOperST) || (currentState == EndOfErrST)) {
            Lexem currentLexem{};
            getLexemFromText(currentLexem, textFromFile, pos, lexemLen, currentState, ind);
            lexemsFromText.push_back(currentLexem);
        } else if (currentState == FinalST) {
            run = false;
        } else if ((currentState == FinalWordST) || (currentState == FinalConstST) ||
                   (currentState == FinalOperST) || (currentState == FinalErrST)) {
            run = false;
            if (lexemLen > 0) {
                Lexem currentLexem{};
                getLexemFromText(currentLexem, textFromFile, pos, lexemLen, currentState, ind);
                lexemsFromText.push_back(currentLexem);
            }
        }
        ++pos;
    }
    return lexemsFromText;
}

/*====================================================================================================================*/

vector<const char*> lexemId = {"do", "lp", "un", "co", "eq", "id", "vl", "ao", "wl"};

ostream &operator<<(ostream& out, const Lexem& lexem) {
    out << lexem.textLexem << '[' << lexemId[lexem.typeLexem] << ']';
    return out;
}

void outLexems(const char* outFilename, vector<Lexem>& lexems)
{
    ofstream outFile(outFilename);
    for (int i = 0; i < lexems.size(); ++i)
    {
        outFile << lexems[i];
        if (i != lexems.size() - 1) outFile << " ";
    }
}

/*====================================================================================================================*/

enum ParserStates {
    ParserStartSt, DoSt, IdSt, EqSt, OprdSt, LpSt, UnSt, Oprd2St, CoSt, FinalSt,
    ErrDo, ErrId, ErrEq, ErrOprd, ErrLpOprd, ErrUn, ErrCo, ErrNone
};


struct Error
{
    int indError;
    char* massageError;

    Error(int index, int state)
    {
        indError = index;
        switch (state) {
            case ErrDo:
                massageError = (char*)"do";
                break;
            case ErrId:
                massageError = (char*)"id";
                break;
            case ErrEq:
                massageError = (char*)"eq";
                break;
            case ErrOprd:
                massageError = (char*)"id vl";
                break;
            case ErrLpOprd:
                massageError = (char*)"lp id vl";
                break;
            case ErrUn:
                massageError = (char*)"un";
                break;
            case ErrCo:
                massageError = (char*)"co";
                break;
            case ErrNone:
                massageError = (char*)"none";
                break;
            default:
                indError = -1;
                massageError = (char*)"OK";
                break;
        }
    }
};


Error getErrorsFromLexems(const vector<Lexem>& lexems) {
    if (lexems.empty()){
        Error resultError(0, ErrDo);
        return resultError;
    }
    // таблица состояний автомата лексического анализатора (parser-а) (для надёжности тоже не в отдельной функции)
    int tableOfParserStates[10][9] = {
{DoSt,      ErrDo,  ErrDo,      ErrDo,      ErrDo,      ErrDo,      ErrDo,      ErrDo,      ErrDo},     // 0.ParserStartSt
{ErrId,     ErrId,  ErrId,      ErrId,      ErrId,      IdSt,       ErrId,      ErrId,      ErrId},     // 1.DoSt
{ErrEq,     ErrEq,  ErrEq,      ErrEq,      EqSt,       ErrEq,      ErrEq,      ErrEq,      ErrEq},     // 2.IdSt
{ErrOprd,   ErrOprd,ErrOprd,    ErrOprd,    ErrOprd,    OprdSt,     OprdSt,     ErrOprd,    ErrOprd},   // 3.EqSt
{ErrLpOprd, LpSt,   ErrLpOprd,  ErrLpOprd,  ErrLpOprd,  ErrLpOprd,  ErrLpOprd,  EqSt,       ErrLpOprd}, // 4.OprdSt
{ErrUn,     ErrUn,  UnSt,       ErrUn,      ErrUn,      ErrUn,      ErrUn,      ErrUn,      ErrUn},     // 5.LpSt
{ErrOprd,   ErrOprd,ErrOprd,    ErrOprd,    ErrOprd,    Oprd2St,    Oprd2St,    ErrOprd,    ErrOprd},   // 6.UnSt
{ErrCo,     ErrCo,  ErrCo,      CoSt,       ErrCo,      ErrCo,      ErrCo,      ErrCo,      ErrCo},     // 7.Oprd2St
{ErrOprd,   ErrOprd,ErrOprd,    ErrOprd,    ErrOprd,    FinalSt,    FinalSt,    ErrOprd,    ErrOprd},   // 8.CoSt
{ErrNone,   ErrNone,ErrNone,    ErrNone,    ErrNone,    ErrNone,    ErrNone,    ErrNone,    ErrNone},   // 9.FinalSt
};//    0.Do,           1.Lp,      2.Un,            3.Co,           4.Eq,           5.Id,           6.Vl,           7.Ao,           8.Wl

    // Начало работы функции-парсера

    int ind = 0;    // номер лексемы
    int curSt = ParserStartSt;

    do {
        // определяем состояние
        curSt = tableOfParserStates[curSt][lexems[ind].typeLexem];
        // и пробегаемся по всем ошибкам, и если есть хотя бы одна из них - выходим из цикла с нужным состоянием ошибки.
        if ((curSt == ErrDo)||(curSt == ErrId)||(curSt == ErrEq)||(curSt == ErrOprd)||
            (curSt == ErrLpOprd)||(curSt == ErrUn)||(curSt == ErrCo)||(curSt == ErrNone))
            break;

        ++ind;
    } while (ind < lexems.size());

    Error resultErr(ind, curSt);
    return resultErr;
}

/*====================================================================================================================*/

void outError(const char* outFilename, Error resultError) {
    ofstream outFile(outFilename, ios::app);    // флажок "app" позволяет добавлять текст в конец файла
    if (resultError.indError == -1) outFile << "\n" << resultError.massageError;
    else outFile << "\n" << resultError.indError << " " << resultError.massageError;
}

/*====================================================================================================================*/


int main() {
    const char inFilename[10] = "input.txt";
    const char outFilename[11] = "output.txt";

    // держи текст = дай текст из файла(файл)
    char *text = getTextFromFile(inFilename);

    // держи лексемы = дай и определи лексемы из текста(держи текст)
    vector<Lexem> lexems = getLexemsFromText(text);

    // выведи лексемы(держи лексемы)
    outLexems(outFilename, lexems);

    // держи ошибку = проанализируй лексемы и дай ошибку(держи лексемы)
    Error resultError = getErrorsFromLexems(lexems);

    // выведи ошибки(держи ошибки)
    outError(outFilename, resultError);

    return 0;
}
