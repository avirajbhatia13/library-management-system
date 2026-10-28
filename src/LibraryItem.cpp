#include "LibraryItem.h"
#include "LibraryException.h"

// Static data member is defined once, outside the class
int LibraryItem::itemCount = 0;

LibraryItem::LibraryItem(int id, const std::string& title,
                         int copies, int available)
    : id(id), title(title), totalCopies(copies),
      availableCopies(available < 0 ? copies : available) {
    if (title.empty())
        throw LibraryException("Title cannot be empty");
    if (copies < 1)
        throw LibraryException("Copies must be at least 1");
    ++itemCount;
}

LibraryItem::~LibraryItem() {
    --itemCount;
}

// Prefix -- : issue one copy
LibraryItem& LibraryItem::operator--() {
    if (availableCopies == 0)
        throw LibraryException("No copies of '" + title +
                               "' are available right now");
    --availableCopies;
    return *this;
}

// Prefix ++ : one copy comes back
LibraryItem& LibraryItem::operator++() {
    if (availableCopies < totalCopies)
        ++availableCopies;
    return *this;
}

PublisherInfo::PublisherInfo(const std::string& publisher,
                             int year)
    : publisher(publisher), year(year) {}

std::string PublisherInfo::publisherText() const {
    return publisher + ", " + std::to_string(year);
}

Book::Book(int id, const std::string& title,
           const std::string& author,
           const std::string& publisher, int year, int copies,
           int available)
    : LibraryItem(id, title, copies, available),
      PublisherInfo(publisher, year), author(author) {
    if (author.empty())
        throw LibraryException("Author cannot be empty");
}

std::string Book::bookFields() const {
    return std::to_string(getId()) + "|" + getTitle() + "|" +
           author + "|" + publisher + "|" +
           std::to_string(year) + "|" +
           std::to_string(getTotalCopies()) + "|" +
           std::to_string(getAvailableCopies());
}

std::string Book::details() const {
    return "by " + author + " (" + publisherText() + ")";
}

std::string Book::serialize() const {
    return "BOOK|" + bookFields();
}

ReferenceBook::ReferenceBook(int id, const std::string& title,
                             const std::string& author,
                             const std::string& publisher,
                             int year, int copies, int available)
    : Book(id, title, author, publisher, year, copies,
           available) {}

std::string ReferenceBook::category() const {
    return "Reference";
}

std::string ReferenceBook::serialize() const {
    return "REFERENCE|" + bookFields();
}

Magazine::Magazine(int id, const std::string& title,
                   int issueNo, int copies, int available)
    : LibraryItem(id, title, copies, available),
      issueNo(issueNo) {
    if (issueNo < 1)
        throw LibraryException("Issue number must be positive");
}

std::string Magazine::category() const {
    return "Magazine";
}

std::string Magazine::details() const {
    return "Issue #" + std::to_string(issueNo);
}

std::string Magazine::serialize() const {
    return "MAGAZINE|" + std::to_string(getId()) + "|" +
           getTitle() + "|" + std::to_string(issueNo) + "|" +
           std::to_string(getTotalCopies()) + "|" +
           std::to_string(getAvailableCopies());
}
