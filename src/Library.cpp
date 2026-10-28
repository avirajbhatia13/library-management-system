#include "Library.h"
#include "LibraryException.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

// Trims spaces and removes '|', which separates fields
// in the data files.
std::string clean(const std::string& text) {
    std::string s = text;
    std::replace(s.begin(), s.end(), '|', '/');
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    if (start == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}

std::string lower(std::string s) {
    for (char& c : s)
        c = static_cast<char>(std::tolower(
            static_cast<unsigned char>(c)));
    return s;
}

std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> fields;
    std::istringstream in(line);
    std::string field;
    while (std::getline(in, field, '|'))
        fields.push_back(field);
    return fields;
}

}  // namespace

Library::Library(const std::string& dataDir)
    : dataDir(dataDir) {}

// ---------------- Catalogue ----------------

int Library::addBook(const std::string& title,
                     const std::string& author,
                     const std::string& publisher, int year,
                     int copies, bool reference) {
    if (clean(publisher).empty())
        throw LibraryException("Publisher cannot be empty");
    if (year < 1450 || year > Date().getYear())
        throw LibraryException("Enter a valid year");
    int id = items.nextId(101);
    if (reference)
        items.add(new ReferenceBook(id, clean(title),
                                    clean(author),
                                    clean(publisher), year,
                                    copies));
    else
        items.add(new Book(id, clean(title), clean(author),
                           clean(publisher), year, copies));
    return id;
}

int Library::addMagazine(const std::string& title,
                         int issueNo, int copies) {
    int id = items.nextId(101);
    items.add(new Magazine(id, clean(title), issueNo, copies));
    return id;
}

// ---------------- Members ----------------

int Library::addStudent(const std::string& name,
                        const std::string& rollNo) {
    int id = members.nextId(1);
    members.add(new Student(id, clean(name), clean(rollNo)));
    return id;
}

int Library::addFaculty(const std::string& name,
                        const std::string& department) {
    int id = members.nextId(1);
    members.add(new Faculty(id, clean(name),
                            clean(department)));
    return id;
}

int Library::addTeachingAssistant(
    const std::string& name, const std::string& rollNo,
    const std::string& department) {
    int id = members.nextId(1);
    members.add(new TeachingAssistant(id, clean(name),
                                      clean(rollNo),
                                      clean(department)));
    return id;
}

// ---------------- Searching ----------------

LibraryItem* Library::findItem(int id) const {
    return items.findById(id);
}

// Matches the text against title, author or issue number
std::vector<LibraryItem*> Library::findItem(
    const std::string& text) const {
    std::vector<LibraryItem*> found;
    std::string key = lower(clean(text));
    for (LibraryItem* item : items.all()) {
        std::string hay =
            lower(item->getTitle() + " " + item->details());
        if (hay.find(key) != std::string::npos)
            found.push_back(item);
    }
    return found;
}

Person* Library::findMember(int id) const {
    return members.findById(id);
}

Transaction* Library::findLoan(int loanId) {
    for (Transaction& loan : loans)
        if (loan.getId() == loanId)
            return &loan;
    return nullptr;
}

// ---------------- Loans ----------------

int Library::issueItem(int itemId, int memberId,
                       const Date& on) {
    LibraryItem* item = findItem(itemId);
    if (!item)
        throw LibraryException("No item with ID " +
                               std::to_string(itemId));
    Person* member = findMember(memberId);
    if (!member)
        throw LibraryException("No member with ID " +
                               std::to_string(memberId));
    if (!item->canBorrow())
        throw LibraryException("'" + item->getTitle() +
                               "' is a reference book and "
                               "cannot leave the library");
    // maxBooks() is virtual: the limit depends on the role
    if (activeLoansOf(memberId) >= member->maxBooks())
        throw LibraryException(
            member->getName() + " (" + member->role() +
            ") already has the limit of " +
            std::to_string(member->maxBooks()) + " items");

    --(*item);   // overloaded operator, throws if none left
    loans.push_back(Transaction(itemId, memberId,
                                item->getTitle(),
                                member->getName(), on,
                                on + item->loanDays()));
    return loans.back().getId();
}

// Returns the fine charged for this return
double Library::returnItem(int loanId, const Date& on) {
    Transaction* loan = findLoan(loanId);
    if (!loan)
        throw LibraryException("No loan with ID " +
                               std::to_string(loanId));
    if (loan->isReturned())
        throw LibraryException("Loan " +
                               std::to_string(loanId) +
                               " was already returned");
    if (on < loan->getIssueDate())
        throw LibraryException(
            "Return date is before the issue date");

    LibraryItem* item = findItem(loan->getItemId());
    int lateDays = on - loan->getDueDate();
    double fine = 0;
    if (lateDays > 0)
        fine = lateDays * item->finePerDay();   // polymorphic
    ++(*item);
    loan->markReturned(on, fine);
    return fine;
}

int Library::activeLoansOf(int memberId) const {
    int count = 0;
    for (const Transaction& loan : loans)
        if (loan.getMemberId() == memberId &&
            !loan.isReturned())
            ++count;
    return count;
}

const std::vector<LibraryItem*>& Library::allItems() const {
    return items.all();
}

const std::vector<Person*>& Library::allMembers() const {
    return members.all();
}

const std::vector<Transaction>& Library::allLoans() const {
    return loans;
}

bool Library::isEmpty() const {
    return items.size() == 0 && members.size() == 0;
}

// ---------------- File storage ----------------

void Library::save() const {
    std::filesystem::create_directories(dataDir);
    std::ofstream itemFile(dataDir + "/items.txt");
    std::ofstream memberFile(dataDir + "/members.txt");
    std::ofstream loanFile(dataDir + "/loans.txt");
    if (!itemFile || !memberFile || !loanFile)
        throw LibraryException("Cannot write to " + dataDir);

    for (LibraryItem* item : items.all())
        itemFile << item->serialize() << '\n';
    for (Person* member : members.all())
        memberFile << member->serialize() << '\n';
    for (const Transaction& loan : loans)
        loanFile << loan.serialize() << '\n';
}

void Library::load() {
    items.clear();
    members.clear();
    loans.clear();
    std::string line;

    std::ifstream itemFile(dataDir + "/items.txt");
    while (std::getline(itemFile, line)) {
        std::vector<std::string> f = split(line);
        if (f.empty())
            continue;
        try {
            if (f[0] == "MAGAZINE" && f.size() == 6)
                items.add(new Magazine(
                    std::stoi(f[1]), f[2], std::stoi(f[3]),
                    std::stoi(f[4]), std::stoi(f[5])));
            else if (f[0] == "BOOK" && f.size() == 8)
                items.add(new Book(
                    std::stoi(f[1]), f[2], f[3], f[4],
                    std::stoi(f[5]), std::stoi(f[6]),
                    std::stoi(f[7])));
            else if (f[0] == "REFERENCE" && f.size() == 8)
                items.add(new ReferenceBook(
                    std::stoi(f[1]), f[2], f[3], f[4],
                    std::stoi(f[5]), std::stoi(f[6]),
                    std::stoi(f[7])));
        } catch (const std::exception&) {
            throw LibraryException("Bad item record: " + line);
        }
    }

    std::ifstream memberFile(dataDir + "/members.txt");
    while (std::getline(memberFile, line)) {
        std::vector<std::string> f = split(line);
        if (f.empty())
            continue;
        try {
            int id = std::stoi(f.at(1));
            if (f[0] == "STUDENT")
                members.add(new Student(id, f.at(2), f.at(3)));
            else if (f[0] == "FACULTY")
                members.add(new Faculty(id, f.at(2), f.at(3)));
            else if (f[0] == "TA")
                members.add(new TeachingAssistant(
                    id, f.at(2), f.at(3), f.at(4)));
        } catch (const std::exception&) {
            throw LibraryException("Bad member record: " +
                                   line);
        }
    }

    int nextLoan = 1;
    std::ifstream loanFile(dataDir + "/loans.txt");
    while (std::getline(loanFile, line)) {
        if (line.empty())
            continue;
        loans.push_back(Transaction::parse(line));
        nextLoan = std::max(nextLoan, loans.back().getId() + 1);
    }
    Transaction::setNextId(nextLoan);
}

// A small starting catalogue so the app is not empty on
// the first run. Loan dates are relative to today.
void Library::loadSampleData() {
    addBook("Object Oriented Programming with C++",
            "E. Balagurusamy", "McGraw Hill", 2020, 4, false);
    addBook("The C++ Programming Language",
            "Bjarne Stroustrup", "Addison-Wesley", 2013, 2,
            false);
    addBook("Let Us C++", "Yashavant Kanetkar",
            "BPB Publications", 2019, 3, false);
    addBook("Digital Design", "M. Morris Mano", "Pearson",
            2018, 3, false);
    addBook("Signals and Systems", "Alan V. Oppenheim",
            "Pearson", 2015, 2, false);
    addBook("Higher Engineering Mathematics", "B. S. Grewal",
            "Khanna Publishers", 2017, 1, true);
    addMagazine("IEEE Spectrum", 10, 3);
    addMagazine("Electronics For You", 245, 2);

    addStudent("Aviraj Singh Bhatia", "24/EC/058");
    addStudent("Riya Sharma", "24/EC/101");
    addFaculty("Dr. Anil Kumar", "ECE");
    addFaculty("Dr. Neha Gupta", "CSE");
    addTeachingAssistant("Rahul Verma", "23/MT/007", "ECE");

    Date today;
    issueItem(102, 1, today + (-20));   // now overdue
    issueItem(104, 2, today + (-3));
    int old = issueItem(105, 3, today + (-30));
    returnItem(old, today + (-10));     // returned 6 days late
    issueItem(107, 5, today + (-5));
}
