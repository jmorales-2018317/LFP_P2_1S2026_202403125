# Proyecto 2 — TaskScript

Carpeta interna requerida por la sección 10 del enunciado. Aquí vive todo
el código fuente, documentación, casos de prueba y ejemplos del proyecto.

## Compilación rápida

Desde la raíz del repositorio (no desde esta carpeta):

```bash
cmake -S Proyecto2/src -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/TaskScriptApp        # Linux
open build/TaskScriptApp.app # macOS
```

> Detalles, requisitos y solución de problemas en el
> [README de la raíz](../README.md) y en
> [`docs/ManualUsuario.md`](docs/ManualUsuario.md).

## Mapa de directorios

| Carpeta | Contenido |
|---------|-----------|
| [`src/core/`](src/core/) | Núcleo independiente de Qt: `Token`, `LexicalAnalyzer`, `SyntaxAnalyzer`, `ParseTree`, `BoardModel`, `ErrorManager`, `ReportGenerator`. |
| [`src/gui/`](src/gui/) | Aplicación Qt 6 (`MainWindow`, `main.cpp`). |
| [`src/CMakeLists.txt`](src/CMakeLists.txt) | Define la librería `taskscript_core` y el ejecutable `TaskScriptApp`. |
| [`docs/`](docs/) | Manuales en Markdown listos para exportar a PDF. |
| [`tests/`](tests/) | 10 archivos `.task` que cubren entradas válidas, errores léxicos, sintácticos, múltiples errores y casos borde. |
| [`examples/`](examples/) | 2 archivos `.task` completos. |

## Comprobación rápida del núcleo

El núcleo no depende de Qt. Si solo se desea verificar el lexer/parser
puede compilarse aisladamente:

```bash
cmake -S Proyecto2/src -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target taskscript_core
```

Esto produce `build/libtaskscript_core.a`, que puede enlazarse desde una
herramienta CLI propia.
