#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "Date.h"

#include <string>

// One loan: which member borrowed which item, and when.
// Concepts: copy and move constructors, static members.
class Transaction {
private:
    int id;
    int itemId;
    int memberId;
    std::string itemTitle;    // kept so the history reads well
    std::string memberName;
    Date issueDate;
    Date dueDate;
    Date returnDate;
    bool returned;
    double fine;

    static int nextId;        // shared by all transactions

public:
    Transaction(int itemId, int memberId,
                const std::string& itemTitle,
                const std::string& memberName,
                const Date& issued, const Date& due);
    Transaction(const Transaction& other);        // copy
    Transaction(Transaction&& other) noexcept;    // move
    Transaction& operator=(const Transaction&) = default;
    Transaction& operator=(Transaction&&) = default;

    int getId() const { return id; }
    int getItemId() const { return itemId; }
    int getMemberId() const { return memberId; }
    std::string getItemTitle() const { return itemTitle; }
    std::string getMemberName() const { return memberName; }
    Date getIssueDate() const { return issueDate; }
    Date getDueDate() const { return dueDate; }
    bool isReturned() const { return returned; }
    double getFine() const { return fine; }

    bool isOverdue(const Date& today) const;
    std::string status(const Date& today) const;
    void markReturned(const Date& on, double fineAmount);

    std::string serialize() const;
    static Transaction parse(const std::string& line);
    static void setNextId(int id) { nextId = id; }
    static std::string money(double amount);   // "Rs 12.00"
};

#endif
