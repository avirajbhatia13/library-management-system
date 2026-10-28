#ifndef REPORT_H
#define REPORT_H

#include "Date.h"
#include "Library.h"

#include <string>

// Builds a text summary of the library.
// Concepts: friend class (reads Library's private data).
class Report {
private:
    const Library& library;

public:
    explicit Report(const Library& library);
    std::string generate(const Date& today) const;
};

#endif
