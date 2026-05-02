#ifndef TASKSCRIPT_ERROR_MANAGER_H
#define TASKSCRIPT_ERROR_MANAGER_H

#include <string>
#include <vector>

namespace taskscript {

enum class ErrorType {
    Lexico,
    Sintactico
};

enum class ErrorSeverity {
    Error,
    Critico
};

struct AnalysisError {
    int number;                 // Numero incremental, asignado por ErrorManager.
    std::string lexeme;         // Lexema/Token que causa el error.
    ErrorType type;             // Lexico o Sintactico.
    std::string description;    // Mensaje legible para el usuario.
    int line;                   // Linea (1-indexada).
    int column;                 // Columna (1-indexada).
    ErrorSeverity severity;     // ERROR o CRITICO.
};

std::string errorTypeToString(ErrorType type);
std::string errorSeverityToString(ErrorSeverity severity);

// Acumula los errores lexicos y sintacticos detectados durante el analisis.
// El analisis NO se detiene tras un error: cada error es agregado y se
// continua hasta finalizar el archivo. Esto permite reportar todos los
// errores en una sola pasada.
class ErrorManager {
public:
    ErrorManager() = default;

    // Registra un error lexico nuevo.
    void addLexicalError(const std::string& lexeme,
                         const std::string& description,
                         int line,
                         int column,
                         ErrorSeverity severity = ErrorSeverity::Error);

    // Registra un error sintactico nuevo.
    void addSyntaxError(const std::string& lexeme,
                        const std::string& description,
                        int line,
                        int column,
                        ErrorSeverity severity = ErrorSeverity::Error);

    const std::vector<AnalysisError>& errors() const { return errors_; }

    bool hasErrors() const { return !errors_.empty(); }

    bool hasLexicalErrors() const;
    bool hasSyntaxErrors() const;

    void clear() { errors_.clear(); }

private:
    std::vector<AnalysisError> errors_;
};

}  // namespace taskscript

#endif  // TASKSCRIPT_ERROR_MANAGER_H
