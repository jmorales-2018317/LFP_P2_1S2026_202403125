#include "ErrorManager.h"

namespace taskscript {

std::string errorTypeToString(ErrorType type) {
    switch (type) {
        case ErrorType::Lexico:     return "Lexico";
        case ErrorType::Sintactico: return "Sintactico";
    }
    return "Desconocido";
}

std::string errorSeverityToString(ErrorSeverity severity) {
    switch (severity) {
        case ErrorSeverity::Error:   return "ERROR";
        case ErrorSeverity::Critico: return "CRITICO";
    }
    return "DESCONOCIDO";
}

void ErrorManager::addLexicalError(const std::string& lexeme,
                                   const std::string& description,
                                   int line,
                                   int column,
                                   ErrorSeverity severity) {
    AnalysisError err;
    err.number = static_cast<int>(errors_.size()) + 1;
    err.lexeme = lexeme;
    err.type = ErrorType::Lexico;
    err.description = description;
    err.line = line;
    err.column = column;
    err.severity = severity;
    errors_.push_back(err);
}

void ErrorManager::addSyntaxError(const std::string& lexeme,
                                  const std::string& description,
                                  int line,
                                  int column,
                                  ErrorSeverity severity) {
    AnalysisError err;
    err.number = static_cast<int>(errors_.size()) + 1;
    err.lexeme = lexeme;
    err.type = ErrorType::Sintactico;
    err.description = description;
    err.line = line;
    err.column = column;
    err.severity = severity;
    errors_.push_back(err);
}

bool ErrorManager::hasLexicalErrors() const {
    for (const auto& e : errors_) {
        if (e.type == ErrorType::Lexico) return true;
    }
    return false;
}

bool ErrorManager::hasSyntaxErrors() const {
    for (const auto& e : errors_) {
        if (e.type == ErrorType::Sintactico) return true;
    }
    return false;
}

}  // namespace taskscript
