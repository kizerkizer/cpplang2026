#pragma once

#include "diagnostics/diagnostics.hpp"
#include "treewalker/outputstream.hpp"
#include "common/source.hpp"

class Driver {
private:
    Diagnostics& m_diagnostics;
    OutputStream* m_outputStream;
public:
    Driver(Diagnostics& diagnostics, OutputStream* outputStream) : m_diagnostics(diagnostics), m_outputStream(outputStream) {}
    void compile(Source* source);
};