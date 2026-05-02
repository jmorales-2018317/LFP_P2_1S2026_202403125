#include "ReportGenerator.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

namespace taskscript {

namespace {

std::string escapeHtml(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&':  out += "&amp;"; break;
            case '<':  out += "&lt;"; break;
            case '>':  out += "&gt;"; break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default:   out.push_back(c); break;
        }
    }
    return out;
}

std::string escapeDot(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        if (c == '"' || c == '\\') {
            out.push_back('\\');
            out.push_back(c);
        } else if (c == '\n') {
            out += "\\n";
        } else {
            out.push_back(c);
        }
    }
    return out;
}

const char* prioridadColor(Prioridad p) {
    switch (p) {
        case Prioridad::Alta:  return "#E74C3C";  // rojo
        case Prioridad::Media: return "#F1C40F";  // amarillo
        case Prioridad::Baja:  return "#27AE60";  // verde
        case Prioridad::Indefinida: return "#7F8C8D";
    }
    return "#7F8C8D";
}

const char* prioridadTextColor(Prioridad p) {
    switch (p) {
        case Prioridad::Media: return "#2C3E50";
        default: return "#FFFFFF";
    }
}

void emitDotNode(std::ostringstream& out, const ParseNodePtr& node, int& counter,
                 int parentId) {
    int myId = counter++;
    const char* fill = node->isTerminal() ? "#D6EAF8" : "#2E75B6";
    const char* fontColor = node->isTerminal() ? "#1B2631" : "#FFFFFF";
    out << "  n" << myId << " [label=\"" << escapeDot(node->label())
        << "\", fillcolor=\"" << fill << "\", fontcolor=\"" << fontColor << "\"];\n";
    if (parentId >= 0) {
        out << "  n" << parentId << " -> n" << myId << ";\n";
    }
    for (const auto& child : node->children()) {
        emitDotNode(out, child, counter, myId);
    }
}

}  // namespace

std::string ReportGenerator::generateKanbanHtml(const Tablero& board) {
    std::ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"es\">\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<title>Tablero Kanban - " << escapeHtml(board.nombre) << "</title>\n";
    html << "<style>\n";
    html << "  * { box-sizing: border-box; }\n";
    html << "  body { margin: 0; font-family: 'Segoe UI', Arial, sans-serif; "
            "background: linear-gradient(135deg, #2C3E50, #34495E); color: #ECF0F1; min-height: 100vh; }\n";
    html << "  header { padding: 24px 32px; background: rgba(0,0,0,0.25); "
            "border-bottom: 1px solid rgba(255,255,255,0.08); }\n";
    html << "  header h1 { margin: 0; font-size: 28px; }\n";
    html << "  header p { margin: 4px 0 0; color: #BDC3C7; font-size: 14px; }\n";
    html << "  .board { display: flex; gap: 18px; padding: 24px; overflow-x: auto; }\n";
    html << "  .column { flex: 0 0 320px; background: rgba(255,255,255,0.06); "
            "border-radius: 10px; padding: 16px; box-shadow: 0 4px 16px rgba(0,0,0,0.25); }\n";
    html << "  .column h2 { margin: 0 0 12px; font-size: 18px; "
            "border-bottom: 2px solid rgba(255,255,255,0.15); padding-bottom: 8px; }\n";
    html << "  .column .count { float: right; font-size: 13px; color: #BDC3C7; }\n";
    html << "  .card { background: #FFFFFF; color: #2C3E50; border-radius: 8px; "
            "padding: 12px 14px; margin-bottom: 12px; box-shadow: 0 2px 6px rgba(0,0,0,0.18); }\n";
    html << "  .card h3 { margin: 0 0 8px; font-size: 15px; }\n";
    html << "  .badge { display: inline-block; font-size: 11px; font-weight: 700; "
            "padding: 3px 8px; border-radius: 999px; letter-spacing: 0.5px; margin-bottom: 8px; }\n";
    html << "  .meta { font-size: 12px; color: #5D6D7E; margin-top: 6px; }\n";
    html << "  .meta span { display: block; }\n";
    html << "  footer { text-align: center; padding: 18px; color: #BDC3C7; font-size: 12px; }\n";
    html << "</style>\n</head>\n<body>\n";

    html << "<header>\n";
    html << "  <h1>Tablero: " << escapeHtml(board.nombre) << "</h1>\n";
    html << "  <p>Reporte 1 - Tablero Kanban Visual generado por TaskScript.</p>\n";
    html << "</header>\n";

    html << "<section class=\"board\">\n";
    for (const auto& col : board.columnas) {
        html << "  <article class=\"column\">\n";
        html << "    <h2>" << escapeHtml(col.nombre)
             << "<span class=\"count\">" << col.tareas.size() << " tareas</span></h2>\n";
        for (const auto& t : col.tareas) {
            html << "    <div class=\"card\">\n";
            html << "      <span class=\"badge\" style=\"background:" << prioridadColor(t.prioridad)
                 << ";color:" << prioridadTextColor(t.prioridad) << ";\">"
                 << escapeHtml(prioridadToString(t.prioridad)) << "</span>\n";
            html << "      <h3>" << escapeHtml(t.nombre) << "</h3>\n";
            html << "      <div class=\"meta\">\n";
            if (!t.responsable.empty()) {
                html << "        <span><strong>Responsable:</strong> "
                     << escapeHtml(t.responsable) << "</span>\n";
            }
            if (!t.fechaLimite.empty()) {
                html << "        <span><strong>Fecha limite:</strong> "
                     << escapeHtml(t.fechaLimite) << "</span>\n";
            }
            html << "      </div>\n";
            html << "    </div>\n";
        }
        if (col.tareas.empty()) {
            html << "    <p style=\"color:#BDC3C7;font-style:italic;\">"
                    "Sin tareas en esta columna.</p>\n";
        }
        html << "  </article>\n";
    }
    html << "</section>\n";

    html << "<footer>Generado por TaskScript - Universidad de San Carlos de Guatemala</footer>\n";
    html << "</body>\n</html>\n";
    return html.str();
}

std::string ReportGenerator::generateCargaResponsableHtml(const Tablero& board) {
    struct Resumen {
        int total = 0;
        int alta = 0;
        int media = 0;
        int baja = 0;
        int sin = 0;
    };
    std::map<std::string, Resumen> porPersona;
    int totalTareas = 0;
    for (const auto& col : board.columnas) {
        for (const auto& t : col.tareas) {
            std::string nombre = t.responsable.empty() ? "(Sin asignar)" : t.responsable;
            auto& r = porPersona[nombre];
            r.total++;
            switch (t.prioridad) {
                case Prioridad::Alta:  r.alta++; break;
                case Prioridad::Media: r.media++; break;
                case Prioridad::Baja:  r.baja++; break;
                case Prioridad::Indefinida: r.sin++; break;
            }
            totalTareas++;
        }
    }

    std::vector<std::pair<std::string, Resumen>> orden(porPersona.begin(), porPersona.end());
    std::sort(orden.begin(), orden.end(), [](const auto& a, const auto& b) {
        if (a.second.total != b.second.total) return a.second.total > b.second.total;
        return a.first < b.first;
    });

    std::ostringstream html;
    html << "<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<title>Carga por Responsable - " << escapeHtml(board.nombre) << "</title>\n";
    html << "<style>\n";
    html << "  body { margin: 0; font-family: 'Segoe UI', Arial, sans-serif; "
            "background: #F4F6F8; color: #2C3E50; }\n";
    html << "  header { padding: 24px 32px; background: #2E75B6; color: white; }\n";
    html << "  header h1 { margin: 0; font-size: 26px; }\n";
    html << "  header p { margin: 4px 0 0; opacity: 0.9; font-size: 14px; }\n";
    html << "  main { padding: 24px 32px; }\n";
    html << "  table { width: 100%; border-collapse: collapse; "
            "background: white; box-shadow: 0 2px 8px rgba(0,0,0,0.08); border-radius: 8px; overflow: hidden; }\n";
    html << "  th, td { padding: 12px 14px; text-align: left; border-bottom: 1px solid #ECEFF1; }\n";
    html << "  th { background: #34495E; color: white; font-size: 13px; "
            "letter-spacing: 0.4px; text-transform: uppercase; }\n";
    html << "  tr:last-child td { border-bottom: none; }\n";
    html << "  td.num { text-align: center; }\n";
    html << "  .bar-wrap { background: #ECEFF1; border-radius: 6px; height: 16px; overflow: hidden; min-width: 180px; }\n";
    html << "  .bar { height: 100%; background: linear-gradient(90deg, #2E75B6, #5DADE2); "
            "color: white; font-size: 11px; padding-left: 6px; line-height: 16px; "
            "white-space: nowrap; }\n";
    html << "  .pill { display: inline-block; min-width: 28px; padding: 2px 8px; border-radius: 12px; "
            "font-size: 12px; font-weight: 600; color: white; }\n";
    html << "  .alta { background: #E74C3C; }\n";
    html << "  .media { background: #F1C40F; color: #2C3E50; }\n";
    html << "  .baja { background: #27AE60; }\n";
    html << "  .sin { background: #7F8C8D; }\n";
    html << "  .summary { margin-bottom: 16px; color: #5D6D7E; font-size: 14px; }\n";
    html << "  footer { text-align: center; padding: 18px; color: #7F8C8D; font-size: 12px; }\n";
    html << "</style>\n</head>\n<body>\n";

    html << "<header>\n";
    html << "  <h1>Carga por Responsable</h1>\n";
    html << "  <p>Reporte 2 - Tablero \"" << escapeHtml(board.nombre)
         << "\". Total de tareas registradas: " << totalTareas << ".</p>\n";
    html << "</header>\n";
    html << "<main>\n";
    html << "  <p class=\"summary\">Cada barra muestra el porcentaje de tareas asignadas "
            "al responsable sobre el total del tablero, junto al desglose por nivel de prioridad.</p>\n";
    html << "  <table>\n";
    html << "    <thead><tr><th>Responsable</th><th>Total</th>"
            "<th>ALTA</th><th>MEDIA</th><th>BAJA</th><th>Sin prioridad</th>"
            "<th>Distribucion</th></tr></thead>\n";
    html << "    <tbody>\n";
    for (const auto& [nombre, r] : orden) {
        double pct = totalTareas > 0 ? (100.0 * r.total / totalTareas) : 0.0;
        html << "      <tr>";
        html << "<td>" << escapeHtml(nombre) << "</td>";
        html << "<td class=\"num\"><strong>" << r.total << "</strong></td>";
        html << "<td class=\"num\"><span class=\"pill alta\">" << r.alta << "</span></td>";
        html << "<td class=\"num\"><span class=\"pill media\">" << r.media << "</span></td>";
        html << "<td class=\"num\"><span class=\"pill baja\">" << r.baja << "</span></td>";
        html << "<td class=\"num\"><span class=\"pill sin\">" << r.sin << "</span></td>";
        html << "<td><div class=\"bar-wrap\"><div class=\"bar\" style=\"width:"
             << static_cast<int>(pct + 0.5) << "%;\">"
             << static_cast<int>(pct + 0.5) << "%</div></div></td>";
        html << "</tr>\n";
    }
    if (orden.empty()) {
        html << "      <tr><td colspan=\"7\" style=\"text-align:center;color:#7F8C8D;\">"
                "Sin tareas registradas en el tablero.</td></tr>\n";
    }
    html << "    </tbody>\n";
    html << "  </table>\n";
    html << "</main>\n";
    html << "<footer>Generado por TaskScript - Universidad de San Carlos de Guatemala</footer>\n";
    html << "</body>\n</html>\n";
    return html.str();
}

std::string ReportGenerator::generateDot(const ParseNodePtr& root) {
    std::ostringstream out;
    out << "digraph ArbolDerivacion {\n";
    out << "  rankdir=TB;\n";
    out << "  node [shape=box, style=filled, fontname=\"Arial\"];\n";
    if (root) {
        int counter = 0;
        emitDotNode(out, root, counter, -1);
    }
    out << "}\n";
    return out.str();
}

bool ReportGenerator::writeToFile(const std::string& path, const std::string& content) {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return false;
    ofs << content;
    return ofs.good();
}

}  // namespace taskscript
