#pragma once

#include "parser/node.hpp"

class Desugarer {
public:
    Desugarer(std::unique_ptr<Node> root) : root(std::move(root)) {};
    std::unique_ptr<Node> desugar();
private:
    std::unique_ptr<Node> root;
};