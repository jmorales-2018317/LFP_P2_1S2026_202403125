cmake_minimum_required(VERSION 3.16)
project(TaskScript LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Auto-handling para Qt (moc/uic/rcc).
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Pista para encontrar Qt en macOS con Homebrew.
if(APPLE AND NOT DEFINED CMAKE_PREFIX_PATH)
    execute_process(
        COMMAND brew --prefix qt
        OUTPUT_VARIABLE BREW_QT_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE BREW_QT_RESULT
    )
    if(BREW_QT_RESULT EQUAL 0 AND BREW_QT_PREFIX)
        list(APPEND CMAKE_PREFIX_PATH "${BREW_QT_PREFIX}")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ---------- Libreria estatica del nucleo (sin dependencia de Qt) ----------
set(TASKSCRIPT_CORE_SOURCES
    core/Token.cpp
    core/Token.h
    core/LexicalAnalyzer.cpp
    core/LexicalAnalyzer.h
    core/SyntaxAnalyzer.cpp
    core/SyntaxAnalyzer.h
    core/ErrorManager.cpp
    core/ErrorManager.h
    core/ParseTree.h
    core/BoardModel.h
    core/ReportGenerator.cpp
    core/ReportGenerator.h
)

add_library(taskscript_core STATIC ${TASKSCRIPT_CORE_SOURCES})
target_include_directories(taskscript_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ---------- Ejecutable Qt ----------
set(TASKSCRIPT_APP_SOURCES
    gui/main.cpp
    gui/MainWindow.cpp
    gui/MainWindow.h
)

qt_add_executable(TaskScriptApp ${TASKSCRIPT_APP_SOURCES})

target_link_libraries(TaskScriptApp PRIVATE
    taskscript_core
    Qt6::Widgets
)

if(APPLE)
    set_target_properties(TaskScriptApp PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "TaskScript"
        MACOSX_BUNDLE_GUI_IDENTIFIER "edu.usac.lfp.taskscript"
    )
endif()

# Mensaje informativo al final de la configuracion.
message(STATUS "TaskScript: Qt6 encontrado en ${Qt6_DIR}")
message(STATUS "TaskScript: Salida principal -> TaskScriptApp")
