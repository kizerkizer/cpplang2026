#pragma once

#include "diagnostics/diagnostics.hpp"
#include "parser/node.hpp"

class Desugarer {
private:
    std::unique_ptr<Node> m_root;
    Diagnostics& m_diagnostics;
    std::unique_ptr<Node> desugarRecursive(std::unique_ptr<Node> node);
    std::unique_ptr<Node> desugarWhileStatement(std::unique_ptr<Node> node);
public:
    Desugarer(std::unique_ptr<Node> root, Diagnostics& diagnostics) : m_root(std::move(root)), m_diagnostics(diagnostics) {};
    std::unique_ptr<Node> desugar();
};