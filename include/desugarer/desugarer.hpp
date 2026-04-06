#pragma once

#include "diagnostics/diagnostics.hpp"
#include "parser/node.hpp"

class Desugarer {
private:
    std::unique_ptr<Node> root;
    Diagnostics& diagnostics;
    std::unique_ptr<Node> _desugar(std::unique_ptr<Node> node);
public:
    Desugarer(std::unique_ptr<Node> root, Diagnostics& diagnostics) : root(std::move(root)), diagnostics(diagnostics) {};
    std::unique_ptr<Node> desugar();
};