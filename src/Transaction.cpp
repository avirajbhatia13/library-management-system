#include "Transaction.h"
#include "LibraryException.h"

#include <iomanip>
#include <sstream>

int Transaction::nextId = 1;

Transaction::Transaction(int itemId, int memberId,
                         const std::string& itemTitle,
                         const std::string& memberName,
                         const Date& issued, const Date& due)
    : id(nextId++), itemId(itemId), memberId(memberId),
      itemTitle(itemTitle), memberName(memberName),
      issueDate(issued), dueDate(due), returnDate(issued),
      returned(false), fine(0) {}

// Copy constructor: duplicates every field
Transaction::Transaction(const Transaction& other)
    : id(other.id), itemId(other.itemId),
      memberId(other.memberId), itemTitle(other.itemTitle),
      memberName(other.memberName),
      issueDate(other.issueDate), dueDate(other.dueDate),
      returnDate(other.returnDate), returned(other.returned),
      fine(other.fine) {}

// Move constructor: takes over the strings instead of
// copying them (used when the vector of loans grows)
Transaction::Transaction(Transaction&& other) noexcept
    : id(other.id), itemId(other.itemId),
      memberId(other.memberId),
      itemTitle(std::move(other.itemTitle)),
      memberName(std::move(other.memberName)),
      issueDate(other.issueDate), dueDate(other.dueDate),
      returnDate(other.returnDate), returned(other.returned),
      fine(other.fine) {}

bool Transaction::isOverdue(const Date& today) const {
    return !returned && dueDate < today;
}

std::string Transaction::status(const Date& today) const {
    if (returned) {
        if (fine > 0)
            return "Returned, fine " + money(fine);
        return "Returned on time";
    }
    if (isOverdue(today))
        return "Overdue by " +
               std::to_string(today - dueDate) + " day(s)";
    return "On loan";
}

void Transaction::markReturned(const Date& on,
                               double fineAmount) {
    returned = true;
    returnDate = on;
    fine = fineAmount;
}

std::string Transaction::serialize() const {
    std::ostringstream out;
    out << id << '|' << itemId << '|' << memberId << '|'
        << itemTitle << '|' << memberName << '|' << issueDate
        << '|' << dueDate << '|' << returned << '|'
        << returnDate << '|' << fine;
    return out.str();
}

Transaction Transaction::parse(const std::string& line) {
    std::istringstream in(line);
    std::string f[10];
    for (std::string& field : f)
        std::getline(in, field, '|');
    try {
        Transaction t(std::stoi(f[1]), std::stoi(f[2]), f[3],
                      f[4], Date(f[5]), Date(f[6]));
        t.id = std::stoi(f[0]);
        t.returned = f[7] == "1";
        t.returnDate = Date(f[8]);
        t.fine = std::stod(f[9]);
        return t;
    } catch (const std::exception&) {
        throw LibraryException("Bad loan record: " + line);
    }
}

std::string Transaction::money(double amount) {
    std::ostringstream out;
    out << "Rs " << std::fixed << std::setprecision(2)
        << amount;
    return out.str();
}
