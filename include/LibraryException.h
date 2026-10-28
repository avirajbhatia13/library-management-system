#ifndef LIBRARY_EXCEPTION_H
#define LIBRARY_EXCEPTION_H

#include <stdexcept>
#include <string>

// Thrown whenever a library rule is broken, for example when
// a book with no copies left is issued. The GUI catches it
// and shows the message to the user.
class LibraryException : public std::runtime_error {
public:
    explicit LibraryException(const std::string& message)
        : std::runtime_error(message) {}
};

#endif
