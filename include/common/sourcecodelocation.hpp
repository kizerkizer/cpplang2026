#pragma once

#include <tuple>

typedef std::tuple<int, int, int> SourceCodeLocation;

typedef struct {
    SourceCodeLocation start;
    SourceCodeLocation end;
} SourceCodeLocationSpan;