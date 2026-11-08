#pragma once

class Exception {
protected:
    const char* message;

public:
    explicit Exception(const char* msg) : message(msg) {}
    virtual ~Exception() = default;

    virtual const char* GetMessage() const {
        return message;
    }
};

class IndexOutOfRange : public Exception {
public:
    explicit IndexOutOfRange(const char* msg) : Exception(msg) {}
};

class InvalidArgument : public Exception {
public:
    explicit InvalidArgument(const char* msg) : Exception(msg) {}
};

class NullPointerException : public Exception {
public:
    explicit NullPointerException(const char* msg) : Exception(msg) {}
};