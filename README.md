# LFP_P2_1S2026_202403125 — TaskScript

Proyecto 2 del curso Lenguajes Formales y de Programación
Universidad de San Carlos de Guatemala · Facultad de Ingeniería · Escuela de Ciencias y Sistemas.

- **Carné:** 202403125
- **Lenguaje:** C++17
- **Interfaz gráfica:** Qt 6 (Widgets)
- **Sistema de construcción:** CMake (≥ 3.16)

---

## ¿Qué hace TaskScript?

`TaskScript` es una aplicación de escritorio que analiza léxica y
sintácticamente archivos `.task` con la estructura del mini-lenguaje
TaskScript (gestión de tableros Kanban) y produce:

- **Tabla de tokens** con número, lexema, tipo, línea y columna.
- **Tabla de errores** léxicos y sintácticos con descripción, posición y
  gravedad. El análisis **no se detiene** ante un error: reporta todos los
  problemas en una sola pasada.
- **2 reportes HTML** con CSS embebido (sec. 3.5 del enunciado):
  - Reporte 1 — Tablero Kanban Visual.
  - Reporte 2 — Carga por Responsable.
- **Árbol de derivación** en formato Graphviz (`arbol.dot`).

### Cumplimiento de las restricciones del enunciado

- AFD manual implementado en `LexicalAnalyzer::nextToken()` con función
  de transición explícita. **Sin `std::regex`.**
- Parser descendente recursivo manual con una función por producción
  de la GLC. **Sin ANTLR/Bison/Yacc/Flex.**
- Construcción del árbol de derivación y exportación a Graphviz.
- GUI Qt 6 con todos los paneles requeridos.
- Tabla de tokens y de errores con posición exacta.
- Modo pánico para recuperación de errores sintácticos.

---

## Estructura del repositorio

```
LFP_P2_1S2026_202403125/
└── Proyecto2/
    ├── src/
    │   ├── core/                 Token, Lexer (AFD), Parser, ErrorManager, Reports
    │   ├── gui/                  Aplicación Qt 6 (MainWindow + main.cpp)
    │   └── CMakeLists.txt        Build script
    ├── docs/
    │   ├── ManualTecnico.md      Arquitectura, AFD, GLC, parser
    │   └── ManualUsuario.md      Guía paso a paso de la GUI
    ├── tests/                    10 archivos .task documentados
    ├── examples/                 2 archivos .task completos de ejemplo
    └── README.md                 Documentación específica del Proyecto 2
```

## Compilación paso a paso

### Requisitos

| Componente | Versión mínima |
|------------|----------------|
| Compilador C++ | `g++` 9 / `clang++` 12 / MSVC 2019 (C++17) |
| Qt | 6.x (`QtWidgets`); también soporta Qt 5.15 |
| CMake | 3.16 (recomendado 3.21+) |
| Graphviz (opcional) | para convertir `arbol.dot` a imagen |

### Comandos exactos

```bash
# Desde la raíz del repositorio:
cmake -S Proyecto2/src -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Ejecutar:
./build/TaskScriptApp           # Linux
open build/TaskScriptApp.app    # macOS
build\TaskScriptApp.exe         # Windows (Qt Command Prompt)
```

### Si CMake no encuentra Qt 6

```bash
# macOS (Homebrew):
cmake -S Proyecto2/src -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"

# Linux (paquetes del sistema):
sudo apt install qt6-base-dev cmake build-essential
cmake -S Proyecto2/src -B build

# Windows: usar el "Qt 6 Command Prompt" instalado por el wizard,
# que ya define CMAKE_PREFIX_PATH automáticamente.
```

## Uso rápido

1. Iniciar `TaskScriptApp`.
2. **Cargar archivo** y elegir un `.task` de
   [`Proyecto2/examples/`](Proyecto2/examples/).
3. **Analizar**. Revisar la pestaña **Tokens** y, si las hay, la
   pestaña **Errores**.
4. **Generar reportes HTML** y **Generar árbol DOT** para producir los
   artefactos en `salida_taskscript/` (junto al ejecutable).
5. (Opcional) Convertir el DOT a imagen:

   ```bash
   dot -Tpng salida_taskscript/arbol.dot -o salida_taskscript/arbol.png
   ```

## Documentación

- 📘 [Manual Técnico](Proyecto2/docs/ManualTecnico.md) — arquitectura,
  diagrama UML, diagrama del AFD, GLC completa, descripción del parser y
  justificación de decisiones de diseño.
- 📗 [Manual de Usuario](Proyecto2/docs/ManualUsuario.md) — guía paso a
  paso de la GUI, ejemplos de archivos `.task` y cómo interpretar los
  reportes.
- 🧪 [Casos de prueba](Proyecto2/tests/README.md) — 10 casos con
  entrada, salida esperada y resultado obtenido.

## Licencia y autoría

Trabajo individual desarrollado para el curso de Lenguajes Formales y
de Programación (1S 2026, USAC). Distribuido únicamente con fines
académicos.
