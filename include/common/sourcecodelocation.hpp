#pragma once

#include <string>
#include <tuple>

typedef std::tuple<int, int, int> SourceCodeLocation;

typedef struct {
    SourceCodeLocation start;
    SourceCodeLocation end;
} SourceCodeLocationSpan;

extern SourceCodeLocationSpan emptySourceCodeLocationSpan;

std::string sourceCodeLocationToString(const SourceCodeLocation& sourceCodeLocation);

std::string sourceCodeLocationSpanToString(const SourceCodeLocationSpan& sourceCodeLocationSpan);