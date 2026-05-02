#include "LexicalAnalyzer.h"

#include <unordered_map>
#include <utility>

namespace taskscript {

namespace {

// Tabla estatica de palabras reservadas. Se usa busqueda exacta (case
// sensitive) tal como aparecen en el enunciado.
const std::unordered_map<std::string, TokenType>& keywordTable() {
    static const std::unordered_map<std::string, TokenType> table = {
        {"TABLERO",      TokenType::TABLERO},
        {"COLUMNA",      TokenType::COLUMNA},
        {"tarea",        TokenType::TAREA_KW},
        {"prioridad",    TokenType::PRIORIDAD_KW},
        {"responsable",  TokenType::RESPONSABLE_KW},
        {"fecha_limite", TokenType::FECHA_LIMITE_KW},
        {"ALTA",         TokenType::ALTA},
        {"MEDIA",        TokenType::MEDIA},
        {"BAJA",         TokenType::BAJA},
    };
    return table;
}

}  // namespace

LexicalAnalyzer::LexicalAnalyzer(std::string source, ErrorManager& errorManager)
    : source_(std::move(source)),
      pos_(0),
      line_(1),
      column_(1),
      errorManager_(errorManager) {}

void LexicalAnalyzer::reset() {
    pos_ = 0;
    line_ = 1;
    column_ = 1;
}

bool LexicalAnalyzer::isAtEnd() const {
    return pos_ >= static_cast<int>(source_.size());
}

char LexicalAnalyzer::peek() const {
    return isAtEnd() ? '\0' : source_[pos_];
}

char LexicalAnalyzer::peek(int offset) const {
    int idx = pos_ + offset;
    if (idx < 0 || idx >= static_cast<int>(source_.size())) return '\0';
    return source_[idx];
}

char LexicalAnalyzer::advance() {
    if (isAtEnd()) return '\0';
    char c = source_[pos_++];
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return c;
}

bool LexicalAnalyzer::match(char expected) {
    if (isAtEnd() || source_[pos_] != expected) return false;
    advance();
    return true;
}

bool LexicalAnalyzer::isLetter(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool LexicalAnalyzer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool LexicalAnalyzer::isIdentifierStart(char c) const {
    return isLetter(c) || c == '_';
}

bool LexicalAnalyzer::isIdentifierPart(char c) const {
    return isLetter(c) || isDigit(c) || c == '_';
}

bool LexicalAnalyzer::isInvalidStringChar(char c) const {
    // Caracteres especiales no permitidos dentro de literales de cadena.
    switch (c) {
        case '@': case '$': case '#': case '&':
        case '~': case '^': case '!': case '|':
            return true;
        default:
            return false;
    }
}

void LexicalAnalyzer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            // Comentario de linea estilo C++ (extension util de calidad de
            // vida; no contradice la gramatica del enunciado).
            while (!isAtEnd() && peek() != '\n') advance();
        } else {
            break;
        }
    }
}

Token LexicalAnalyzer::nextToken() {
    skipWhitespace();

    int startLine = line_;
    int startCol = column_;

    if (isAtEnd()) {
        return Token(TokenType::END_OF_FILE, "", startLine, startCol);
    }

    char c = peek();

    // Delimitadores de un solo caracter.
    switch (c) {
        case '{': advance(); return Token(TokenType::LBRACE,    "{", startLine, startCol);
        case '}': advance(); return Token(TokenType::RBRACE,    "}", startLine, startCol);
        case '[': advance(); return Token(TokenType::LBRACKET,  "[", startLine, startCol);
        case ']': advance(); return Token(TokenType::RBRACKET,  "]", startLine, startCol);
        case ':': advance(); return Token(TokenType::COLON,     ":", startLine, startCol);
        case ',': advance(); return Token(TokenType::COMMA,     ",", startLine, startCol);
        case ';': advance(); return Token(TokenType::SEMICOLON, ";", startLine, startCol);
        default: break;
    }

    if (c == '"') {
        return readString();
    }
    if (isDigit(c)) {
        return readNumberOrDate();
    }
    if (isIdentifierStart(c)) {
        return readIdentifier();
    }

    // Caracter no reconocido por el AFD.
    advance();
    std::string lex(1, c);
    errorManager_.addLexicalError(
        lex,
        "Caracter no reconocido '" + lex + "'.",
        startLine, startCol,
        ErrorSeverity::Error);
    return Token(TokenType::INVALID, lex, startLine, startCol);
}

Token LexicalAnalyzer::readIdentifier() {
    int startLine = line_;
    int startCol = column_;
    std::string buffer;
    while (!isAtEnd() && isIdentifierPart(peek())) {
        buffer.push_back(advance());
    }
    return classifyKeyword(buffer, startLine, startCol);
}

Token LexicalAnalyzer::classifyKeyword(const std::string& lexeme, int line, int column) {
    const auto& table = keywordTable();
    auto it = table.find(lexeme);
    if (it != table.end()) {
        return Token(it->second, lexeme, line, column);
    }
    // No es palabra reservada => identificador no reconocido por la gramatica
    // de TaskScript. Lo reportamos como error lexico para que el parser pueda
    // sincronizar.
    errorManager_.addLexicalError(
        lexeme,
        "Identificador '" + lexeme + "' no reconocido por el lenguaje.",
        line, column,
        ErrorSeverity::Error);
    return Token(TokenType::INVALID, lexeme, line, column);
}

Token LexicalAnalyzer::readString() {
    int startLine = line_;
    int startCol = column_;
    advance();  // consumir comilla de apertura

    std::string buffer;
    bool closed = false;
    while (!isAtEnd()) {
        char c = peek();
        if (c == '"') {
            advance();
            closed = true;
            break;
        }
        if (c == '\n') {
            // Cadena sin cerrar antes de fin de linea: error CRITICO.
            break;
        }
        if (isInvalidStringChar(c)) {
            std::string ch(1, c);
            errorManager_.addLexicalError(
                ch,
                "Caracter invalido '" + ch + "' dentro de cadena.",
                line_, column_,
                ErrorSeverity::Error);
        }
        buffer.push_back(advance());
    }

    if (!closed) {
        errorManager_.addLexicalError(
            "\"" + buffer,
            "Cadena no cerrada antes de fin de linea/archivo.",
            startLine, startCol,
            ErrorSeverity::Critico);
        // Aun asi devolvemos un CADENA con el contenido leido para permitir
        // que el parser intente continuar.
        return Token(TokenType::CADENA, buffer, startLine, startCol);
    }
    return Token(TokenType::CADENA, buffer, startLine, startCol);
}

Token LexicalAnalyzer::readNumberOrDate() {
    int startLine = line_;
    int startCol = column_;
    std::string buffer;
    while (!isAtEnd() && isDigit(peek())) {
        buffer.push_back(advance());
    }

    // Posible fecha: AAAA-MM-DD. Solo intentamos si los primeros digitos son
    // exactamente 4 y a continuacion viene un '-'.
    if (buffer.size() == 4 && peek() == '-') {
        // Guardamos estado para hacer rollback si falla el patron.
        int savedPos = pos_;
        int savedLine = line_;
        int savedCol = column_;

        std::string fecha = buffer;
        // consumir '-'
        fecha.push_back(advance());

        // 2 digitos (mes)
        std::string mes;
        for (int i = 0; i < 2; ++i) {
            if (!isAtEnd() && isDigit(peek())) {
                mes.push_back(advance());
            } else {
                break;
            }
        }

        bool ok = mes.size() == 2 && peek() == '-';
        if (ok) {
            fecha += mes;
            fecha.push_back(advance());  // segundo '-'
            std::string dia;
            for (int i = 0; i < 2; ++i) {
                if (!isAtEnd() && isDigit(peek())) {
                    dia.push_back(advance());
                } else {
                    break;
                }
            }
            ok = dia.size() == 2;
            if (ok) {
                fecha += dia;
                int mm = (mes[0] - '0') * 10 + (mes[1] - '0');
                int dd = (dia[0] - '0') * 10 + (dia[1] - '0');
                if (mm < 1 || mm > 12 || dd < 1 || dd > 31) {
                    errorManager_.addLexicalError(
                        fecha,
                        "Fecha invalida '" + fecha + "' (mes 01-12, dia 01-31).",
                        startLine, startCol,
                        ErrorSeverity::Error);
                    return Token(TokenType::INVALID, fecha, startLine, startCol);
                }
                return Token(TokenType::FECHA, fecha, startLine, startCol);
            }
        }

        // Rollback: lo que parecia fecha no era valido. Devolvemos los 4
        // digitos como ENTERO y dejamos al parser/lexer manejar el resto.
        pos_ = savedPos;
        line_ = savedLine;
        column_ = savedCol;
    }

    return Token(TokenType::ENTERO, buffer, startLine, startCol);
}

std::vector<Token> LexicalAnalyzer::tokenizeAll() {
    std::vector<Token> tokens;
    while (true) {
        Token t = nextToken();
        tokens.push_back(t);
        if (t.type() == TokenType::END_OF_FILE) break;
    }
    return tokens;
}

}  // namespace taskscript
