// Console tests for the model and service layers (no GUI).
// Build and run with:  make test

#include "Library.h"
#include "LibraryException.h"
#include "Report.h"

#include <cassert>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

static int passed = 0;

#define CHECK(condition)                                      \
    do {                                                      \
        assert(condition);                                    \
        ++passed;                                             \
    } while (0)

// Runs `code` and checks that it throws LibraryException
template <typename Code>
void expectError(Code code) {
    bool thrown = false;
    try {
        code();
    } catch (const LibraryException&) {
        thrown = true;
    }
    CHECK(thrown);
}

void testDate() {
    Date a(28, 2, 2024);
    CHECK((a + 1).toString() == "29/02/2024");    // leap year
    CHECK((a + 2).toString() == "01/03/2024");
    CHECK(Date(1, 3, 2024) - a == 2);
    CHECK(Date("15/08/2025") == Date(15, 8, 2025));
    CHECK(a < Date(1, 3, 2024));

    // operator<< must not leave the '0' fill on the stream
    std::ostringstream out;
    out << Date(1, 2, 2025) << std::setw(3) << 7;
    CHECK(out.str() == "01/02/2025  7");
    expectError([] { Date bad(31, 2, 2025); });
    expectError([] { Date bad("2025-01-01"); });
}

void testItemsAndMembers() {
    Book book(1, "Title", "Author", "Pub", 2020, 2);
    ReferenceBook ref(2, "Ref", "Author", "Pub", 2020, 1);
    Magazine mag(3, "Mag", 5, 1);

    // Base-class pointers call the derived versions
    LibraryItem* items[] = {&book, &ref, &mag};
    CHECK(items[0]->loanDays() == 14);
    CHECK(items[1]->canBorrow() == false);
    CHECK(items[2]->finePerDay() == 1.0);
    CHECK(LibraryItem::getItemCount() == 3);

    --book;
    CHECK(book.getAvailableCopies() == 1);
    ++book;
    CHECK(book.getAvailableCopies() == 2);
    expectError([&] { --mag; --mag; });

    // Diamond: one shared Person inside the TA
    TeachingAssistant ta(9, "Rahul", "23/MT/007", "ECE");
    Student& asStudent = ta;
    Faculty& asFaculty = ta;
    CHECK(&static_cast<Person&>(asStudent) ==
          &static_cast<Person&>(asFaculty));
    CHECK(ta.maxBooks() == 4);
    CHECK(ta.getName() == "Rahul");
}

void testLoans(const std::string& dir) {
    Library lib(dir);
    int book = lib.addBook("C++ Primer", "Lippman", "AW",
                           2012, 1, false);
    int ref = lib.addBook("Handbook", "Grewal", "Khanna",
                          2017, 1, true);
    int student = lib.addStudent("Riya", "24/EC/101");

    Date issued(1, 1, 2025);
    int loan = lib.issueItem(book, student, issued);
    CHECK(lib.findItem(book)->getAvailableCopies() == 0);
    CHECK(lib.findItem("primer").size() == 1);   // overload

    // No copies left, reference books never leave
    expectError([&] { lib.issueItem(book, student, issued); });
    expectError([&] { lib.issueItem(ref, student, issued); });
    expectError([&] { lib.issueItem(999, student, issued); });

    // Due on 15/01, returned on 20/01: 5 days x Rs 2
    double fine = lib.returnItem(loan, Date(20, 1, 2025));
    CHECK(fine == 10.0);
    CHECK(lib.findItem(book)->getAvailableCopies() == 1);
    expectError([&] { lib.returnItem(loan, Date()); });

    // A student may hold at most 3 items
    for (int i = 0; i < 3; i++) {
        int id = lib.addMagazine("Mag", i + 1, 1);
        lib.issueItem(id, student, issued);
    }
    int extra = lib.addMagazine("Extra", 9, 1);
    expectError([&] { lib.issueItem(extra, student, issued); });

    // Save, reload into a new object, compare
    lib.save();
    Library copy(dir);
    copy.load();
    CHECK(copy.allItems().size() == lib.allItems().size());
    CHECK(copy.allMembers().size() == 1);
    CHECK(copy.allLoans().size() == 4);
    CHECK(copy.allLoans()[0].getFine() == 10.0);
    CHECK(copy.activeLoansOf(student) == 3);
}

void testSampleDataAndReport(const std::string& dir) {
    Library lib(dir);
    lib.loadSampleData();
    Report report(lib);
    std::string text = report.generate(Date());
    // labels are padded to 22 characters
    CHECK(text.find("Overdue" + std::string(15, ' ') + ": 1") !=
          std::string::npos);
    CHECK(text.find("Fines collected" + std::string(7, ' ') +
                    ": Rs 12.00") != std::string::npos);
}

int main() {
    std::string dir = "build/test-data";
    std::filesystem::remove_all(dir);

    testDate();
    testItemsAndMembers();
    testLoans(dir);
    testSampleDataAndReport(dir + "-sample");

    std::filesystem::remove_all(dir);
    std::cout << "All " << passed << " checks passed.\n";
    return 0;
}
