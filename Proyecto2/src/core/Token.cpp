#include "Token.h"

namespace taskscript {

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::TABLERO:        return "TABLERO";
        case TokenType::COLUMNA:        return "COLUMNA";
        case TokenType::TAREA_KW:       return "tarea";
        case TokenType::PRIORIDAD_KW:   return "prioridad";
        case TokenType::RESPONSABLE_KW: return "responsable";
        case TokenType::FECHA_LIMITE_KW:return "fecha_limite";
        case TokenType::ALTA:           return "ALTA";
        case TokenType::MEDIA:          return "MEDIA";
        case TokenType::BAJA:           return "BAJA";
        case TokenType::CADENA:         return "CADENA";
        case TokenType::FECHA:          return "FECHA";
        case TokenType::ENTERO:         return "ENTERO";
        case TokenType::LBRACE:         return "LLAVE_IZQ";
        case TokenType::RBRACE:         return "LLAVE_DER";
        case TokenType::LBRACKET:       return "CORCHETE_IZQ";
        case TokenType::RBRACKET:       return "CORCHETE_DER";
        case TokenType::COLON:          return "DOS_PUNTOS";
        case TokenType::COMMA:          return "COMA";
        case TokenType::SEMICOLON:      return "PUNTO_Y_COMA";
        case TokenType::END_OF_FILE:    return "EOF";
        case TokenType::INVALID:        return "INVALIDO";
    }
    return "DESCONOCIDO";
}

}  // namespace taskscript
