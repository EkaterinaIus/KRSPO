#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stdexcept>
#include <iomanip>

enum class TT {
    NUM, WHILE, INPUT, PRINT,
    PLUS, MINUS, STAR, SLASH,
    ASSIGN, EQ, NEQ, LT, GT, LEQ, GEQ,
    LPAREN, RPAREN, COLON, COMMA,
    NEWLINE, INDENT, DEDENT, EOFF
};

static std::string ttStr(TT t) {
    switch (t) {
        case TT::NUM: return "NUM";
        case TT::ID: return "ID";
        case TT::WHILE: return "while";
        case TT::INPUT: return "input";
        case TT::PRINT: return "print";
        case TT::PLUS: return "+";
        case TT::MINUS: return "-";
        case TT::STAR: return "*";
        case TT::SLASH: return "/";
        case TT::ASSIGN: return "=";
        case TT::EQ: return "==";
        case TT::NEQ: return "!=";
        case TT::LT: return "<";
        case TT::GT: return ">";
        case TT::LEQ: return "<=";
        case TT::GEQ: return ">=";
        case TT::LPAREN: return "(";
        case TT::RPAREN: return ")";
        case TT::COLON: return ":";
        case TT::COMMA: return ",";
        case TT::NEWLINE: return "NEWLINE";
        case TT::INDENT: return "INDENT";
        case TT::DEDENT: return "DEDENT";
        case TT::EOFF: return "EOF";
        default: return "?";
    }
}

struct Token {
    TT type;
    std::string val;
    int line, col;
    Token(TT t, std::string v, int l, int c) : type(t), val(std::move(v)), line(l), col(c) {}
};

class Lexer {
public:
    std::vector<Token> tokens;
    explicit Lexer(const std::string& src) { tokenize(src); }

private:
    void tokenize(const std::string& src) {
        std::istringstream ss(src);
        std::string line;
        int lineNum = 0;
        std::vector<int> indentStack = {0};

        while (std::getline(ss, line)) {
            ++lineNum;
            int indent = 0;
            size_t i = 0;
            while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
                indent += (line[i] == '\t') ? 4 : 1;
                ++i;
            }
            if (i >= line.size() || line[i] == '#') continue;

            if (indent > indentStack.back()) {
                indentStack.push_back(indent);
                tokens.emplace_back(TT::INDENT, "INDENT", lineNum, 1);
            } else {
                while (indent < indentStack.back()) {
                    indentStack.pop_back();
                    tokens.emplace_back(TT::DEDENT, "DEDENT", lineNum, 1);
                }
                if (indent != indentStack.back())
                    throw std::runtime_error("Line " + std::to_string(lineNum) + ": indent error");
            }

            while (i < line.size() && line[i] != '#') {
                if (line[i] == ' ' || line[i] == '\t') { ++i; continue; }
                if (isdigit(line[i]) || (line[i] == '.' && i+1 < line.size() && isdigit(line[i+1]))) {
                    size_t s = i;
                    bool hasDot = false;
                    while (i < line.size() && (isdigit(line[i]) || line[i] == '.')) {
                        if (line[i] == '.') hasDot = true;
                        ++i;
                    }
                    std::string v = line.substr(s, i - s);
                    if (!hasDot) v += ".0";
                    tokens.emplace_back(TT::NUM, v, lineNum, (int)s + 1);
                    continue;
                }
                if (isalpha(line[i]) || line[i] == '_') {
                    size_t s = i;
                    while (i < line.size() && (isalnum(line[i]) || line[i] == '_')) ++i;
                    std::string w = line.substr(s, i - s);
                    TT tt = TT::ID;
                    if (w == "while") tt = TT::WHILE;
                    else if (w == "input") tt = TT::INPUT;
                    else if (w == "print") tt = TT::PRINT;
                    tokens.emplace_back(tt, w, lineNum, (int)s + 1);
                    continue;
                }
                if (i + 1 < line.size()) {
                    std::string two = line.substr(i, 2);
                    if (two == "==") { tokens.emplace_back(TT::EQ, two, lineNum, (int)i+1); i+=2; continue; }
                    if (two == "!=") { tokens.emplace_back(TT::NEQ, two, lineNum, (int)i+1); i+=2; continue; }
                    if (two == "<=") { tokens.emplace_back(TT::LEQ, two, lineNum, (int)i+1); i+=2; continue; }
                    if (two == ">=") { tokens.emplace_back(TT::GEQ, two, lineNum, (int)i+1); i+=2; continue; }
                }
                TT tt;
                switch (line[i]) {
                    case '+': tt = TT::PLUS; break;
                    case '-': tt = TT::MINUS; break;
                    case '*': tt = TT::STAR; break;
                    case '/': tt = TT::SLASH; break;
                    case '=': tt = TT::ASSIGN; break;
                    case '<': tt = TT::LT; break;
                    case '>': tt = TT::GT; break;
                    case '(': tt = TT::LPAREN; break;
                    case ')': tt = TT::RPAREN; break;
                    case ':': tt = TT::COLON; break;
                    case ',': tt = TT::COMMA; break;
                    default:
                        throw std::runtime_error("Line " + std::to_string(lineNum) + ": unknown character '" + line[i] + "'");
                }
                tokens.emplace_back(tt, std::string(1, line[i]), lineNum, (int)i + 1);
                ++i;
            }
            tokens.emplace_back(TT::NEWLINE, "\\n", lineNum, (int)line.size() + 1);
        }
        while (indentStack.size() > 1) {
            indentStack.pop_back();
            tokens.emplace_back(TT::DEDENT, "DEDENT", lineNum, 1);
        }
        tokens.emplace_back(TT::EOFF, "", lineNum, 1);
    }
};

struct Quad {
    std::string op, a1, a2, res;
    Quad() = default;
    Quad(std::string o, std::string a, std::string b, std::string r) : op(std::move(o)), a1(std::move(a)), a2(std::move(b)), res(std::move(r)) {}
};

class QuadTable {
public:
    std::vector<Quad> q;
    int tempCnt = 0;
    std::string newTemp() { return "_t" + std::to_string(++tempCnt); }
    int emit(std::string op, std::string a1, std::string a2, std::string res) {
        q.emplace_back(std::move(op), std::move(a1), std::move(a2), std::move(res));
        return (int)q.size() - 1;
    }
    void patch(int idx, int target) { q[idx].res = std::to_string(target); }
    int next() const { return (int)q.size(); }
    void print() const {
        const int W = 13;
        std::cout << "\n+-----+-------------+-------------+-------------+-------------+\n";
        std::cout << "|  #  | Operation   | Argument 1  | Argument 2  | Result      |\n";
        std::cout << "+-----+-------------+-------------+-------------+-------------+\n";
        for (size_t i = 0; i < q.size(); i++) {
            std::cout << "| " << std::setw(3) << i << " "
                      << "| " << std::setw(W) << std::left << q[i].op
                      << "| " << std::setw(W) << q[i].a1
                      << "| " << std::setw(W) << q[i].a2
                      << "| " << std::setw(W) << q[i].res << "|\n";
        }
        std::cout << "+-----+-------------+-------------+-------------+-------------+\n";
    }
};

class SymTable {
public:
    std::map<std::string, bool> vars;
    void declare(const std::string& name) { vars[name] = true; }
    bool has(const std::string& name) const { return vars.count(name) > 0; }
    void print() const {
        std::cout << "\n=== SYMBOL TABLE ===\n";
        for (const auto& [k, _] : vars) std::cout << "  double " << k << ";\n";
    }
};

class Parser {
public:
    const std::vector<Token>& toks;
    size_t pos = 0;
    QuadTable& qt;
    SymTable& st;
    Parser(const std::vector<Token>& t, QuadTable& q, SymTable& s) : toks(t), qt(q), st(s) {}

    struct BtState { size_t pos; int quadSz; int tempCnt; };
    BtState save() const { return { pos, (int)qt.q.size(), qt.tempCnt }; }
    void restore(const BtState& s) { pos = s.pos; qt.q.resize(s.quadSz); qt.tempCnt = s.tempCnt; }

    const Token& cur() const { return toks[pos < toks.size() ? pos : toks.size() - 1]; }
    bool check(TT t) const { return cur().type == t; }
    bool match(TT t) { if (check(t)) { ++pos; return true; } return false; }
    Token expect(TT t) {
        if (!check(t)) throw std::runtime_error("Line " + std::to_string(cur().line) + ": expected token [" + ttStr(t) + "]");
        return toks[pos++];
    }
    void skipNL() { while (match(TT::NEWLINE)); }

    void parseProgram() { skipNL(); while (!check(TT::EOFF)) { parseStatement(); skipNL(); } }

    void parseStatement() {
        BtState s0 = save();
        if (tryAssignment()) return;
        restore(s0);
        if (tryWhile()) return;
        restore(s0);
        if (tryPrint()) return;
        restore(s0);
        throw std::runtime_error("Line " + std::to_string(cur().line) + ": syntax error");
    }

    bool tryAssignment() {
        if (!check(TT::ID)) return false;
        BtState s0 = save();
        std::string name = cur().val;
        ++pos;
        if (!match(TT::ASSIGN)) { restore(s0); return false; }
        if (check(TT::INPUT)) {
            BtState si = save();
            ++pos;
            if (match(TT::LPAREN) && match(TT::RPAREN) && match(TT::NEWLINE)) {
                st.declare(name);
                qt.emit("INPUT", "_", "_", name);
                return true;
            }
            restore(si);
        }
        std::string res;
        try { res = parseExpr(); } catch (...) { restore(s0); return false; }
        if (!match(TT::NEWLINE)) { restore(s0); return false; }
        st.declare(name);
        qt.emit("=", res, "_", name);
        return true;
    }

    bool tryWhile() {
        if (!match(TT::WHILE)) return false;
        int condStart = qt.next();
        std::string cond;
        try { cond = parseCond(); } catch (...) { return false; }
        if (!match(TT::COLON)) return false;
        if (!match(TT::NEWLINE)) return false;
        if (!match(TT::INDENT)) return false;
        int jfIdx = qt.emit("JF", cond, "_", "?");
        skipNL();
        while (!check(TT::DEDENT) && !check(TT::EOFF)) { parseStatement(); skipNL(); }
        if (!match(TT::DEDENT)) return false;
        qt.emit("JMP", "_", "_", std::to_string(condStart));
        qt.patch(jfIdx, qt.next());
        return true;
    }

    bool tryPrint() {
        if (!match(TT::PRINT)) return false;
        BtState s0 = save();
        if (!match(TT::LPAREN)) { restore(s0); return false; }
        std::string arg;
        try { arg = parseExpr(); } catch (...) { restore(s0); return false; }
        qt.emit("PRINT", arg, "_", "_");
        while (match(TT::COMMA)) {
            try { arg = parseExpr(); } catch (...) { restore(s0); return false; }
            qt.emit("PRINT", arg, "_", "_");
        }
        if (!match(TT::RPAREN)) { restore(s0); return false; }
        if (!match(TT::NEWLINE)) { restore(s0); return false; }
        return true;
    }

    std::string parseCond() {
        std::string left = parseExpr();
        std::string op;
        if (match(TT::EQ)) op = "==";
        else if (match(TT::NEQ)) op = "!=";
        else if (match(TT::LEQ)) op = "<=";
        else if (match(TT::GEQ)) op = ">=";
        else if (match(TT::LT)) op = "<";
        else if (match(TT::GT)) op = ">";
        else throw std::runtime_error("Line " + std::to_string(cur().line) + ": expected comparison operator");
        std::string right = parseExpr();
        std::string tmp = qt.newTemp();
        qt.emit(op, left, right, tmp);
        return tmp;
    }

    std::string parseExpr() {
        std::string res = parseTerm();
        while (check(TT::PLUS) || check(TT::MINUS)) {
            std::string op = cur().val; ++pos;
            std::string r = parseTerm();
            std::string tmp = qt.newTemp();
            qt.emit(op, res, r, tmp);
            res = tmp;
        }
        return res;
    }

    std::string parseTerm() {
        std::string res = parseFactor();
        while (check(TT::STAR) || check(TT::SLASH)) {
            std::string op = cur().val; ++pos;
            std::string r = parseFactor();
            std::string tmp = qt.newTemp();
            qt.emit(op, res, r, tmp);
            res = tmp;
        }
        return res;
    }

    std::string parseFactor() {
        if (match(TT::MINUS)) {
            std::string operand = parseFactor();
            std::string tmp = qt.newTemp();
            qt.emit("NEG", operand, "_", tmp);
            return tmp;
        }
        if (match(TT::LPAREN)) {
            std::string res = parseExpr();
            expect(TT::RPAREN);
            return res;
        }
        if (check(TT::NUM)) { std::string v = cur().val; ++pos; return v; }
        if (check(TT::ID)) {
            std::string name = cur().val; ++pos;
            if (!st.has(name)) st.declare(name);
            return name;
        }
        throw std::runtime_error("Line " + std::to_string(cur().line) + ": unexpected token in expression");
    }
};

class CodeGen {
public:
    const QuadTable& qt;
    const SymTable& st;
    CodeGen(const QuadTable& q, const SymTable& s) : qt(q), st(s) {}

    std::string generate() {
        std::set<int> jumpTargets;
        for (const auto& qd : qt.q) if (qd.op == "JF" || qd.op == "JMP") jumpTargets.insert(std::stoi(qd.res));
        jumpTargets.insert(qt.next());
        std::ostringstream o;
        o << "#include <cstdio>\n\nint main() {\n";
        for (const auto& [n, _] : st.vars) o << "    double " << n << " = 0.0;\n";
        for (int i = 1; i <= qt.tempCnt; i++) o << "    double _t" << i << " = 0.0;\n";
        o << "\n";
        for (int i = 0; i < (int)qt.q.size(); i++) {
            if (jumpTargets.count(i)) o << "L" << i << ":;\n";
            const auto& qd = qt.q[i];
            if (qd.op == "+" || qd.op == "-" || qd.op == "*" || qd.op == "/") {
                o << "    " << qd.res << " = " << qd.a1 << " " << qd.op << " " << qd.a2 << ";\n";
            } else if (qd.op == "NEG") {
                o << "    " << qd.res << " = -" << qd.a1 << ";\n";
            } else if (qd.op == "=") {
                o << "    " << qd.res << " = " << qd.a1 << ";\n";
            } else if (qd.op == "==" || qd.op == "!=" || qd.op == "<" || qd.op == ">" || qd.op == "<=" || qd.op == ">=") {
                o << "    " << qd.res << " = (" << qd.a1 << " " << qd.op << " " << qd.a2 << ") ? 1.0 : 0.0;\n";
            } else if (qd.op == "JF") {
                o << "    if (" << qd.a1 << " == 0.0) goto L" << qd.res << ";\n";
            } else if (qd.op == "JMP") {
                o << "    goto L" << qd.res << ";\n";
            } else if (qd.op == "PRINT") {
                o << "    printf(\"%g\\n\", " << qd.a1 << ");\n";
            } else if (qd.op == "INPUT") {
                o << "    scanf(\"%lf\", &" << qd.res << ");\n";
            }
            o << "\n";
        }
        if (jumpTargets.count(qt.next())) o << "L" << qt.next() << ":;\n";
        o << "    return 0;\n}\n";
        return o.str();
    }
};

int main(int argc, char* argv[]) {
    std::string src;
    if (argc > 1) {
        std::ifstream f(argv[1]);
        if (!f) { std::cerr << "Cannot open file: " << argv[1] << "\n"; return 1; }
        std::ostringstream ss; ss << f.rdbuf();
        src = ss.str();
    } else {
        src = "a = 5.5 * 2.0\nb = a + 3.0\nwhile b > 0.0:\n    print(b)\n    b = b - 1.0\n";
    }
    std::cout << "=== SOURCE CODE ===\n" << src << "\n";
    try {
        Lexer lexer(src);
        QuadTable qt;
        SymTable st;
        Parser parser(lexer.tokens, qt, st);
        parser.parseProgram();
        st.print();
        qt.print();
        CodeGen cg(qt, st);
        std::string cppSrc = cg.generate();
        std::cout << "\n=== GENERATED C++ CODE ===\n" << cppSrc;
        std::ofstream outF("output.cpp");
        outF << cppSrc;
    } catch (const std::exception& ex) {
        std::cerr << "\n*** COMPILATION ERROR: " << ex.what() << " ***\n";
        return 1;
    }
    return 0;
}


