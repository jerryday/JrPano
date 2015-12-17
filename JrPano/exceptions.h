#ifndef __JRPANO_EXCEPTIONS_H__
#define __JRPANO_EXCEPTIONS_H__

#include <string>
#include <stdexcept>

class JrException : public std::exception {
private:
    std::string what_;
public:
    JrException() {}
    JrException(const std::string& what);
    const char* what();
};

class JrInvalidArgument : public JrException {
public:
    JrInvalidArgument() {}
    JrInvalidArgument(const std::string& what) : JrException(what) {}
};

class JrTooFewImages : public JrException {
public:
    JrTooFewImages() {}
    JrTooFewImages(const std::string& what) : JrException(what) {}
};

class JrUnmatchedPairs : public JrException {
public:
    JrUnmatchedPairs() {}
    JrUnmatchedPairs(const std::string& what) : JrException(what) {}
};

#endif  // #ifndef __JRPANO_EXCEPTIONS_H__