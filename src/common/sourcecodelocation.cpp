#include "common/sourcecodelocation.hpp"
#include <string>

SourceCodeLocationSpan englishbreakfast::emptySourceCodeLocationSpan = SourceCodeLocationSpan(SourceCodeLocation{ -1, -1, -1, -1 }, SourceCodeLocation{ -1, -1, -1, -1 });

std::string englishbreakfast::sourceCodeLocationToString(const SourceCodeLocation& sourceCodeLocation) {
    auto [byteIndex, codepointIndex, line, column] = sourceCodeLocation;
    return std::to_string(line) + ":" + std::to_string(column);
}

std::string englishbreakfast::sourceCodeLocationSpanToString(const SourceCodeLocationSpan& sourceCodeLocationSpan) {
    return englishbreakfast::sourceCodeLocationToString(sourceCodeLocationSpan.start) + "-" + englishbreakfast::sourceCodeLocationToString(sourceCodeLocationSpan.end);
}