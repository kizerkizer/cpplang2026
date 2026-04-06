#include "common/util.hpp"

[[noreturn]] void unreachable(const char* message) {
    fprintf(stderr, "%s\n", message);
    UNREACHABLE();
}