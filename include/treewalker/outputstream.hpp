#pragma once

#include <string>

class OutputStream {
public:
    virtual ~OutputStream() = default;
    virtual void print(const std::string& str) = 0;
    virtual void println(const std::string& str) = 0;
    virtual OutputStream& operator<< (const std::string& str) = 0;
};

class StdOutOutputStream : public OutputStream {
public:
    void print(const std::string& str) override;
    void println(const std::string& str) override;
    StdOutOutputStream& operator<< (const std::string& str) override;
};

class StringOutputStream : public OutputStream {
public:
    void print(const std::string& str) override;
    void println(const std::string& str) override;
    StringOutputStream& operator<< (const std::string& str) override;
    std::string getString() const;
private:
    std::string string;
};