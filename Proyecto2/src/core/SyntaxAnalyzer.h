#ifndef TASKSCRIPT_SYNTAX_ANALYZER_H
#define TASKSCRIPT_SYNTAX_ANALYZER_H

#include <initializer_list>
#include <string>
#include <vector>

#include "BoardModel.h"
#include "ErrorManager.h"
#include "ParseTree.h"
#include "Token.h"

namespace taskscript {

// Parser descendente recursivo manual para TaskScript.
//
// Gramatica (con pequena extension respecto a la del enunciado: se acepta una
// coma colgante al final de las listas de tareas y atributos, tal como lo
// muestra el ejemplo de archivo valido del PDF).
//
//   programa   -> TABLERO CADENA "{" columnas "}" ";"
//   columnas   -> columna columnas | columna
//   columna    -> COLUMNA CADENA "{" tareas "}" ";"
//   tareas     -> tarea ("," tarea)* (",")? | epsilon
//   tarea      -> "tarea" ":" CADENA "[" atributos "]"
//   atributos  -> atributo ("," atributo)* (",")?
//   atributo   -> "prioridad" ":" prioridad
//               | "responsable" ":" CADENA
//               | "fecha_limite" ":" FECHA
//   prioridad  -> ALTA | MEDIA | BAJA
//
// El parser construye al mismo tiempo:
//   1. Un ParseTree (CST) que se exporta a Graphviz.
//   2. Un Tablero (modelo semantico) usado por los reportes.
//
// Al detectar un error sintactico se registra en ErrorManager y se entra en
// modo panico: el parser consume tokens hasta encontrar un punto de
// sincronizacion (",", ";", "}", "]" o palabras estructurales) y prosigue.
class SyntaxAnalyzer {
public:
    SyntaxAnalyzer(std::vector<Token> tokens, ErrorManager& errorManager);

    // Ejecuta el analisis sintactico. Devuelve la raiz del arbol de
    // derivacion (siempre no nula). El modelo del tablero queda accesible
    // via board().
    ParseNodePtr parse();

    const Tablero& board() const { return board_; }

private:
    // Reglas de la gramatica.
    ParseNodePtr parsePrograma();
    ParseNodePtr parseColumnas();
    ParseNodePtr parseColumna();
    ParseNodePtr parseTareas(Columna& column);
    ParseNodePtr parseTarea(Columna& column);
    ParseNodePtr parseAtributos(Tarea& tarea);
    ParseNodePtr parseAtributo(Tarea& tarea);
    ParseNodePtr parsePrioridad(Tarea& tarea);

    // Cursor sobre la lista de tokens.
    const Token& peek() const;
    const Token& peek(int offset) const;
    const Token& previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool matchAny(std::initializer_list<TokenType> types);
    Token advance();

    // expect: si el siguiente token tiene el tipo esperado lo consume; si no,
    // registra un error sintactico y entra en modo panico.
    Token expect(TokenType type, const std::string& human);

    // Modo panico: consume tokens hasta el siguiente punto de sincronizacion
    // o hasta encontrar uno de los tipos pasados como argumento.
    void synchronize(std::initializer_list<TokenType> stopAt);

    void reportError(const Token& tok, const std::string& description);

    std::vector<Token> tokens_;
    int current_;
    ErrorManager& errorManager_;
    Tablero board_;
    bool panicMode_;
};

}  // namespace taskscript

#endif  // TASKSCRIPT_SYNTAX_ANALYZER_H
