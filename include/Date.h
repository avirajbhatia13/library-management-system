#ifndef DATE_H
#define DATE_H

#include <iostream>
#include <string>

// A calendar date used for issue, due and return dates.
// Concepts: constructor overloading, operator overloading
// and friend functions.
class Date {
private:
    int day;
    int month;
    int year;

public:
    Date();                              // default: today
    Date(int d, int m, int y);           // parameterized
    explicit Date(const std::string& text);   // "DD/MM/YYYY"

    int getYear() const { return year; }
    std::string toString() const;

    Date operator+(int days) const;      // date after n days
    bool operator==(const Date& other) const;
    bool operator<(const Date& other) const;

    // Friend functions can read the private day/month/year
    friend int operator-(const Date& a, const Date& b);
    friend std::ostream& operator<<(std::ostream& out,
                                    const Date& d);
};

#endif
