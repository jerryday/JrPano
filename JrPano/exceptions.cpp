#include "stdafx.h"
#include "exceptions.h"

JrException::JrException(const std::string& what) {
    what_ = what;
}

const char* JrException::what() {
    return what_.c_str();
}