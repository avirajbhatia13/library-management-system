#include "Report.h"

#include <iomanip>
#include <map>
#include <sstream>

namespace {

// Prints "  label ........ : value" with aligned colons
void line(std::ostringstream& out, const std::string& label,
          const std::string& value) {
    out << "  " << std::left << std::setw(22) << label << ": "
        << value << "\n";
}

}  // namespace

Report::Report(const Library& library) : library(library) {}

std::string Report::generate(const Date& today) const {
    // library.items, .members and .loans are private members
    // of Library; Report can use them because it is a friend.
    std::map<std::string, int> byCategory;
    int copies = 0, available = 0;
    for (LibraryItem* item : library.items.all()) {
        byCategory[item->category()]++;
        copies += item->getTotalCopies();
        available += item->getAvailableCopies();
    }

    std::map<std::string, int> byRole;
    for (Person* member : library.members.all())
        byRole[member->role()]++;

    int active = 0, overdue = 0;
    double collected = 0, pending = 0;
    std::map<std::string, int> timesBorrowed;
    for (const Transaction& loan : library.loans) {
        timesBorrowed[loan.getItemTitle()]++;
        collected += loan.getFine();
        if (!loan.isReturned())
            active++;
        if (loan.isOverdue(today)) {
            overdue++;
            LibraryItem* item =
                library.findItem(loan.getItemId());
            pending += (today - loan.getDueDate()) *
                       item->finePerDay();
        }
    }

    std::ostringstream out;
    out << "LIBRARY REPORT  (" << today << ")\n";
    out << "==================================\n\n";

    out << "CATALOGUE\n";
    line(out, "Titles", std::to_string(library.items.size()));
    for (const auto& entry : byCategory)
        line(out, "  " + entry.first,
             std::to_string(entry.second));
    line(out, "Copies (total)", std::to_string(copies));
    line(out, "Copies on shelf", std::to_string(available));
    line(out, "Copies issued",
         std::to_string(copies - available));

    out << "\nMEMBERS\n";
    line(out, "Total", std::to_string(library.members.size()));
    for (const auto& entry : byRole)
        line(out, "  " + entry.first,
             std::to_string(entry.second));

    out << "\nLOANS\n";
    line(out, "All time", std::to_string(library.loans.size()));
    line(out, "Active", std::to_string(active));
    line(out, "Overdue", std::to_string(overdue));
    line(out, "Fines collected", Transaction::money(collected));
    line(out, "Fines pending", Transaction::money(pending));

    out << "\nOVERDUE ITEMS\n";
    if (overdue == 0)
        out << "  None\n";
    for (const Transaction& loan : library.loans)
        if (loan.isOverdue(today))
            out << "  Loan " << loan.getId() << ": "
                << loan.getItemTitle() << " - "
                << loan.getMemberName() << " (due "
                << loan.getDueDate() << ")\n";

    out << "\nMOST BORROWED\n";
    std::string top;
    int best = 0;
    for (const auto& entry : timesBorrowed)
        if (entry.second > best) {
            best = entry.second;
            top = entry.first;
        }
    if (best == 0)
        out << "  No loans yet\n";
    else
        out << "  " << top << " (" << best << " time(s))\n";

    return out.str();
}
