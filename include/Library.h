#ifndef LIBRARY_H
#define LIBRARY_H

#include "LibraryItem.h"
#include "Member.h"
#include "Repository.h"
#include "Transaction.h"

#include <string>
#include <vector>

// Service layer: the only class the GUI talks to. It owns
// all data, enforces the library rules and saves to files.
class Library {
private:
    Repository<LibraryItem> items;
    Repository<Person> members;
    std::vector<Transaction> loans;
    std::string dataDir;

    Transaction* findLoan(int loanId);

public:
    explicit Library(const std::string& dataDir);

    // Catalogue (each returns the new id)
    int addBook(const std::string& title,
                const std::string& author,
                const std::string& publisher, int year,
                int copies, bool reference);
    int addMagazine(const std::string& title, int issueNo,
                    int copies);

    // Members (each returns the new id)
    int addStudent(const std::string& name,
                   const std::string& rollNo);
    int addFaculty(const std::string& name,
                   const std::string& department);
    int addTeachingAssistant(const std::string& name,
                             const std::string& rollNo,
                             const std::string& department);

    // Function overloading: search by id or by text
    LibraryItem* findItem(int id) const;
    std::vector<LibraryItem*> findItem(
        const std::string& text) const;
    Person* findMember(int id) const;

    // Loans
    int issueItem(int itemId, int memberId, const Date& on);
    double returnItem(int loanId, const Date& on);
    int activeLoansOf(int memberId) const;

    const std::vector<LibraryItem*>& allItems() const;
    const std::vector<Person*>& allMembers() const;
    const std::vector<Transaction>& allLoans() const;
    bool isEmpty() const;

    // File storage
    void save() const;
    void load();
    void loadSampleData();

    // Report may read the private data directly
    friend class Report;
};

#endif
