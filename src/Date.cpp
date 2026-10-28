#include "Date.h"
#include "LibraryException.h"

#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace {

// Builds a std::tm for noon on the given day. Noon keeps
// daylight-saving changes from shifting the date.
std::tm makeTm(int d, int m, int y) {
    std::tm t{};
    t.tm_mday = d;
    t.tm_mon = m - 1;
    t.tm_year = y - 1900;
    t.tm_hour = 12;
    t.tm_isdst = -1;
    return t;
}

// mktime() turns 31/02 into 03/03, so a changed field
// means the date does not exist.
bool isRealDate(int d, int m, int y) {
    std::tm t = makeTm(d, m, y);
    std::mktime(&t);
    return t.tm_mday == d && t.tm_mon == m - 1 &&
           t.tm_year == y - 1900;
}

}  // namespace

Date::Date() {
    std::time_t now = std::time(nullptr);
    std::tm* t = std::localtime(&now);
    day = t->tm_mday;
    month = t->tm_mon + 1;
    year = t->tm_year + 1900;
}

Date::Date(int d, int m, int y) : day(d), month(m), year(y) {
    if (!isRealDate(d, m, y))
        throw LibraryException("Invalid date");
}

// Delegates to Date() first, then overwrites the fields
Date::Date(const std::string& text) : Date() {
    int d = 0, m = 0, y = 0;
    char slash1 = 0, slash2 = 0;
    std::istringstream in(text);
    in >> d >> slash1 >> m >> slash2 >> y;
    if (!in || slash1 != '/' || slash2 != '/' ||
        !isRealDate(d, m, y))
        throw LibraryException("Invalid date '" + text +
                               "', use DD/MM/YYYY");
    day = d;
    month = m;
    year = y;
}

std::string Date::toString() const {
    std::ostringstream out;
    out << *this;
    return out.str();
}

Date Date::operator+(int days) const {
    std::tm t = makeTm(day, month, year);
    t.tm_mday += days;
    std::mktime(&t);
    return Date(t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
}

bool Date::operator==(const Date& other) const {
    return day == other.day && month == other.month &&
           year == other.year;
}

bool Date::operator<(const Date& other) const {
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    return day < other.day;
}

// Number of days from b to a (negative if a is earlier)
int operator-(const Date& a, const Date& b) {
    std::tm ta = makeTm(a.day, a.month, a.year);
    std::tm tb = makeTm(b.day, b.month, b.year);
    double seconds = std::difftime(std::mktime(&ta),
                                   std::mktime(&tb));
    return static_cast<int>(std::lround(seconds / 86400.0));
}

std::ostream& operator<<(std::ostream& out, const Date& d) {
    char oldFill = out.fill('0');   // restored below
    out << std::setw(2) << d.day << '/' << std::setw(2)
        << d.month << '/' << d.year;
    out.fill(oldFill);
    return out;
}
