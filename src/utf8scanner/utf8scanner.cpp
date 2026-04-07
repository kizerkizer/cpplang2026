#include "utf8scanner/utf8scanner.hpp"
#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnosticmessage.hpp"
#include "diagnostics/diagnostics.hpp"

Utf8Scanner::Utf8Scanner(Source* source, Diagnostics& diagnostics) : m_source(source), m_diagnostics(diagnostics) {
    this->m_input = source->getSourceString();
}

void Utf8Scanner::addError(std::string message, SourceCodeLocationSpan sourceCodeLocationSpan) {
    this->m_diagnostics.addDiagnosticMessage(DiagnosticMessage(50, DiagnosticMessageKind::Error, DiagnosticMessageStage::Scanner, sourceCodeLocationSpan, this->m_source, message));
}

bool Utf8Scanner::isDone() const {
    return this->m_byteIndex >= this->m_input.size();
}

size_t Utf8Scanner::getByteIndex() const {
    return this->m_byteIndex;
}

size_t Utf8Scanner::getCodepointIndex() const {
    return this->m_codepointIndex;
}

SourceCodeLocation Utf8Scanner::getCurrentSourceCodeLocation() const {
    return SourceCodeLocation(this->m_byteIndex, this->m_codepointIndex, this->m_line, this->m_column);
}

std::string_view Utf8Scanner::substr(size_t byteIndex, size_t byteLength) {
    // TODO probably should add some error handling here for out of bounds byteIndex and byteLength
    if (byteIndex + byteLength > this->m_input.size()) {
        byteLength = this->m_input.size() - byteIndex;
    }
    return std::string_view(this->m_input.data() + byteIndex, byteLength);
}

size_t Utf8Scanner::getLine() const {
    return this->m_line;
}

size_t Utf8Scanner::getColumn() const {
    return this->m_column;
}

void Utf8Scanner::reset() {
    this->m_byteIndex = 0;
    this->m_codepointIndex = 0;
    this->m_line = 1;
    this->m_column = 1;
}

void Utf8Scanner::advance(size_t count) {
    char nlChar0 = 0;
    char nlChar1 = 0;
    for (size_t i = 0; i < count; i++) {
        if (this->isDone()) {
            return;
        }
        unsigned char currentByte = this->m_input[this->m_byteIndex];
        if (!nlChar0) {
            nlChar0 = currentByte;
        } else if (!nlChar1) {
            nlChar1 = currentByte;
        } else {
            nlChar0 = nlChar1;
            nlChar1 = currentByte;
        }
        if (nlChar0 && nlChar1 && nlChar0 == '\r' && nlChar1 == '\n') {
            // Windows newline
            this->m_line++;
            this->m_column = 1;
            this->m_byteIndex += 2;
            this->m_codepointIndex += 2;
            i++; // We consumed an extra byte for the newline, so we need to increment i an extra time to account for that
            continue;
        } else if (nlChar0 && nlChar0 == '\n') {
            // Unix newline
            this->m_line++;
            this->m_column = 1;
            this->m_byteIndex += 1;
            this->m_codepointIndex += 1;
                // No need to increment i an extra time here since we only consumed one byte for the newline
            continue;
        }
        size_t currentCodepointLength;
        if ((currentByte & 0b10000000) == 0) {
            currentCodepointLength = 1;
        } else if ((currentByte & 0b11100000) == 0b11000000) {
            currentCodepointLength = 2;
        } else if ((currentByte & 0b11110000) == 0b11100000) {
            currentCodepointLength = 3;
        } else if ((currentByte & 0b11111000) == 0b11110000) {
            currentCodepointLength = 4;
        } else {
            // Invalid UTF-8 byte sequence, skip it
            this->addError("Invalid UTF-8 byte sequence", SourceCodeLocationSpan(SourceCodeLocation(this->m_byteIndex, this->m_codepointIndex, this->m_line, this->m_column), SourceCodeLocation(this->m_byteIndex, this->m_codepointIndex, this->m_line, this->m_column + 1)));
            this->m_byteIndex++;
            this->m_column++;
            continue;
        }
        this->m_byteIndex += currentCodepointLength;
        this->m_codepointIndex++;
        this->m_column++;
    }
}

char32_t Utf8Scanner::peekCodepoint(size_t offset) {
    if (this->isDone()) {
        return 0;
    }
    size_t tempByteIndex = this->m_byteIndex;
    size_t tempCodepointIndex = this->m_codepointIndex;
    size_t tempLine = this->m_line;
    size_t tempColumn = this->m_column;
    char nlChar0 = 0;
    char nlChar1 = 0;
    while (tempCodepointIndex < this->m_codepointIndex + offset) {
        if (tempByteIndex >= this->m_input.size()) {
            return 0;
        }
        if (nlChar0 && nlChar1 && nlChar0 == '\r' && nlChar1 == '\n') {
            // Windows newline
            tempLine++;
            tempColumn = 1;
        } else if (nlChar0 && nlChar0 == '\n') {
            // Unix newline
            tempLine++;
            tempColumn = 1;
        }
        unsigned char currentByte = this->m_input[tempByteIndex];
        if (!nlChar0) {
            nlChar0 = currentByte;
        } else if (!nlChar1) {
            nlChar1 = currentByte;
        } else {
            nlChar0 = nlChar1;
            nlChar1 = currentByte;
        }
        size_t codepointLength;
        if ((currentByte & 0b10000000) == 0) {
            codepointLength = 1;
        } else if ((currentByte & 0b11100000) == 0b11000000) {
            codepointLength = 2;
        } else if ((currentByte & 0b11110000) == 0b11100000) {
            codepointLength = 3;
        } else if ((currentByte & 0b11111000) == 0b11110000) {
            codepointLength = 4;
        } else {
            // Invalid UTF-8 byte sequence, skip it
            this->addError("Invalid UTF-8 byte sequence", SourceCodeLocationSpan(SourceCodeLocation(tempByteIndex, tempCodepointIndex, tempLine, tempColumn), SourceCodeLocation(tempByteIndex, tempCodepointIndex, tempLine, tempColumn + 1)));
            tempByteIndex++;
            tempColumn++;
            continue;
        }
        tempByteIndex += codepointLength;
        tempCodepointIndex++;
        tempColumn++;
    }
    if (tempByteIndex >= this->m_input.size()) {
        return 0;
    }
    // Decode the UTF-8 code point at tempByteIndex
    unsigned char firstByte = this->m_input[tempByteIndex];
    size_t codepointLength;
    char32_t codepointValue;
    if ((firstByte & 0b10000000) == 0) {
        codepointLength = 1;
        codepointValue = firstByte & 0b01111111;
    } else if ((firstByte & 0b11100000) == 0b11000000) {
        codepointLength = 2;
        codepointValue = firstByte & 0b00011111;
    } else if ((firstByte & 0b11110000) == 0b11100000) {
        codepointLength = 3;
        codepointValue = firstByte & 0b00001111;
    } else if ((firstByte & 0b11111000) == 0b11110000) {
        codepointLength = 4;
        codepointValue = firstByte & 0b00000111;
    } else {
        // Invalid UTF-8 byte sequence, return 0
        return 0;
    }
    for (size_t i = 1; i < codepointLength; i++) {
        if (tempByteIndex + i >= this->m_input.size()) {
            // Invalid UTF-8 byte sequence, return 0
            this->addError("Invalid UTF-8 byte sequence", SourceCodeLocationSpan(SourceCodeLocation(tempByteIndex, tempLine, tempColumn), SourceCodeLocation(tempByteIndex, tempLine, tempColumn + 1)));
            return 0;
        }
        unsigned char nextByte = this->m_input[tempByteIndex + i];
        if ((nextByte & 0b11000000) != 0b10000000) {
            // Invalid UTF-8 byte sequence, return 0
            this->addError("Invalid UTF-8 byte sequence", SourceCodeLocationSpan(SourceCodeLocation(tempByteIndex, tempLine, tempColumn), SourceCodeLocation(tempByteIndex, tempLine, tempColumn + 1)));
            return 0;
        }
        codepointValue = (codepointValue << 6) | (nextByte & 0b00111111);
        tempColumn++;
    }
    return codepointValue;
}

/*char32_t Utf8Scanner::nextCodepoint() {
    char32_t codepoint = this->peekCodepoint();
    if (codepoint == 0) {
        return 0;
    }
    size_t codepointLength;
    if ((codepoint & 0b10000000) == 0) {
        codepointLength = 1;
    } else if ((codepoint & 0b11100000) == 0b11000000) {
        codepointLength = 2;
    } else if ((codepoint & 0b11110000) == 0b11100000) {
        codepointLength = 3;
    } else if ((codepoint & 0b11111000) == 0b11110000) {
        codepointLength = 4;
    } else {
        // Invalid UTF-8 byte sequence, skip it
        this->addError("Invalid UTF-8 byte sequence", SourceCodeLocationSpan(SourceCodeLocation(this->codepointIndex, this->line, this->column), SourceCodeLocation(this->codepointIndex, this->line, this->column + 1)));
        this->byteIndex++;
        this->codepointIndex++;
        this->column++;
        return 0;
    }
    this->byteIndex += codepointLength;
    this->codepointIndex++;
    if (codepoint == '\n') {
        this->line++;
        this->column = 1;
    } else if (codepoint == '\r' && this->peekCodepoint() == '\n') {
        this->line++;
        this->column = 1;
    } else {
        this->column++;
    }
    return codepoint;
}*/