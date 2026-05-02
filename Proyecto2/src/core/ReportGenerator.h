#ifndef TASKSCRIPT_REPORT_GENERATOR_H
#define TASKSCRIPT_REPORT_GENERATOR_H

#include <string>

#include "BoardModel.h"
#include "ParseTree.h"

namespace taskscript {

// Genera los artefactos de salida solicitados por el enunciado:
//   * Reporte 1: Tablero Kanban Visual (HTML + CSS embebido).
//   * Reporte 2: Carga por Responsable (HTML + CSS embebido).
//   * Arbol de derivacion en formato Graphviz DOT.
class ReportGenerator {
public:
    static std::string generateKanbanHtml(const Tablero& board);
    static std::string generateCargaResponsableHtml(const Tablero& board);
    static std::string generateDot(const ParseNodePtr& root);

    // Helpers de escritura a disco. Devuelven true en caso de exito.
    static bool writeToFile(const std::string& path, const std::string& content);
};

}  // namespace taskscript

#endif  // TASKSCRIPT_REPORT_GENERATOR_H
