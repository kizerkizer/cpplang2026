#pragma once

#include <string>

#include "common/source.hpp"

using SourceCodeLocation = struct {
    int byteIndex;
    int codepointIndex;
    int line;
    int column;
};

using SourceCodeLocationSpan = struct {
    SourceCodeLocation start;
    SourceCodeLocation end;
};

using FullSourceCodeLocation = struct {
    Source* source;
    SourceCodeLocationSpan sourceCodeLocationSpan;
};

namespace englishbreakfast {

extern SourceCodeLocationSpan emptySourceCodeLocationSpan;

std::string sourceCodeLocationToString(const SourceCodeLocation& sourceCodeLocation);

std::string sourceCodeLocationSpanToString(const SourceCodeLocationSpan& sourceCodeLocationSpan);

} // namespace englishbreakfast