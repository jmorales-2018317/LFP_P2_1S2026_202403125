#ifndef TASKSCRIPT_TOKEN_H
#define TASKSCRIPT_TOKEN_H

#include <string>

namespace taskscript {

enum class TokenType {
    // Palabras reservadas
    TABLERO,
    COLUMNA,
    TAREA_KW,
    PRIORIDAD_KW,
    RESPONSABLE_KW,
    FECHA_LIMITE_KW,
    // Enumeracion de prioridad
    ALTA,
    MEDIA,
    BAJA,
    // Literales
    CADENA,
    FECHA,
    ENTERO,
    // Delimitadores
    LBRACE,     // {
    RBRACE,     // }
    LBRACKET,   // [
    RBRACKET,   // ]
    COLON,      // :
    COMMA,      // ,
    SEMICOLON,  // ;
    // Especiales
    END_OF_FILE,
    INVALID
};

// Devuelve un nombre legible para mostrar en la tabla de tokens.
std::string tokenTypeToString(TokenType type);

// Token producido por el analizador lexico.
class Token {
public:
    Token() : type_(TokenType::INVALID), lexeme_(), line_(0), column_(0) {}

    Token(TokenType type, std::string lexeme, int line, int column)
        : type_(type), lexeme_(std::move(lexeme)), line_(line), column_(column) {}

    TokenType type() const { return type_; }
    const std::string& lexeme() const { return lexeme_; }
    int line() const { return line_; }
    int column() const { return column_; }

    void setType(TokenType type) { type_ = type; }
    void setLexeme(const std::string& lexeme) { lexeme_ = lexeme; }
    void setLine(int line) { line_ = line; }
    void setColumn(int column) { column_ = column; }

    // Util para comparaciones del parser.
    bool is(TokenType type) const { return type_ == type; }

private:
    TokenType type_;
    std::string lexeme_;
    int line_;
    int column_;
};

}  // namespace taskscript

#endif  // TASKSCRIPT_TOKEN_H
