#ifndef TASKSCRIPT_BOARD_MODEL_H
#define TASKSCRIPT_BOARD_MODEL_H

#include <string>
#include <vector>

namespace taskscript {

// Representacion semantica del tablero. La construye el parser conforme
// reconoce la entrada y la consume el ReportGenerator.

enum class Prioridad {
    Alta,
    Media,
    Baja,
    Indefinida
};

inline std::string prioridadToString(Prioridad p) {
    switch (p) {
        case Prioridad::Alta:   return "ALTA";
        case Prioridad::Media:  return "MEDIA";
        case Prioridad::Baja:   return "BAJA";
        case Prioridad::Indefinida: return "INDEFINIDA";
    }
    return "INDEFINIDA";
}

struct Tarea {
    std::string nombre;
    Prioridad prioridad = Prioridad::Indefinida;
    std::string responsable;
    std::string fechaLimite;  // formato AAAA-MM-DD
    int line = 0;
    int column = 0;
};

struct Columna {
    std::string nombre;
    std::vector<Tarea> tareas;
};

struct Tablero {
    std::string nombre;
    std::vector<Columna> columnas;

    bool valid = false;  // true si el parser logro construir un tablero coherente
};

}  // namespace taskscript

#endif  // TASKSCRIPT_BOARD_MODEL_H
