#include "SyntaxAnalyzer.h"

#include <algorithm>
#include <utility>

namespace taskscript {

namespace {

std::string quoted(const std::string& s) {
    return "'" + s + "'";
}

}  // namespace

SyntaxAnalyzer::SyntaxAnalyzer(std::vector<Token> tokens, ErrorManager& errorManager)
    : tokens_(std::move(tokens)),
      current_(0),
      errorManager_(errorManager),
      board_(),
      panicMode_(false) {}

const Token& SyntaxAnalyzer::peek() const {
    return tokens_[current_];
}

const Token& SyntaxAnalyzer::peek(int offset) const {
    int idx = current_ + offset;
    if (idx < 0) idx = 0;
    if (idx >= static_cast<int>(tokens_.size())) idx = static_cast<int>(tokens_.size()) - 1;
    return tokens_[idx];
}

const Token& SyntaxAnalyzer::previous() const {
    return tokens_[std::max(0, current_ - 1)];
}

bool SyntaxAnalyzer::isAtEnd() const {
    return peek().type() == TokenType::END_OF_FILE;
}

bool SyntaxAnalyzer::check(TokenType type) const {
    if (isAtEnd()) return type == TokenType::END_OF_FILE;
    return peek().type() == type;
}

bool SyntaxAnalyzer::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool SyntaxAnalyzer::matchAny(std::initializer_list<TokenType> types) {
    for (TokenType t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

Token SyntaxAnalyzer::advance() {
    Token t = peek();
    if (!isAtEnd()) ++current_;
    return t;
}

void SyntaxAnalyzer::reportError(const Token& tok, const std::string& description) {
    if (panicMode_) return;  // evita cascada de errores
    panicMode_ = true;
    std::string lex = tok.lexeme().empty() ? std::string("EOF") : tok.lexeme();
    errorManager_.addSyntaxError(lex, description, tok.line(), tok.column());
}

Token SyntaxAnalyzer::expect(TokenType type, const std::string& human) {
    if (check(type)) {
        panicMode_ = false;
        return advance();
    }
    reportError(peek(),
                "Se esperaba " + human + ", se encontro " + quoted(peek().lexeme()) + ".");
    // Modo panico hasta encontrar el token esperado o un sincronizador.
    return Token(TokenType::INVALID, "", peek().line(), peek().column());
}

void SyntaxAnalyzer::synchronize(std::initializer_list<TokenType> stopAt) {
    while (!isAtEnd()) {
        TokenType t = peek().type();
        for (TokenType s : stopAt) {
            if (t == s) {
                panicMode_ = false;
                return;
            }
        }
        // Tokens estructurales tambien son buenos puntos de re-entrada.
        if (t == TokenType::TABLERO || t == TokenType::COLUMNA ||
            t == TokenType::TAREA_KW) {
            panicMode_ = false;
            return;
        }
        advance();
    }
    panicMode_ = false;
}

ParseNodePtr SyntaxAnalyzer::parse() {
    auto root = parsePrograma();
    return root;
}

ParseNodePtr SyntaxAnalyzer::parsePrograma() {
    auto node = ParseNode::makeNonTerminal("<programa>");

    if (check(TokenType::TABLERO)) {
        Token t = advance();
        node->addTerminal(t.lexeme());
    } else {
        reportError(peek(), "Se esperaba la palabra reservada TABLERO al inicio del archivo.");
        synchronize({TokenType::TABLERO, TokenType::LBRACE});
        if (check(TokenType::TABLERO)) advance();
    }

    if (check(TokenType::CADENA)) {
        Token t = advance();
        board_.nombre = t.lexeme();
        node->addTerminal("\"" + t.lexeme() + "\"");
    } else {
        reportError(peek(), "Se esperaba el nombre del tablero (cadena entre comillas).");
        synchronize({TokenType::LBRACE});
    }

    if (match(TokenType::LBRACE)) {
        node->addTerminal("{");
    } else {
        reportError(peek(), "Se esperaba '{' para abrir el bloque del tablero.");
        synchronize({TokenType::COLUMNA, TokenType::RBRACE});
    }

    auto columnas = parseColumnas();
    if (columnas) node->addChild(columnas);

    if (match(TokenType::RBRACE)) {
        node->addTerminal("}");
    } else {
        reportError(peek(), "Se esperaba '}' para cerrar el bloque del tablero.");
        synchronize({TokenType::SEMICOLON});
    }

    if (match(TokenType::SEMICOLON)) {
        node->addTerminal(";");
    } else {
        reportError(peek(), "Se esperaba ';' al final del bloque del tablero.");
        synchronize({TokenType::END_OF_FILE});
    }

    board_.valid = !errorManager_.hasErrors();
    return node;
}

ParseNodePtr SyntaxAnalyzer::parseColumnas() {
    auto node = ParseNode::makeNonTerminal("<columnas>");
    while (check(TokenType::COLUMNA)) {
        auto col = parseColumna();
        if (col) node->addChild(col);
    }
    return node;
}

ParseNodePtr SyntaxAnalyzer::parseColumna() {
    auto node = ParseNode::makeNonTerminal("<columna>");
    Columna columna;

    if (check(TokenType::COLUMNA)) {
        Token t = advance();
        node->addTerminal(t.lexeme());
    } else {
        reportError(peek(), "Se esperaba la palabra reservada COLUMNA.");
        synchronize({TokenType::CADENA, TokenType::LBRACE, TokenType::SEMICOLON});
    }

    if (check(TokenType::CADENA)) {
        Token t = advance();
        columna.nombre = t.lexeme();
        node->addTerminal("\"" + t.lexeme() + "\"");
    } else {
        reportError(peek(), "Se esperaba el nombre de la columna (cadena entre comillas).");
        synchronize({TokenType::LBRACE});
    }

    if (match(TokenType::LBRACE)) {
        node->addTerminal("{");
    } else {
        reportError(peek(), "Se esperaba '{' para abrir el bloque de la columna.");
        synchronize({TokenType::TAREA_KW, TokenType::RBRACE});
    }

    auto tareas = parseTareas(columna);
    if (tareas) node->addChild(tareas);

    if (match(TokenType::RBRACE)) {
        node->addTerminal("}");
    } else {
        reportError(peek(), "Se esperaba '}' para cerrar el bloque de la columna.");
        synchronize({TokenType::SEMICOLON, TokenType::COLUMNA, TokenType::RBRACE});
    }

    if (match(TokenType::SEMICOLON)) {
        node->addTerminal(";");
    } else {
        reportError(peek(), "Se esperaba ';' al final de la columna.");
        synchronize({TokenType::COLUMNA, TokenType::RBRACE});
    }

    board_.columnas.push_back(std::move(columna));
    return node;
}

ParseNodePtr SyntaxAnalyzer::parseTareas(Columna& column) {
    auto node = ParseNode::makeNonTerminal("<tareas>");
    if (!check(TokenType::TAREA_KW)) {
        // Se permite columna sin tareas como caso borde.
        return node;
    }

    auto t = parseTarea(column);
    if (t) node->addChild(t);

    while (check(TokenType::COMMA)) {
        advance();
        node->addTerminal(",");
        if (check(TokenType::TAREA_KW)) {
            auto more = parseTarea(column);
            if (more) node->addChild(more);
        } else {
            // Coma colgante valida (ver ejemplo del PDF).
            break;
        }
    }
    return node;
}

ParseNodePtr SyntaxAnalyzer::parseTarea(Columna& column) {
    auto node = ParseNode::makeNonTerminal("<tarea>");
    Tarea tarea;

    if (check(TokenType::TAREA_KW)) {
        Token t = advance();
        tarea.line = t.line();
        tarea.column = t.column();
        node->addTerminal(t.lexeme());
    } else {
        reportError(peek(), "Se esperaba la palabra reservada 'tarea'.");
        synchronize({TokenType::COMMA, TokenType::RBRACE, TokenType::SEMICOLON});
        return node;
    }

    if (match(TokenType::COLON)) {
        node->addTerminal(":");
    } else {
        reportError(peek(), "Se esperaba ':' despues de 'tarea'.");
        synchronize({TokenType::CADENA, TokenType::LBRACKET, TokenType::COMMA, TokenType::RBRACE});
    }

    if (check(TokenType::CADENA)) {
        Token t = advance();
        tarea.nombre = t.lexeme();
        node->addTerminal("\"" + t.lexeme() + "\"");
    } else {
        reportError(peek(), "Se esperaba el nombre de la tarea (cadena entre comillas).");
        synchronize({TokenType::LBRACKET, TokenType::COMMA, TokenType::RBRACE});
    }

    if (match(TokenType::LBRACKET)) {
        node->addTerminal("[");
    } else {
        reportError(peek(), "Se esperaba '[' para abrir los atributos de la tarea.");
        synchronize({TokenType::PRIORIDAD_KW, TokenType::RESPONSABLE_KW,
                     TokenType::FECHA_LIMITE_KW, TokenType::RBRACKET,
                     TokenType::COMMA, TokenType::RBRACE});
    }

    auto attrs = parseAtributos(tarea);
    if (attrs) node->addChild(attrs);

    if (match(TokenType::RBRACKET)) {
        node->addTerminal("]");
    } else {
        reportError(peek(), "Se esperaba ']' para cerrar los atributos de la tarea.");
        synchronize({TokenType::COMMA, TokenType::RBRACE, TokenType::SEMICOLON});
    }

    column.tareas.push_back(std::move(tarea));
    return node;
}

ParseNodePtr SyntaxAnalyzer::parseAtributos(Tarea& tarea) {
    auto node = ParseNode::makeNonTerminal("<atributos>");
    if (check(TokenType::RBRACKET)) {
        return node;  // permitimos lista vacia para que el parser no se cuelgue
    }

    auto a = parseAtributo(tarea);
    if (a) node->addChild(a);

    while (check(TokenType::COMMA)) {
        advance();
        node->addTerminal(",");
        if (check(TokenType::RBRACKET)) break;  // coma colgante
        auto more = parseAtributo(tarea);
        if (more) node->addChild(more);
    }
    return node;
}

ParseNodePtr SyntaxAnalyzer::parseAtributo(Tarea& tarea) {
    auto node = ParseNode::makeNonTerminal("<atributo>");

    if (check(TokenType::PRIORIDAD_KW)) {
        Token kw = advance();
        node->addTerminal(kw.lexeme());
        if (match(TokenType::COLON)) {
            node->addTerminal(":");
        } else {
            reportError(peek(), "Se esperaba ':' despues de 'prioridad'.");
            synchronize({TokenType::ALTA, TokenType::MEDIA, TokenType::BAJA,
                         TokenType::COMMA, TokenType::RBRACKET});
        }
        auto p = parsePrioridad(tarea);
        if (p) node->addChild(p);
        return node;
    }

    if (check(TokenType::RESPONSABLE_KW)) {
        Token kw = advance();
        node->addTerminal(kw.lexeme());
        if (match(TokenType::COLON)) {
            node->addTerminal(":");
        } else {
            reportError(peek(), "Se esperaba ':' despues de 'responsable'.");
            synchronize({TokenType::CADENA, TokenType::COMMA, TokenType::RBRACKET});
        }
        if (check(TokenType::CADENA)) {
            Token v = advance();
            tarea.responsable = v.lexeme();
            node->addTerminal("\"" + v.lexeme() + "\"");
        } else {
            reportError(peek(), "Se esperaba una cadena con el nombre del responsable.");
            synchronize({TokenType::COMMA, TokenType::RBRACKET});
        }
        return node;
    }

    if (check(TokenType::FECHA_LIMITE_KW)) {
        Token kw = advance();
        node->addTerminal(kw.lexeme());
        if (match(TokenType::COLON)) {
            node->addTerminal(":");
        } else {
            reportError(peek(), "Se esperaba ':' despues de 'fecha_limite'.");
            synchronize({TokenType::FECHA, TokenType::COMMA, TokenType::RBRACKET});
        }
        if (check(TokenType::FECHA)) {
            Token v = advance();
            tarea.fechaLimite = v.lexeme();
            node->addTerminal(v.lexeme());
        } else {
            reportError(peek(), "Se esperaba una fecha en formato AAAA-MM-DD.");
            synchronize({TokenType::COMMA, TokenType::RBRACKET});
        }
        return node;
    }

    reportError(peek(),
                "Se esperaba un atributo (prioridad/responsable/fecha_limite), se encontro " +
                    quoted(peek().lexeme()) + ".");
    synchronize({TokenType::COMMA, TokenType::RBRACKET});
    return node;
}

ParseNodePtr SyntaxAnalyzer::parsePrioridad(Tarea& tarea) {
    auto node = ParseNode::makeNonTerminal("<prioridad>");
    if (check(TokenType::ALTA)) {
        Token t = advance();
        tarea.prioridad = Prioridad::Alta;
        node->addTerminal(t.lexeme());
    } else if (check(TokenType::MEDIA)) {
        Token t = advance();
        tarea.prioridad = Prioridad::Media;
        node->addTerminal(t.lexeme());
    } else if (check(TokenType::BAJA)) {
        Token t = advance();
        tarea.prioridad = Prioridad::Baja;
        node->addTerminal(t.lexeme());
    } else {
        reportError(peek(),
                    "Se esperaba un nivel de prioridad (ALTA, MEDIA o BAJA), se encontro " +
                        quoted(peek().lexeme()) + ".");
        synchronize({TokenType::COMMA, TokenType::RBRACKET});
    }
    return node;
}

}  // namespace taskscript
