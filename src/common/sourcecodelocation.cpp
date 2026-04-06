#include "common/sourcecodelocation.hpp"
#include <string>

SourceCodeLocationSpan emptySourceCodeLocationSpan = SourceCodeLocationSpan(SourceCodeLocation{ -1, -1, -1, -1 }, SourceCodeLocation{ -1, -1, -1, -1 });

std::string sourceCodeLocationToString(const SourceCodeLocation& sourceCodeLocation) {
    auto [byteIndex, codepointIndex, line, column] = sourceCodeLocation;
    return std::to_string(line) + ":" + std::to_string(column);
}

std::string sourceCodeLocationSpanToString(const SourceCodeLocationSpan& sourceCodeLocationSpan) {
    return sourceCodeLocationToString(sourceCodeLocationSpan.start) + "-" + sourceCodeLocationToString(sourceCodeLocationSpan.end);
}