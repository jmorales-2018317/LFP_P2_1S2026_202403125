#ifndef TASKSCRIPT_LEXICAL_ANALYZER_H
#define TASKSCRIPT_LEXICAL_ANALYZER_H

#include <string>
#include <vector>

#include "ErrorManager.h"
#include "Token.h"

namespace taskscript {

// Analizador lexico basado en un AFD manual (sin std::regex).
//
// Estados conceptuales del AFD:
//   Inicio       -> punto de entrada
//   EnIdentif    -> letra/`_` -> letra/digito/`_` ... -> palabra reservada o INVALIDO
//   EnCadena     -> `"` -> cualquier caracter ... -> `"`  (errores: salto de
//                   linea o EOF antes del cierre = CRITICO "cadena no cerrada")
//   EnNumero     -> digito ... -> ENTERO o, si tras 4 digitos hay `-`,
//                   transita a EnFecha para reconocer AAAA-MM-DD
//   EnFecha      -> AAAA `-` MM `-` DD con validacion de rango
//
// La funcion de transicion esta implementada explicitamente en nextToken()
// con switch/if. Los caracteres no reconocidos generan un token INVALID y
// un error lexico, pero el AFD continua para reportar todos los errores.
class LexicalAnalyzer {
public:
    LexicalAnalyzer(std::string source, ErrorManager& errorManager);

    // Devuelve el siguiente token. Cuando se alcanza EOF, devuelve siempre
    // un token de tipo END_OF_FILE.
    Token nextToken();

    // Tokeniza todo el archivo y devuelve la lista resultante (incluye un
    // token END_OF_FILE al final).
    std::vector<Token> tokenizeAll();

    // Reinicia el lexer al inicio del archivo (util si se quiere recorrer
    // dos veces, una para tokens y otra para el parser).
    void reset();

private:
    // Helpers de cursor.
    bool isAtEnd() const;
    char peek() const;
    char peek(int offset) const;
    char advance();
    bool match(char expected);

    void skipWhitespace();
    bool isLetter(char c) const;
    bool isDigit(char c) const;
    bool isIdentifierStart(char c) const;
    bool isIdentifierPart(char c) const;
    bool isInvalidStringChar(char c) const;

    // Estados terminales del AFD (cada uno emite UN token).
    Token readIdentifier();
    Token readString();
    Token readNumberOrDate();

    // Convierte una palabra ya leida en un Token con el TokenType correcto.
    Token classifyKeyword(const std::string& lexeme, int line, int column);

    std::string source_;
    int pos_;
    int line_;
    int column_;

    ErrorManager& errorManager_;
};

}  // namespace taskscript

#endif  // TASKSCRIPT_LEXICAL_ANALYZER_H
