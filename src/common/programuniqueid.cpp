#include "common/programuniqueid.hpp"

int getNextId () {
    static int currentId = 0;
    return currentId++;
}