#pragma once

#include "diagnostics/diagnostics.hpp"
#include "parser/node.hpp"

class Desugarer {
public:
    Desugarer(std::unique_ptr<Node> root, Diagnostics& diagnostics) : root(std::move(root)), diagnostics(diagnostics) {};
    std::unique_ptr<Node> desugar();
private:
    std::unique_ptr<Node> root;
    Diagnostics& diagnostics;
};