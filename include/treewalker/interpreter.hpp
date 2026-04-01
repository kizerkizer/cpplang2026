#pragma once

#include "parser/node.hpp"
#include "treewalker/environment.hpp"
#include "treewalker/outputstream.hpp"

class Interpreter {
public:
    void interpret(Node* node, Environment* environment, OutputStream* outputStream);
};