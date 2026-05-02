#ifndef TASKSCRIPT_PARSE_TREE_H
#define TASKSCRIPT_PARSE_TREE_H

#include <memory>
#include <string>
#include <vector>

namespace taskscript {

class ParseNode;
using ParseNodePtr = std::shared_ptr<ParseNode>;

// Nodo del arbol de derivacion (concrete syntax tree).
//
//   * Los nodos NO TERMINALES llevan como label el nombre de la produccion
//     entre angulos, p.ej. "<programa>".
//   * Los nodos TERMINALES llevan como label el lexema o el nombre del
//     terminal cuando este sea un simbolo unico (p.ej. "TABLERO", "{").
class ParseNode {
public:
    static ParseNodePtr makeNonTerminal(const std::string& label) {
        return std::make_shared<ParseNode>(label, false);
    }
    static ParseNodePtr makeTerminal(const std::string& label) {
        return std::make_shared<ParseNode>(label, true);
    }

    ParseNode(std::string label, bool terminal)
        : label_(std::move(label)), terminal_(terminal) {}

    const std::string& label() const { return label_; }
    bool isTerminal() const { return terminal_; }
    const std::vector<ParseNodePtr>& children() const { return children_; }

    void addChild(ParseNodePtr child) {
        children_.push_back(std::move(child));
    }
    void addTerminal(const std::string& lexeme) {
        children_.push_back(makeTerminal(lexeme));
    }

private:
    std::string label_;
    bool terminal_;
    std::vector<ParseNodePtr> children_;
};

}  // namespace taskscript

#endif  // TASKSCRIPT_PARSE_TREE_H
