#include <print>

#include "treewalker/outputstream.hpp"

StdOutOutputStream& StdOutOutputStream::operator<< (const std::string& str) {
    std::print("{}", str);
    return *this;
}

void StdOutOutputStream::print(const std::string& str) {
    std::print("{}", str);
}

void StdOutOutputStream::println(const std::string& str) {
    std::print("{}\n", str);
}

StringOutputStream& StringOutputStream::operator<< (const std::string& str) {
    string += str;
    return *this;
}

void StringOutputStream::print(const std::string& str) {
    string += str;
}

void StringOutputStream::println(const std::string& str) {
    string += str + "\n";
}

std::string StringOutputStream::getString() const {
    return string;
}